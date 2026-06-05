/*
 * ╔══════════════════════════════════════════════════════════════╗
 *   CLAWD MOCHI STANDALONE — ESP32-C3 Super Mini + ST7789
 *   无需 WiFi，按键切换 + 自动轮播表情
 *
 *   Wiring:
 *     SDA → GPIO 10  (hardware SPI MOSI)
 *     SCL → GPIO 8   (hardware SPI SCK)
 *     RST → GPIO 2
 *     DC  → GPIO 1
 *     CS  → GPIO 4
 *     BL  → GPIO 3
 *     BTN → GPIO 0   (短按切表情，长按开关背光)
 *     VCC → 3V3
 *     GND → GND
 *
 *   GPIO 0 是 ESP32-C3 的 BOOT 按钮，大多数开发板自带
 * ╚══════════════════════════════════════════════════════════════╝
 */

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <math.h>

// ── Pins ──────────────────────────────────────────────────────
#define TFT_CS  4
#define TFT_DC  1
#define TFT_RST 2
#define TFT_BLK 3
#define BTN_PIN 0

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// ── Display ───────────────────────────────────────────────────
#define DISP_W 240
#define DISP_H 240

// ── Eye base constants ────────────────────────────────────────
#define EYE_W   30
#define EYE_H   60
#define EYE_GAP 120
#define EYE_OX  0
#define EYE_OY  40

#define MOUTH_Y 140  // 嘴巴垂直位置

// ── Colours ───────────────────────────────────────────────────
uint16_t C_ORANGE, C_DARKBG, C_MUTED, C_GREEN, C_BG;
#define C_WHITE ST77XX_WHITE
#define C_BLACK ST77XX_BLACK

// ── Views ─────────────────────────────────────────────────────
#define VIEW_LOGO           0
#define VIEW_EYES_NORMAL    1
#define VIEW_EYES_SQUISH    2
#define VIEW_EYES_HAPPY     3   // ^_^
#define VIEW_EYES_SURPRISED 4   // O_O
#define VIEW_EYES_HEART     5   // ♥_♥
#define VIEW_EYES_SLEEPY    6   // -_-
#define VIEW_EYES_WINK      7   // ;_)
#define VIEW_EYES_ANGRY     8   // ಠ_ಠ
#define VIEW_EYES_CRY       9   // ;_;
#define VIEW_EYES_SWEAT     10  // 尴尬 😅
#define VIEW_EYES_DROOL     11  // 流口水
#define VIEW_EYES_BLUSH     12  // 脸红害羞
#define VIEW_EYES_JEALOUS   13  // 😤
#define VIEW_EYES_ROLL      14  // 🙄
#define VIEW_EYES_EXPLODE   15  // 🤯
#define VIEW_EYES_DEVIL     16  // 😈
#define VIEW_EYES_SICK      17  // 🤮
#define VIEW_PET            18
#define VIEW_EYES_THINK     19
#define VIEW_COUNT          20

uint8_t currentView = VIEW_LOGO;
bool    backlightOn = true;

// ── Timing ────────────────────────────────────────────────────
unsigned long lastAnimMs    = 0;
unsigned long lastCycleMs   = 0;
unsigned long btnPressMs    = 0;
unsigned long startupMs     = 0;
int  autoCycleSec = 10;
bool btnWasPressed = false;
bool btnState = HIGH;

// ── Mochi Pixel Pet state ──────────────────────────────────────
#define PET_CY 202
#define PET_R  22
int16_t petX = 120;
int8_t  petDir = 1;
int8_t  petLegFrame = 0;
unsigned long petMoveMs = 0;
unsigned long petActionMs = 0;
uint8_t petAction = 0;     // 0=walk, 1=jump-up, 2=jump-down, 3=idle
int8_t  petJumpOff = 0;

// ═══════════════════════════════════════════════════════════════
//  LOGO DATA (Anthropic logo — 162 triangles)
// ═══════════════════════════════════════════════════════════════

#define LOGO_CX 120
#define LOGO_CY 105

#define LOGO_TRI_COUNT 162
static const int16_t LOGO_TRIS[][6] PROGMEM = {
  {120,105,65,134,100,114},{120,105,100,114,101,113},{120,105,101,113,100,112},
  {120,105,100,112,99,112},{120,105,99,112,93,111},{120,105,93,111,73,111},
  {120,105,73,111,55,110},{120,105,55,110,38,109},{120,105,38,109,34,108},
  {120,105,34,108,30,103},{120,105,30,103,30,100},{120,105,30,100,34,98},
  {120,105,34,98,39,98},{120,105,39,98,50,99},{120,105,50,99,67,100},
  {120,105,67,100,80,101},{120,105,80,101,98,103},{120,105,98,103,101,103},
  {120,105,101,103,101,102},{120,105,101,102,100,101},{120,105,100,101,100,100},
  {120,105,100,100,82,88},{120,105,82,88,63,76},{120,105,63,76,53,69},
  {120,105,53,69,48,65},{120,105,48,65,45,61},{120,105,45,61,44,54},
  {120,105,44,54,49,49},{120,105,49,49,55,49},{120,105,55,49,57,49},
  {120,105,57,49,64,55},{120,105,64,55,78,66},{120,105,78,66,96,79},
  {120,105,96,79,99,81},{120,105,99,81,100,81},{120,105,100,81,100,80},
  {120,105,100,80,99,78},{120,105,99,78,89,60},{120,105,89,60,78,41},
  {120,105,78,41,73,34},{120,105,73,34,72,29},{120,105,72,29,72,28},
  {120,105,72,28,72,27},{120,105,72,27,71,26},{120,105,71,26,71,25},
  {120,105,71,25,71,24},{120,105,71,24,77,16},{120,105,77,16,80,15},
  {120,105,80,15,87,16},{120,105,87,16,91,19},{120,105,91,19,95,29},
  {120,105,95,29,103,46},{120,105,103,46,114,68},{120,105,114,68,118,75},
  {120,105,118,75,119,81},{120,105,119,81,120,83},{120,105,120,83,121,83},
  {120,105,121,83,121,82},{120,105,121,82,122,69},{120,105,122,69,124,54},
  {120,105,124,54,126,34},{120,105,126,34,126,28},{120,105,126,28,129,21},
  {120,105,129,21,135,18},{120,105,135,18,139,20},{120,105,139,20,143,25},
  {120,105,143,25,142,28},{120,105,142,28,140,42},{120,105,140,42,136,64},
  {120,105,136,64,133,78},{120,105,133,78,135,78},{120,105,135,78,136,76},
  {120,105,136,76,144,67},{120,105,144,67,156,51},{120,105,156,51,162,45},
  {120,105,162,45,168,38},{120,105,168,38,172,35},{120,105,172,35,180,35},
  {120,105,180,35,185,43},{120,105,185,43,183,52},{120,105,183,52,175,62},
  {120,105,175,62,168,71},{120,105,168,71,159,83},{120,105,159,83,153,94},
  {120,105,153,94,154,94},{120,105,154,94,155,94},{120,105,155,94,176,90},
  {120,105,176,90,188,88},{120,105,188,88,201,85},{120,105,201,85,208,88},
  {120,105,208,88,208,91},{120,105,208,91,206,97},{120,105,206,97,191,101},
  {120,105,191,101,174,104},{120,105,174,104,148,110},{120,105,148,110,148,111},
  {120,105,148,111,148,111},{120,105,148,111,160,112},{120,105,160,112,165,112},
  {120,105,165,112,177,112},{120,105,177,112,200,114},{120,105,200,114,205,118},
  {120,105,205,118,209,123},{120,105,209,123,208,126},{120,105,208,126,199,131},
  {120,105,199,131,187,128},{120,105,187,128,159,121},{120,105,159,121,149,119},
  {120,105,149,119,147,119},{120,105,147,119,147,120},{120,105,147,120,156,128},
  {120,105,156,128,170,141},{120,105,170,141,189,158},{120,105,189,158,190,163},
  {120,105,190,163,188,166},{120,105,188,166,185,166},{120,105,185,166,169,153},
  {120,105,169,153,162,148},{120,105,162,148,148,136},{120,105,148,136,147,136},
  {120,105,147,136,147,137},{120,105,147,137,150,142},{120,105,150,142,168,168},
  {120,105,168,168,169,176},{120,105,169,176,168,179},{120,105,168,179,163,180},
  {120,105,163,180,158,179},{120,105,158,179,148,165},{120,105,148,165,137,149},
  {120,105,137,149,129,134},{120,105,129,134,128,135},{120,105,128,135,123,189},
  {120,105,123,189,120,192},{120,105,120,192,115,194},{120,105,115,194,110,191},
  {120,105,110,191,108,185},{120,105,108,185,110,174},{120,105,110,174,113,160},
  {120,105,113,160,116,148},{120,105,116,148,118,134},{120,105,118,134,119,129},
  {120,105,119,129,119,129},{120,105,119,129,118,129},{120,105,118,129,107,144},
  {120,105,107,144,91,166},{120,105,91,166,78,180},{120,105,78,180,75,181},
  {120,105,75,181,70,178},{120,105,70,178,70,173},{120,105,70,173,73,169},
  {120,105,73,169,91,146},{120,105,91,146,102,132},{120,105,102,132,109,124},
  {120,105,109,124,109,123},{120,105,109,123,108,123},{120,105,108,123,61,153},
  {120,105,61,153,52,155},{120,105,52,155,49,151},{120,105,49,151,49,146},
  {120,105,49,146,51,144},{120,105,51,144,65,134},{120,105,65,134,65,134},
};

