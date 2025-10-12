# PC 클라이언트

ESP32로 웹캠 영상을 UDP 스트리밍하는 Python 클라이언트입니다.

## 요구사항

- Python 3.7 이상
- 웹캠 (내장 또는 외장)
- ESP32와 동일한 네트워크에 연결

## 설치

### 가상환경 생성 및 패키지 설치

```bash
# 가상환경 생성
python3 -m venv venv

# 가상환경 활성화
source venv/bin/activate  # macOS/Linux
# 또는
venv\Scripts\activate  # Windows

# 패키지 설치
pip install -r requirements.txt
```

## 사용법

### 기본 스트리밍

```bash
ESP32_IP=<ESP32_IP_ADDRESS> make stream
```

또는

```bash
python pc_sender.py <ESP32_IP_ADDRESS>
```

예시:

```bash
ESP32_IP=192.168.0.100 make stream
```

### 고급 옵션

```bash
# 포트 지정
python pc_sender.py 192.168.0.100 --port 5005

# JPEG 품질 조정 (0-100, 기본값: 20)
python pc_sender.py 192.168.0.100 --quality 30
```

### 연결 테스트

ESP32가 정상적으로 네트워크에 연결되었는지 확인:

```bash
python test_connection.py <ESP32_IP_ADDRESS>
```

## 스크립트 설명

### pc_sender.py

웹캠 영상을 캡처하여 JPEG로 압축하고 UDP로 ESP32에 전송합니다.

주요 기능:

- 웹캠 캡처 (320x480 해상도)
- JPEG 압축 (품질 조정 가능)
- UDP 패킷 분할 전송
- 실시간 FPS 모니터링

### test_connection.py

ESP32와의 네트워크 연결을 테스트합니다.

## 문제 해결

### 웹캠을 찾을 수 없음

```python
# pc_sender.py의 웹캠 인덱스 변경
self.cap = cv2.VideoCapture(0)  # 0을 1, 2 등으로 변경
```

### 패킷 손실이 많음

1. JPEG 품질을 낮추기 (`--quality 10`)
2. WiFi 신호 강도 확인
3. 다른 네트워크 트래픽 최소화

### 종료 방법

스트리밍 중 `q` 키를 눌러 종료합니다.
