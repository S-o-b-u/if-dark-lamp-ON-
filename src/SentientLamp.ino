#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------- OLED SETTINGS ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------- PINS ----------
const int ldrPin = A0;
const int ledPin = 9;
const int buzzerPin = 6;
const int buttonPin = 2;

// ---------- CONFIGURATION ----------
const int DARK_ON = 750;
const int BRIGHT_OFF = 600;

// ---------- EYE GEOMETRY (BIGGER PUPILS) ----------
const int EYE_WIDTH = 56;
const int EYE_HEIGHT = 64;
const int EYE_GAP = 8;
const int EYE_RADIUS = 10;

// *** UPDATED: BIGGER PUPILS ***
const int PUPIL_RADIUS = 16;  // Was 12, now 16 (Bigger!)
const int EYE_Y = 0;

const int LEFT_EYE_X = (SCREEN_WIDTH - (EYE_WIDTH * 2 + EYE_GAP)) / 2;
const int RIGHT_EYE_X = LEFT_EYE_X + EYE_WIDTH + EYE_GAP;

// ---------- STATE MACHINE ----------
enum State {
  AUTO_BRIGHT_OFF,
  AUTO_DARK_ON,
  FORCE_ON,
  FORCE_OFF
};

State currentState = AUTO_BRIGHT_OFF;

// ---------- ANIMATION VARS ----------
unsigned long lastFrame = 0;
int blinkState = 0;
unsigned long nextBlinkTime = 0;
int eyelidHeight = 0;

// Pupil Movement
int pupilX = 0;
int pupilY = 0;
unsigned long nextGazeTime = 0;

// Hardware Timers
bool lastButtonState = HIGH;
bool buzzerActive = false;
unsigned long buzzerStart = 0;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  randomSeed(analogRead(A0));

  Serial.begin(9600);

  // Remember: If screen stays black, try changing 0x3C to 0x3D
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;)
      ;
  }

  display.clearDisplay();
  display.display();
}