#define LOGO_SEG_COUNT 162
static const int16_t LOGO_SEGS[][4] PROGMEM = {
  {65,134,100,114},{100,114,101,113},{101,113,100,112},{100,112,99,112},
  {99,112,93,111},{93,111,73,111},{73,111,55,110},{55,110,38,109},
  {38,109,34,108},{34,108,30,103},{30,103,30,100},{30,100,34,98},
  {34,98,39,98},{39,98,50,99},{50,99,67,100},{67,100,80,101},
  {80,101,98,103},{98,103,101,103},{101,103,101,102},{101,102,100,101},
  {100,101,100,100},{100,100,82,88},{82,88,63,76},{63,76,53,69},
  {53,69,48,65},{48,65,45,61},{45,61,44,54},{44,54,49,49},
  {49,49,55,49},{55,49,57,49},{57,49,64,55},{64,55,78,66},
  {78,66,96,79},{96,79,99,81},{99,81,100,81},{100,81,100,80},
  {100,80,99,78},{99,78,89,60},{89,60,78,41},{78,41,73,34},
  {73,34,72,29},{72,29,72,28},{72,28,72,27},{72,27,71,26},
  {71,26,71,25},{71,25,71,24},{71,24,77,16},{77,16,80,15},
  {80,15,87,16},{87,16,91,19},{91,19,95,29},{95,29,103,46},
  {103,46,114,68},{114,68,118,75},{118,75,119,81},{119,81,120,83},
  {120,83,121,83},{121,83,121,82},{121,82,122,69},{122,69,124,54},
  {124,54,126,34},{126,34,126,28},{126,28,129,21},{129,21,135,18},
  {135,18,139,20},{139,20,143,25},{143,25,142,28},{142,28,140,42},
  {140,42,136,64},{136,64,133,78},{133,78,135,78},{135,78,136,76},
  {136,76,144,67},{144,67,156,51},{156,51,162,45},{162,45,168,38},
  {168,38,172,35},{172,35,180,35},{180,35,185,43},{185,43,183,52},
  {183,52,175,62},{175,62,168,71},{168,71,159,83},{159,83,153,94},
  {153,94,154,94},{154,94,155,94},{155,94,176,90},{176,90,188,88},
  {188,88,201,85},{201,85,208,88},{208,88,208,91},{208,91,206,97},
  {206,97,191,101},{191,101,174,104},{174,104,148,110},{148,110,148,111},
  {148,111,148,111},{148,111,160,112},{160,112,165,112},{165,112,177,112},
  {177,112,200,114},{200,114,205,118},{205,118,209,123},{209,123,208,126},
  {208,126,199,131},{199,131,187,128},{187,128,159,121},{159,121,149,119},
  {149,119,147,119},{147,119,147,120},{147,120,156,128},{156,128,170,141},
  {170,141,189,158},{189,158,190,163},{190,163,188,166},{188,166,185,166},
  {185,166,169,153},{169,153,162,148},{162,148,148,136},{148,136,147,136},
  {147,136,147,137},{147,137,150,142},{150,142,168,168},{168,168,169,176},
  {169,176,168,179},{168,179,163,180},{163,180,158,179},{158,179,148,165},
  {148,165,137,149},{137,149,129,134},{129,134,128,135},{128,135,123,189},
  {123,189,120,192},{120,192,115,194},{115,194,110,191},{110,191,108,185},
  {108,185,110,174},{110,174,113,160},{113,160,116,148},{116,148,118,134},
  {118,134,119,129},{119,129,119,129},{119,129,118,129},{118,129,107,144},
  {107,144,91,166},{91,166,78,180},{78,180,75,181},{75,181,70,178},
  {70,178,70,173},{70,173,73,169},{73,169,91,146},{91,146,102,132},
  {102,132,109,124},{109,124,109,123},{109,123,108,123},{108,123,61,153},
  {61,153,52,155},{52,155,49,151},{49,151,49,146},{49,146,51,144},
  {51,144,65,134},{65,134,65,134},
};

// ═════════════════════════════════════════════════════════════
//  HELPERS
// ═════════════════════════════════════════════════════════════

void setBacklight(bool on) {
  backlightOn = on;
  digitalWrite(TFT_BLK, on ? HIGH : LOW);
}

void initColours() {
  C_ORANGE = tft.color565(218, 17, 0);
  C_DARKBG = tft.color565(10,  12,  16);
  C_MUTED  = tft.color565(90,  88,  86);
  C_GREEN  = tft.color565(80, 220, 130);
  C_BG     = C_ORANGE;
}

uint16_t bgColour()       { return C_BG; }
void     setBg(uint16_t c){ C_BG = c; }
static void fillBg(uint16_t c) { tft.fillScreen(c); }

uint16_t hueToRgb565(uint8_t h) {
  uint8_t r, g, b;
  uint8_t region = h / 32;
  uint8_t phase  = h % 32;
  if (region == 0) { r = 255;         g = phase * 8;     b = 0; }
  else if (region == 1) { r = 255 - phase * 8; g = 255;         b = 0; }
  else if (region == 2) { r = 0;         g = 255;         b = phase * 8; }
  else if (region == 3) { r = 0;         g = 255 - phase * 8; b = 255; }
  else if (region == 4) { r = phase * 8; g = 0;           b = 255; }
  else                  { r = 255;         g = 0;           b = 255 - phase * 8; }
  return tft.color565(r, g, b);
}

uint16_t heartColour() {
  uint16_t c = tft.color565(240, 20 + (millis() / 200 % 3) * 12, 10);
  return c;
}

// 画水滴形状（圆形 + 三角尖底）
void drawTeardrop(int16_t x, int16_t y, uint8_t r, uint16_t color) {
  tft.fillCircle(x, y, r, color);
  tft.fillTriangle(x - r, y, x + r, y, x, y + r * 2, color);
}

// ═════════════════════════════════════════════════════════════
//  LOGO
// ═════════════════════════════════════════════════════════════

void drawLogoFilled(uint16_t bg, uint16_t fg) {
  tft.fillScreen(bg);
  for (uint16_t i = 0; i < LOGO_TRI_COUNT; i++) {
    tft.fillTriangle(
      pgm_read_word(&LOGO_TRIS[i][0]), pgm_read_word(&LOGO_TRIS[i][1]),
      pgm_read_word(&LOGO_TRIS[i][2]), pgm_read_word(&LOGO_TRIS[i][3]),
      pgm_read_word(&LOGO_TRIS[i][4]), pgm_read_word(&LOGO_TRIS[i][5]),
      fg);
  }
  tft.setTextColor(fg); tft.setTextSize(2);
  tft.setCursor(LOGO_CX - 54, 210); tft.print("Anthropic");
  tft.setCursor(LOGO_CX - 53, 210); tft.print("Anthropic");
}

void animLogoReveal() {
  fillBg(bgColour());
  for (uint16_t i = 0; i < LOGO_SEG_COUNT; i++) {
    int16_t x1 = pgm_read_word(&LOGO_SEGS[i][0]);
    int16_t y1 = pgm_read_word(&LOGO_SEGS[i][1]);
    int16_t x2 = pgm_read_word(&LOGO_SEGS[i][2]);
    int16_t y2 = pgm_read_word(&LOGO_SEGS[i][3]);
    tft.drawLine(x1, y1, x2, y2, C_WHITE);
    tft.drawLine(x1 + 1, y1, x2 + 1, y2, C_WHITE);
    delay(12);
  }
  drawLogoFilled(bgColour(), C_WHITE);
  delay(2000);
}

// ═════════════════════════════════════════════════════════════
//  EYE HELPERS
// ═════════════════════════════════════════════════════════════

inline int16_t eyeLX(int16_t ox) {
  return (DISP_W - (EYE_W * 2 + EYE_GAP)) / 2 + EYE_OX + ox;
}
inline int16_t eyeRX(int16_t ox) { return eyeLX(ox) + EYE_W + EYE_GAP; }
inline int16_t eyeY()            { return (DISP_H - EYE_H) / 2 - EYE_OY; }
inline int16_t eyeCY()           { return eyeY() + EYE_H / 2; }

// ═════════════════════════════════════════════════════════════
//  EXPRESSIONS
// ═════════════════════════════════════════════════════════════

