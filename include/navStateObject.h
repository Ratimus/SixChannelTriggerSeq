#ifndef NAV_STATE_OBJECT_H
#define NAV_STATE_OBJECT_H
#include "Arduino.h"
#include "globals.h"

void setPatternPointer();
void resetSeqPattern();

class NavState
{
public:
  NavState():
    encMode(mute0_MODE),
    selTrack(0),
    stepEditSubmode(false),
    muteRegister(0),
    fillsEnabled(true),
    modeIdx(0),
    modePage(0),
    autoMode(true),
    newPatternFlag(false),
    genreIdx(0),
    newGenreFlag(false),
    patternSelectIdx(-1),
    nextPattern(0),
    loopLenIdx(0),
    setLenIdx(0),
    selStep(0)
  { ; }

  modeType  encMode;
  int8_t    selTrack;
  bool      stepEditSubmode;
  uint8_t   muteRegister;
  bool      fillsEnabled;
  int8_t    modeIdx;
  uint8_t   modePage;
  bool      autoMode;
  bool      newPatternFlag;
  uint8_t   genreIdx;
  bool      newGenreFlag;
  int8_t    patternSelectIdx;
  int8_t    nextPattern;
  int8_t    loopLenIdx;
  int8_t    setLenIdx;
  int8_t    selStep;

  void click();
  void doubleClick();
  void move(bool up);
  void updateSelection();
  bool handleEncoder(encEvnts event);
  void changeModes();
  void changeGenres();
  void changeSetLength();
  void changeLoopLength();
  void loadPattern();
  void fillPatternRegisters();
  bool cvSelectPattern();
  void drawTrackLabels();
  void drawTrackPatterns();
  void drawStepIndicator(int8_t currentStep);
  void drawModeOptions(uint8_t loopCounter);
  void oled_redraw(int8_t currentStep, uint8_t loopCounter);
};

extern NavState navState;

#endif