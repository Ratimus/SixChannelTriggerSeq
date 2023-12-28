#pragma once
#include <Arduino.h>

//preset step pattern
//[パターンの数][CHの数*2。頭6個が通常時、後ろ6個がfillin]
//16信数はドラムパターンを示す。

//bnk1 TECHNO
const static word bnk1_ptn[8][12] PROGMEM = {
 { 0x8888, 0x0808, 0xDDDD, 0x2222, 0x1000, 0x0022, 0x8888, 0x88AF, 0xDDDD, 0x2222, 0x1000, 0x0022 },
 { 0x8888, 0x0808, 0xFFFF, 0x2222, 0x1000, 0x0022, 0x8888, 0x88AF, 0xFFFF, 0x2222, 0x1000, 0x0022 },
 { 0x8888, 0x0808, 0xCCCC, 0x2222, 0x1000, 0x0022, 0x8888, 0x88AF, 0xCCCC, 0x2222, 0x1000, 0x0022 },
 { 0x8888, 0x0808, 0x4545, 0x2222, 0x1000, 0x0022, 0x88AF, 0x0808, 0x4545, 0x2222, 0x1000, 0x0022 },
 { 0x8888, 0x0808, 0xFFFF, 0x2222, 0x1000, 0x0022, 0x88AF, 0x0808, 0xFFFF, 0x2222, 0x1000, 0x0022 },
 { 0x8888, 0x0809, 0xDDDD, 0x2222, 0x1000, 0x0022, 0x0000, 0x0809, 0xDDDD, 0x2222, 0x1000, 0x0022 },
 { 0x8888, 0x0849, 0xDDDD, 0x2222, 0x1000, 0x0022, 0x0000, 0x0849, 0xDDDD, 0x2222, 0x1000, 0x0022 },
 { 0x8888, 0x0802, 0xDDDD, 0x2222, 0x1000, 0x0022, 0x8896, 0x0869, 0xDDDD, 0x2222, 0x1000, 0x0022 }
};

//bunk2 DUBTECHNO
const static word bnk2_ptn[8][12] PROGMEM = {
 { 0x8888, 0x0808, 0xDDDD, 0x2222, 0x1240, 0x0022, 0x8888, 0x0809, 0xDDDD, 0x2222, 0x1240, 0x0022 },
 { 0x8888, 0x0808, 0xFFFF, 0x2222, 0x1240, 0x0022, 0x000A, 0x0849, 0xDDDD, 0x2222, 0x1000, 0x0022 },
 { 0x8889, 0x0808, 0xCCCC, 0x2222, 0x1240, 0x0022, 0x8888, 0x0000, 0xCCCC, 0x2222, 0x1240, 0x0022 },
 { 0x8889, 0x0808, 0x4545, 0x2222, 0x1240, 0x0022, 0x8888, 0x0809, 0x4545, 0x2222, 0x1240, 0x0022 },
 { 0x888C, 0x0808, 0xFFFF, 0x2222, 0x1240, 0x0022, 0x8888, 0x8888, 0xFFFF, 0x2222, 0x1240, 0x0022 },
 { 0x888C, 0x0809, 0xDDDD, 0x2222, 0x1240, 0x0022, 0x999F, 0x0000, 0xDDDD, 0x2222, 0x1240, 0x0022 },
 { 0x0000, 0x0849, 0xDDDD, 0x2222, 0x1240, 0x0022, 0x000A, 0x0849, 0xDDDD, 0x2222, 0x1000, 0x0022 },
 { 0x0000, 0x0802, 0xDDDD, 0x2222, 0x1240, 0x0022, 0x000A, 0x0802, 0xDDDD, 0x2222, 0x1000, 0x0022 }
};

//bunk3 HOUSE
const static word bnk3_ptn[8][12]PROGMEM = {
 { 0x8888, 0x0808, 0x2222, 0x0000, 0x0040, 0x0101, 0x8888, 0x88AF, 0x2222, 0x0000, 0x0040, 0x0101 },
 { 0x888A, 0x0808, 0x2323, 0x0000, 0x0040, 0x0101, 0x8888, 0x88AF, 0x2323, 0x0000, 0x0040, 0x0101 },
 { 0x8888, 0x0808, 0xCCCC, 0x2222, 0x0040, 0x0101, 0x8888, 0x88AF, 0xCCCC, 0x2222, 0x0040, 0x0101 },
 { 0x888A, 0x0808, 0xCCCC, 0x2222, 0x0040, 0x0101, 0x8888, 0x88AF, 0xCCCC, 0x2222, 0x0040, 0x0101 },
 { 0x8888, 0x0808, 0xFFFF, 0x2222, 0x0040, 0x0101, 0x0000, 0x0808, 0xFFFF, 0x2222, 0x0040, 0x0101 },
 { 0x888A, 0x0808, 0xFFFF, 0x2222, 0x0040, 0x0101, 0x0000, 0x0808, 0xFFFF, 0x2222, 0x0040, 0x0101 },
 { 0x888A, 0x0808, 0xFFFF, 0x2222, 0x0040, 0x4112, 0x0000, 0x0808, 0xFFFF, 0x2222, 0x0040, 0x4112 },
 { 0x8888, 0x0808, 0xCCCC, 0x2222, 0x0040, 0x4112, 0x88AF, 0x0808, 0xCCCC, 0x2222, 0x0040, 0x4112 }
};

const static word bnk4_ptn[8][12]PROGMEM = {
  { 0x8888, 0x0808, 0x0000, 0x2222, 0x0040, 0x0101, 0x8888, 0x88AF, 0x2222, 0x0040, 0x0000, 0x0101 },
  { 0x888A, 0x0808, 0x2323, 0x0000, 0x0040, 0x0101, 0x8888, 0x88AF, 0x2323, 0x0000, 0x0040, 0x0101 },
  { 0x8888, 0x0808, 0xCCCC, 0x2222, 0x0040, 0x0101, 0x8888, 0x88AF, 0xCCCC, 0x2222, 0x0040, 0x0101 },
  { 0x888A, 0x0808, 0xCCCC, 0x2222, 0x0040, 0x0101, 0x8888, 0x88AF, 0xCCCC, 0x2222, 0x0040, 0x0101 },
  { 0x8888, 0x0808, 0xFFFF, 0x2222, 0x0040, 0x0101, 0x0000, 0x0808, 0xFFFF, 0x2222, 0x0040, 0x0101 },
  { 0x888A, 0x0808, 0xFFFF, 0x2222, 0x0040, 0x0101, 0x0000, 0x0808, 0xFFFF, 0x2222, 0x0040, 0x0101 },
  { 0x888A, 0x0808, 0xFFFF, 0x2222, 0x0040, 0x4112, 0x0000, 0x0808, 0xFFFF, 0x2222, 0x0040, 0x4112 },
  { 0x8888, 0x0808, 0xCCCC, 0x2222, 0x0040, 0x4112, 0x88AF, 0x0808, 0xCCCC, 0x2222, 0x0040, 0x4112 }
};