// ── Normal (▬ ▬) ─────────────────────────────────────────────
void drawNormalEyes(int16_t ox = 0, bool blink = false) {
  fillBg(bgColour());
  const int16_t lx = eyeLX(ox), rx = eyeRX(ox), ey = eyeY();
  if (!blink) {
    tft.fillRect(lx, ey, EYE_W, EYE_H, C_BLACK);
    tft.fillRect(rx, ey, EYE_W, EYE_H, C_BLACK);
  } else {
    tft.fillRect(lx, ey + EYE_H / 2 - 3, EYE_W, 6, C_BLACK);
    tft.fillRect(rx, ey + EYE_H / 2 - 3, EYE_W, 6, C_BLACK);
  }
  // 中性平嘴 ───
  tft.drawLine(98, MOUTH_Y, 142, MOUTH_Y, C_BLACK);
  tft.drawLine(98, MOUTH_Y + 1, 142, MOUTH_Y + 1, C_BLACK);
}

// ── Squish (> <) ─────────────────────────────────────────────
void drawChevron(int16_t cx, int16_t cy, int16_t arm, int16_t reach,
                 uint8_t thk, bool rightFacing, uint16_t col) {
  for (int8_t t = -(int8_t)thk; t <= (int8_t)thk; t++) {
    if (rightFacing) {
      tft.drawLine(cx - reach/2, cy - arm + t, cx + reach/2, cy + t,      col);
      tft.drawLine(cx + reach/2, cy + t,       cx - reach/2, cy + arm + t, col);
    } else {
      tft.drawLine(cx + reach/2, cy - arm + t, cx - reach/2, cy + t,      col);
      tft.drawLine(cx - reach/2, cy + t,       cx + reach/2, cy + arm + t, col);
    }
  }
}

void drawSquishEyes(bool closed = false) {
  fillBg(bgColour());
  const int16_t lx = eyeLX(0), rx = eyeRX(0), cy = eyeCY();
  const int16_t arm   = EYE_H / 2;
  const int16_t reach = EYE_W / 2;
  const int16_t lcx   = lx + EYE_W / 2;
  const int16_t rcx   = rx + EYE_W / 2;
  if (!closed) {
    drawChevron(lcx, cy, arm, reach, 10, true,  C_BLACK);
    drawChevron(rcx, cy, arm, reach, 10, false, C_BLACK);
  } else {
    tft.fillRect(lx, cy - 5, EYE_W, 10, C_BLACK);
    tft.fillRect(rx, cy - 5, EYE_W, 10, C_BLACK);
  }
  // 锯齿嘴 >_<
  for (int8_t t = 0; t < 2; t++) {
    tft.drawLine(98, MOUTH_Y + t, 110, MOUTH_Y + 5 + t, C_BLACK);
    tft.drawLine(110, MOUTH_Y + 5 + t, 120, MOUTH_Y + 2 + t, C_BLACK);
    tft.drawLine(120, MOUTH_Y + 2 + t, 130, MOUTH_Y + 5 + t, C_BLACK);
    tft.drawLine(130, MOUTH_Y + 5 + t, 142, MOUTH_Y + t, C_BLACK);
  }
}

// ── Happy (^_^) ──────────────────────────────────────────────
void drawHappyEyes(bool squish = false) {
  fillBg(bgColour());
  const int16_t lx = eyeLX(0), rx = eyeRX(0), ey = eyeY();
  const int16_t cy = ey + EYE_H / 2;
  const int16_t r  = EYE_W * 3 / 5;
  if (!squish) {
    for (int16_t x = -r; x <= r; x++) {
      int16_t y = cy - sqrt(r * r - x * x) * 0.7;
      tft.drawPixel(lx + EYE_W / 2 + x * EYE_W / (2 * r), y, C_BLACK);
      tft.drawPixel(rx + EYE_W / 2 + x * EYE_W / (2 * r), y, C_BLACK);
    }
    for (int8_t t = 1; t <= 3; t++) {
      for (int16_t x = -r; x <= r; x++) {
        int16_t y = cy - sqrt(r * r - x * x) * 0.7 + t;
        tft.drawPixel(lx + EYE_W / 2 + x * EYE_W / (2 * r), y, C_BLACK);
        tft.drawPixel(rx + EYE_W / 2 + x * EYE_W / (2 * r), y, C_BLACK);
      }
    }
    tft.fillCircle(lx - 4, cy + 6, 4, tft.color565(255, 80, 60));
    tft.fillCircle(rx + EYE_W + 4, cy + 6, 4, tft.color565(255, 80, 60));
  } else {
    tft.fillRect(lx, cy - 2, EYE_W, 4, C_BLACK);
    tft.fillRect(rx, cy - 2, EYE_W, 4, C_BLACK);
    for (int16_t x = 0; x < EYE_W; x++) {
      int16_t y = cy - 1 + sin(x * 3.14 / EYE_W) * 3;
      tft.drawPixel(lx + x, y, C_BLACK);
      tft.drawPixel(rx + x, y, C_BLACK);
    }
    tft.fillCircle(lx - 3, cy + 6, 3, tft.color565(255, 80, 60));
    tft.fillCircle(rx + EYE_W + 3, cy + 6, 3, tft.color565(255, 80, 60));
  }
  // 弧形微笑 ^_^
  for (int8_t t = 0; t < 2; t++) {
    tft.drawLine(96, 149 + t, 105, 140 + t, C_BLACK);
    tft.drawLine(105, 140 + t, 115, 136 + t, C_BLACK);
    tft.drawLine(115, 136 + t, 125, 136 + t, C_BLACK);
    tft.drawLine(125, 136 + t, 135, 140 + t, C_BLACK);
    tft.drawLine(135, 140 + t, 144, 149 + t, C_BLACK);
  }
}

// ── Surprised (O_O) ──────────────────────────────────────────
void drawSurprisedEyes() {
  fillBg(bgColour());
  const int16_t lx = eyeLX(0), rx = eyeRX(0), ey = eyeY();
  const int16_t cx = EYE_W / 2, cy = EYE_H / 2;
  const int16_t r  = EYE_W * 2 / 5;
  tft.fillCircle(lx + cx, ey + cy, r, C_BLACK);
  tft.fillCircle(rx + cx, ey + cy, r, C_BLACK);
  tft.fillCircle(lx + cx - 4, ey + cy - 5, 4, C_WHITE);
  tft.fillCircle(rx + cx - 4, ey + cy - 5, 4, C_WHITE);
  // 惊讶 O 型嘴
  tft.drawCircle(120, MOUTH_Y, 9, C_BLACK);
  tft.fillCircle(120, MOUTH_Y, 7, C_BLACK);
  // 嘴内高光
  tft.drawPixel(119, MOUTH_Y - 4, C_WHITE);
  tft.drawPixel(123, MOUTH_Y - 3, C_WHITE);
}

// ── Heart (♥_♥) ──────────────────────────────────────────────
void drawHeartEyes() {
  fillBg(bgColour());
  const int16_t lx = eyeLX(0), rx = eyeRX(0), ey = eyeY();
  const int16_t cy = ey + EYE_H / 2;
  uint16_t hc = heartColour();

  int16_t hcx = lx + EYE_W / 2;
  int16_t hcy = cy - 4;
  tft.fillCircle(hcx - 6, hcy - 2, 7, hc);
  tft.fillCircle(hcx + 6, hcy - 2, 7, hc);
  tft.fillTriangle(hcx - 12, hcy, hcx + 12, hcy, hcx, hcy + 12, hc);
  tft.fillRect(hcx - 1, hcy - 2, 3, 4, hc);

  hcx = rx + EYE_W / 2;
  tft.fillCircle(hcx - 6, hcy - 2, 7, hc);
  tft.fillCircle(hcx + 6, hcy - 2, 7, hc);
  tft.fillTriangle(hcx - 12, hcy, hcx + 12, hcy, hcx, hcy + 12, hc);
  tft.fillRect(hcx - 1, hcy - 2, 3, 4, hc);

  tft.fillCircle(lx - 3, cy + 8, 5, tft.color565(200, 40, 30));
  tft.fillCircle(rx + EYE_W + 3, cy + 8, 5, tft.color565(200, 40, 30));

  // 开心微笑 ♥_♥
  for (int8_t t = 0; t < 2; t++) {
    tft.drawLine(96, 149 + t, 105, 140 + t, C_BLACK);
    tft.drawLine(105, 140 + t, 115, 136 + t, C_BLACK);
    tft.drawLine(115, 136 + t, 125, 136 + t, C_BLACK);
    tft.drawLine(125, 136 + t, 135, 140 + t, C_BLACK);
    tft.drawLine(135, 140 + t, 144, 149 + t, C_BLACK);
  }
}

