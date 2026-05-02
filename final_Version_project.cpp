#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "frame.h"

/* ================= OLED ================= */
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

/* ================= BUTTON ================= */
#define BTN_UP     2
#define BTN_DOWN   5
#define BTN_SELECT 4

/* ================= BUZZER ================= */
#define BUZZER 18

/* ================= STATE ================= */
enum GameState {
  STATE_MENU,
  STATE_FLAPPY,
  STATE_SNAKE,
  STATE_PONG,
  STATE_CAT
};
GameState currentState = STATE_MENU;

/* ================= MENU ================= */
const char* menuItems[] = { "Flappy Bird", "Snake", "Pong", "Cat Jump" };
const int menuCount = 4;
int menuIndex = 0;

/* ================= INTRO MUSIC ================= */
int introMelody[] = { 988, 988, 1319, 988, 784, 659, 784, 988 };
int introDuration[] = { 120, 120, 240, 120, 120, 240, 120, 360 };

void playIntroMusic() {
  for (int i = 0; i < 8; i++) {
    tone(BUZZER, introMelody[i], introDuration[i]);
    delay(introDuration[i] * 1.3);
    noTone(BUZZER);
  }
}

/* ================= SFX ================= */
void sfxEat()   { tone(BUZZER, 1400, 60); delay(80); noTone(BUZZER); }
void sfxFlap()  { tone(BUZZER, 1600, 30); delay(40); noTone(BUZZER); }
void sfxCrash() {
  int notes[] = {900,700,500,300};
  for (int i = 0; i < 4; i++) {
    tone(BUZZER, notes[i], 90);
    delay(110);
  }
  noTone(BUZZER);
}

/* ================= BUTTON ================= */
bool buttonPressed(int pin) {
  static unsigned long lastTime = 0;
  if (digitalRead(pin) == LOW && millis() - lastTime > 150) {
    lastTime = millis();
    return true;
  }
  return false;
}

/* =================================================
   ========== NOTE DEFINITIONS (SMOOTH) ============
   ================================================= */
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_REST 0

/* =================================================
   ========= HAPPY BIRTHDAY (SMOOTH) ================
   ================================================= */
int hbMelody[] = {
  NOTE_G4, NOTE_G4, NOTE_A4, NOTE_G4, NOTE_C5, NOTE_AS4,
  NOTE_G4, NOTE_G4, NOTE_A4, NOTE_G4, NOTE_D5, NOTE_C5,
  NOTE_G4, NOTE_G4, NOTE_G5, NOTE_E5, NOTE_C5, NOTE_AS4, NOTE_A4,
  NOTE_F5, NOTE_F5, NOTE_E5, NOTE_C5, NOTE_D5, NOTE_C5
};

int hbDuration[] = {
  300,300,600,600,600,900,
  300,300,600,600,600,900,
  300,300,600,600,600,600,900,
  300,300,600,600,600,1200
};

const int HB_LENGTH = sizeof(hbMelody) / sizeof(int);
int hbIndex = 0;
unsigned long hbLastTime = 0;
bool hbFinished = false;

/* =================================================
   ================= CAT JUMP ======================
   ================================================= */
int catFrame = 0;

void catInit() {
  catFrame = 0;
  hbIndex = 0;
  hbLastTime = millis();
  hbFinished = false;
}

void catLoop() {

  if (!hbFinished && millis() - hbLastTime >= hbDuration[hbIndex]) {
    hbLastTime = millis();

    if (hbMelody[hbIndex] == NOTE_REST)
      noTone(BUZZER);
    else
      tone(BUZZER, hbMelody[hbIndex], hbDuration[hbIndex]);

    catFrame = (catFrame + 1) % FRAME_COUNT;
    hbIndex++;

    if (hbIndex >= HB_LENGTH) {
      hbFinished = true;
      noTone(BUZZER);
      delay(600);
      currentState = STATE_MENU;
      return;
    }
  }

  display.clearDisplay();
  int x = (SCREEN_WIDTH - FRAME_WIDTH) / 2;
  int y = (SCREEN_HEIGHT - FRAME_HEIGHT) / 2 + 16;
  display.drawBitmap(x, y, frames[catFrame],
                     FRAME_WIDTH, FRAME_HEIGHT, WHITE);
  display.setCursor(5, 0);  // geser sedikit ke kanan untuk rata tengah
  display.println("-- HAPPY BIRTHDAY --");

// Teks "NANDALITOS!!! :3"
  display.setCursor(26, 10); // geser lebih ke kanan agar berada di tengah
  display.println("NANDALITOS!!!");

  display.display();
}

