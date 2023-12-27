// These define's must be placed at the beginning before #include "TimerInterrupt.h"
// _TIMERINTERRUPT_LOGLEVEL_ from 0 to 4
// Don't define _TIMERINTERRUPT_LOGLEVEL_ > 0. Only for special ISR debugging only. Can hang the system.
#define TIMER_INTERRUPT_DEBUG         0
#define _TIMERINTERRUPT_LOGLEVEL_     0
#define USE_TIMER_1     true

// #if ( defined(__AVR_ATmega644__)   || defined(__AVR_ATmega644A__)          || defined(__AVR_ATmega644P__)      || defined(__AVR_ATmega644PA__)         || \
//         defined(ARDUINO_AVR_UNO)   || defined(ARDUINO_AVR_NANO)            || defined(ARDUINO_AVR_MINI)        || defined(ARDUINO_AVR_ETHERNET)        || \
//         defined(ARDUINO_AVR_FIO)   || defined(ARDUINO_AVR_BT)              || defined(ARDUINO_AVR_LILYPAD)     || defined(ARDUINO_AVR_PRO)             || \
//         defined(ARDUINO_AVR_NG)    || defined(ARDUINO_AVR_UNO_WIFI_DEV_ED) || defined(ARDUINO_AVR_DUEMILANOVE) || defined(ARDUINO_AVR_FEATHER328P)     || \
//         defined(ARDUINO_AVR_METRO) || defined(ARDUINO_AVR_PROTRINKET5)     || defined(ARDUINO_AVR_PROTRINKET3) || defined(ARDUINO_AVR_PROTRINKET5FTDI) || \
//         defined(ARDUINO_AVR_PROTRINKET3FTDI) )
//   #define USE_TIMER_1     true
//   #warning Using Timer1
// #else
//   #define USE_TIMER_3     true
//   #warning Using Timer3
// #endif
#include "TimerInterrupt.h"

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_ADDRESS 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#include <EEPROM.h>

#include "riddims.h"
#include "FastShiftOut.h"

#include "MagicButton.h"
#include "ClickEncoder.h"
#include "ClickEncoderInterface.h"

constexpr uint8_t ENC_A             (2);
constexpr uint8_t ENC_B             (3);
constexpr uint8_t ENC_SW            (4);
constexpr uint8_t CLOCK_PIN         (11);
constexpr uint8_t RESET_PIN         (12);

constexpr uint8_t SR_CLK            (5);
constexpr uint8_t SR_LCH            (6);
constexpr uint8_t SR_DAT            (8);

constexpr uint8_t CTRL              (A7);
const uint8_t ENC_SENSITIVITY       (1);
const bool    ENC_ACTIVE_LOW        (1);
const uint8_t ENC_STEPS_PER_NOTCH   (4);

constexpr uint8_t TRIGGER_LENGTH(10);
constexpr uint8_t TIMER_INTERVAL_MS (1);
constexpr uint8_t OUTPUT_MAP[]{5, 0, 4, 7, 3, 6, 1, 2};
uint8_t           outputRegister(0);

enum modeType
{
  mute0_MODE,
  mute1_MODE,
  mute2_MODE,
  mute3_MODE,
  mute4_MODE,
  mute5_MODE,
  edit0_MODE,
  edit1_MODE,
  edit2_MODE,
  edit3_MODE,
  edit4_MODE,
  edit5_MODE,
  selectMode_MODE,
  selectGenre_MODE,
  toggleFills_MODE,
  loopLength_MODE,
  setLength_MODE,
  saveData_MODE,
  resetPattern_MODE
} encMode;

const uint8_t NUM_AUTO_MODES(11);
modeType AUTO_MODES[NUM_AUTO_MODES]{
  mute0_MODE,
  mute1_MODE,
  mute2_MODE,
  mute3_MODE,
  mute4_MODE,
  mute5_MODE,
  selectMode_MODE,
  selectGenre_MODE,
  toggleFills_MODE,
  loopLength_MODE,
  setLength_MODE
};