// ── Sleepy (-_-) ─────────────────────────────────────────────
void drawSleepyEyes(uint8_t closeAmount = 0) {
  fillBg(bgColour());
  const int16_t lx = eyeLX(0), rx = eyeRX(0), ey = eyeY();
  const int16_t cy = ey + EYE_H / 2;
  int16_t lineH = 3 + closeAmount * EYE_H / 2 / 255;
  if (lineH > EYE_H / 2) lineH = EYE_H / 2;

  if (closeAmount < 128) {
    tft.setTextSize(1); tft.setTextColor(C_MUTED);
    tft.setCursor(lx + EYE_W + 4, ey - 2); tft.print("z");
    tft.setTextSize(2);
    tft.setCursor(lx + EYE_W + 16, ey - 10); tft.print("Z");
  }

  if (closeAmount < 10) {
    tft.drawLine(lx + 4, cy, lx + EYE_W - 4, cy, C_BLACK);
    tft.drawLine(rx + 4, cy, rx + EYE_W - 4, cy, C_BLACK);
  } else {
    int16_t half = lineH / 2;
    tft.fillRect(lx, cy - half, EYE_W, lineH, C_BLACK);
    tft.fillRect(rx, cy - half, EYE_W, lineH, C_BLACK);
  }
  // 困倦下垂嘴 ⌢
  tft.drawLine(105, MOUTH_Y, 120, MOUTH_Y + 4, C_BLACK);
  tft.drawLine(120, MOUTH_Y + 4, 135, MOUTH_Y, C_BLACK);
  tft.drawLine(105, MOUTH_Y + 1, 120, MOUTH_Y + 5, C_BLACK);
  tft.drawLine(120, MOUTH_Y + 5, 135, MOUTH_Y + 1, C_BLACK);
}

// ── Wink (;_) ────────────────────────────────────────────────
void drawWinkEyes(bool leftClosed = false) {
  fillBg(bgColour());
  const int16_t lx = eyeLX(0), rx = eyeRX(0), ey = eyeY();
  const int16_t cy = eyeCY();

  if (!leftClosed) {
    for (int8_t t = 0; t < 4; t++) {
      tft.drawLine(lx, cy + t, lx + EYE_W, cy + t, C_BLACK);
    }
    tft.fillRect(rx, ey, EYE_W, EYE_H, C_BLACK);
  } else {
    tft.fillRect(lx, ey, EYE_W, EYE_H, C_BLACK);
    for (int8_t t = 0; t < 4; t++) {
      tft.drawLine(rx, cy + t, rx + EYE_W, cy + t, C_BLACK);
    }
  }

  // 歪嘴笑（坏笑）
  tft.drawLine(98, MOUTH_Y, 120, MOUTH_Y - 2, C_BLACK);
  tft.drawLine(120, MOUTH_Y - 2, 142, MOUTH_Y + 3, C_BLACK);
  tft.drawLine(98, MOUTH_Y + 1, 120, MOUTH_Y - 1, C_BLACK);
  tft.drawLine(120, MOUTH_Y - 1, 142, MOUTH_Y + 4, C_BLACK);

  // 俏皮小舌头
  tft.fillCircle(120, MOUTH_Y + 8, 5, tft.color565(255, 100, 100));
  tft.fillRect(116, MOUTH_Y + 4, 8, 6, tft.color565(255, 100, 100));
}

// ── Angry (ಠ_ಠ) ─────────────────────────────────────────────
void drawAngryEyes() {
  fillBg(bgColour());
  const int16_t lx = eyeLX(0), rx = eyeRX(0), ey = eyeY();
  const int16_t cy = eyeCY();
  uint16_t red = tft.color565(200, 0, 0);

  for (int8_t t = -2; t <= 2; t++) {
    tft.drawLine(lx - 2, ey - 8 + t, lx + EYE_W + 2, ey + 6 + t, red);
    tft.drawLine(rx + EYE_W + 2, ey - 6 + t, rx - 2, ey + 8 + t, red);
  }

  tft.fillRect(lx, ey + 4, EYE_W, EYE_H - 8, C_BLACK);
  tft.fillRect(rx, ey + 4, EYE_W, EYE_H - 8, C_BLACK);

  tft.fillCircle(lx + EYE_W / 2, ey + EYE_H / 2, 5, C_WHITE);
  tft.fillCircle(rx + EYE_W / 2, ey + EYE_H / 2, 5, C_WHITE);

  // 愤怒倒 V 嘴 >:(
  for (int8_t t = 0; t < 2; t++) {
    tft.drawLine(96, 136 + t, 110, 146 + t, C_BLACK);
    tft.drawLine(110, 146 + t, 120, 148 + t, C_BLACK);
    tft.drawLine(120, 148 + t, 130, 146 + t, C_BLACK);
    tft.drawLine(130, 146 + t, 144, 136 + t, C_BLACK);
  }
  // 咬牙切齿（牙齿细节）
  tft.fillRect(109, 147, 3, 3, C_WHITE);
  tft.fillRect(114, 148, 3, 3, C_WHITE);
  tft.fillRect(123, 148, 3, 3, C_WHITE);
  tft.fillRect(128, 147, 3, 3, C_WHITE);
}

// ── Cry (;_;) ────────────────────────────────────────────────
void drawCryEyes() {
  fillBg(bgColour());
  const int16_t lx = eyeLX(0), rx = eyeRX(0), ey = eyeY(), cy = eyeCY();
  uint16_t tearBlue = tft.color565(80, 160, 255);

  // 泪眼（下弯弧线 — 像倒 U 的悲伤眼）
  for (int8_t t = 0; t < 4; t++) {
    tft.drawLine(lx, ey + EYE_H/4 + t, lx + EYE_W/2, ey + EYE_H/2 + t, C_BLACK);
    tft.drawLine(lx + EYE_W/2, ey + EYE_H/2 + t, lx + EYE_W, ey + EYE_H/4 + t, C_BLACK);
    tft.drawLine(rx, ey + EYE_H/4 + t, rx + EYE_W/2, ey + EYE_H/2 + t, C_BLACK);
    tft.drawLine(rx + EYE_W/2, ey + EYE_H/2 + t, rx + EYE_W, ey + EYE_H/4 + t, C_BLACK);
  }

  // 蓝色眼泪
  drawTeardrop(lx + 8,  ey + EYE_H + 4, 4, tearBlue);
  drawTeardrop(lx + 22, ey + EYE_H + 14, 3, tearBlue);
  drawTeardrop(rx + 8,  ey + EYE_H + 4, 4, tearBlue);
  drawTeardrop(rx + 22, ey + EYE_H + 14, 3, tearBlue);

  // 泪痕线
  tft.drawLine(lx + 6, ey + EYE_H + 2, lx + 10, ey + EYE_H + 10, tearBlue);
  tft.drawLine(rx + 6, ey + EYE_H + 2, rx + 10, ey + EYE_H + 10, tearBlue);

  // 悲伤波浪嘴 ～>～<～
  for (int8_t t = 0; t < 2; t++) {
    tft.drawLine(98, MOUTH_Y + t, 108, MOUTH_Y - 4 + t, C_BLACK);
    tft.drawLine(108, MOUTH_Y - 4 + t, 118, MOUTH_Y + 2 + t, C_BLACK);
    tft.drawLine(118, MOUTH_Y + 2 + t, 128, MOUTH_Y - 3 + t, C_BLACK);
    tft.drawLine(128, MOUTH_Y - 3 + t, 138, MOUTH_Y + 1 + t, C_BLACK);
    tft.drawLine(138, MOUTH_Y + 1 + t, 142, MOUTH_Y + t, C_BLACK);
  }
}

// ── Sweat / Embarrassed (😅) ─────────────────────────────────
void drawSweatEyes() {
  fillBg(bgColour());
  const int16_t lx = eyeLX(0), rx = eyeRX(0), ey = eyeY(), cy = eyeCY();
  uint16_t sweatBlue = tft.color565(100, 180, 255);

  // 尴尬直线眼（两条横线）
  tft.drawLine(lx + 2, cy, lx + EYE_W - 2, cy, C_BLACK);
  tft.drawLine(lx + 2, cy + 1, lx + EYE_W - 2, cy + 1, C_BLACK);
  tft.drawLine(rx + 2, cy, rx + EYE_W - 2, cy, C_BLACK);
  tft.drawLine(rx + 2, cy + 1, rx + EYE_W - 2, cy + 1, C_BLACK);

  // 大颗汗滴（右上方）
  drawTeardrop(rx + EYE_W + 16, ey - 6, 6, sweatBlue);
  // 小汗滴（左上方）
  drawTeardrop(lx - 10, ey + 2, 4, sweatBlue);

  // 尴尬歪嘴（左边高右边低）
  tft.drawLine(100, MOUTH_Y + 2, 120, MOUTH_Y - 1, C_BLACK);
  tft.drawLine(120, MOUTH_Y - 1, 140, MOUTH_Y, C_BLACK);
  tft.drawLine(100, MOUTH_Y + 3, 120, MOUTH_Y, C_BLACK);
  tft.drawLine(120, MOUTH_Y, 140, MOUTH_Y + 1, C_BLACK);

  // 淡淡脸红
  tft.fillCircle(lx - 6, cy + 10, 6, tft.color565(200, 60, 50));
  tft.fillCircle(rx + EYE_W + 6, cy + 10, 6, tft.color565(200, 60, 50));
}