/* =================================================
   ================= FLAPPY BIRD ===================
   ================================================= */
#define SPRITE_WIDTH 16
#define SPRITE_HEIGHT 16
#define GAME_SPEED 40

int flappyScore = 0;
int flappyHighScore = 0;
bool flappyGameOver = false;

int bird_x = SCREEN_WIDTH / 4;
int bird_y;
int momentum;
int wall_x[2], wall_y[2];
int wall_gap = 30;
int wall_width = 10;

static const unsigned char PROGMEM wing_up_bmp[] = {
  0x00,0x00,0x00,0x00,0x03,0xC0,0x1F,0xF0,
  0x3F,0x38,0x71,0xFE,0xEE,0xC1,0xDF,0x7E,
  0xDF,0x78,0xFF,0xF8,0xFF,0xF8,0xFF,0xF0,
  0x7F,0xE0,0x3F,0xC0,0x07,0x00,0x00,0x00
};

void flappyInit() {
  bird_y = SCREEN_HEIGHT / 2;
  momentum = -4;
  flappyScore = 0;
  flappyGameOver = false;
  wall_x[0] = SCREEN_WIDTH;
  wall_y[0] = random(10, SCREEN_HEIGHT - wall_gap);
  wall_x[1] = SCREEN_WIDTH + 64;
  wall_y[1] = random(10, SCREEN_HEIGHT - wall_gap);
}

void flappyLoop() {
  if (flappyGameOver) {
    display.clearDisplay();
    display.setCursor(30, 18); display.println("GAME OVER");
    display.setCursor(20, 34);
    display.print("S:"); display.print(flappyScore);
    display.print(" H:"); display.print(flappyHighScore);
    display.setCursor(10, 56); display.println("SELECT = MENU");
    display.display();
    if (buttonPressed(BTN_SELECT)) currentState = STATE_MENU;
    return;
  }

  if (buttonPressed(BTN_SELECT)) {
    currentState = STATE_MENU;
    return;
  }

  if (digitalRead(BTN_UP) == LOW) {
    momentum = -4;
    sfxFlap();
  }

  momentum++;
  bird_y += momentum;
  bird_y = constrain(bird_y, 0, SCREEN_HEIGHT - SPRITE_HEIGHT);

  display.clearDisplay();
  display.drawBitmap(bird_x, bird_y, wing_up_bmp, 16, 16, WHITE);

  for (int i = 0; i < 2; i++) {
    display.fillRect(wall_x[i], 0, wall_width, wall_y[i], WHITE);
    display.fillRect(wall_x[i], wall_y[i] + wall_gap,
                     wall_width, SCREEN_HEIGHT, WHITE);

    if (wall_x[i] < -wall_width) {
      wall_x[i] = SCREEN_WIDTH;
      wall_y[i] = random(10, SCREEN_HEIGHT - wall_gap);
      flappyScore++;
      flappyHighScore = max(flappyScore, flappyHighScore);
    }

    if (bird_x + SPRITE_WIDTH > wall_x[i] &&
        bird_x < wall_x[i] + wall_width &&
        (bird_y < wall_y[i] ||
         bird_y + SPRITE_HEIGHT > wall_y[i] + wall_gap)) {
      flappyGameOver = true;
      sfxCrash();
    }

    wall_x[i] -= 3;
  }

  display.setCursor(0, 0);
  display.print("S:"); display.print(flappyScore);
  display.print(" H:"); display.print(flappyHighScore);
  display.display();
  delay(GAME_SPEED);
}

