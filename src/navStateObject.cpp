#include "navStateObject.h"
#include "riddims.h"

NavState navState;

void NavState :: click()
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
      changeModes();
      break;

    case selectGenre_MODE:
      changeGenres();
      if (currentStep == -1)
      {
        loadPattern();
        setPatternPointer();
        navState.oled_redraw(currentStep, loopCounter);
      }
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


void NavState :: doubleClick()
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


void NavState :: move(bool up)
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


void NavState :: updateSelection()
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


bool NavState :: handleEncoder(encEvnts event)
{
  switch(event)
  {
    case encEvnts::Left:
      move(0);
      break;

    case encEvnts::Right:
      move(1);
      break;

    case encEvnts::Click:
      click();
      break;

    case encEvnts::DblClick:
      doubleClick();
      break;

    case encEvnts::ShiftRight:
    case encEvnts::ShiftLeft:
    case encEvnts::Hold:
    case encEvnts::Press:
    case encEvnts::ClickHold:
    case encEvnts::NUM_ENC_EVNTS:
    default:
      return false;
  }

  return true;
}


void NavState :: changeModes()
{
  autoMode = !autoMode;
  if (navState.autoMode)
  {
    savePatternToEeprom();
    loadPattern();
    modeIdx = MODE_SELECT_LOCATION[0];
  }
  else
  {
    loadPatternFromEeprom();
    modeIdx = MODE_SELECT_LOCATION[1];
  }
}


void NavState :: changeGenres()
{
  ++genreIdx;
  if (genreIdx >= NUM_GENRES)
  {
    genreIdx = 0;
  }
  newGenreFlag = 1;
}


void NavState :: changeSetLength()
{
  ++setLenIdx;
  if (setLenIdx >= NUM_SET_LENGTHS)
  {
    setLenIdx = 0;
    loopCounter %= SET_LENGTHS[setLenIdx];
  }
}


void NavState :: changeLoopLength()
{
  ++loopLenIdx;
  if (loopLenIdx >= NUM_LOOP_LENGTHS)
  {
    loopLenIdx = 0;
    barCounter %= LOOP_LENGTHS[loopLenIdx];
  }
}


void NavState :: loadPattern()
{
  newGenreFlag = false;
  newPatternFlag = false;
  patternSelectIdx = nextPattern;
  fillPatternRegisters();
}


void NavState :: fillPatternRegisters()
{
  for (uint8_t ch(0); ch < 6; ++ch)
  {
    switch (genreIdx)
    {
      case 0:
        patternRegister[ch] = pgm_read_word(bnk1_ptn[patternSelectIdx] + ch);
        variationRegister[ch] = pgm_read_word(bnk1_ptn[patternSelectIdx] + ch + 6);
        break;

      case 1:
        patternRegister[ch] = pgm_read_word(bnk2_ptn[patternSelectIdx] + ch);
        variationRegister[ch] = pgm_read_word(bnk2_ptn[patternSelectIdx] + ch + 6);
        break;

      case 2:
        patternRegister[ch] = pgm_read_word(bnk3_ptn[patternSelectIdx] + ch);
        variationRegister[ch] = pgm_read_word(bnk2_ptn[patternSelectIdx] + ch + 6);
        break;

      case 3:
        patternRegister[ch] = pgm_read_word(bnk4_ptn[patternSelectIdx] + ch);
        variationRegister[ch] = pgm_read_word(bnk2_ptn[patternSelectIdx] + ch + 6);
        break;

      default:
        break;
    }
  }
}

bool NavState :: cvSelectPattern()
{
  bool retVal(false);

  int16_t cv(analogRead(CTRL));
  uint8_t cvPattern(map(cv, 0, 704, 0, 7 + 1));
  if (cvPattern > 7)
  {
    cvPattern = 7;
  }

  if (cvPattern != nextPattern)
  {
    // Redraw oled to show *new* nextPattern
    nextPattern = cvPattern;
    retVal = true;
  }

  if (nextPattern != patternSelectIdx)
  {
    newPatternFlag = true;
  }

  return retVal;
}


void NavState :: oled_redraw(int8_t currentStep, uint8_t loopCounter)
{
  display.clearDisplay();
  drawTrackLabels();
  drawTrackPatterns();
  drawStepIndicator(currentStep);
  drawModeOptions(loopCounter);
  display.setCursor(0, 0);
  display.display();
}


void NavState :: drawStepIndicator(int8_t currentStep)
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