void loop() {
  unsigned long now = millis();
  int ldrValue = analogRead(ldrPin);
  bool buttonState = digitalRead(buttonPin);



  // =========================================
  // 1. INPUT HANDLING (DEBOUNCED)
  // =========================================
  static unsigned long lastPress = 0;

  if (lastButtonState == HIGH && buttonState == LOW && now - lastPress > 200) {  // 200 ms debounce window

    lastPress = now;

    if (currentState == AUTO_DARK_ON || currentState == AUTO_BRIGHT_OFF)
      currentState = FORCE_ON;
    else if (currentState == FORCE_ON)
      currentState = FORCE_OFF;
    else
      currentState = AUTO_BRIGHT_OFF;

    buzz(now);
  }

  lastButtonState = buttonState;


  // =========================================
  // 2. SENSOR LOGIC
  // =========================================
  if (isAutoMode()) {
    if (currentState == AUTO_BRIGHT_OFF && ldrValue >= DARK_ON) {
      currentState = AUTO_DARK_ON;
      buzz(now);
    } else if (currentState == AUTO_DARK_ON && ldrValue <= BRIGHT_OFF) {
      currentState = AUTO_BRIGHT_OFF;
      buzz(now);
    }
  }

  // =========================================
  // 3. HARDWARE OUTPUT
  // =========================================
  bool isLightOn = (currentState == AUTO_DARK_ON || currentState == FORCE_ON);
  digitalWrite(ledPin, isLightOn);

  if (buzzerActive && now - buzzerStart >= 50) {
    digitalWrite(buzzerPin, LOW);
    buzzerActive = false;
  }

  // =========================================
  // 4. ANIMATION UPDATE (30 FPS)
  // =========================================
  if (now - lastFrame > 33) {
    updatePhysics(now);
    drawFace(ldrValue);
    lastFrame = now;
  }
}
bool isAutoMode() {
  return currentState == AUTO_BRIGHT_OFF || currentState == AUTO_DARK_ON;
}
void updatePhysics(unsigned long now) {
  // --- BLINK LOGIC ---
  if (now > nextBlinkTime && blinkState == 0) {
    blinkState = 1;
  }

  if (blinkState == 1) {  // Closing
    eyelidHeight += 6;
    if (eyelidHeight >= EYE_HEIGHT) {
      eyelidHeight = EYE_HEIGHT;
      blinkState = 2;
    }
  } else if (blinkState == 2) {  // Opening
    eyelidHeight -= 6;
    if (eyelidHeight <= 0) {
      eyelidHeight = 0;
      blinkState = 0;
      nextBlinkTime = now + random(2000, 5000);
    }
  }

  // --- GAZE LOGIC ---
  if (currentState == AUTO_DARK_ON || currentState == FORCE_ON) {
    if (now > nextGazeTime) {
      // Look further since eyes are bigger
      pupilX = random(-14, 15);
      pupilY = random(-8, 9);
      nextGazeTime = now + random(500, 2000);
    }
  } else {
    // Reset gaze when sleeping
    pupilX = 0;
    pupilY = 0;
  }
}
void drawFace(int ldrValue) {
  display.clearDisplay();

  // ============================================
  // 1. SPECIAL CASE: FORCE OFF ("- -" Look)
  // ============================================
  if (currentState == FORCE_OFF) {
    // Draw two horizontal lines (The sleeping "- -")
    // Left Line
    display.fillRect(LEFT_EYE_X + 5, EYE_Y + EYE_HEIGHT/2 - 2, EYE_WIDTH - 10, 6, SSD1306_WHITE);
    // Right Line
    display.fillRect(RIGHT_EYE_X + 5, EYE_Y + EYE_HEIGHT/2 - 2, EYE_WIDTH - 10, 6, SSD1306_WHITE);
    
    // Optional: Tiny "OFF" text at bottom center
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(SCREEN_WIDTH/2 - 9, SCREEN_HEIGHT - 8);
    display.print("OFF");

    display.display();
    return; // STOP here. Don't draw the rest.
  }

  // ============================================
  // 2. NORMAL MODES (Big Eyes / Auto / On)
  // ============================================
  
  int currentLid = eyelidHeight;

  // Day Mode (Lazy Eyes)
  if (currentState == AUTO_BRIGHT_OFF) {
    currentLid = max(eyelidHeight, EYE_HEIGHT / 2 + 2);
  }

  // Draw Eye Backgrounds (White)
  display.fillRoundRect(LEFT_EYE_X, EYE_Y, EYE_WIDTH, EYE_HEIGHT, EYE_RADIUS, SSD1306_WHITE);
  display.fillRoundRect(RIGHT_EYE_X, EYE_Y, EYE_WIDTH, EYE_HEIGHT, EYE_RADIUS, SSD1306_WHITE);

  // Draw Pupils (Black)
  int pX = constrain(pupilX, -15, 15);
  int pY = constrain(pupilY, -10, 10);
  int pR = (currentState == FORCE_ON) ? PUPIL_RADIUS + 3 : PUPIL_RADIUS; // Huge pupils if Manual ON

  display.fillCircle(LEFT_EYE_X + EYE_WIDTH / 2 + pX, EYE_Y + EYE_HEIGHT / 2 + pY, pR, SSD1306_BLACK);
  display.fillCircle(RIGHT_EYE_X + EYE_WIDTH / 2 + pX, EYE_Y + EYE_HEIGHT / 2 + pY, pR, SSD1306_BLACK);

  // Draw Eyelids (Black covering top)
  if (currentLid > 0) {
    display.fillRect(LEFT_EYE_X, EYE_Y, EYE_WIDTH, currentLid, SSD1306_BLACK);
    display.fillRect(RIGHT_EYE_X, EYE_Y, EYE_WIDTH, currentLid, SSD1306_BLACK);
  }

  // Status Dot (Manual ON)
  if (currentState == FORCE_ON) {
    display.fillCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 4, 2, SSD1306_WHITE);
  }

  // LDR Bar
  int barH = map(ldrValue, 0, 1023, 0, 16);
  display.drawFastVLine(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 20, 16, SSD1306_BLACK);
  display.drawFastVLine(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 4 - barH, barH, SSD1306_WHITE);

  display.display();
}
void buzz(unsigned long now) {
  digitalWrite(buzzerPin, HIGH);
  buzzerStart = now;
  buzzerActive = true;
}
