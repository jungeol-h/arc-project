#!/usr/bin/env python3
"""
PC 송신 측 스크립트 - 웹캠 영상을 JPEG로 압축하여 ESP32로 UDP 전송
"""

import cv2
import socket
import struct
import time
import numpy as np
import argparse
from typing import List

class VideoStreamer:
    def __init__(self, esp32_ip: str, port: int = 5005, jpeg_quality: int = 10):
        """
        비디오 스트리밍 클래스 초기화

        Args:
            esp32_ip: ESP32의 IP 주소
            port: UDP 포트 번호 (기본값: 5005)
            jpeg_quality: JPEG 압축 품질 (0-100, 기본값: 20, 패킷 손실 최소화)
        """
        self.esp32_ip = esp32_ip
        self.port = port
        self.jpeg_quality = jpeg_quality
        self.max_packet_size = 1300  # MTU 제한 고려
        self.frame_id = 0

        # UDP 소켓 초기화
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

        # UDP 소켓 버퍼 크기 증가 (패킷 손실 감소)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, 65536)

        # 웹캠 초기화
        self.cap = cv2.VideoCapture(0)  # 내장 웹캠 (인덱스 0)

        # 해상도 설정 (ST7796 디스플레이 전체 화면, 가로 방향)
        self.width = 480
        self.height = 320

        print(f"스트리밍 시작: {esp32_ip}:{port}")
        print(f"해상도: {self.width}x{self.height}")
        print(f"JPEG 품질: {jpeg_quality}")
        print("종료하려면 'q' 키를 누르세요...")

    def create_packet_header(self, frame_id: int, packet_index: int,
                            total_packets: int, data_size: int) -> bytes:
        """
        패킷 헤더 생성

        헤더 구조 (12 바이트):
        - frame_id: 4 bytes (uint32)
        - packet_index: 2 bytes (uint16)
        - total_packets: 2 bytes (uint16)
        - data_size: 4 bytes (uint32)
        """
        return struct.pack('<IHHH', frame_id, packet_index, total_packets, data_size)

    def split_frame_to_packets(self, jpeg_data: bytes) -> List[bytes]:
        """
        JPEG 데이터를 패킷 단위로 분할

        Args:
            jpeg_data: 압축된 JPEG 이미지 데이터

        Returns:
            헤더가 포함된 패킷 리스트
        """
        header_size = 12
        payload_size = self.max_packet_size - header_size

        # 전체 패킷 개수 계산
        total_packets = (len(jpeg_data) + payload_size - 1) // payload_size

        packets = []
        for i in range(total_packets):
            start_idx = i * payload_size
            end_idx = min((i + 1) * payload_size, len(jpeg_data))
            chunk = jpeg_data[start_idx:end_idx]

            # 헤더 생성 및 패킷 조합
            header = self.create_packet_header(
                self.frame_id, i, total_packets, len(chunk)
            )
            packet = header + chunk
            packets.append(packet)

        return packets

    def send_frame(self, frame: np.ndarray) -> bool:
        """
        프레임을 JPEG로 압축하여 UDP로 전송

        Args:
            frame: OpenCV 프레임 (numpy array, BGR 순서)

        Returns:
            전송 성공 여부
        """
        try:
            # 밝기 반전 수정 (네거티브 필름 효과 제거)
            frame_inverted = cv2.bitwise_not(frame)

            # JPEG 압축 (cv2.imencode는 BGR 순서를 직접 처리)
            encode_param = [int(cv2.IMWRITE_JPEG_QUALITY), self.jpeg_quality]
            success, jpeg_data = cv2.imencode('.jpg', frame_inverted, encode_param)

            if not success:
                print("JPEG 인코딩 실패")
                return False

            # 바이트 배열로 변환
            jpeg_bytes = jpeg_data.tobytes()

            # 패킷 분할
            packets = self.split_frame_to_packets(jpeg_bytes)

            # 패킷 수에 따른 동적 딜레이 계산 (FPS 향상)
            num_packets = len(packets)
            if num_packets <= 2:
                packet_delay = 0.0008  # 2패킷: 0.8ms
            elif num_packets == 3:
                packet_delay = 0.0010  # 3패킷: 1.0ms
            else:
                packet_delay = 0.0012  # 4패킷 이상: 1.2ms

            # 모든 패킷 전송
            for packet in packets:
                self.sock.sendto(packet, (self.esp32_ip, self.port))
                time.sleep(packet_delay)

            # 프레임 ID 증가
            self.frame_id = (self.frame_id + 1) % 0xFFFFFFFF

            return True

        except Exception as e:
            print(f"프레임 전송 오류: {e}")
            return False

    def stream(self):
        """
        메인 스트리밍 루프
        """
        fps = 20  # 목표 FPS (안정성 우선)
        frame_interval = 1.0 / fps
        last_frame_time = 0

        # 통계 변수
        frame_count = 0
        start_time = time.time()

        try:
            while True:
                # 프레임 캡처
                ret, frame = self.cap.read()
                if not ret:
                    print("웹캠 캡처 실패")
                    break

                # 프레임 리사이즈 (480x320)
                resized = cv2.resize(frame, (self.width, self.height))

                # FPS 제한
                current_time = time.time()
                if current_time - last_frame_time < frame_interval:
                    continue
                last_frame_time = current_time

                # 프레임 전송
                if self.send_frame(resized):
                    frame_count += 1

                # 로컬 미리보기 (옵션)
                cv2.imshow('Streaming Preview', resized)

                # 통계 출력 (1초마다)
                if frame_count % fps == 0:
                    elapsed = time.time() - start_time
                    actual_fps = frame_count / elapsed
                    print(f"프레임 {frame_count} | 실제 FPS: {actual_fps:.2f}")

                # 'q' 키로 종료
                if cv2.waitKey(1) & 0xFF == ord('q'):
                    break

        except KeyboardInterrupt:
            print("\n스트리밍 중단...")
        finally:
            self.cleanup()

    def cleanup(self):
        """
        리소스 정리
        """
        self.cap.release()
        cv2.destroyAllWindows()
        self.sock.close()
        print("종료됨")

def main():
    """
    메인 함수
    """
    parser = argparse.ArgumentParser(description='ESP32로 웹캠 영상 스트리밍')
    parser.add_argument('esp32_ip', help='ESP32의 IP 주소')
    parser.add_argument('--port', type=int, default=5005, help='UDP 포트 번호 (기본값: 5005)')
    parser.add_argument('--quality', type=int, default=20,
                       help='JPEG 품질 (0-100, 기본값: 20, 패킷 손실 최소화)')

    args = parser.parse_args()

    # 스트리머 생성 및 실행
    streamer = VideoStreamer(args.esp32_ip, args.port, args.quality)
    streamer.stream()

if __name__ == '__main__':
    main()