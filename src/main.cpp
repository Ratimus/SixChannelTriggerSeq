// These define's must be placed at the beginning before #include "TimerInterrupt.h"
// _TIMERINTERRUPT_LOGLEVEL_ from 0 to 4
// Don't define _TIMERINTERRUPT_LOGLEVEL_ > 0. Only for special ISR debugging only. Can hang the system.
#define TIMER_INTERRUPT_DEBUG         0
#define _TIMERINTERRUPT_LOGLEVEL_     0

#if ( defined(__AVR_ATmega644__) || defined(__AVR_ATmega644A__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644PA__)  || \
        defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO) || defined(ARDUINO_AVR_MINI) ||    defined(ARDUINO_AVR_ETHERNET) || \
        defined(ARDUINO_AVR_FIO) || defined(ARDUINO_AVR_BT)   || defined(ARDUINO_AVR_LILYPAD) || defined(ARDUINO_AVR_PRO)      || \
        defined(ARDUINO_AVR_NG) || defined(ARDUINO_AVR_UNO_WIFI_DEV_ED) || defined(ARDUINO_AVR_DUEMILANOVE) || defined(ARDUINO_AVR_FEATHER328P) || \
        defined(ARDUINO_AVR_METRO) || defined(ARDUINO_AVR_PROTRINKET5) || defined(ARDUINO_AVR_PROTRINKET3) || defined(ARDUINO_AVR_PROTRINKET5FTDI) || \
        defined(ARDUINO_AVR_PROTRINKET3FTDI) )
  #define USE_TIMER_1     true
  #warning Using Timer1
#else
  #define USE_TIMER_3     true
  #warning Using Timer3
#endif
#include "TimerInterrupt.h"

#include <Arduino.h>
#include<Wire.h>
#include<Adafruit_GFX.h>
#include<Adafruit_SSD1306.h>

#define OLED_ADDRESS 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#include <EEPROM.h>

#include "riddims.h"

#include "MagicButton.h"
#include "ClickEncoder.h"
#include "ClickEncoderInterface.h"

constexpr uint8_t ENC_A(3);
constexpr uint8_t ENC_B(2);
constexpr uint8_t CLOCK_PIN(6);
constexpr uint8_t ENC_SW(12);

const uint8_t ENC_SENSITIVITY       (1);
const bool    ENC_ACTIVE_LOW        (1);
const uint8_t ENC_STEPS_PER_NOTCH   (4);

constexpr uint8_t TIMER_INTERVAL_MS(1);

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

// uint8_t OUTPUT_PIN[]{5, 6, 7, 8, 9, 10};

MagicButton manualClock(CLOCK_PIN, 1, 0);

ClickEncoder encoder(ENC_A, ENC_B, ENC_SW, ENC_STEPS_PER_NOTCH, ENC_ACTIVE_LOW);
ClickEncoderInterface encoderInterface(encoder, ENC_SENSITIVITY);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void updateOled();

void savePatternToEeprom();
void loadPatternFromEeprom();

void changeModes();
void changeGenres();
void changeSetLength();
void changeLoopLength();

void encoderClick();
void encoderMove(bool up);
void encoderDoubleClick();

void reset();
void calcPattern();

void drawTrackLabels();
void drawTrackPatterns();
void drawModeOptions();
void drawStepIndicator();

void setHighlighted(bool inverted);

constexpr uint8_t OLED_INTERVAL_MS(100);
uint8_t oledTimer(0);
volatile bool oledTimerExpired(0);

constexpr uint8_t NUM_CHANNELS(6);
constexpr uint8_t CHANNEL_LABEL_NUM_CHARS(4);
constexpr char CHANNEL_LABELS[NUM_CHANNELS][CHANNEL_LABEL_NUM_CHARS]{"CH1", "CH2", "CH3", "CH4", "CH5", "CH6"};

constexpr uint8_t ROW_HEIGHT_NO_MARGIN(8);
constexpr uint8_t OLED_ROW_HEIGHT(ROW_HEIGHT_NO_MARGIN + 1);
constexpr uint8_t NUM_OLED_ROWS(SCREEN_HEIGHT / OLED_ROW_HEIGHT);

constexpr uint8_t COL_WIDTH_NO_MARGIN(6);
constexpr uint8_t OLED_COL_WIDTH(COL_WIDTH_NO_MARGIN + 1);
constexpr uint8_t NUM_OLED_COLS(SCREEN_WIDTH / OLED_COL_WIDTH);