const uint8_t NUM_MANUAL_MODES(15);
modeType MANUAL_MODES[NUM_MANUAL_MODES]{
  mute0_MODE,
  mute1_MODE,
  mute2_MODE,
  mute3_MODE,
  mute4_MODE,
  mute5_MODE,
  edit0_MODE,
  edit1_MODE,
  edit2_MODE,
  edit3_MODE,
  edit4_MODE,
  edit5_MODE,
  selectMode_MODE,
  saveData_MODE,
  resetPattern_MODE
};

const uint8_t MODE_SELECT_LOCATION[2]{6, 12};

FastShiftOut outputs(SR_DAT, SR_CLK, MSBFIRST);

// #define CASSIDEBUG

bool newPatternFlag(0);

volatile bool _clockFlag(0);
volatile bool _resetFlag(0);
volatile bool clockPulse[]{0, 0};
volatile bool resetPulse[]{0, 0};

ClickEncoder encoder(ENC_A, ENC_B, ENC_SW, ENC_STEPS_PER_NOTCH, ENC_ACTIVE_LOW);
ClickEncoderInterface encoderInterface(encoder, ENC_SENSITIVITY);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

bool CLOCK_FLAG();
bool RESET_FLAG();
bool OUTPUTS_EXPIRED();

uint8_t getOutputVal();
void writeOutputRegister(uint8_t outval = 0);

void doNav();
void seqStep();
void oled_redraw();

void savePatternToEeprom();
void loadPatternFromEeprom();

void changeModes();
void changeGenres();
void changeSetLength();
void changeLoopLength();
void loadNextPattern();

void encoderClick();
void encoderMove(bool up);
void encoderDoubleClick();

void decideNextPattern();
void calculateNextStep();
void resetSeqPattern();

void drawTrackLabels();
void drawTrackPatterns();
void drawModeOptions();
void drawStepIndicator();

void highlightTextIf(bool inverted);
void getStepChar(uint8_t track, uint8_t step, bool mute, char * buf);

constexpr uint8_t OLED_INTERVAL_MS(200);

constexpr uint8_t NUM_CHANNELS(6);

constexpr uint8_t ROW_HEIGHT_NO_MARGIN(8);
constexpr uint8_t OLED_ROW_HEIGHT(ROW_HEIGHT_NO_MARGIN + 1);
constexpr uint8_t NUM_OLED_ROWS(SCREEN_HEIGHT / OLED_ROW_HEIGHT);

constexpr uint8_t COL_WIDTH_NO_MARGIN(6);
constexpr uint8_t OLED_COL_WIDTH(COL_WIDTH_NO_MARGIN + 1);
constexpr uint8_t NUM_OLED_COLS(SCREEN_WIDTH / OLED_COL_WIDTH);
constexpr uint8_t CHANNEL_LABEL_NUM_CHARS(4);
constexpr uint8_t TRACK_STEP_X_VAL(CHANNEL_LABEL_NUM_CHARS * OLED_COL_WIDTH + 2);
constexpr uint8_t BOTTOM_ROW_Y_VAL(OLED_ROW_HEIGHT * (NUM_OLED_ROWS - 1));

constexpr uint8_t REP_COUNT_X_VAL(0);
constexpr uint8_t SWITCH_COUNT_X_VAL(82);
constexpr uint8_t GENRE_LABEL_X_VAL(30);
constexpr uint8_t FILL_LABEL_X_VAL(90);
constexpr uint8_t MODE_LABEL_X_VAL(0);
constexpr uint8_t RESET_LABEL_X_VAL(96);
constexpr uint8_t SAVE_LABEL_X_VAL(48);

uint8_t ROW_Y_VALS[NUM_OLED_ROWS];
uint8_t COL_X_VALS[NUM_OLED_COLS];

int8_t            outputOnTime(-1);
bool              outputsExpired(0);

uint8_t           genreIdx(0);
constexpr uint8_t NUM_GENRES(4);
bool              newGenreFlag(0);
constexpr char    genreNames[NUM_GENRES][7]{"TECHNO", "DUBTEK", "HOUSE ", "SHIT  "};

