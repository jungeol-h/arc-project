#!/usr/bin/env python3
"""
ESP32 UDP 연결 테스트 스크립트
간단한 테스트 패킷을 전송하여 연결 상태를 확인합니다.
"""

import socket
import struct
import time
import argparse
import numpy as np
import cv2

def test_basic_udp(esp32_ip: str, port: int = 5005):
    """
    기본 UDP 연결 테스트
    """
    print(f"UDP 연결 테스트 시작: {esp32_ip}:{port}")

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    # 테스트 패킷 전송 (작은 더미 데이터)
    test_data = b"ESP32_CONNECTION_TEST"

    try:
        for i in range(5):
            sock.sendto(test_data, (esp32_ip, port))
            print(f"테스트 패킷 {i+1}/5 전송됨")
            time.sleep(0.5)

        print("✓ UDP 패킷 전송 성공")
        return True

    except Exception as e:
        print(f"✗ UDP 전송 실패: {e}")
        return False
    finally:
        sock.close()

def test_single_frame(esp32_ip: str, port: int = 5005):
    """
    단일 테스트 프레임 전송
    """
    print("\n단일 프레임 전송 테스트...")

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    try:
        # 테스트 이미지 생성 (320x480, 그라데이션)
        height, width = 480, 320

        # 색상 그라데이션 이미지 생성
        image = np.zeros((height, width, 3), dtype=np.uint8)
        for y in range(height):
            for x in range(width):
                image[y, x] = [
                    int(x * 255 / width),      # Red
                    int(y * 255 / height),      # Green
                    128                         # Blue
                ]

        # JPEG 압축
        encode_param = [int(cv2.IMWRITE_JPEG_QUALITY), 70]
        success, jpeg_data = cv2.imencode('.jpg', image, encode_param)

        if not success:
            print("✗ JPEG 인코딩 실패")
            return False

        jpeg_bytes = jpeg_data.tobytes()
        print(f"JPEG 크기: {len(jpeg_bytes)} bytes")

        # 패킷 분할
        max_packet_size = 1300
        header_size = 12
        payload_size = max_packet_size - header_size

        total_packets = (len(jpeg_bytes) + payload_size - 1) // payload_size
        frame_id = 1

        print(f"총 패킷 수: {total_packets}")

        # 패킷 전송
        for i in range(total_packets):
            start_idx = i * payload_size
            end_idx = min((i + 1) * payload_size, len(jpeg_bytes))
            chunk = jpeg_bytes[start_idx:end_idx]

            # 헤더 생성
            header = struct.pack('<IHHH', frame_id, i, total_packets, len(chunk))
            packet = header + chunk

            sock.sendto(packet, (esp32_ip, port))
            time.sleep(0.001)  # 1ms 딜레이

            if i % 10 == 0:
                print(f"  패킷 {i}/{total_packets} 전송 중...")

        print("✓ 테스트 프레임 전송 완료")
        return True

    except Exception as e:
        print(f"✗ 프레임 전송 실패: {e}")
        return False
    finally:
        sock.close()

def test_continuous_stream(esp32_ip: str, port: int = 5005, duration: int = 5):
    """
    연속 스트리밍 테스트
    """
    print(f"\n{duration}초간 연속 스트리밍 테스트...")

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    try:
        start_time = time.time()
        frame_count = 0
        frame_id = 0

        while time.time() - start_time < duration:
            # 간단한 테스트 패턴 생성
            height, width = 480, 320
            image = np.zeros((height, width, 3), dtype=np.uint8)

            # 움직이는 박스 그리기
            box_size = 50
            box_x = int((frame_count * 5) % (width - box_size))
            box_y = int((frame_count * 3) % (height - box_size))
            cv2.rectangle(image, (box_x, box_y),
                         (box_x + box_size, box_y + box_size),
                         (0, 255, 0), -1)

            # 프레임 번호 표시
            cv2.putText(image, f"Frame {frame_count}", (10, 30),
                       cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)

            # JPEG 압축
            encode_param = [int(cv2.IMWRITE_JPEG_QUALITY), 60]
            success, jpeg_data = cv2.imencode('.jpg', image, encode_param)

            if not success:
                continue

            jpeg_bytes = jpeg_data.tobytes()

            # 패킷 분할 및 전송
            max_packet_size = 1300
            header_size = 12
            payload_size = max_packet_size - header_size
            total_packets = (len(jpeg_bytes) + payload_size - 1) // payload_size

            for i in range(total_packets):
                start_idx = i * payload_size
                end_idx = min((i + 1) * payload_size, len(jpeg_bytes))
                chunk = jpeg_bytes[start_idx:end_idx]

                header = struct.pack('<IHHH', frame_id, i, total_packets, len(chunk))
                packet = header + chunk

                sock.sendto(packet, (esp32_ip, port))
                time.sleep(0.0001)  # 100us 딜레이

            frame_id = (frame_id + 1) % 0xFFFFFFFF
            frame_count += 1

            # FPS 제한 (약 10 FPS)
            time.sleep(0.1)

        elapsed = time.time() - start_time
        fps = frame_count / elapsed

        print(f"✓ 연속 스트리밍 테스트 완료")
        print(f"  전송 프레임 수: {frame_count}")
        print(f"  평균 FPS: {fps:.2f}")
        return True

    except Exception as e:
        print(f"✗ 연속 스트리밍 실패: {e}")
        return False
    finally:
        sock.close()