// ── Drool (流口水) ──────────────────────────────────────────
void drawDroolEyes() {
  fillBg(bgColour());
  const int16_t lx = eyeLX(0), rx = eyeRX(0), ey = eyeY(), cy = eyeCY();

  // 迷离半闭眼（向下弯的弧线，像 ^ 但向下）
  for (int8_t t = 0; t < 4; t++) {
    tft.drawLine(lx + 2, ey + EYE_H/2 + t, lx + EYE_W/2, ey + EYE_H*3/4 + t, C_BLACK);
    tft.drawLine(lx + EYE_W/2, ey + EYE_H*3/4 + t, lx + EYE_W - 2, ey + EYE_H/2 + t, C_BLACK);
    tft.drawLine(rx + 2, ey + EYE_H/2 + t, rx + EYE_W/2, ey + EYE_H*3/4 + t, C_BLACK);
    tft.drawLine(rx + EYE_W/2, ey + EYE_H*3/4 + t, rx + EYE_W - 2, ey + EYE_H/2 + t, C_BLACK);
  }

  // 发呆傻笑嘴
  for (int8_t t = 0; t < 2; t++) {
    tft.drawLine(96, 148 + t, 105, 141 + t, C_BLACK);
    tft.drawLine(105, 141 + t, 115, 138 + t, C_BLACK);
    tft.drawLine(115, 138 + t, 125, 138 + t, C_BLACK);
    tft.drawLine(125, 138 + t, 135, 141 + t, C_BLACK);
    tft.drawLine(135, 141 + t, 144, 148 + t, C_BLACK);
  }

  // 口水滴（嘴角处）
  uint16_t droolCol = tft.color565(180, 220, 255);
  drawTeardrop(136, MOUTH_Y + 10, 4, droolCol);
  // 口水丝
  tft.drawLine(134, MOUTH_Y + 4, 138, MOUTH_Y + 16, droolCol);
  tft.drawLine(135, MOUTH_Y + 5, 139, MOUTH_Y + 15, droolCol);

  // 微微脸红
  tft.fillCircle(lx - 4, cy + 10, 5, tft.color565(255, 80, 60));
  tft.fillCircle(rx + EYE_W + 4, cy + 10, 5, tft.color565(255, 80, 60));
}

// ── Blush / Shy (脸红害羞) ──────────────────────────────────
void drawBlushEyes() {
  fillBg(bgColour());
  const int16_t lx = eyeLX(0), rx = eyeRX(0), ey = eyeY(), cy = eyeCY();

  // 正常大眼睛
  tft.fillRect(lx, ey, EYE_W, EYE_H, C_BLACK);
  tft.fillRect(rx, ey, EYE_W, EYE_H, C_BLACK);

  // 瞳孔偏移（往右看，不敢直视）
  tft.fillCircle(lx + EYE_W/4, cy, 5, C_WHITE);
  tft.fillCircle(rx + EYE_W/4, cy, 5, C_WHITE);
  // 高光
  tft.fillCircle(lx + EYE_W/4 - 2, cy - 3, 2, bgColour());
  tft.fillCircle(rx + EYE_W/4 - 2, cy - 3, 2, bgColour());

  // 大片脸红
  uint16_t blushCol = tft.color565(220, 40, 40);
  tft.fillCircle(lx - 8, cy + 6, 11, blushCol);
  tft.fillCircle(rx + EYE_W + 8, cy + 6, 11, blushCol);
  tft.fillCircle(lx - 5, cy + 14, 8, tft.color565(200, 30, 30));
  tft.fillCircle(rx + EYE_W + 5, cy + 14, 8, tft.color565(200, 30, 30));

  // 害羞小嘴（小小的 ω 形）
  tft.drawLine(112, MOUTH_Y - 1, 116, MOUTH_Y - 4, C_BLACK);
  tft.drawLine(116, MOUTH_Y - 4, 120, MOUTH_Y - 1, C_BLACK);
  tft.drawLine(120, MOUTH_Y - 1, 124, MOUTH_Y - 4, C_BLACK);
  tft.drawLine(124, MOUTH_Y - 4, 128, MOUTH_Y - 1, C_BLACK);
  // 加粗
  tft.drawLine(112, MOUTH_Y, 116, MOUTH_Y - 3, C_BLACK);
  tft.drawLine(116, MOUTH_Y - 3, 120, MOUTH_Y, C_BLACK);
  tft.drawLine(120, MOUTH_Y, 124, MOUTH_Y - 3, C_BLACK);
  tft.drawLine(124, MOUTH_Y - 3, 128, MOUTH_Y, C_BLACK);
}

// ── Jealous (😤) ──────────────────────────────────────────────
void drawJealousEyes() {
  fillBg(bgColour());
  const int16_t lx = eyeLX(0), rx = eyeRX(0), ey = eyeY(), cy = eyeCY();
  uint16_t red = tft.color565(200, 0, 0);

  tft.fillRect(lx, ey, EYE_W, EYE_H, C_BLACK);
  tft.fillRect(rx, ey, EYE_W, EYE_H, C_BLACK);

  // 侧目瞳孔
  tft.fillCircle(lx + EYE_W*3/4, cy, 5, C_WHITE);
  tft.fillCircle(rx + EYE_W*3/4, cy, 5, C_WHITE);

  // 红色不对称眉毛
  tft.drawLine(lx - 2, ey + 2, lx + EYE_W + 2, ey - 8, red);
  tft.drawLine(rx - 2, ey - 8, rx + EYE_W + 2, ey + 2, red);
  tft.drawLine(lx - 2, ey + 3, lx + EYE_W + 2, ey - 7, red);
  tft.drawLine(rx - 2, ey - 7, rx + EYE_W + 2, ey + 3, red);

  // 撇嘴
  for (int8_t t = 0; t < 2; t++) {
    tft.drawLine(98, MOUTH_Y + 6 + t, 120, MOUTH_Y + 2 + t, C_BLACK);
    tft.drawLine(120, MOUTH_Y + 2 + t, 142, MOUTH_Y + 6 + t, C_BLACK);
  }
}

// ── Roll Eyes (🙄) ────────────────────────────────────────────
void drawRollEyes() {
  fillBg(bgColour());
  const int16_t lx = eyeLX(0), rx = eyeRX(0), ey = eyeY(), cy = eyeCY();

  // 白色眼白 + 黑框
  tft.fillRect(lx + 2, ey + 2, EYE_W - 4, EYE_H - 4, C_WHITE);
  tft.fillRect(rx + 2, ey + 2, EYE_W - 4, EYE_H - 4, C_WHITE);
  tft.drawRect(lx, ey, EYE_W, EYE_H, C_BLACK);
  tft.drawRect(rx, ey, EYE_W, EYE_H, C_BLACK);

  // 瞳孔翻到最上面
  tft.fillCircle(lx + EYE_W/2, ey + 10, 4, C_BLACK);
  tft.fillCircle(rx + EYE_W/2, ey + 10, 4, C_BLACK);

  // 无奈小嘴
  tft.drawLine(98, MOUTH_Y, 142, MOUTH_Y, C_BLACK);
  tft.drawLine(98, MOUTH_Y + 1, 120, MOUTH_Y - 1, C_BLACK);
  tft.drawLine(120, MOUTH_Y - 1, 142, MOUTH_Y + 1, C_BLACK);
}

// ── Explode (🤯) ─────────────────────────────────────────────
void drawExplodeEyes() {
  fillBg(bgColour());
  const int16_t lx = eyeLX(0), rx = eyeRX(0), ey = eyeY(), cy = eyeCY();
  uint16_t orange = tft.color565(200, 30, 0);

  // 放射爆炸线
  for (uint8_t i = 0; i < 10; i++) {
    int16_t a = i * 36;
    float r = a * 3.14159 / 180;
    int16_t x1 = 120 + cos(r) * 50, y1 = 60 + sin(r) * 40;
    int16_t x2 = 120 + cos(r) * 110, y2 = 60 + sin(r) * 85;
    tft.drawLine(x1, y1, x2, y2, orange);
    tft.drawLine(x1 + 1, y1, x2 + 1, y2, orange);
  }

  // 巨大圆形震惊眼
  tft.fillCircle(lx + EYE_W/2, cy, EYE_W*3/5, C_BLACK);
  tft.fillCircle(rx + EYE_W/2, cy, EYE_W*3/5, C_BLACK);

  // 缩小瞳孔
  tft.fillCircle(lx + EYE_W/2, cy, 5, C_WHITE);
  tft.fillCircle(rx + EYE_W/2, cy, 5, C_WHITE);
  tft.fillCircle(lx + EYE_W/2 - 2, cy - 2, 2, bgColour());
  tft.fillCircle(rx + EYE_W/2 - 2, cy - 2, 2, bgColour());

  // 震惊喜 O 嘴
  tft.drawCircle(120, MOUTH_Y + 2, 10, C_BLACK);
  tft.fillCircle(120, MOUTH_Y + 2, 8, C_BLACK);
  tft.drawPixel(118, MOUTH_Y - 2, C_WHITE);
  tft.drawPixel(124, MOUTH_Y - 1, C_WHITE);
}

