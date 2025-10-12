## 개요

PC에서 웹캠 영상을 JPEG로 인코딩해 UDP로 ESP32에 쏘고, ESP32는 바로 TFT에 송출

## 하드웨어 구성

ESP32 supermini
SPI TFT (ST7796, 320×480)
PC (맥북)

## 소프트웨어 스택

PC: opencv-python, socket, struct
ESP32: WiFi.h, WiFiUdp, TFT_eSPI, TJpg_Decoder
압축방식: MJPEG
전송 프로토콜: UDP

## 데이터 흐름

[PC 카메라]
→ OpenCV로 프레임 캡처
→ JPEG(품질 60~75)로 압축
→ UDP 패킷으로 조각 전송 (MTU≈1300B)
→ ESP32에서 수신 후 조립
→ TJpg_Decoder로 즉시 디코드
→ TFT_eSPI로 SPI DMA 출력

## 핀맵

(모듈 - ESP32)
VCC - 5V
GND - GND
SCK - GPIO18
MOSI - GPIO23
MISO - GPIO19
CS - GPIO5
DC - GPIO17
RST - GPIO16
LED - 3v3

## 프로젝트 구조

```
ST7796-ESP32-Streaming/
├── firmware/         # ESP32 펌웨어 (PlatformIO)
│   ├── src/          # 소스 코드
│   └── README.md     # 펌웨어 빌드 가이드
│
├── client/           # PC 클라이언트 (Python)
│   ├── pc_sender.py
│   ├── test_connection.py
│   └── README.md     # 클라이언트 사용 가이드
│
├── docs/             # 문서
│   └── USAGE.md      # 상세 사용법
│
└── scripts/          # 설정 스크립트
    └── setup_venv.sh
└── Makefile # 빌드 스크립트
```

## 빠른 시작

### 1. ESP32 펌웨어 업로드

```bash
cd firmware
pio run --target upload
```

상세 가이드: [firmware/README.md](firmware/README.md)

### 2. PC 클라이언트 설정

venv를 사용해 독립된 가상환경에서 python을 띄웁니다.

```bash
cd client
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
```

### 3. 스트리밍 실행

루트 폴더에서

```bash
ESP32_IP=192.168.0.100 make stream
```

상세 가이드: [client/README.md](client/README.md)

## 문서

- [firmware/README.md](firmware/README.md) - ESP32 펌웨어 빌드 및 설정
- [client/README.md](client/README.md) - PC 클라이언트 사용법
- [docs/USAGE.md](docs/USAGE.md) - 상세한 사용 가이드