void NavState :: drawTrackLabels()
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


void NavState ::drawTrackPatterns()
{
  uint8_t xVal(0);
  uint8_t yVal(0);
  for (uint8_t track(0); track < NUM_CHANNELS; ++track)
  {
    bool editTrack(navState.stepEditSubmode && track == navState.selTrack);
    bool mute(bitRead(navState.muteRegister, track) && !editTrack);

    yVal = ROW_Y_VALS[track];
    for (uint8_t step(0); step < patternLength; ++step)
    {
      xVal = TRACK_STEP_X_VAL + step * COL_WIDTH_NO_MARGIN;
      display.setCursor(xVal, yVal);
      highlightTextIf(editTrack && step == navState.selStep);
      char val[2];
      getStepChar(track, step, mute, val);
      display.print(val);
    }

    if (navState.encMode != edit0_MODE &&
        navState.encMode != edit1_MODE &&
        navState.encMode != edit2_MODE &&
        navState.encMode != edit3_MODE &&
        navState.encMode != edit4_MODE &&
        navState.encMode != edit5_MODE)
    {
      continue;
    }

    if (navState.selTrack != track)
    {
      continue;
    }

    display.setTextColor(WHITE);
    display.setCursor(TRACK_STEP_X_VAL - OLED_COL_WIDTH, yVal);
    display.print(char(16));
  }
}


void NavState ::drawModeOptions(uint8_t loopCounter)
{
  if (modePage)
  {
    display.setCursor(REP_COUNT_X_VAL, BOTTOM_ROW_Y_VAL);
    display.setTextColor(WHITE);
    display.print("BAR:");
    display.print(barCounter + 1);
    display.print("/");
    highlightTextIf(encMode == loopLength_MODE);
    display.print(LOOP_LENGTHS[navState.loopLenIdx]);

    display.setTextColor(WHITE);
    display.setCursor(SWITCH_COUNT_X_VAL, BOTTOM_ROW_Y_VAL);
    display.print("LP:");
    display.print(loopCounter + 1);
    display.print("/");
    highlightTextIf(encMode == setLength_MODE);
    uint8_t len(SET_LENGTHS[navState.setLenIdx]);
    display.print(len);
    display.setTextColor(BLACK);
    return;
  }

  display.setCursor(MODE_LABEL_X_VAL, BOTTOM_ROW_Y_VAL);
  highlightTextIf(encMode == selectMode_MODE);
  display.print(navState.autoMode ? "AUTO" : "MNAL");

  if (navState.autoMode)
  {
    display.setCursor(GENRE_LABEL_X_VAL, BOTTOM_ROW_Y_VAL);
    highlightTextIf(navState.encMode == selectGenre_MODE);
    if (navState.newGenreFlag)
    {
      display.print(char(26));
      display.print(genreNames[navState.genreIdx]);
      display.print(":");
      display.print(navState.nextPattern + 1);
    }
    else
    {
      display.print(genreNames[navState.genreIdx]);
      if (navState.nextPattern != navState.patternSelectIdx)
      {
        display.print(char(26));
        display.print(navState.nextPattern + 1);
      }
      else
      {
        display.print(":");
        display.print(navState.patternSelectIdx + 1);
      }
    }

    display.setTextColor(WHITE);
    display.setCursor(FILL_LABEL_X_VAL, BOTTOM_ROW_Y_VAL);
    highlightTextIf(playFillThisBar);
    display.print("Fill:");
    highlightTextIf(encMode == toggleFills_MODE);
    display.print(navState.fillsEnabled ? "Y" : "N");
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


void setPatternPointer()
{
  if (barCounter == LOOP_LENGTHS[navState.loopLenIdx] - 1)
  {
    if (navState.fillsEnabled)
    {
      playFillThisBar = true;
      pCurrentPattern = variationRegister;
    }
  }
  else
  {
    pCurrentPattern = patternRegister;
    playFillThisBar = false;
  }

  for (uint8_t ch(0); ch < NUM_CHANNELS; ++ch)
  {
    oledTrackRegister[ch] = *(pCurrentPattern + ch);
  }
}


void resetSeqPattern()
{
  currentStep     = -1;
  nextStep        = 0;
  barCounter      = 0;
  loopCounter     = 0;
  setCounter      = 0;
  navState.newPatternFlag  = true;
  navState.loadPattern();
  setPatternPointer();
  navState.oled_redraw(currentStep, loopCounter);
}