// ── Devilish (😈) ─────────────────────────────────────────────
void drawDevilEyes() {
  fillBg(bgColour());
  const int16_t lx = eyeLX(0), rx = eyeRX(0), ey = eyeY(), cy = eyeCY();
  uint16_t red = tft.color565(255, 30, 30);

  // 恶魔角
  tft.fillTriangle(lx + 4, ey - 2, lx + 16, ey - 2, lx + 10, ey - 28, C_BLACK);
  tft.fillTriangle(rx + EYE_W - 16, ey - 2, rx + EYE_W - 4, ey - 2, rx + EYE_W - 10, ey - 28, C_BLACK);

  // 邪恶眯眼
  for (int8_t t = 0; t < 5; t++) {
    tft.drawLine(lx + 2, ey + 6 + t*8, lx + EYE_W - 2, ey + t*8 + 2, C_BLACK);
    tft.drawLine(rx + 2, ey + 6 + t*8, rx + EYE_W - 2, ey + t*8 + 2, C_BLACK);
  }

  // 红色发光瞳孔
  tft.fillCircle(lx + EYE_W/2, cy + 4, 6, red);
  tft.fillCircle(rx + EYE_W/2, cy + 4, 6, red);
  tft.fillCircle(lx + EYE_W/2 - 2, cy + 2, 2, C_WHITE);
  tft.fillCircle(rx + EYE_W/2 - 2, cy + 2, 2, C_WHITE);

  // 奸笑
  for (int8_t t = 0; t < 2; t++) {
    tft.drawLine(98, MOUTH_Y - 4 + t, 115, MOUTH_Y - 8 + t, C_BLACK);
    tft.drawLine(115, MOUTH_Y - 8 + t, 130, MOUTH_Y - 4 + t, C_BLACK);
    tft.drawLine(130, MOUTH_Y - 4 + t, 142, MOUTH_Y + 2 + t, C_BLACK);
  }

  // 尖牙
  tft.fillTriangle(116, MOUTH_Y - 8, 120, MOUTH_Y - 8, 118, MOUTH_Y - 2, C_WHITE);
  tft.fillTriangle(130, MOUTH_Y - 4, 134, MOUTH_Y - 4, 132, MOUTH_Y + 1, C_WHITE);
}

// ── Sick (🤮) ─────────────────────────────────────────────────
void drawSickEyes() {
  fillBg(bgColour());
  const int16_t lx = eyeLX(0), rx = eyeRX(0), ey = eyeY(), cy = eyeCY();
  uint16_t greenTint = tft.color565(150, 195, 130);

  // 晕眩绿眼
  tft.fillRoundRect(lx, ey, EYE_W, EYE_H, 4, greenTint);
  tft.fillRoundRect(rx, ey, EYE_W, EYE_H, 4, greenTint);
  tft.drawRoundRect(lx, ey, EYE_W, EYE_H, 4, C_BLACK);
  tft.drawRoundRect(rx, ey, EYE_W, EYE_H, 4, C_BLACK);

  // 旋涡瞳孔
  tft.fillCircle(lx + EYE_W/2, cy, 8, C_BLACK);
  tft.fillCircle(rx + EYE_W/2, cy, 8, C_BLACK);
  tft.fillCircle(lx + EYE_W/2 - 2, cy - 2, 3, C_WHITE);
  tft.fillCircle(rx + EYE_W/2 - 2, cy - 2, 3, C_WHITE);

  // 张嘴呕吐
  tft.fillCircle(120, MOUTH_Y + 2, 13, C_BLACK);
  tft.fillCircle(120, MOUTH_Y + 6, 14, tft.color565(255, 160, 150)); // 舌头
  tft.fillCircle(110, MOUTH_Y + 18, 3, tft.color565(200, 220, 150));
  tft.fillCircle(128, MOUTH_Y + 20, 3, tft.color565(200, 220, 150));
  tft.fillCircle(120, MOUTH_Y + 16, 2, tft.color565(220, 200, 130));
}

// ── Mochi Pixel Pet ───────────────────────────────────────────
void initPet() {
  petX = 120; petDir = 1; petLegFrame = 0;
  petAction = 0; petJumpOff = 0;
  petMoveMs = millis(); petActionMs = millis();
}

void drawPetScene() {
  fillBg(bgColour());
  tft.drawFastHLine(0, 224, DISP_W, tft.color565(200, 100, 70));
  tft.fillRect(0, 225, DISP_W, 15, tft.color565(255, 200, 150));

  uint16_t skin = tft.color565(255, 215, 175);
  int16_t cx = petX;
  int16_t cy = PET_CY - petJumpOff;

  // 地面阴影
  tft.fillCircle(cx, 224, 18, tft.color565(200, 100, 70));

  // 腿部动画
  if (petAction == 0) {
    int16_t lo = petLegFrame ? 4 : -4;
    tft.fillRoundRect(cx - 11 + lo, cy + PET_R - 5, 7, 10, 3, skin);
    tft.fillRoundRect(cx + 4 - lo, cy + PET_R - 5, 7, 10, 3, skin);
  } else {
    tft.fillRoundRect(cx - 9, cy + PET_R - 2, 7, 7, 3, skin);
    tft.fillRoundRect(cx + 2, cy + PET_R - 2, 7, 7, 3, skin);
  }

  // 身体
  tft.fillCircle(cx, cy, PET_R, skin);
  tft.fillCircle(cx, cy + 1, PET_R - 2, tft.color565(240, 200, 160));

  // 眼睛（朝向运动方向）
  int16_t eo = petDir * 2;
  tft.fillCircle(cx - 8 + eo, cy - 4, 3, C_BLACK);
  tft.fillCircle(cx + 8 + eo, cy - 4, 3, C_BLACK);
  tft.drawPixel(cx - 7 + eo, cy - 5, C_WHITE);
  tft.drawPixel(cx + 9 + eo, cy - 5, C_WHITE);

  // 嘴巴
  if (petAction >= 1 && petAction <= 2) {
    tft.drawLine(cx - 5, cy + 4, cx, cy + 9, C_BLACK);
    tft.drawLine(cx, cy + 9, cx + 5, cy + 4, C_BLACK);
  } else {
    tft.drawLine(cx - 4, cy + 5, cx, cy + 8, C_BLACK);
    tft.drawLine(cx, cy + 8, cx + 4, cy + 5, C_BLACK);
  }

  // 腮红
  tft.fillCircle(cx - 14, cy + 2, 4, tft.color565(255, 150, 150));
  tft.fillCircle(cx + 14, cy + 2, 4, tft.color565(255, 150, 150));
}

void updatePet() {
  unsigned long now = millis();

  if (now - petActionMs > 3000 + random(2000)) {
    petActionMs = now;
    uint8_t r = random(10);
    if (r < 7)      petAction = 0;
    else if (r < 9) petAction = 3;
    else            { petAction = 1; petJumpOff = 0; }
  }

  bool dirty = false;

  if (petAction == 1) {
    petJumpOff += 4;
    if (petJumpOff >= 28) { petJumpOff = 28; petAction = 2; }
    dirty = true;
  } else if (petAction == 2) {
    petJumpOff -= 3;
    if (petJumpOff <= 0) { petJumpOff = 0; petAction = 0; }
    dirty = true;
  }

  if (petAction == 0 && now - petMoveMs > 160) {
    petMoveMs = now;
    petX += petDir * 4;
    petLegFrame = !petLegFrame;
    if (petX < PET_R + 5) { petX = PET_R + 5;  petDir = 1; }
    if (petX > DISP_W - PET_R - 5) { petX = DISP_W - PET_R - 5; petDir = -1; }
    dirty = true;
  } else if (petAction == 3 && now - petMoveMs > 400) {
    petMoveMs = now;
    dirty = true;
  }

  if (dirty) drawPetScene();
}