def check_network_connectivity(esp32_ip: str):
    """
    네트워크 연결 상태 확인 (ping 테스트)
    """
    import subprocess
    import platform

    print(f"네트워크 연결 확인: {esp32_ip}")

    # OS에 따라 ping 명령 조정
    param = '-n' if platform.system().lower() == 'windows' else '-c'
    command = ['ping', param, '4', esp32_ip]

    try:
        result = subprocess.run(command, capture_output=True, text=True, timeout=10)

        if result.returncode == 0:
            print("✓ ESP32에 ping 성공")
            # ping 통계 출력
            lines = result.stdout.split('\n')
            for line in lines:
                if 'min/avg/max' in line or 'Minimum/Maximum/Average' in line:
                    print(f"  {line.strip()}")
            return True
        else:
            print("✗ ESP32에 ping 실패")
            return False

    except subprocess.TimeoutExpired:
        print("✗ Ping 타임아웃")
        return False
    except Exception as e:
        print(f"✗ Ping 테스트 실행 실패: {e}")
        return False

def main():
    parser = argparse.ArgumentParser(description='ESP32 연결 테스트')
    parser.add_argument('esp32_ip', help='ESP32의 IP 주소')
    parser.add_argument('--port', type=int, default=5005, help='UDP 포트 번호 (기본값: 5005)')
    parser.add_argument('--full', action='store_true', help='모든 테스트 실행')

    args = parser.parse_args()

    print("=" * 50)
    print("ESP32 비디오 스트리밍 연결 테스트")
    print("=" * 50)

    # 1. 네트워크 연결 확인
    print("\n[1/4] 네트워크 연결 테스트")
    print("-" * 30)
    network_ok = check_network_connectivity(args.esp32_ip)

    if not network_ok:
        print("\n⚠ 네트워크 연결 실패. ESP32 IP 주소와 네트워크 설정을 확인하세요.")
        return

    # 2. 기본 UDP 테스트
    print("\n[2/4] 기본 UDP 연결 테스트")
    print("-" * 30)
    udp_ok = test_basic_udp(args.esp32_ip, args.port)

    if not udp_ok:
        print("\n⚠ UDP 연결 실패. 포트 번호와 방화벽 설정을 확인하세요.")
        return

    # 3. 단일 프레임 테스트
    print("\n[3/4] 단일 프레임 전송 테스트")
    print("-" * 30)
    frame_ok = test_single_frame(args.esp32_ip, args.port)

    # 4. 연속 스트리밍 테스트 (--full 옵션 시)
    if args.full:
        print("\n[4/4] 연속 스트리밍 테스트")
        print("-" * 30)
        stream_ok = test_continuous_stream(args.esp32_ip, args.port, duration=5)

    # 결과 요약
    print("\n" + "=" * 50)
    print("테스트 결과 요약")
    print("=" * 50)
    print(f"✓ 네트워크 연결: {'성공' if network_ok else '실패'}")
    print(f"✓ UDP 연결: {'성공' if udp_ok else '실패'}")
    print(f"✓ 프레임 전송: {'성공' if frame_ok else '실패'}")

    if network_ok and udp_ok and frame_ok:
        print("\n✅ 모든 테스트 통과! 스트리밍을 시작할 수 있습니다.")
        print(f"\n실행 명령:")
        print(f"  python pc_sender.py {args.esp32_ip}")
    else:
        print("\n❌ 일부 테스트 실패. 위의 오류 메시지를 확인하세요.")

if __name__ == '__main__':
    main()