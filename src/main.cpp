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

#include "globals.h"
#include "navStateObject.h"

#define CASSIDEBUG

void handleClockFlag();
void handleResetFlag();
void handleExpiredOutputs();

void clearOutputRegister();
void updateOutputRegister();
void writeOutputRegister(uint8_t outval);

void advanceStepCounter();
void resetSeqPattern();


////////////////////////////////////////////////////////////////////////////////
// Reads gate inputs, writes outputs on clock pulse, calculates expired outputs,
// services Nav controls
void timer1_ISR()
{
  clockPulse[1] = clockPulse[0];
  clockPulse[0] = !digitalRead(CLOCK_PIN);
  if (clockPulse[0] && !clockPulse[1])
  {
    clearOutputRegister();
    currentStep = nextStep;
    updateOutputRegister();
    outputOnTime = TRIGGER_LENGTH;
    _clockFlag = true;
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
  handleExpiredOutputs();
  handleResetFlag();
  handleClockFlag();

  if (navState.handleEncoder(encoderInterface.getEvent()))
  {
    navState.oled_redraw(currentStep, loopCounter);
  }

  if (navState.cvSelectPattern())
  {
    if (currentStep == -1)
    {
      navState.loadPattern();
      setPatternPointer();
    }
    navState.oled_redraw(currentStep, loopCounter);
  }
}


void handleExpiredOutputs()
{
  if (!outputsExpired)
  {
    return;
  }

  outputsExpired = 0;
  clearOutputRegister();
}


void handleClockFlag()
{
  if (!_clockFlag)
  {
    return;
  }
  // Outputs have already been written but we need to calculate our next move
  _clockFlag = false;
  navState.oled_redraw(currentStep, loopCounter);
  advanceStepCounter();
  if (nextStep == 0)
  {
    setPatternPointer();
  }
}


void handleResetFlag()
{
  if (!_resetFlag)
  {
    return;
  }
  _resetFlag = 0;
  resetSeqPattern();
}


void clearOutputRegister() { writeOutputRegister(0); }


void updateOutputRegister()
{
  uint8_t outVal(0);
  for (uint8_t ch(0); ch < NUM_CHANNELS; ++ch)
  {
    if bitRead(navState.muteRegister, ch)
    {
      continue;
    }

    if (!bitRead(*(pCurrentPattern + ch), 15 - currentStep))
    {
      continue;
    }

    bitWrite(outVal, OUTPUT_MAP[ch], 1);
  }

  writeOutputRegister(outVal);
}


void writeOutputRegister(uint8_t inVal)
{
  digitalWrite(SR_LCH, LOW);
  outputs.write(inVal);
  digitalWrite(SR_LCH, HIGH);
}


void advanceStepCounter()
{
  ++nextStep;
  if (nextStep < patternLength)
  {
    return;
  }

  nextStep = 0;
  ++barCounter;

  if (barCounter >= LOOP_LENGTHS[navState.loopLenIdx])
  {
    ++loopCounter;
    barCounter = 0;
  }

  if (loopCounter >= SET_LENGTHS[navState.setLenIdx])
  {
    loopCounter = 0;
    navState.loadPattern();
  }
}