uint8_t  muteRegister(0);
uint16_t patternRegister[NUM_CHANNELS]{0, 0, 0, 0, 0, 0};
uint16_t variationRegister[NUM_CHANNELS]{0, 0, 0, 0, 0, 0};
uint16_t oledTrackRegister[NUM_CHANNELS]{0, 0, 0, 0, 0, 0};
uint16_t *pCurrentPattern(patternRegister);

int8_t  modeIdx(0);
uint8_t modePage(0);

bool autoMode(1);
bool stepEditSubmode(0);

int8_t  selStep(0);
int8_t  selTrack(0);
uint8_t patternLength(16);

bool fillsEnabled(1);
bool playFillThisBar(0);

int8_t patternSelectIdx(-1);
int8_t nextPattern(0);

int8_t           loopLenIdx(0);
constexpr int8_t NUM_LOOP_LENGTHS(4);
constexpr int8_t LOOP_LENGTHS[NUM_LOOP_LENGTHS]{4, 8, 16, 32};

int8_t           setLenIdx(0);
constexpr int8_t NUM_SET_LENGTHS(4);
constexpr int8_t SET_LENGTHS[NUM_SET_LENGTHS]{2, 4, 8, 16};

int8_t  currentStep(-1);
int8_t  nextStep(0);

uint8_t barCounter(0);
uint8_t loopCounter(0);
uint8_t setCounter(0);

bool resetBar(0);
bool resetLoop(0);
bool newSetStarted(0);
bool lastBarOfLoop(0);

/*
Weird stuff:

Fills are NOT calculated correctly
Pattern changes on Cycle Reset are NOT calculated correctly
Genre changes on Cycle Reset are NOT calculated correctly
Pattern Select CTRL CV has no effect
*/
void loadPattern(uint8_t patternIdx);


void timer1_ISR()
{
  clockPulse[1] = clockPulse[0];
  clockPulse[0] = !digitalRead(CLOCK_PIN);
  if (clockPulse[0] && !clockPulse[1])
  {
    writeOutputRegister(0);
    currentStep = nextStep;
    writeOutputRegister(getOutputVal());
    outputOnTime = TRIGGER_LENGTH;
    _clockFlag = 1;
  }

  resetPulse[1] = resetPulse[0];
  resetPulse[0] = !digitalRead(RESET_PIN);
  if (resetPulse[0] && !resetPulse[1])
  {
    _resetFlag = 1;
  }

  if (outputOnTime >= 0)
  {
    --outputOnTime;
  }

  if (outputOnTime < 0)
  {
    outputsExpired = 1;
  }

  encoder.service();
}


void setup()
{
#ifdef CASSIDEBUG
  Serial.begin(115200);
  delay(100);
#endif

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextSize(1);
  display.setTextColor(WHITE);

  delay(100);
  pinMode(SR_LCH, OUTPUT);
  writeOutputRegister(0x00);
  pinMode(CTRL, INPUT);

  pinMode(RESET_PIN, INPUT_PULLUP);
  pinMode(CLOCK_PIN, INPUT_PULLUP);

  for (uint8_t row(0); row < NUM_OLED_ROWS; ++row)
  {
    ROW_Y_VALS[row] = row * OLED_ROW_HEIGHT;
  }

  for (uint8_t col(0); col < NUM_OLED_COLS; ++col)
  {
    COL_X_VALS[col] = col * OLED_COL_WIDTH;
  }

  resetSeqPattern();

  ITimer1.init();
  ITimer1.attachInterruptInterval(TIMER_INTERVAL_MS, timer1_ISR);
}


void loop()
{
  if (OUTPUTS_EXPIRED())
  {
    writeOutputRegister(0);
  }

  if (RESET_FLAG())
  {
    resetSeqPattern();
  }

  if (CLOCK_FLAG())
  {
    seqStep();
  }

  doNav();
  decideNextPattern();
  if ((nextStep == -1) && (newGenreFlag || newPatternFlag))
  {
    calculateNextStep();
  }
}


void resetSeqPattern()
{
  patternSelectIdx = -1;
  currentStep = -1;
  nextPattern = 0;
  nextStep = 0;
  barCounter = 0;
  loopCounter = 0;
  setCounter = 0;
  newPatternFlag = true;
}


