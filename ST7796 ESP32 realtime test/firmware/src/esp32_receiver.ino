/**
 * ESP32 수신 측 - UDP로 받은 JPEG 스트림을 ST7796 TFT에 표시
 *
 * 필요한 라이브러리:
 * - TFT_eSPI
 * - TJpg_Decoder
 * - WiFi (ESP32 내장)
 */

#include <WiFi.h>
#include <WiFiUdp.h>
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>

// ====== WiFi 설정 ======
const char* ssid = "soundartssu";       // WiFi SSID 입력
const char* password = "jungeolsound"; // WiFi 비밀번호 입력

// ====== 고정 IP 설정 (사용하려면 USE_STATIC_IP를 true로) ======
#define USE_STATIC_IP true  // true로 변경하면 고정 IP 사용

IPAddress local_IP(192, 168, 0, 100);    // ESP32 고정 IP
IPAddress gateway(192, 168, 0, 1);       // 게이트웨이 주소
IPAddress subnet(255, 255, 255, 0);      // 서브넷 마스크
IPAddress primaryDNS(8, 8, 8, 8);        // 기본 DNS (선택)
IPAddress secondaryDNS(8, 8, 4, 4);      // 보조 DNS (선택)

// ====== UDP 설정 ======
WiFiUDP udp;
const int localPort = 5005;  // 수신 포트

// ====== 디스플레이 설정 ======
TFT_eSPI tft = TFT_eSPI();

// ====== 프레임 버퍼 설정 ======
const int MAX_JPEG_SIZE = 65536;  // 최대 JPEG 크기 (64KB)
uint8_t jpeg_buffer[MAX_JPEG_SIZE];
int jpeg_size = 0;

// ====== 패킷 수신 버퍼 ======
const int MAX_PACKET_SIZE = 1300;  // PC sender와 동일하게 설정
const int PAYLOAD_SIZE = MAX_PACKET_SIZE - 12;  // 헤더 제외한 실제 데이터 크기 (1288 bytes)
uint8_t packet_buffer[MAX_PACKET_SIZE];

// ====== 프레임 조립 변수 ======
uint32_t current_frame_id = 0;
uint32_t expected_frame_id = 0;
uint16_t received_packets = 0;
uint16_t total_packets = 0;
bool* packet_received = nullptr;  // 패킷 수신 여부 추적

// ====== 패킷 헤더 구조체 ======
struct PacketHeader {
  uint32_t frame_id;
  uint16_t packet_index;
  uint16_t total_packets;
  uint16_t data_size;
} __attribute__((packed));

// ====== TJpg_Decoder 콜백 함수 ======
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  if (y >= tft.height()) return 0;
  tft.pushImage(x, y, w, h, bitmap);
  return 1;
}

// ====== WiFi 연결 함수 ======
void connectToWiFi() {
  Serial.println("===============================");
  Serial.println("  ESP32 WiFi 연결 시작");
  Serial.println("===============================");

#if USE_STATIC_IP
  // 고정 IP 설정
  Serial.println("[모드] 고정 IP 사용");
  Serial.print("설정할 IP: ");
  Serial.println(local_IP);

  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("고정 IP 설정 실패!");
  }
#else
  // DHCP 사용
  Serial.println("[모드] DHCP (자동 IP 할당)");
