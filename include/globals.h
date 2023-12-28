#ifndef GLOBALZZZ_H
#define GLOBALZZZ_H

#include "Arduino.h"
#include <EEPROM.h>

#include "FastShiftOut.h"
#include "ClickEncoder.h"
#include "ClickEncoderInterface.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_ADDRESS 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

void highlightTextIf(bool inverted);
void getStepChar(uint8_t track, uint8_t step, bool mute, char * buf);

void savePatternToEeprom();
void loadPatternFromEeprom();

constexpr uint8_t ENC_A               (2);
constexpr uint8_t ENC_B               (3);
constexpr uint8_t ENC_SW              (4);
constexpr uint8_t CLOCK_PIN           (11);
constexpr uint8_t RESET_PIN           (12);

constexpr uint8_t SR_CLK              (5);
constexpr uint8_t SR_LCH              (6);
constexpr uint8_t SR_DAT              (8);

constexpr uint8_t CTRL                (A7);
constexpr uint8_t ENC_SENSITIVITY     (1);
constexpr bool    ENC_ACTIVE_LOW      (1);
constexpr uint8_t ENC_STEPS_PER_NOTCH (4);

constexpr uint8_t TRIGGER_LENGTH      (10);
constexpr uint8_t TIMER_INTERVAL_MS   (1);
constexpr uint8_t OUTPUT_MAP[]{5, 0, 4, 7, 3, 6, 1, 2};

constexpr uint8_t NUM_AUTO_MODES      (11);
constexpr uint8_t NUM_MANUAL_MODES    (15);

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
};

const uint8_t MODE_SELECT_LOCATION[2]{6, 12};
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

constexpr uint8_t NUM_GENRES(4);
constexpr char    genreNames[NUM_GENRES][7]{"TECHNO", "DUBTEK", "HOUSE ", "SHIT  "};

const char space[2] = " ";
const char splat[2] = "*";
const char under[2] = "_";

constexpr int8_t NUM_LOOP_LENGTHS(4);
constexpr int8_t LOOP_LENGTHS[NUM_LOOP_LENGTHS]{4, 8, 16, 32};

constexpr int8_t NUM_SET_LENGTHS(4);
constexpr int8_t SET_LENGTHS[NUM_SET_LENGTHS]{2, 4, 8, 16};


extern Adafruit_SSD1306 display;

extern modeType AUTO_MODES[NUM_AUTO_MODES];
extern modeType MANUAL_MODES[NUM_MANUAL_MODES];

extern uint8_t  outputRegister;
extern int8_t   outputOnTime;
extern bool     outputsExpired;
extern uint8_t  patternLength;
extern bool     playFillThisBar;

extern int8_t   currentStep;
extern int8_t   nextStep;

extern uint8_t  barCounter;
extern uint8_t  loopCounter;
extern uint8_t  setCounter;

extern uint8_t  ROW_Y_VALS[NUM_OLED_ROWS];
extern uint8_t  COL_X_VALS[NUM_OLED_COLS];
extern uint16_t patternRegister[NUM_CHANNELS];
extern uint16_t variationRegister[NUM_CHANNELS];
extern uint16_t oledTrackRegister[NUM_CHANNELS];
extern uint16_t *pCurrentPattern;

extern volatile bool _clockFlag;
extern volatile bool _resetFlag;
extern volatile bool clockPulse[2];
extern volatile bool resetPulse[2];

extern FastShiftOut outputs;
extern ClickEncoder encoder;
extern ClickEncoderInterface encoderInterface;

#endif