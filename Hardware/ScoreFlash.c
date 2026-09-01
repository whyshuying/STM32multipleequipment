#include "stm32f10x.h"
#include "stm32f10x_flash.h"
#include "ScoreFlash.h"

/* STM32F103C8 64KB Flash最后一个1KB页 */
#define SCORE_FLASH_ADDRESS  ((uint32_t)0x0800FC00)
#define SCORE_FLASH_MAGIC    ((uint16_t)0x5343)

typedef struct
{
	uint16_t magic;
	uint16_t snake_high;
	uint16_t dino_high;
	uint16_t checksum;
} ScoreFlash_Data;

static ScoreFlash_Data score_data;

static uint16_t ScoreFlash_Checksum(const ScoreFlash_Data *data)
{
	return (uint16_t)(data->magic ^ data->snake_high ^
					  data->dino_high ^ 0xA55A);
}

static void ScoreFlash_Save(void)
{
	FLASH_Status status;

	score_data.magic=SCORE_FLASH_MAGIC;
	score_data.checksum=ScoreFlash_Checksum(&score_data);

	FLASH_Unlock();
	status=FLASH_ErasePage(SCORE_FLASH_ADDRESS);
	if(status==FLASH_COMPLETE)
	{
		FLASH_ProgramHalfWord(SCORE_FLASH_ADDRESS,
							  score_data.magic);
		FLASH_ProgramHalfWord(SCORE_FLASH_ADDRESS+2,
							  score_data.snake_high);
		FLASH_ProgramHalfWord(SCORE_FLASH_ADDRESS+4,
							  score_data.dino_high);
		FLASH_ProgramHalfWord(SCORE_FLASH_ADDRESS+6,
							  score_data.checksum);
	}
	FLASH_Lock();
}

void ScoreFlash_Init(void)
{
	const ScoreFlash_Data *saved_data=
		(const ScoreFlash_Data *)SCORE_FLASH_ADDRESS;

	if(saved_data->magic==SCORE_FLASH_MAGIC &&
	   saved_data->checksum==ScoreFlash_Checksum(saved_data))
	{
		score_data=*saved_data;
	}
	else
	{
		score_data.magic=SCORE_FLASH_MAGIC;
		score_data.snake_high=0;
		score_data.dino_high=0;
		score_data.checksum=ScoreFlash_Checksum(&score_data);
	}
}

uint16_t ScoreFlash_GetSnakeHigh(void)
{
	return score_data.snake_high;
}

uint16_t ScoreFlash_GetDinoHigh(void)
{
	return score_data.dino_high;
}

void ScoreFlash_UpdateSnake(uint16_t score)
{
	if(score>score_data.snake_high)
	{
		score_data.snake_high=score;
		ScoreFlash_Save();
	}
}

void ScoreFlash_UpdateDino(uint16_t score)
{
	if(score>score_data.dino_high)
	{
		score_data.dino_high=score;
		ScoreFlash_Save();
	}
}
