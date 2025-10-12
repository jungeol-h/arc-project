# ESP32 실시간 비디오 스트리밍 사용 가이드

## 1. 하드웨어 준비

### 필요한 부품
- ESP32 SuperMini 보드
- ST7796 320x480 SPI TFT 디스플레이
- 점퍼 와이어
- PC/맥북 (웹캠 포함)

### 핀 연결
README.md의 핀맵을 참고하여 다음과 같이 연결합니다:

| TFT 모듈 | ESP32 핀 | 설명 |
|----------|----------|------|
| VCC      | 5V       | 전원 |
| GND      | GND      | 그라운드 |
| SCK      | GPIO18   | SPI 클럭 |
| MOSI     | GPIO23   | SPI 데이터 출력 |
| MISO     | GPIO19   | SPI 데이터 입력 |
| CS       | GPIO5    | 칩 선택 |
| DC       | GPIO17   | 데이터/명령 선택 |
| RST      | GPIO16   | 리셋 |
| LED      | 3.3V     | 백라이트 |

## 2. 소프트웨어 설치

### ESP32 설정

1. **PlatformIO 설치**
   - [Visual Studio Code](https://code.visualstudio.com/) 다운로드 및 설치
   - VS Code에서 PlatformIO IDE 확장 설치
     - Extensions 탭 열기 (Ctrl+Shift+X / Cmd+Shift+X)
     - "PlatformIO IDE" 검색 후 설치

   또는 PlatformIO Core CLI 설치:
   ```bash
   # macOS/Linux
   curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py -o get-platformio.py
   python3 get-platformio.py

   # 또는 pip로 설치
   pip install platformio
   ```

2. **프로젝트 열기**
   - VS Code에서 `firmware` 폴더 열기
   - PlatformIO가 자동으로 `platformio.ini` 파일을 인식하고 필요한 도구와 라이브러리를 설치합니다

3. **라이브러리 및 설정 (자동 설치)**
   - `platformio.ini` 파일에 다음이 이미 설정되어 있습니다:
     - ESP32 플랫폼 및 보드 설정
     - 필요한 라이브러리 (TFT_eSPI, TJpg_Decoder)
     - TFT 디스플레이 핀맵 및 설정
     - 빌드 플래그 (User_Setup.h 파일 불필요)

4. **ESP32 코드 수정**
   - `firmware/src/main.cpp` 파일 열기
   - WiFi 정보 수정:
     ```cpp
     const char* ssid = "YOUR_WIFI_SSID";       // 실제 WiFi SSID로 변경
     const char* password = "YOUR_WIFI_PASSWORD"; // 실제 WiFi 비밀번호로 변경
     ```

5. **ESP32에 빌드 및 업로드**

   **방법 1: VS Code 사용**
   - VS Code 하단 상태바의 PlatformIO 버튼 사용:
     - ✓ (체크) 버튼: 빌드
     - → (화살표) 버튼: 업로드
     - 🔌 (플러그) 버튼: 시리얼 모니터

   **방법 2: CLI 사용**
   ```bash
   cd firmware

   # 빌드
   pio run

   # 업로드 (ESP32 연결 후)
   pio run --target upload

   # 시리얼 모니터 열기
   pio device monitor
   ```

### PC(맥북) 설정

1. **Python 가상환경 설정 및 패키지 설치**

   **방법 1: Makefile 사용 (추천)**
   ```bash
   # 가상환경 생성 및 패키지 설치
   make install
   ```

   **방법 2: 수동 설정**
   ```bash
   # 가상환경 생성
   python3 -m venv venv

   # 가상환경 활성화
   source venv/bin/activate

   # 패키지 설치
   pip install -r requirements.txt
   ```

   **방법 3: setup 스크립트 사용**
   ```bash
   ./setup_venv.sh
   ```

2. **ESP32 IP 주소 확인**
   - PlatformIO 시리얼 모니터 열기
     - VS Code: 하단 상태바의 🔌 (플러그) 버튼 클릭
     - CLI: `pio device monitor` 명령 실행
   - ESP32 리셋 또는 전원 재연결
   - WiFi 연결 후 표시되는 IP 주소 확인

## 3. 실행 방법

### ESP32 실행
1. ESP32에 전원 연결
2. 시리얼 모니터에서 WiFi 연결 확인
3. TFT 화면에 "Ready to receive" 메시지 확인

### PC 스트리밍 시작

**Makefile 사용 (가상환경 자동 활성화)**
```bash
# 스트리밍 시작
ESP32_IP=192.168.1.100 make stream

# 연결 테스트
ESP32_IP=192.168.1.100 make test
```

**수동 실행**
```bash
# 가상환경 활성화 (이미 활성화되어 있지 않다면)
source venv/bin/activate

# 기본 실행 (JPEG 품질 70)
python pc_sender.py <ESP32_IP_ADDRESS>

# 예시
python pc_sender.py 192.168.1.100

# JPEG 품질 조정 (60-80 권장)
python pc_sender.py 192.168.1.100 --quality 60

# 포트 변경 (기본: 5005)
python pc_sender.py 192.168.1.100 --port 5005

# 사용 후 가상환경 비활성화
deactivate
```

## 4. 테스트 및 문제 해결

### 연결 테스트 스크립트
```bash
# UDP 연결 테스트 (test_connection.py 실행)
python test_connection.py <ESP32_IP_ADDRESS>
```

### 일반적인 문제 해결

1. **ESP32가 WiFi에 연결되지 않음**
   - SSID와 비밀번호 확인
   - 2.4GHz WiFi 사용 확인 (5GHz 지원 안 함)

2. **영상이 표시되지 않음**
   - ESP32 IP 주소가 정확한지 확인
   - 방화벽에서 UDP 포트 5005 허용
   - 같은 네트워크에 연결되어 있는지 확인

3. **프레임 드롭이 심함**
   - JPEG 품질을 60으로 낮추기
   - WiFi 신호 강도 확인
   - 네트워크 혼잡도 확인

4. **색상이 이상함**
   - User_Setup.h의 `TFT_RGB_ORDER` 설정 확인
   - `TFT_INVERSION_ON/OFF` 토글 시도

5. **TFT 화면이 켜지지 않음**
   - 핀 연결 재확인
   - 5V 전원 공급 확인
   - RST 핀을 GND에 잠시 연결 후 해제

## 5. 성능 최적화 팁

### PC 측 최적화
- JPEG 품질: 60-75 사이 조정
- 프레임 레이트: 네트워크 상황에 따라 10-20 FPS 조정
- 해상도: 320x480 고정 (TFT 크기에 맞춤)

### ESP32 측 최적화
- SPI 주파수: 40MHz (안정성과 속도 균형)
- DMA 버퍼: 자동 설정 사용
- WiFi 전송 속도: 802.11n 사용

### 네트워크 최적화
- 라우터와 가까운 거리 유지
- 다른 무선 장치 간섭 최소화
- QoS 설정으로 UDP 트래픽 우선순위 높이기

## 6. 추가 기능 개발

### 프레임 레이트 동적 조정
`pc_sender.py`에서 네트워크 상태에 따라 FPS 자동 조정 가능

### 압축률 동적 조정
네트워크 대역폭에 따라 JPEG 품질 자동 조정 구현 가능

### 양방향 통신
ESP32에서 PC로 상태 정보 전송 구현 가능

## 7. 주의사항

- ESP32는 3.3V 로직 레벨 사용 (5V 신호 직접 연결 금지)
- TFT 디스플레이는 전류를 많이 소비하므로 충분한 전원 공급 필요
- 장시간 사용 시 ESP32와 TFT 발열 주의
- UDP는 패킷 손실 가능성이 있으므로 중요한 데이터 전송에는 부적합