/* =================================================
   ==================== SNAKE ======================
   ================================================= */
#define GRID 4
#define MAX_SNAKE 64

int snakeX[MAX_SNAKE], snakeY[MAX_SNAKE];
int snakeLength;
int foodX, foodY;
int snakeDir = 0;
bool snakeGameOver = true;
unsigned long snakeLastMove = 0;
unsigned long snakeLastBtn = 0;
int snakeSpeed = 150;

int snakeScore = 0;
int snakeHighScore = 0;

void spawnFood() {
  foodX = random(2, SCREEN_WIDTH / GRID - 2);
  foodY = random(2, SCREEN_HEIGHT / GRID - 2);
}

void snakeInit() {
  snakeLength = 3;
  snakeScore = 0;
  snakeDir = 0;
  snakeGameOver = false;
  snakeLastMove = millis();
  for (int i = 0; i < snakeLength; i++) {
    snakeX[i] = 10 - i;
    snakeY[i] = 8;
  }
  spawnFood();
}

void snakeInput() {
  if (millis() - snakeLastBtn < 120) return;
  if (digitalRead(BTN_UP) == LOW)   { snakeDir = (snakeDir + 3) % 4; snakeLastBtn = millis(); }
  if (digitalRead(BTN_DOWN) == LOW) { snakeDir = (snakeDir + 1) % 4; snakeLastBtn = millis(); }
}

void moveSnake() {
  for (int i = snakeLength - 1; i > 0; i--) {
    snakeX[i] = snakeX[i - 1];
    snakeY[i] = snakeY[i - 1];
  }

  if (snakeDir == 0) snakeX[0]++;
  if (snakeDir == 1) snakeY[0]++;
  if (snakeDir == 2) snakeX[0]--;
  if (snakeDir == 3) snakeY[0]--;

  if (snakeX[0] < 0 || snakeY[0] < 0 ||
      snakeX[0] >= SCREEN_WIDTH / GRID ||
      snakeY[0] >= SCREEN_HEIGHT / GRID) {
    snakeGameOver = true;
    sfxCrash();
  }

  for (int i = 1; i < snakeLength; i++)
    if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
      snakeGameOver = true;
      sfxCrash();
    }

  if (snakeX[0] == foodX && snakeY[0] == foodY) {
    if (snakeLength < MAX_SNAKE) {
      snakeLength++;
      snakeScore++;
      snakeHighScore = max(snakeScore, snakeHighScore);
    }
    sfxEat();
    spawnFood();
  }
}

void snakeLoop() {
  if (buttonPressed(BTN_SELECT)) { currentState = STATE_MENU; return; }
  snakeInput();

  if (!snakeGameOver && millis() - snakeLastMove > snakeSpeed) {
    moveSnake();
    snakeLastMove = millis();
  }

  display.clearDisplay();

  if (snakeGameOver) {
    display.setCursor(30, 18); display.println("GAME OVER");
    display.setCursor(10, 34);
    display.print("S:"); display.print(snakeScore);
    display.print(" H:"); display.print(snakeHighScore);
    display.setCursor(10, 56); display.println("SELECT = MENU");
    display.display();
    return;
  }

  display.setCursor(0, 0);
  display.print("S:"); display.print(snakeScore);
  display.print(" H:"); display.print(snakeHighScore);

  display.fillRect(foodX * GRID, foodY * GRID, GRID, GRID, WHITE);
  for (int i = 0; i < snakeLength; i++)
    display.fillRect(snakeX[i] * GRID, snakeY[i] * GRID, GRID, GRID, WHITE);

  display.display();
}

/* =================================================
   ===================== PONG ======================
   ================================================= */
int paddleY, aiY;
int ballX, ballY, ballVX, ballVY;
bool pongGameOver = false;
int pongScore = 0;
int pongHighScore = 0;