constexpr uint8_t TRACK_STEP_X_VAL(CHANNEL_LABEL_NUM_CHARS * OLED_COL_WIDTH + 2);
constexpr uint8_t BOTTOM_ROW_Y_VAL(OLED_ROW_HEIGHT * NUM_OLED_ROWS);

constexpr uint8_t REP_COUNT_X_VAL(0);
constexpr uint8_t SWITCH_COUNT_X_VAL(70);
constexpr uint8_t GENRE_LABEL_X_VAL(36);
constexpr uint8_t FILL_LABEL_X_VAL(84);
constexpr uint8_t MODE_LABEL_X_VAL(0);
constexpr uint8_t RESET_LABEL_X_VAL(48);
constexpr uint8_t SAVE_LABEL_X_VAL(102);

uint8_t ROW_Y_VALS[NUM_OLED_ROWS];
uint8_t COL_X_VALS[NUM_OLED_COLS];

constexpr uint8_t NUM_GENRES(4);
constexpr char genreNames[NUM_GENRES][7]{"TECHNO", "DUBTCH", "HOUSE ", "SHIT  "};
uint8_t genreIdx(0);

uint8_t muteRegister(0);
uint16_t patternRegister[NUM_CHANNELS]{0x8888, 0x0808, 0xCCCC, 0x2222, 0xFFFF, 0x0000};

int8_t modeIdx(0);
uint8_t modePage(0);

bool AUTO_MODE(1);
bool STEP_EDIT_SUBMODE(0);

int8_t selStep(0);
int8_t selTrack(0);
uint8_t patternLength(16);

bool FILLS_ON(1);

uint8_t pattern_idx(0);
typedef word pPattern[8][12];
pPattern * usePattern;

int8_t loopLenIdx(0);
constexpr int8_t NUM_LOOP_LENGTHS(4);
constexpr int8_t LOOP_LENGTHS[NUM_LOOP_LENGTHS]{4, 8, 16, 32};

int8_t setLenIdx(0);
constexpr int8_t NUM_SET_LENGTHS(4);
constexpr int8_t SET_LENGTHS[NUM_SET_LENGTHS]{2, 4, 8, 16};

int8_t stepCounter(0);
uint8_t barCounter(0);
uint8_t loopCounter(0);
uint8_t setCounter(0);


void reset()
{
  stepCounter = 0;
  barCounter = 0;
  loopCounter = 0;
  setCounter = 0;

  calcPattern();
}


void timer1_ISR()
{
  encoder.service();
  manualClock.service();

  ++oledTimer;
  if (oledTimer >= OLED_INTERVAL_MS)
  {
    oledTimerExpired = true;
    oledTimer = 0;
  }
}


bool OLED_FLAG()
{
  bool ret(0);
  cli();
  if (oledTimerExpired)
  {
    oledTimerExpired = 0;
    ret = 1;
  }
  sei();
  return ret;
}


bool CLOCK_FLAG()
{
  return (manualClock.readAndFree() == ButtonState :: Clicked);
}


void testOLED()
{
  uint8_t val(0);
  display.clearDisplay();
  for (uint8_t row(0); row < NUM_OLED_ROWS; ++row)
  {
    for (uint8_t col(0); col < NUM_OLED_COLS; ++col)
    {
      display.setCursor(COL_X_VALS[col], ROW_Y_VALS[row]);
      display.print(char(val++));
      display.display();
      delay(66);
    }
  }
  delay(1500);
}


void setup()
{
  Serial.begin(9600);
  // OLED setting
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextSize(1);
  display.setTextColor(WHITE);

  updateOled();
  for (uint8_t row(0); row < NUM_OLED_ROWS; ++row)
  {
    ROW_Y_VALS[row] = row * OLED_ROW_HEIGHT;
  }

  for (uint8_t col(0); col < NUM_OLED_COLS; ++col)
  {
    COL_X_VALS[col] = col * OLED_COL_WIDTH;
  }
  // testOLED();
  ITimer1.init();

  // Using ATmega328 used in UNO => 16MHz CPU clock ,
  ITimer1.attachInterruptInterval(TIMER_INTERVAL_MS, timer1_ISR);
  reset();
}


void drawTrackLabels()
{
  for (uint8_t track(0); track < NUM_CHANNELS; ++track)
  {
    display.setCursor(0, ROW_Y_VALS[track]);
    setHighlighted((encMode >= mute0_MODE && encMode <= mute5_MODE) && (selTrack == track));
    display.print(CHANNEL_LABELS[track]);
  }
  display.setTextColor(WHITE);
}


