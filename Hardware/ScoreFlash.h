#ifndef __SCORE_FLASH_H
#define __SCORE_FLASH_H

#include "stm32f10x.h"

void ScoreFlash_Init(void);
uint16_t ScoreFlash_GetSnakeHigh(void);
uint16_t ScoreFlash_GetDinoHigh(void);
void ScoreFlash_UpdateSnake(uint16_t score);
void ScoreFlash_UpdateDino(uint16_t score);

#endif
