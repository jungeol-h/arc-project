# ESP32 펌웨어

ESP32용 UDP MJPEG 스트리밍 수신기 펌웨어입니다.

## 하드웨어 요구사항

- ESP32 dev
- ST7796 SPI TFT 디스플레이 (480x320)

## 핀맵

| 모듈 | ESP32 핀 |
| ---- | -------- |
| VCC  | 5V       |
| GND  | GND      |
| SCK  | GPIO18   |
| MOSI | GPIO23   |
| MISO | GPIO19   |
| CS   | GPIO5    |
| DC   | GPIO17   |
| RST  | GPIO16   |
| LED  | 3.3V     |

## 빌드 및 업로드

### PlatformIO IDE (VS Code, 추천)

1. VS Code에서 이 폴더 열기
2. PlatformIO 확장 설치
3. 하단 상태바에서:
   - Build 아이콘 클릭
   - Upload 아이콘 클릭
   - Serial Monitor 아이콘 클릭

### PlatformIO CLI 사용

```bash
# 빌드
pio run

# 업로드
pio run --target upload

# 시리얼 모니터
pio device monitor
```

## 주요 라이브러리

- `TFT_eSPI`: ST7796 TFT 디스플레이 드라이버
- `TJpg_Decoder`: JPEG 디코더
- `WiFi.h`: WiFi 연결
- `WiFiUdp.h`: UDP 통신

디스플레이 설정은 `src/User_Setup.h`에서 수정할 수 있습니다.