// ── Thinking (🤔) ────────────────────────────────────────────
void drawThinkingEyes() {
  fillBg(bgColour());
  const int16_t lx = eyeLX(0), rx = eyeRX(0), ey = eyeY(), cy = eyeCY();

  // 半闭思考眼
  tft.fillRect(lx, ey + 6, EYE_W, EYE_H - 12, C_BLACK);
  tft.fillRect(rx, ey + 6, EYE_W, EYE_H - 12, C_BLACK);

  // 瞳孔向上看（思考状）
  tft.fillCircle(lx + EYE_W/2, ey + EYE_H/3, 5, C_WHITE);
  tft.fillCircle(rx + EYE_W/2, ey + EYE_H/3, 5, C_WHITE);
  tft.fillCircle(lx + EYE_W/2 - 2, ey + EYE_H/3 - 2, 2, bgColour());
  tft.fillCircle(rx + EYE_W/2 - 2, ey + EYE_H/3 - 2, 2, bgColour());

  // 思考纹（额头弧线）
  tft.drawLine(95, ey - 12, 105, ey - 18, C_MUTED);
  tft.drawLine(105, ey - 18, 115, ey - 12, C_MUTED);

  // 微撇嘴
  tft.drawLine(105, MOUTH_Y + 3, 120, MOUTH_Y + 6, C_BLACK);
  tft.drawLine(120, MOUTH_Y + 6, 135, MOUTH_Y + 3, C_BLACK);
  tft.drawLine(105, MOUTH_Y + 4, 120, MOUTH_Y + 7, C_BLACK);
  tft.drawLine(120, MOUTH_Y + 7, 135, MOUTH_Y + 4, C_BLACK);

  // 腮红
  tft.fillCircle(lx - 6, cy + 8, 5, tft.color565(200, 100, 80));
  tft.fillCircle(rx + EYE_W + 6, cy + 8, 5, tft.color565(200, 100, 80));
}

void updateThinkingEyes() {
  static unsigned long lastRedraw = 0;
  unsigned long now = millis();
  if (now - lastRedraw < 350) return;
  lastRedraw = now;

  // 重绘表情
  const int16_t lx = eyeLX(0), rx = eyeRX(0), ey = eyeY(), cy = eyeCY();
  fillBg(bgColour());
  tft.fillRect(lx, ey + 6, EYE_W, EYE_H - 12, C_BLACK);
  tft.fillRect(rx, ey + 6, EYE_W, EYE_H - 12, C_BLACK);
  tft.fillCircle(lx + EYE_W/2, ey + EYE_H/3, 5, C_WHITE);
  tft.fillCircle(rx + EYE_W/2, ey + EYE_H/3, 5, C_WHITE);
  tft.fillCircle(lx + EYE_W/2 - 2, ey + EYE_H/3 - 2, 2, bgColour());
  tft.fillCircle(rx + EYE_W/2 - 2, ey + EYE_H/3 - 2, 2, bgColour());
  tft.drawLine(95, ey - 12, 105, ey - 18, C_MUTED);
  tft.drawLine(105, ey - 18, 115, ey - 12, C_MUTED);
  tft.drawLine(105, MOUTH_Y + 3, 120, MOUTH_Y + 6, C_BLACK);
  tft.drawLine(120, MOUTH_Y + 6, 135, MOUTH_Y + 3, C_BLACK);
  tft.drawLine(105, MOUTH_Y + 4, 120, MOUTH_Y + 7, C_BLACK);
  tft.drawLine(120, MOUTH_Y + 7, 135, MOUTH_Y + 4, C_BLACK);
  tft.fillCircle(lx - 6, cy + 8, 5, tft.color565(200, 100, 80));
  tft.fillCircle(rx + EYE_W + 6, cy + 8, 5, tft.color565(200, 100, 80));

  // "..." 弹跳动画
  static int8_t thinkDotPhase = 0;
  thinkDotPhase = (thinkDotPhase + 1) % 4;
  int16_t bounceY[4] = {0, -4, 0, 0};
  for (uint8_t i = 0; i < 3; i++) {
    int16_t dx = i * 14 - 14;
    int16_t dy = (i == thinkDotPhase % 3) ? bounceY[thinkDotPhase] : 0;
    tft.fillCircle(120 + dx, 185 + dy, 4, C_MUTED);
    tft.fillCircle(120 + dx, 185 + dy - 1, 3, tft.color565(120, 118, 116));
  }
}

// ═════════════════════════════════════════════════════════════
//  ANIMATION UPDATES (called from loop)
// ═════════════════════════════════════════════════════════════

void updateNormalEyes() {
  // 眨眼间隔从 3s 改为 5s，眨眼动作放慢
  if (millis() - lastAnimMs > 5000) {
    lastAnimMs = millis();
    drawNormalEyes(0, true);  delay(120);
    drawNormalEyes(0, false); delay(100);
    drawNormalEyes(0, true);  delay(120);
    drawNormalEyes(0, false);
    return;
  }
  if (millis() - lastCycleMs % 16000 < 200) {
    lastAnimMs = millis();
    const int16_t offs[] = {-16, 16, -16, 16, 0};
    for (uint8_t i = 0; i < 5; i++) {
      drawNormalEyes(offs[i]);
      delay(80);
    }
  }
}

void updateSquishEyes() {
  if (millis() - lastAnimMs > 3500) {
    lastAnimMs = millis();
    for (uint8_t i = 0; i < 2; i++) {
      drawSquishEyes(false); delay(140);
      drawSquishEyes(true);  delay(100);
    }
    drawSquishEyes(false);
  }
}

void updateHappyEyes() {
  if (millis() - lastAnimMs > 5000) {
    lastAnimMs = millis();
    drawHappyEyes(true);   delay(200);
    drawHappyEyes(false);  delay(100);
    drawHappyEyes(true);   delay(150);
    drawHappyEyes(false);
  }
}

void updateSurprisedEyes() {
  if (millis() - lastAnimMs > 3000) {
    lastAnimMs = millis();
    fillBg(bgColour());
    const int16_t lx = eyeLX(0), rx = eyeRX(0), ey = eyeY();
    const int16_t cx = EYE_W / 2, cy = EYE_H / 2;
    tft.fillCircle(lx + cx, ey + cy, EYE_W * 3 / 5, C_BLACK);
    tft.fillCircle(rx + cx, ey + cy, EYE_W * 3 / 5, C_BLACK);
    delay(120);
    drawSurprisedEyes();
  }
}

void updateHeartEyes() {
  if (millis() - lastAnimMs > 4000) {
    lastAnimMs = millis();
    const int16_t lx = eyeLX(0), rx = eyeRX(0), ey = eyeY();
    const int16_t cy2 = ey + EYE_H / 2;
    uint16_t hc = heartColour();
    for (int8_t t = 0; t < 3; t++) {
      int16_t hcx = lx + EYE_W / 2;
      int16_t hcy = cy2 - 4 - t;
      tft.fillCircle(hcx - 7, hcy - 2, 8, hc);
      tft.fillCircle(hcx + 7, hcy - 2, 8, hc);
      tft.fillTriangle(hcx - 14, hcy, hcx + 14, hcy, hcx, hcy + 15, hc);
      hcx = rx + EYE_W / 2;
      tft.fillCircle(hcx - 7, hcy - 2, 8, hc);
      tft.fillCircle(hcx + 7, hcy - 2, 8, hc);
      tft.fillTriangle(hcx - 14, hcy, hcx + 14, hcy, hcx, hcy + 15, hc);
      delay(60);
    }
    drawHeartEyes();
  }
}

void updateSleepyEyes() {
  static uint8_t sleepLevel = 0;
  static bool waking = false;
  if (millis() - lastAnimMs > 200) {
    lastAnimMs = millis();
    if (!waking) {
      sleepLevel += 8;
      if (sleepLevel >= 255) {
        waking = true;
        delay(400);
      }
    } else {
      sleepLevel -= 20;
      if (sleepLevel <= 0) {
        sleepLevel = 0;
        waking = false;
        lastAnimMs += 3000;
      }
    }
    drawSleepyEyes(sleepLevel);
  }
}

void updateWinkEyes() {
  if (millis() - lastAnimMs > 2500) {
    lastAnimMs = millis();
    drawWinkEyes(true);   delay(150);
    drawWinkEyes(false);  delay(80);
    drawWinkEyes(true);   delay(100);
    drawWinkEyes(false);
  }
}

void updateAngryEyes() {
  if (millis() - lastAnimMs > 3000) {
    lastAnimMs = millis();
    for (uint8_t i = 0; i < 4; i++) {
      drawAngryEyes();
      delay(200);
    }
  }
}