bool OUTPUTS_EXPIRED()
{
  bool ret(0);
  if (outputsExpired)
  {
    outputsExpired = 0;
    ret = 1;
  }
  return ret;
}


bool CLOCK_FLAG()
{
  bool ret(0);
  if (_clockFlag)
  {
    ret = 1;
    _clockFlag = 0;
  }
  return ret;
}


bool RESET_FLAG()
{
  bool ret(0);
  if (_resetFlag)
  {
    ret = 1;
    _resetFlag = 0;
  }
  return ret;
}


void testOLED()
{
  uint8_t printChar(0);
  display.clearDisplay();
  for (uint8_t row(0); row < NUM_OLED_ROWS; ++row)
  {
    for (uint8_t col(0); col < NUM_OLED_COLS; ++col)
    {
      display.setCursor(COL_X_VALS[col], ROW_Y_VALS[row]);
      display.print(char(printChar++));
      display.display();
      delay(66);
    }
  }
  delay(1500);
}


void writeOutputRegister(uint8_t regval)
{
  digitalWrite(SR_LCH, LOW);
  outputs.write(regval);
  digitalWrite(SR_LCH, HIGH);
}


void highlightTextIf(bool inverted)
{
  if (inverted)
  {
    display.setTextColor(BLACK, WHITE);
  }
  else
  {
    display.setTextColor(WHITE);
  }
}


void drawTrackLabels()
{
  for (uint8_t track(0); track < NUM_CHANNELS; ++track)
  {
    display.setCursor(0, ROW_Y_VALS[track]);
    highlightTextIf((encMode >= mute0_MODE && encMode <= mute5_MODE) && (selTrack == track));
    display.print("CH");
    display.print(track + 1);
  }
  display.setCursor(0, 0);
}


void drawTrackPatterns()
{
  uint8_t xVal(0);
  uint8_t yVal(0);
  for (uint8_t track(0); track < NUM_CHANNELS; ++track)
  {
    bool editTrack(stepEditSubmode && track == selTrack);
    bool mute(bitRead(muteRegister, track) && !editTrack);

    yVal = ROW_Y_VALS[track];
    for (uint8_t step(0); step < patternLength; ++step)
    {
      xVal = TRACK_STEP_X_VAL + step * COL_WIDTH_NO_MARGIN;
      display.setCursor(xVal, yVal);
      highlightTextIf(editTrack && step == selStep);
      char val[2];
      getStepChar(track, step, mute, val);
      display.print(val);
    }

    if (encMode != edit0_MODE &&
        encMode != edit1_MODE &&
        encMode != edit2_MODE &&
        encMode != edit3_MODE &&
        encMode != edit4_MODE &&
        encMode != edit5_MODE)
    {
      continue;
    }

    if (selTrack != track)
    {
      continue;
    }

    display.setTextColor(WHITE);
    display.setCursor(TRACK_STEP_X_VAL - OLED_COL_WIDTH, yVal);
    display.print(char(16));
  }
}