void pongInit() {
  paddleY = 24;
  aiY = 24;
  ballX = 64;
  ballY = 32;
  ballVX = -2;
  ballVY = 2;
  pongScore = 0;
  pongGameOver = false;
}

void pongLoop() {
  if (pongGameOver) {
    display.clearDisplay();
    display.setCursor(30, 18); display.println("GAME OVER");
    display.setCursor(20, 34);
    display.print("S:"); display.print(pongScore);
    display.print(" H:"); display.print(pongHighScore);
    display.setCursor(10, 56); display.println("SELECT = MENU");
    display.display();
    if (buttonPressed(BTN_SELECT)) currentState = STATE_MENU;
    return;
  }

  if (buttonPressed(BTN_SELECT)) { currentState = STATE_MENU; return; }

  if (digitalRead(BTN_UP) == LOW && paddleY > 0) paddleY -= 3;
  if (digitalRead(BTN_DOWN) == LOW && paddleY < 48) paddleY += 3;

  if (ballY > aiY + 8) aiY += 2;
  if (ballY < aiY + 8) aiY -= 2;

  ballX += ballVX;
  ballY += ballVY;

  if (ballY <= 0 || ballY >= 63) ballVY *= -1;

  if (ballVX < 0 &&
      ballX <= 6 &&
      ballY >= paddleY &&
      ballY <= paddleY + 16) {
    ballVX = abs(ballVX);
    pongScore++;
    pongHighScore = max(pongScore, pongHighScore);
    sfxEat();
  }

  if (ballVX > 0 &&
      ballX >= 122 &&
      ballY >= aiY &&
      ballY <= aiY + 16) {
    ballVX = -abs(ballVX);
  }

  if (ballX < 0) {
    pongGameOver = true;
    sfxCrash();
  }

  display.clearDisplay();
  display.fillRect(2, paddleY, 3, 16, WHITE);
  display.fillRect(123, aiY, 3, 16, WHITE);
  display.fillCircle(ballX, ballY, 2, WHITE);

  display.setCursor(0, 0);
  display.print("S:"); display.print(pongScore);
  display.print(" H:"); display.print(pongHighScore);

  display.display();
  delay(30);
}

/* ================= MENU ================= */
void menuLoop() {
  display.clearDisplay();
  display.setCursor(18, 0);
  display.println("=== GAME MENU ===");

  for (int i = 0; i < menuCount; i++) {
    int y = 16 + i * 14;
    if (i == menuIndex) {
      display.fillRect(0, y - 2, SCREEN_WIDTH, 12, WHITE);
      display.setTextColor(BLACK);
    } else display.setTextColor(WHITE);

    display.setCursor(10, y);
    display.println(menuItems[i]);
    display.setTextColor(WHITE);
  }

  display.display();

  if (buttonPressed(BTN_UP))   menuIndex = (menuIndex + menuCount - 1) % menuCount;
  if (buttonPressed(BTN_DOWN)) menuIndex = (menuIndex + 1) % menuCount;

  if (buttonPressed(BTN_SELECT)) {
    if (menuIndex == 0) { flappyInit(); currentState = STATE_FLAPPY; }
    if (menuIndex == 1) { snakeInit();  currentState = STATE_SNAKE;  }
    if (menuIndex == 2) { pongInit();   currentState = STATE_PONG;   }
    if (menuIndex == 3) { catInit();    currentState = STATE_CAT;    }
  }
}

/* ================= SETUP & LOOP ================= */
void setup() {
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextColor(WHITE);
  randomSeed(analogRead(0));
  playIntroMusic();
}

void loop() {
  switch (currentState) {
    case STATE_MENU:   menuLoop();   break;
    case STATE_FLAPPY: flappyLoop(); break;
    case STATE_SNAKE:  snakeLoop();  break;
    case STATE_PONG:   pongLoop();   break;
    case STATE_CAT:    catLoop();    break;
  }
}
