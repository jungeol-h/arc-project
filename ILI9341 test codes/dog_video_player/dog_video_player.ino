/*******************************************************************************
 * Dog Video Player for ILI9341
 * SD카드에서 dog.mjpeg 파일을 재생하는 예제
 *
 * 필요 라이브러리:
 * - Arduino_GFX: https://github.com/moononournation/Arduino_GFX
 * - JPEGDEC: https://github.com/bitbank2/JPEGDEC
 ******************************************************************************/

#define MJPEG_FILENAME "/dog.mjpeg"
#define MJPEG_BUFFER_SIZE (240 * 240 * 2 / 10) // 240x240 해상도용 버퍼

// ESP32 핀 설정
#define TFT_CS    5
#define TFT_DC    27
#define TFT_RST   33
#define TFT_BL    22
#define TFT_SCK   18
#define TFT_MOSI  23
#define TFT_MISO  19

#define SD_CS     15  // SD카드 CS 핀

#include <Arduino_GFX_Library.h>
#include <SD.h>
#include "MjpegClass.h"

// 데이터 버스 및 디스플레이 설정
Arduino_DataBus *bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, TFT_MISO);
Arduino_GFX *gfx = new Arduino_ILI9341(bus, TFT_RST, 3 /* rotation */, false /* IPS */);

static MjpegClass mjpeg;

// 성능 측정 변수
static int total_frames = 0;
static unsigned long total_read_video = 0;
static unsigned long total_decode_video = 0;
static unsigned long total_show_video = 0;
static unsigned long start_ms, curr_ms;

// JPEG 그리기 콜백 함수
static int jpegDrawCallback(JPEGDRAW *pDraw)
{
  unsigned long start = millis();
  gfx->draw16bitBeRGBBitmap(pDraw->x, pDraw->y, pDraw->pPixels, pDraw->iWidth, pDraw->iHeight);
  total_show_video += millis() - start;
  return 1;
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Dog Video Player - ILI9341");

  // 디스플레이 초기화
  if (!gfx->begin())
  {
    Serial.println("디스플레이 초기화 실패!");
  }
  gfx->fillScreen(BLACK);

  // 백라이트 켜기
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  // SD카드 초기화
  if (!SD.begin(SD_CS))
  {
    Serial.println("SD카드 초기화 실패!");
    gfx->println("SD카드 초기화 실패!");
    return;
  }
  Serial.println("SD카드 초기화 완료");

  // dog.mjpeg 파일 열기
  File mjpegFile = SD.open(MJPEG_FILENAME, FILE_READ);

  if (!mjpegFile || mjpegFile.isDirectory())
  {
    Serial.println("dog.mjpeg 파일을 찾을 수 없습니다!");
    gfx->println("dog.mjpeg 파일을 찾을 수 없습니다!");
    return;
  }

  // MJPEG 버퍼 할당
  uint8_t *mjpeg_buf = (uint8_t *)malloc(MJPEG_BUFFER_SIZE);
  if (!mjpeg_buf)
  {
    Serial.println("메모리 할당 실패!");
    return;
  }

  Serial.println("영상 재생 시작");

  start_ms = millis();
  curr_ms = millis();

  // MJPEG 설정
  mjpeg.setup(
      &mjpegFile, mjpeg_buf, jpegDrawCallback, true /* useBigEndian */,
      0 /* x */, 0 /* y */, gfx->width() /* widthLimit */, gfx->height() /* heightLimit */);

  // 영상 재생 루프
  while (mjpegFile.available() && mjpeg.readMjpegBuf())
  {
    // 비디오 읽기
    total_read_video += millis() - curr_ms;
    curr_ms = millis();

    // 비디오 재생
    mjpeg.drawJpg();
    total_decode_video += millis() - curr_ms;

    curr_ms = millis();
    total_frames++;
  }

  // 재생 완료 및 통계 출력
  int time_used = millis() - start_ms;
  Serial.println("영상 재생 완료");
  mjpegFile.close();

  float fps = 1000.0 * total_frames / time_used;
  total_decode_video -= total_show_video;

  Serial.printf("총 프레임: %d\n", total_frames);
  Serial.printf("재생 시간: %d ms\n", time_used);
  Serial.printf("평균 FPS: %0.1f\n", fps);
  Serial.printf("읽기 시간: %lu ms (%0.1f %%)\n", total_read_video, 100.0 * total_read_video / time_used);
  Serial.printf("디코딩 시간: %lu ms (%0.1f %%)\n", total_decode_video, 100.0 * total_decode_video / time_used);
  Serial.printf("화면 출력 시간: %lu ms (%0.1f %%)\n", total_show_video, 100.0 * total_show_video / time_used);

  // 화면에 통계 출력
  gfx->setCursor(0, 0);
  gfx->setTextColor(WHITE);
  gfx->printf("총 프레임: %d\n", total_frames);
  gfx->printf("재생 시간: %d ms\n", time_used);
  gfx->printf("평균 FPS: %0.1f\n", fps);
  gfx->printf("읽기: %lu ms (%0.1f%%)\n", total_read_video, 100.0 * total_read_video / time_used);
  gfx->printf("디코딩: %lu ms (%0.1f%%)\n", total_decode_video, 100.0 * total_decode_video / time_used);
  gfx->printf("출력: %lu ms (%0.1f%%)\n", total_show_video, 100.0 * total_show_video / time_used);

  free(mjpeg_buf);
}

void loop()
{
  // 메인 루프는 비어있음 (setup에서 모든 작업 완료)
}