void drawModeOptions()
{
  if (modePage)
  {
    display.setCursor(REP_COUNT_X_VAL, BOTTOM_ROW_Y_VAL);
    display.setTextColor(WHITE);
    display.print("BAR:");
    display.print(barCounter + 1);
    display.print("/");
    highlightTextIf(encMode == loopLength_MODE);
    display.print(LOOP_LENGTHS[loopLenIdx]);

    display.setTextColor(WHITE);
    display.setCursor(SWITCH_COUNT_X_VAL, BOTTOM_ROW_Y_VAL);
    display.print("LP:");
    display.print(loopCounter + 1);
    display.print("/");
    highlightTextIf(encMode == setLength_MODE);
    uint8_t len(SET_LENGTHS[setLenIdx]);
    display.print(len);
    display.setTextColor(BLACK);
    return;
  }

  display.setCursor(MODE_LABEL_X_VAL, BOTTOM_ROW_Y_VAL);
  highlightTextIf(encMode == selectMode_MODE);
  display.print(autoMode ? "AUTO" : "MNAL");

  if (autoMode)
  {
    display.setCursor(GENRE_LABEL_X_VAL, BOTTOM_ROW_Y_VAL);
    highlightTextIf(encMode == selectGenre_MODE);
    if (newGenreFlag)
    {
      display.print(char(26));
      display.print(genreNames[genreIdx]);
      display.print(":");
      display.print(nextPattern + 1);
    }
    else
    {
      display.print(genreNames[genreIdx]);
      if (nextPattern != patternSelectIdx)
      {
        display.print(char(26));
        display.print(nextPattern + 1);
      }
      else
      {
        display.print(":");
        display.print(patternSelectIdx + 1);
      }
    }

    display.setTextColor(WHITE);
    display.setCursor(FILL_LABEL_X_VAL, BOTTOM_ROW_Y_VAL);
    highlightTextIf(playFillThisBar);
    display.print("Fill:");
    highlightTextIf(encMode == toggleFills_MODE);
    display.print(fillsEnabled ? "Y" : "N");
    display.setTextColor(BLACK);

    return;
  }

  display.setCursor(RESET_LABEL_X_VAL, BOTTOM_ROW_Y_VAL);
  highlightTextIf(encMode == resetPattern_MODE);
  display.print("RESET");

  display.setCursor(SAVE_LABEL_X_VAL, BOTTOM_ROW_Y_VAL);
  highlightTextIf(encMode == saveData_MODE);
  display.print("SAVE");
}


void drawStepIndicator()
{
  if (currentStep == -1)
  {
    return;
  }
  uint8_t xVal(TRACK_STEP_X_VAL + currentStep * COL_WIDTH_NO_MARGIN);
  uint8_t yVal(0);
  uint8_t width(COL_WIDTH_NO_MARGIN);
  uint8_t height(NUM_CHANNELS * OLED_ROW_HEIGHT - 1);
  display.drawRect(xVal, yVal, width, height, WHITE);
}


const char space[2] = " ";
const char splat[2] = "*";
const char under[2] = "_";
void getStepChar(uint8_t track, uint8_t step, bool mute, char * buf)
{
  if (mute)
  {
    memcpy(buf, &space, 2);
    return;
  }

  if (bitRead(oledTrackRegister[track], 15 - step))
  {
    memcpy(buf, &splat, 2);
    return;
  }

  memcpy(buf, &under, 2);
}


void seqStep()
{
  oled_redraw();
  ++nextStep;
  calculateNextStep();
}


uint8_t getOutputVal()
{
  uint8_t outVal(0);
  for (uint8_t ch(0); ch < NUM_CHANNELS; ++ch)
  {
    if bitRead(muteRegister, currentStep)
    {
      continue;
    }

    if (!bitRead(*(pCurrentPattern + ch), currentStep))
    {
      continue;
    }

    bitWrite(outVal, OUTPUT_MAP[ch], 1);
  }
  return outVal;
}


void loadPattern(uint8_t patternIdx)
{
  for (uint8_t ch(0); ch < 6; ++ch)
  {
    switch (genreIdx)
    {
      case 0:
        patternRegister[ch] = pgm_read_word(bnk1_ptn[patternIdx] + ch);
        variationRegister[ch] = pgm_read_word(bnk1_ptn[patternIdx] + ch + 6);
        break;

      case 1:
        patternRegister[ch] = pgm_read_word(bnk2_ptn[patternIdx] + ch);
        variationRegister[ch] = pgm_read_word(bnk2_ptn[patternIdx] + ch + 6);
        break;

      case 2:
        patternRegister[ch] = pgm_read_word(bnk3_ptn[patternIdx] + ch);
        variationRegister[ch] = pgm_read_word(bnk2_ptn[patternIdx] + ch + 6);
        break;

      case 3:
        patternRegister[ch] = pgm_read_word(bnk4_ptn[patternIdx] + ch);
        variationRegister[ch] = pgm_read_word(bnk2_ptn[patternIdx] + ch + 6);
        break;

      default:
        break;
    }
  }
}