#endif

  Serial.print("WiFi 연결 중: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);  // Station 모드로 설정
  WiFi.begin(ssid, password);

  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

    attempt++;
    if (attempt > 40) {  // 20초 후 재시도
      Serial.println("\n연결 실패! 재시도...");
      WiFi.disconnect();
      delay(1000);
      WiFi.begin(ssid, password);
      attempt = 0;
    }
  }

  Serial.println("");
  Serial.println("===============================");
  Serial.println("  WiFi 연결 성공!");
  Serial.println("===============================");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP 주소: ");
  Serial.println(WiFi.localIP());
  Serial.print("게이트웨이: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("서브넷 마스크: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("DNS: ");
  Serial.println(WiFi.dnsIP());
  Serial.print("신호 강도 (RSSI): ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
  Serial.println("===============================");

  // WiFi 파워 세이브 모드 비활성화 (성능 향상)
  WiFi.setSleep(false);
  Serial.println("WiFi 파워 세이브 모드 비활성화됨");
}

// ====== 디버깅 설정 ======
#define DEBUG_VERBOSE false  // 상세 로그 출력 (성능 저하)

// ====== 패킷 처리 함수 ======
void processPacket(uint8_t* packet, int packet_size) {
  if (packet_size < sizeof(PacketHeader)) {
    return;  // 헤더보다 작은 패킷은 무시
  }

  // 헤더 파싱
  PacketHeader* header = (PacketHeader*)packet;

  // 새 프레임 시작 확인
  if (header->frame_id != current_frame_id) {
    // 이전 프레임이 완성되었으면 표시
    if (received_packets == total_packets && jpeg_size > 0) {
      displayFrame();
#if DEBUG_VERBOSE
      Serial.printf("[완료] 프레임 표시 (received=%u/%u, size=%d)\n",
                    received_packets, total_packets, jpeg_size);
#endif
    } else if (received_packets > 0 && DEBUG_VERBOSE) {
      Serial.printf("[불완전] 프레임 손실 (received=%u/%u)\n",
                    received_packets, total_packets);
      logMissingPackets();  // 손실된 패킷 번호 출력
    }

    // 새 프레임 초기화
    current_frame_id = header->frame_id;
    total_packets = header->total_packets;
    received_packets = 0;
    jpeg_size = 0;

    // 패킷 수신 추적 배열 재할당
    if (packet_received != nullptr) {
      delete[] packet_received;
    }
    packet_received = new bool[total_packets];
    memset(packet_received, 0, total_packets * sizeof(bool));
  }

  // 같은 프레임의 패킷인지 확인
  if (header->frame_id == current_frame_id &&
      header->packet_index < total_packets &&
      !packet_received[header->packet_index]) {

    // 데이터 복사
    uint8_t* data = packet + sizeof(PacketHeader);
    int data_offset = header->packet_index * PAYLOAD_SIZE;  // 고정 payload 크기 사용

    if (data_offset + header->data_size <= MAX_JPEG_SIZE) {
      memcpy(jpeg_buffer + data_offset, data, header->data_size);
      jpeg_size = max(jpeg_size, data_offset + header->data_size);

      packet_received[header->packet_index] = true;
      received_packets++;

#if DEBUG_VERBOSE
      // 디버그: 패킷 상세 정보 (첫 3개 패킷만)
      if (header->packet_index < 3) {
        Serial.printf("[패킷 #%u] data_size=%u, offset=%d, end=%d\n",
                      header->packet_index, header->data_size,
                      data_offset, data_offset + header->data_size);

        // JPEG 시그니처 확인 (첫 패킷)
        if (header->packet_index == 0 && header->data_size >= 2) {
          if (data[0] == 0xFF && data[1] == 0xD8) {
            Serial.println("[OK] JPEG 시그니처 확인");
          } else {
            Serial.printf("[ERROR] 잘못된 JPEG 시그니처: 0x%02X 0x%02X\n",
                          data[0], data[1]);
          }
        }
      }
#endif
    }
  }

  // 모든 패킷을 받았으면 프레임 표시
  if (received_packets == total_packets && jpeg_size > 0) {
    displayFrame();

    // 다음 프레임 준비
    current_frame_id = 0;
    received_packets = 0;
    total_packets = 0;
    jpeg_size = 0;
  }
}

// ====== 손실된 패킷 로그 출력 함수 ======
void logMissingPackets() {
  if (total_packets == 0 || packet_received == nullptr) return;

  Serial.print("[손실 패킷] ");
  int missing_count = 0;
  for (uint16_t i = 0; i < total_packets; i++) {
    if (!packet_received[i]) {
      if (missing_count > 0) Serial.print(", ");
      Serial.printf("#%u", i);
      missing_count++;

      // 너무 많으면 생략
      if (missing_count >= 20) {
        Serial.print("...");
        break;
      }
    }
  }
  Serial.printf(" (총 %d개 손실)\n", total_packets - received_packets);
}

// ====== 프레임 표시 함수 ======
void displayFrame() {
  // JPEG 디코딩 및 표시 (Serial.print 제거로 성능 향상)
  TJpgDec.drawJpg(0, 0, jpeg_buffer, jpeg_size);
}

// ====== 셋업 함수 ======
void setup() {
  // CPU 클럭 240MHz로 증가 (성능 향상)
  setCpuFrequencyMhz(240);

  Serial.begin(115200);
  Serial.println("ESP32 비디오 수신기 시작...");
  Serial.printf("CPU 클럭: %d MHz\n", getCpuFrequencyMhz());

  // TFT 초기화
  tft.init();
  tft.setRotation(1);  // 1 = 가로 방향 (480x320)
  tft.fillScreen(TFT_BLACK);

  // TFT에 시작 메시지 표시
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Connecting WiFi...", 240, 160, 2);

  // WiFi 연결
  connectToWiFi();

  // WiFi 연결 완료 메시지
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("WiFi Connected!", 240, 120, 2);

  tft.setTextDatum(ML_DATUM);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  String ipStr = "IP: " + WiFi.localIP().toString();
  tft.drawString(ipStr, 10, 160, 2);

  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  String portStr = "Port: " + String(localPort);
  tft.drawString(portStr, 10, 190, 2);

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  String rssiStr = "RSSI: " + String(WiFi.RSSI()) + " dBm";
  tft.drawString(rssiStr, 10, 220, 2);

  delay(3000);

  // UDP 시작
  if (udp.begin(localPort)) {
    Serial.print("UDP 리스닝 포트: ");
    Serial.println(localPort);
  } else {
    Serial.println("UDP 시작 실패!");
  }

  // TJpg_Decoder 초기화
  TJpgDec.setJpgScale(1);  // 스케일 1:1
  TJpgDec.setSwapBytes(true);  // RGB565 바이트 순서 (TFT_eSPI 표준)
  TJpgDec.setCallback(tft_output);  // 콜백 함수 설정

  // 준비 완료 메시지
  tft.fillScreen(TFT_BLACK);
  tft.drawString("Ready to receive", 240, 160, 2);
  delay(1000);
  tft.fillScreen(TFT_BLACK);

  Serial.println("준비 완료! 스트림 대기 중...");
}

// ====== 메인 루프 ======
void loop() {
  // UDP 패킷 수신 확인
  int packet_size = udp.parsePacket();

  if (packet_size > 0) {
    // 패킷 읽기
    int len = udp.read(packet_buffer, MAX_PACKET_SIZE);

    if (len > 0) {
      // 패킷 처리
      processPacket(packet_buffer, len);
    }
  }

  // 타임아웃 체크 (3초간 패킷이 없으면 리셋)
  static unsigned long last_packet_time = millis();
  if (packet_size > 0) {
    last_packet_time = millis();
  } else if (millis() - last_packet_time > 3000 && jpeg_size > 0) {
    // 버퍼 클리어
    jpeg_size = 0;
    received_packets = 0;
    total_packets = 0;
    current_frame_id = 0;
    Serial.println("타임아웃 - 버퍼 리셋");
  }
}