void drawTrackPatterns()
{
  uint8_t xVal(0);
  uint8_t yVal(0);
  for (uint8_t track(0); track < NUM_CHANNELS; ++track)
  {
    bool editTrack(STEP_EDIT_SUBMODE && track == selTrack);
    bool mute(bitRead(muteRegister, track) && !editTrack);

    yVal = ROW_Y_VALS[track];
    for (uint8_t step(0); step < 16; ++step)
    {
      xVal = TRACK_STEP_X_VAL + step * COL_WIDTH_NO_MARGIN;
      display.setCursor(xVal, yVal);
      setHighlighted(editTrack && step == selStep);
      display.print(getStepChar(track, step, mute));
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
    display.setTextColor(WHITE);
    display.setCursor(REP_COUNT_X_VAL, BOTTOM_ROW_Y_VAL);

    display.print(" ");

    display.print("BAR:");
    display.print(barCounter + 1);
    display.print("/");
    setHighlighted(encMode == loopLength_MODE);
    display.print(LOOP_LENGTHS[loopLenIdx]);

    display.setTextColor(WHITE);
    display.setCursor(SWITCH_COUNT_X_VAL, BOTTOM_ROW_Y_VAL);
    display.print("LP:");
    display.print(loopCounter + 1);
    display.print("/");
    setHighlighted(encMode == setLength_MODE);
    display.print(SET_LENGTHS[setLenIdx]);
    return;
  }

  display.setCursor(MODE_LABEL_X_VAL, BOTTOM_ROW_Y_VAL);
  setHighlighted(encMode == selectMode_MODE);
  display.print(AUTO_MODE ? "AUTO" : "MNAL");

  if (AUTO_MODE)
  {
    display.setCursor(GENRE_LABEL_X_VAL, BOTTOM_ROW_Y_VAL);
    setHighlighted(encMode == selectGenre_MODE);
    display.print(genreNames[genreIdx]);

    display.setTextColor(WHITE);
    display.setCursor(FILL_LABEL_X_VAL, BOTTOM_ROW_Y_VAL);
    display.print("FilIN:");
    setHighlighted(encMode == toggleFills_MODE);
    display.print(FILLS_ON ? "Y" : "N");
    return;
  }

  display.setCursor(RESET_LABEL_X_VAL, BOTTOM_ROW_Y_VAL);
  setHighlighted(encMode == resetPattern_MODE);
  display.print("RESET");

  display.setCursor(SAVE_LABEL_X_VAL, BOTTOM_ROW_Y_VAL);
  setHighlighted(encMode == saveData_MODE);
  display.print("SAVE");
}


void drawStepIndicator()
{
  uint8_t xVal(TRACK_STEP_X_VAL + stepCounter * COL_WIDTH_NO_MARGIN);
  uint8_t yVal(0);
  uint8_t width(COL_WIDTH_NO_MARGIN);
  uint8_t height(NUM_CHANNELS * OLED_ROW_HEIGHT - 1);
  display.drawRect(xVal, yVal, width, height, WHITE);
}

char getStepChar(uint8_t track, uint8_t step, bool mute)
{
  if (mute)
  {
    return ' ';
  }

  if (bitRead(patternRegister[track], 15 - step))
  {
    return '*';
  }

  return '_';
}


void getPattern()
{
  switch (genreIdx)
  {
    case 0:
      usePattern = (pPattern*)(&bnk1_ptn);
      break;
    case 1:
      usePattern = (pPattern*)(&bnk2_ptn);
      break;
    case 2:
      usePattern = (pPattern*)(&bnk3_ptn);
      break;
  }

  for (uint8_t ch(0); ch < 6; ++ch)
  {
    patternRegister[ch] = pgm_read_word((*usePattern)[pattern_idx][ch + 6]);
  }
}

void newPattern()
{
  switch (genreIdx)
  {
    case 0:
      pattern_idx = random(0, 7);
      break;
    case 1:
      pattern_idx = random(0, 4);
      break;
    case 2:
      pattern_idx = random(0, 4);
      break;
  }

  getPattern();
}

void getFill()
{
  switch (genreIdx)
  {
    case 0:
      usePattern = (pPattern*)(&bnk1_ptn);
      break;
    case 1:
      usePattern = (pPattern*)(&bnk2_ptn);
      break;
    case 2:
      usePattern = (pPattern*)(&bnk3_ptn);
      break;
  }

  for (uint8_t ch(0); ch < 6; ++ch)
  {
    patternRegister[ch] = pgm_read_word((*usePattern)[pattern_idx][ch]);
  }
}


bool NEW_BAR(0);
bool NEW_LOOP(0);
bool NEW_SET(0);
bool FILL_STEP(0);

void calcPattern()
{
  FILL_STEP = 0;
  NEW_BAR = (stepCounter == 0);
  if (NEW_BAR)
  {
    Serial.println("New bar");
    if (barCounter >= LOOP_LENGTHS[loopLenIdx])
    {
      ++loopCounter;
      barCounter = 0;
    }
    else if (barCounter == LOOP_LENGTHS[loopLenIdx] - 1)
    {
      FILL_STEP = 1;
    }
  }

  NEW_LOOP = NEW_BAR && (barCounter == 0);
  if (NEW_LOOP)
  {
    Serial.println("New loop");
    if (loopCounter >= SET_LENGTHS[setLenIdx])
    {
      loopCounter = 0;
    }
  }

  NEW_SET = NEW_LOOP && (loopCounter == 0);

  if (NEW_SET)
  {
    // Serial.println("New set");
    newPattern();
  }
  else if (FILL_STEP)
  {
    // Serial.println("Last bar - play fill");
    getFill();
  }
  else if (NEW_BAR)
  {
    // Serial.println("Same ol' pattern");
    getPattern();
  }
}


void seqStep()
{
  Serial.println(stepCounter);

  ++stepCounter;
  if (stepCounter >= 16)
  {
    stepCounter = 0;
    ++barCounter;
  }

  calcPattern();
}


void writeOutputVals(uint8_t outVal = 0)
{
  for (uint8_t ch(0); ch < NUM_CHANNELS; ++ch)
  {
    bitRead(outVal, ch);
  }
}


void updateOutputs()
{
  uint8_t outVal(0);
  for (uint8_t ch(0); ch < NUM_CHANNELS; ++ch)
  {
    if bitRead(muteRegister, stepCounter)
    {
      continue;
    }

    if (!bitRead(patternRegister[ch], stepCounter))
    {
      continue;
    }

    bitWrite(outVal, ch, 1);
  }
}


void doNav()
{
  switch(encoderInterface.getEvent())
  {
    case encEvnts::Left:
      encoderMove(0);
      break;

    case encEvnts::ShiftLeft:
      break;

    case encEvnts::Right:
      encoderMove(1);
      break;

    case encEvnts::ShiftRight:
      break;

    case encEvnts::Click:
      encoderClick();
      break;

    case encEvnts::DblClick:
      encoderDoubleClick();
      break;

    case encEvnts::Hold:
    case encEvnts::Press:
    case encEvnts::ClickHold:
    case encEvnts::NUM_ENC_EVNTS:
    default:
      break;
  }
}


void loop()
{
  if (CLOCK_FLAG())
  {
    seqStep();
    updateOutputs();
  }

  doNav();

  if (OLED_FLAG())
  {
    updateOled();
  }
}


void setHighlighted(bool inverted)
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
  if (AUTO_MODE)
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

  if (STEP_EDIT_SUBMODE)
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
  for (uint8_t ch(0); ch < 6; ++ch)
  {
    EEPROM.update(2 * ch + 1,  highByte(patternRegister[ch]));
    EEPROM.update(2 * ch + 2,   lowByte(patternRegister[ch]));
  }
}


void loadPatternFromEeprom()
{
  for (uint8_t ch(0); ch < 6; ++ch)
  {
    uint8_t idx(2 * ch);
    patternRegister[ch] = (EEPROM.read(idx + 1) << 8) + EEPROM.read(idx + 2);
  }
}


void changeModes()
{
  if (AUTO_MODE)
  {
    savePatternToEeprom();
    newPattern();
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


void updateOled()
{
  display.clearDisplay();
  drawTrackLabels();
  drawTrackPatterns();
  drawModeOptions();
  drawStepIndicator();
  display.display();
}


void encoderClick()
{
  if (STEP_EDIT_SUBMODE)
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
      AUTO_MODE = !AUTO_MODE;
      changeModes();
      break;

    case selectGenre_MODE:
      changeGenres();
      break;

    case toggleFills_MODE:
      FILLS_ON = !FILLS_ON;
      break;

    case loopLength_MODE:
      changeLoopLength();
      break;

    case setLength_MODE:
      changeSetLength();
      break;

    case saveData_MODE:
    case resetPattern_MODE:
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
      STEP_EDIT_SUBMODE = !STEP_EDIT_SUBMODE;
      break;

    default:
      break;
  }
}