void decideNextPattern()
{
  int16_t cv(analogRead(CTRL));
  uint8_t maxIdx(4);
  if (genreIdx == 0 || genreIdx == 3)
  {
    maxIdx += 3;
  }

  uint8_t cvPattern(map(cv, 0, 704, 0, maxIdx + 1));
  if (cvPattern > maxIdx)
  {
    cvPattern = maxIdx;
  }

  if (cvPattern != nextPattern)
  {
    nextPattern = cvPattern;
    if (currentStep != -1)
    {
      oled_redraw();
    }
  }

  if (nextPattern != patternSelectIdx)
  {
    newPatternFlag = true;
  }
  else if (!newGenreFlag)
  {
    return;
  }

  if (currentStep != -1)
  {
    oled_redraw();
    return;
  }

  loadNextPattern();
  calculateNextStep();
  oled_redraw();
}


void calculateNextStep()
{
  if (nextStep >= patternLength)
  {
    nextStep = 0;
    ++barCounter;

    if (barCounter == LOOP_LENGTHS[loopLenIdx] - 1)
    {
      if (fillsEnabled)
      {
        playFillThisBar = true;
        pCurrentPattern = variationRegister;
        for (uint8_t ch(0); ch < NUM_CHANNELS; ++ch)
        {
          oledTrackRegister[ch] = *(pCurrentPattern + ch);
        }
      }
      return;
    }

    if (barCounter == LOOP_LENGTHS[loopLenIdx])
    {
      ++loopCounter;
      barCounter = 0;
    }
  }

  if (nextStep != 0)
  {
    return;
  }

  if (loopCounter == SET_LENGTHS[setLenIdx])
  {
    loopCounter = 0;
    loadNextPattern();
  }

  pCurrentPattern = patternRegister;
  playFillThisBar = false;

  for (uint8_t ch(0); ch < NUM_CHANNELS; ++ch)
  {
    oledTrackRegister[ch] = *(pCurrentPattern + ch);
  }
}


void loadNextPattern()
{
  patternSelectIdx = nextPattern;
  newPatternFlag = false;
  newGenreFlag = false;
  loadPattern(patternSelectIdx);
#ifdef CASSIDEBUG
  Serial.print(F("using bank "));Serial.print(patternSelectIdx + 1);Serial.println(F(" of 8"));
#endif
}


void doNav()
{
  switch(encoderInterface.getEvent())
  {
    case encEvnts::Left:
      encoderMove(0);
      break;

    case encEvnts::Right:
      encoderMove(1);
      break;

    case encEvnts::Click:
      encoderClick();
      break;

    case encEvnts::DblClick:
      encoderDoubleClick();
      break;

    case encEvnts::ShiftRight:
    case encEvnts::ShiftLeft:
    case encEvnts::Hold:
    case encEvnts::Press:
    case encEvnts::ClickHold:
    case encEvnts::NUM_ENC_EVNTS:
    default:
      return;
  }
  oled_redraw();
}


void setModePage()
{
  switch(encMode)
  {
    case selectMode_MODE:
    case selectGenre_MODE:
    case toggleFills_MODE:
    case resetPattern_MODE:
    case saveData_MODE:
      modePage = 0;
      break;

    case loopLength_MODE:
    case setLength_MODE:
      modePage = 1;
      break;

    default:
      break;
  }
}


void updateSelection()
{
  switch(encMode)
  {
    case mute0_MODE:
    case edit0_MODE:
      selTrack = 0;
      break;

    case mute1_MODE:
    case edit1_MODE:
      selTrack = 1;
      break;

    case mute2_MODE:
    case edit2_MODE:
      selTrack = 2;
      break;

    case mute3_MODE:
    case edit3_MODE:
      selTrack = 3;
      break;

    case mute4_MODE:
    case edit4_MODE:
      selTrack = 4;
      break;

    case mute5_MODE:
    case edit5_MODE:
      selTrack = 5;
      break;

    default:
      break;
  }

  setModePage();
}


