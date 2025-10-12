/**
 * TFT_eSPI 라이브러리 설정 파일 - ST7796 320x480 디스플레이용
 *
 * 이 파일을 Arduino 라이브러리 폴더의 TFT_eSPI 폴더에 복사하세요:
 * - Windows: C:\Users\{username}\Documents\Arduino\libraries\TFT_eSPI\
 * - macOS: ~/Documents/Arduino/libraries/TFT_eSPI/
 * - Linux: ~/Arduino/libraries/TFT_eSPI/
 *
 * 원본 User_Setup.h 파일을 백업하시기 바랍니다!
 */

// ========================================
//              사용자 설정 섹션
// ========================================

// 드라이버 선택 - ST7796 480x320 디스플레이
#define ST7796_DRIVER

// 디스플레이 크기 설정
#define TFT_WIDTH  320
#define TFT_HEIGHT 480

// ESP32 핀맵 설정 (ESP32 SuperMini용)
// README.md에 명시된 핀맵을 따름
#define TFT_MISO 19   // MISO (Master In Slave Out)
#define TFT_MOSI 23   // MOSI (Master Out Slave In)
#define TFT_SCLK 18   // SCK (Serial Clock)
#define TFT_CS   5    // CS (Chip Select)
#define TFT_DC   17   // DC/RS (Data/Command or Register Select)
#define TFT_RST  16   // RST (Reset)

// 백라이트 제어 핀 (옵션)
// #define TFT_BL   32   // LED 백라이트 제어 (필요시 주석 해제)

// ========================================
//           색상 및 폰트 설정
// ========================================

// RGB 색상 순서 설정 (일반적으로 BGR)
#define TFT_RGB_ORDER TFT_BGR

// 바이트 순서 스왑 (색상이 이상할 경우 변경)
#define TFT_INVERSION_ON
// #define TFT_INVERSION_OFF

// ========================================
//             SPI 설정
// ========================================

// SPI 주파수 설정
#define SPI_FREQUENCY       40000000  // 40MHz (안정적인 속도)
#define SPI_READ_FREQUENCY  20000000  // 읽기 속도
#define SPI_TOUCH_FREQUENCY  2500000  // 터치스크린 속도 (사용 안 함)

// DMA 사용 설정 (ESP32는 자동)
// ESP32는 자동으로 DMA를 사용하므로 별도 설정 불필요

// ========================================
//           폰트 설정 (옵션)
// ========================================

// 기본 폰트 로드 (필요한 것만 선택)
#define LOAD_GLCD   // Font 1. 기본 고정폭 폰트
#define LOAD_FONT2  // Font 2. 작은 16픽셀 ASCII 폰트
#define LOAD_FONT4  // Font 4. 중간 26픽셀 ASCII 폰트
#define LOAD_FONT6  // Font 6. 큰 48픽셀 숫자 폰트
#define LOAD_FONT7  // Font 7. 7세그먼트 48픽셀 폰트
#define LOAD_FONT8  // Font 8. 큰 75픽셀 숫자 폰트
#define LOAD_GFXFF  // FreeFonts 사용 가능

// Smooth font를 위한 설정
#define SMOOTH_FONT

// ========================================
//          최적화 설정
// ========================================

// Transaction 지원 (멀티태스킹 시 필요)
#define SUPPORT_TRANSACTIONS

// DMA 버퍼 크기 (ESP32 자동 설정)
// #define TFT_SDA_READ  // 빠른 읽기를 위한 설정

// ========================================
//             디버그 설정
// ========================================

// 디버그 출력 활성화 (문제 해결 시)
// #define TFT_DEBUG

// ========================================
//          추가 옵션 설정
// ========================================

// 터치스크린 칩셋 (사용하지 않음)
// #define TOUCH_CS PIN_D2  // 터치스크린 CS 핀

// SD 카드 CS 핀 (사용하지 않음)
// #define SD_CS    PIN_D4  // SD 카드 CS 핀

// ========================================
//     사용자 정의 명령 (ST7796 전용)
// ========================================

// ST7796 전용 초기화 명령 (필요시)
// 대부분의 경우 TFT_eSPI 라이브러리의 기본값으로 충분함

// ========================================
//          경고 및 주의사항
// ========================================

/*
 * 주의사항:
 * 1. 이 파일을 TFT_eSPI 라이브러리 폴더에 복사해야 합니다.
 * 2. 기존 User_Setup.h 파일을 백업하세요.
 * 3. User_Setup_Select.h에서 이 파일을 선택하거나,
 *    기본 User_Setup.h를 이 내용으로 교체하세요.
 * 4. 핀 연결이 정확한지 확인하세요.
 * 5. 3.3V 전원 공급이 충분한지 확인하세요 (TFT는 전류를 많이 소비합니다).
 */