void updateCryEyes() {
  // 眼泪偶尔"刷新"（模拟流泪）
  if (millis() - lastAnimMs > 4000) {
    lastAnimMs = millis();
    drawCryEyes();
    delay(150);
    // 再画一遍让眼泪"流下"
    fillBg(bgColour());
    const int16_t lx = eyeLX(0), rx = eyeRX(0), ey = eyeY(), cy = eyeCY();
    uint16_t tearBlue = tft.color565(80, 160, 255);
    for (int8_t t = 0; t < 4; t++) {
      tft.drawLine(lx, ey + EYE_H/4 + t, lx + EYE_W/2, ey + EYE_H/2 + t, C_BLACK);
      tft.drawLine(lx + EYE_W/2, ey + EYE_H/2 + t, lx + EYE_W, ey + EYE_H/4 + t, C_BLACK);
      tft.drawLine(rx, ey + EYE_H/4 + t, rx + EYE_W/2, ey + EYE_H/2 + t, C_BLACK);
      tft.drawLine(rx + EYE_W/2, ey + EYE_H/2 + t, rx + EYE_W, ey + EYE_H/4 + t, C_BLACK);
    }
    // 眼泪位置下移（流下效果）
    drawTeardrop(lx + 8,  ey + EYE_H + 10, 4, tearBlue);
    drawTeardrop(lx + 22, ey + EYE_H + 20, 3, tearBlue);
    drawTeardrop(rx + 8,  ey + EYE_H + 10, 4, tearBlue);
    drawTeardrop(rx + 22, ey + EYE_H + 20, 3, tearBlue);
    tft.drawLine(lx + 6, ey + EYE_H + 6, lx + 10, ey + EYE_H + 16, tearBlue);
    tft.drawLine(rx + 6, ey + EYE_H + 6, rx + 10, ey + EYE_H + 16, tearBlue);
    for (int8_t t = 0; t < 2; t++) {
      tft.drawLine(98, MOUTH_Y + t, 108, MOUTH_Y - 4 + t, C_BLACK);
      tft.drawLine(108, MOUTH_Y - 4 + t, 118, MOUTH_Y + 2 + t, C_BLACK);
      tft.drawLine(118, MOUTH_Y + 2 + t, 128, MOUTH_Y - 3 + t, C_BLACK);
      tft.drawLine(128, MOUTH_Y - 3 + t, 138, MOUTH_Y + 1 + t, C_BLACK);
      tft.drawLine(138, MOUTH_Y + 1 + t, 142, MOUTH_Y + t, C_BLACK);
    }
    delay(200);
    drawCryEyes();
  }
}

void updateSweatEyes() {
  // 汗滴闪烁效果
  if (millis() - lastAnimMs > 3000) {
    lastAnimMs = millis();
    drawSweatEyes();
  }
}

void updateDroolEyes() {
  // 口水周期性摆动
  if (millis() - lastAnimMs > 3000) {
    lastAnimMs = millis();
    drawDroolEyes();
  }
}

void updateBlushEyes() {
  // 脸红微微脉动
  if (millis() - lastAnimMs > 2000) {
    lastAnimMs = millis();
    drawBlushEyes();
  }
}

void updateJealousEyes() {
  if (millis() - lastAnimMs > 3000) {
    lastAnimMs = millis();
    drawJealousEyes();
  }
}

void updateRollEyes() {
  if (millis() - lastAnimMs > 2500) {
    lastAnimMs = millis();
    drawRollEyes();
  }
}

void updateExplodeEyes() {
  if (millis() - lastAnimMs > 2000) {
    lastAnimMs = millis();
    drawExplodeEyes();
    delay(150);
    drawExplodeEyes();
  }
}

void updateDevilEyes() {
  if (millis() - lastAnimMs > 2500) {
    lastAnimMs = millis();
    drawDevilEyes();
  }
}

void updateSickEyes() {
  if (millis() - lastAnimMs > 2000) {
    lastAnimMs = millis();
    drawSickEyes();
  }
}

// ═════════════════════════════════════════════════════════════
//  VIEW MANAGEMENT
// ═════════════════════════════════════════════════════════════

void enterView(uint8_t v) {
  currentView = v;
  lastAnimMs = 0;
  lastCycleMs = millis();

  switch (v) {
    case VIEW_LOGO:
      animLogoReveal();
      currentView = VIEW_EYES_NORMAL;
      lastCycleMs = millis();
      drawNormalEyes();
      break;
    case VIEW_EYES_NORMAL:    drawNormalEyes();    break;
    case VIEW_EYES_SQUISH:    drawSquishEyes(false); break;
    case VIEW_EYES_HAPPY:     drawHappyEyes(false);  break;
    case VIEW_EYES_SURPRISED: drawSurprisedEyes();   break;
    case VIEW_EYES_HEART:     drawHeartEyes();       break;
    case VIEW_EYES_SLEEPY:    drawSleepyEyes(0);     break;
    case VIEW_EYES_WINK:      drawWinkEyes(false);   break;
    case VIEW_EYES_ANGRY:     drawAngryEyes();       break;
    case VIEW_EYES_CRY:       drawCryEyes();         break;
    case VIEW_EYES_SWEAT:     drawSweatEyes();       break;
    case VIEW_EYES_DROOL:     drawDroolEyes();       break;
    case VIEW_EYES_BLUSH:     drawBlushEyes();       break;
    case VIEW_EYES_JEALOUS:   drawJealousEyes();     break;
    case VIEW_EYES_ROLL:      drawRollEyes();        break;
    case VIEW_EYES_EXPLODE:   drawExplodeEyes();     break;
    case VIEW_EYES_DEVIL:     drawDevilEyes();       break;
    case VIEW_EYES_SICK:      drawSickEyes();        break;
    case VIEW_EYES_THINK:     drawThinkingEyes();    break;
    case VIEW_PET:
      initPet();
      drawPetScene();
      break;
  }
}

void nextView() {
  uint8_t next = currentView + 1;
  if (next >= VIEW_COUNT) next = VIEW_EYES_NORMAL;
  enterView(next);
}

// ═════════════════════════════════════════════════════════════
//  BUTTON
// ═════════════════════════════════════════════════════════════

void handleButton() {
  const bool reading = digitalRead(BTN_PIN);
  const unsigned long now = millis();

  if (reading != btnState) {
    btnState = reading;
    if (reading == LOW) {
      btnPressMs = now;
      btnWasPressed = false;
    }
  }

  if (reading == LOW && !btnWasPressed && now - btnPressMs > 800) {
    btnWasPressed = true;
    setBacklight(!backlightOn);
    if (!backlightOn) {
      tft.fillScreen(C_DARKBG);
      tft.setTextColor(C_MUTED); tft.setTextSize(2);
      tft.setCursor(40, 110); tft.print("sleep...");
      delay(600);
      digitalWrite(TFT_BLK, LOW);
    } else {
      enterView(currentView);
    }
    return;
  }

  if (reading == HIGH && btnPressMs > 0 && !btnWasPressed && now - btnPressMs > 50) {
    btnPressMs = 0;
    if (backlightOn) {
      nextView();
    } else {
      setBacklight(true);
      enterView(currentView);
    }
  }
}

// ═════════════════════════════════════════════════════════════
//  SETUP
// ═════════════════════════════════════════════════════════════

void setup() {
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(TFT_BLK, OUTPUT);
  setBacklight(true);

  SPI.begin(8, -1, 10, TFT_CS);
  tft.init(240, 240);
  tft.setSPISpeed(40000000);
  tft.setRotation(1);
  initColours();

  startupMs = millis();

  fillBg(bgColour());
  tft.setTextColor(C_WHITE); tft.setTextSize(3);
  tft.setCursor(DISP_W / 2 - 54, DISP_H / 2 - 22); tft.print("Clawd");
  tft.setCursor(DISP_W / 2 - 54, DISP_H / 2 + 14); tft.print("Mochi");
  delay(1200);

  animLogoReveal();

  enterView(VIEW_EYES_NORMAL);
}

// ═════════════════════════════════════════════════════════════
//  LOOP
// ═════════════════════════════════════════════════════════════

void loop() {
  handleButton();

  const unsigned long now = millis();

  if (backlightOn && currentView >= VIEW_EYES_NORMAL && now - lastCycleMs > (unsigned long)autoCycleSec * 1000) {
    lastCycleMs = now;
    nextView();
    return;
  }

  if (!backlightOn) return;

  switch (currentView) {
    case VIEW_EYES_NORMAL:    updateNormalEyes();    break;
    case VIEW_EYES_SQUISH:    updateSquishEyes();    break;
    case VIEW_EYES_HAPPY:     updateHappyEyes();     break;
    case VIEW_EYES_SURPRISED: updateSurprisedEyes(); break;
    case VIEW_EYES_HEART:     updateHeartEyes();     break;
    case VIEW_EYES_SLEEPY:    updateSleepyEyes();    break;
    case VIEW_EYES_WINK:      updateWinkEyes();      break;
    case VIEW_EYES_ANGRY:     updateAngryEyes();     break;
    case VIEW_EYES_CRY:       updateCryEyes();       break;
    case VIEW_EYES_SWEAT:     updateSweatEyes();     break;
    case VIEW_EYES_DROOL:     updateDroolEyes();     break;
    case VIEW_EYES_BLUSH:     updateBlushEyes();     break;
    case VIEW_EYES_JEALOUS:   updateJealousEyes();   break;
    case VIEW_EYES_ROLL:      updateRollEyes();      break;
    case VIEW_EYES_EXPLODE:   updateExplodeEyes();   break;
    case VIEW_EYES_DEVIL:     updateDevilEyes();     break;
    case VIEW_EYES_SICK:      updateSickEyes();      break;
    case VIEW_EYES_THINK:     updateThinkingEyes();  break;
    case VIEW_PET:            updatePet();           break;
  }

}
