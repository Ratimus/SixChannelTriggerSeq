#include "Arduino.h"
#include "globals.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

uint8_t           outputRegister(0);
int8_t            outputOnTime(-1);
bool              outputsExpired(0);
uint8_t           patternLength(16);
bool              playFillThisBar(0);

int8_t            currentStep(-1);
int8_t            nextStep(0);

uint8_t           barCounter(0);
uint8_t           loopCounter(0);
uint8_t           setCounter(0);


extern Adafruit_SSD1306 display;

extern uint8_t           outputRegister;
extern int8_t            outputOnTime;
extern bool              outputsExpired;
extern uint8_t           patternLength;
extern bool              playFillThisBar;

extern int8_t            currentStep;
extern int8_t            nextStep;

extern uint8_t           barCounter;
extern uint8_t           loopCounter;
extern uint8_t           setCounter;

uint8_t ROW_Y_VALS[NUM_OLED_ROWS];
uint8_t COL_X_VALS[NUM_OLED_COLS];
uint16_t patternRegister[NUM_CHANNELS]{0, 0, 0, 0, 0, 0};
uint16_t variationRegister[NUM_CHANNELS]{0, 0, 0, 0, 0, 0};
uint16_t oledTrackRegister[NUM_CHANNELS]{0, 0, 0, 0, 0, 0};
uint16_t *pCurrentPattern(patternRegister);

volatile bool _clockFlag(0);
volatile bool _resetFlag(0);
volatile bool clockPulse[2]{0, 0};
volatile bool resetPulse[2]{0, 0};


FastShiftOut outputs(SR_DAT, SR_CLK, MSBFIRST);
ClickEncoder encoder(ENC_A, ENC_B, ENC_SW, ENC_STEPS_PER_NOTCH, ENC_ACTIVE_LOW);
ClickEncoderInterface encoderInterface(encoder, ENC_SENSITIVITY);


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