void encoderMove(bool up)
{
  if (autoMode)
  {
    modeIdx += up ? 1 : -1;
    if (modeIdx < 0)
    {
      modeIdx = NUM_AUTO_MODES - 1;
    }
    else if (modeIdx >= NUM_AUTO_MODES)
    {
      modeIdx = 0;
    }

    encMode = AUTO_MODES[modeIdx];
    updateSelection();
    return;
  }

  if (stepEditSubmode)
  {
    selStep  += up ? 1 : -1;
    if (selStep >= patternLength)
    {
      selStep = 0;
    }
    else if (selStep < 0)
    {
      selStep = patternLength - 1;
    }
    return;
  }

  modeIdx += up ? 1 : -1;
  if (modeIdx < 0)
  {
    modeIdx = NUM_MANUAL_MODES - 1;
  }
  else if (modeIdx >= NUM_MANUAL_MODES)
  {
    modeIdx = 0;
  }

  encMode = MANUAL_MODES[modeIdx];
  updateSelection();
}


void savePatternToEeprom()
{
  uint8_t writeIdx(0);
  for (uint8_t ch(0); ch < 6; ++ch)
  {
    EEPROM.update(writeIdx++,  highByte(patternRegister[ch]));
    EEPROM.update(writeIdx++,   lowByte(patternRegister[ch]));
  }
}


void loadPatternFromEeprom()
{
  uint8_t readIdx(0);
  for (uint8_t ch(0); ch < NUM_CHANNELS; ++ch)
  {
    patternRegister[ch] = EEPROM.read(readIdx++);
    patternRegister[ch] <<= 8;
    patternRegister[ch] |= EEPROM.read(readIdx++);
  }
}


void changeModes()
{
  if (autoMode)
  {
    savePatternToEeprom();
    loadNextPattern();
    modeIdx = MODE_SELECT_LOCATION[0];
  }
  else
  {
    loadPatternFromEeprom();
    modeIdx = MODE_SELECT_LOCATION[1];
  }
}


void changeGenres()
{
  ++genreIdx;
  if (genreIdx >= NUM_GENRES)
  {
    genreIdx = 0;
  }
  newGenreFlag = 1;

  if (currentStep != -1)
  {
    return;
  }

  calculateNextStep();
  oled_redraw();
}


void changeSetLength()
{
  ++setLenIdx;
  if (setLenIdx >= NUM_SET_LENGTHS)
  {
    setLenIdx = 0;
    loopCounter %= SET_LENGTHS[setLenIdx];
  }
}


void changeLoopLength()
{
  ++loopLenIdx;
  if (loopLenIdx >= NUM_LOOP_LENGTHS)
  {
    loopLenIdx = 0;
    barCounter %= LOOP_LENGTHS[loopLenIdx];
  }
}


void oled_redraw()
{
  display.clearDisplay();
  drawTrackLabels();
  drawTrackPatterns();
  drawStepIndicator();
  drawModeOptions();
  display.setCursor(0, 0);
  display.display();
}


void encoderClick()
{
  if (stepEditSubmode)
  {
    bitToggle(patternRegister[selTrack], 15 - selStep);
    return;
  }

  switch (encMode)
  {
    case mute0_MODE:
    case mute1_MODE:
    case mute2_MODE:
    case mute3_MODE:
    case mute4_MODE:
    case mute5_MODE:
      bitToggle(muteRegister, selTrack);
      break;

    case edit0_MODE:
    case edit1_MODE:
    case edit2_MODE:
    case edit3_MODE:
    case edit4_MODE:
    case edit5_MODE:
      break;

    case selectMode_MODE:
      autoMode = !autoMode;
      changeModes();
      break;

    case selectGenre_MODE:
      changeGenres();
      break;

    case toggleFills_MODE:
      fillsEnabled = !fillsEnabled;
      break;

    case loopLength_MODE:
      changeLoopLength();
      break;

    case setLength_MODE:
      changeSetLength();
      break;

    case saveData_MODE:
      savePatternToEeprom();
      break;

    case resetPattern_MODE:
      resetSeqPattern();
      break;

    default:
      break;
  }
}


void encoderDoubleClick()
{
  switch (encMode)
  {
    case edit0_MODE:
    case edit1_MODE:
    case edit2_MODE:
    case edit3_MODE:
    case edit4_MODE:
    case edit5_MODE:
      stepEditSubmode = !stepEditSubmode;
      break;

    default:
      break;
  }
}
