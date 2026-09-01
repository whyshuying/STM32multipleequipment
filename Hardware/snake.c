#include "stm32f10x.h"
#include "OLED.h"
#include "Key.h"
#include "Delay.h"
#include "snake.h"
#include "ScoreFlash.h"
#include <stdlib.h>


/* ==================================================
   蛇的数据
   ================================================== */

/* 蛇身体 */
volatile struct Snake_Position snake[SNAKE_MAX_LENGTH];

/* 当前蛇长度 */
volatile uint8_t Snake_Length;

/* 当前方向 */
volatile uint8_t Snake_Direction;

volatile uint8_t Snake_LastMoveDirection;

/* 食物 */
volatile struct Snake_Position food;

/* 分数 */
volatile uint16_t Snake_Score;

/* 游戏结束标志 */
volatile uint8_t Snake_GameOver;

volatile uint8_t Snake_GameActive;


/* ==================================================
   生成食物
   ================================================== */

static uint8_t Snake_IsOpposite(uint8_t direction1, uint8_t direction2)
{
	return ((direction1==SNAKE_UP && direction2==SNAKE_DOWN) ||
			(direction1==SNAKE_DOWN && direction2==SNAKE_UP) ||
			(direction1==SNAKE_LEFT && direction2==SNAKE_RIGHT) ||
			(direction1==SNAKE_RIGHT && direction2==SNAKE_LEFT));
}

static uint8_t Snake_TurnDirection(uint8_t direction, uint8_t key)
{
	if(key==1)
	{
		if(direction==SNAKE_UP) return SNAKE_LEFT;
		if(direction==SNAKE_LEFT) return SNAKE_DOWN;
		if(direction==SNAKE_DOWN) return SNAKE_RIGHT;
		return SNAKE_UP;
	}

	if(direction==SNAKE_UP) return SNAKE_RIGHT;
	if(direction==SNAKE_RIGHT) return SNAKE_DOWN;
	if(direction==SNAKE_DOWN) return SNAKE_LEFT;
	return SNAKE_UP;
}
void Snake_CreateFood(void)
{
	uint8_t i;
	uint8_t flag;

	do
	{
		flag=0;

		/* 随机生成食物 */
		food.x=1+rand()%(SNAKE_MAP_WIDTH-2);
		food.y=1+rand()%(SNAKE_MAP_HEIGHT-2);

		/* 检查食物是否生成在蛇身上 */
		for(i=0;i<Snake_Length;i++)
		{
			if(food.x==snake[i].x &&
			   food.y==snake[i].y)
			{
				flag=1;
				break;
			}
		}

	}while(flag==1);
}


/* ==================================================
   初始化
   ================================================== */

void Snake_Init(void)
{
	/* 蛇初始长度 */
	Snake_Length=3;

	/* 初始方向：向右 */
	Snake_Direction=SNAKE_RIGHT;
	Snake_LastMoveDirection=SNAKE_RIGHT;

	/* 分数 */
	Snake_Score=0;

	/* 游戏结束标志 */
	Snake_GameOver=0;
	Snake_GameActive=1;


	/* =========================
	   设置蛇初始位置
	   ========================= */

	snake[0].x=10;
	snake[0].y=7;

	snake[1].x=9;
	snake[1].y=7;

	snake[2].x=8;
	snake[2].y=7;


	/* 生成第一个食物 */
	Snake_CreateFood();
}


/* ==================================================
   按键处理
   ================================================== */

void Snake_Key(void)
{
	extern uint8_t KeyNum;
	uint8_t new_direction;

	KeyNum=Key_GetNum();

	if(KeyNum==1 || KeyNum==2)
	{
		new_direction=Snake_TurnDirection(Snake_Direction,KeyNum);
		if(!Snake_IsOpposite(new_direction,Snake_LastMoveDirection))
		{
			Snake_Direction=new_direction;
		}
	}
}

void Snake_Move(void)
{
	uint8_t i;

	for(i=Snake_Length-1;i>0;i--)
	{
		snake[i]=snake[i-1];
	}

	if(Snake_Direction==SNAKE_UP)
	{
		snake[0].y--;
	}
	else if(Snake_Direction==SNAKE_DOWN)
	{
		snake[0].y++;
	}
	else if(Snake_Direction==SNAKE_LEFT)
	{
		snake[0].x--;
	}
	else
	{
		snake[0].x++;
	}

	Snake_LastMoveDirection=Snake_Direction;
}

uint8_t Snake_CheckWall(void)
{
	/*
		因为x、y是uint8_t，
		减到0再减会变成255。
	*/

	if(snake[0].x==0 || snake[0].x>=SNAKE_MAP_WIDTH-1)
	{
		return 1;
	}

	if(snake[0].y==0 || snake[0].y>=SNAKE_MAP_HEIGHT-1)
	{
		return 1;
	}

	if(snake[0].x==255)
	{
		return 1;
	}

	if(snake[0].y==255)
	{
		return 1;
	}

	return 0;
}


/* ==================================================
   撞自己检测
   ================================================== */

uint8_t Snake_CheckSelf(void)
{
	uint8_t i;

	for(i=1;i<Snake_Length;i++)
	{
		if(snake[0].x==snake[i].x &&
		   snake[0].y==snake[i].y)
		{
			return 1;
		}
	}

	return 0;
}


/* ==================================================
   吃食物检测
   ================================================== */

void Snake_CheckFood(void)
{
	if(snake[0].x==food.x &&
	   snake[0].y==food.y)
	{
		/* 蛇长度增加 */
		if(Snake_Length<SNAKE_MAX_LENGTH)
		{
			/*
				新增加的蛇身
				先放到原来的蛇尾位置
			*/

			snake[Snake_Length]=snake[Snake_Length-1];

			Snake_Length++;
		}

		/* 分数增加 */
		Snake_Score+=10;

		/* 重新生成食物 */
		Snake_CreateFood();
	}
}


/* ==================================================
   Tick
   ================================================== */

void Snake_Tick(void)
{
	static uint8_t move_count;

	if(Snake_GameActive==0 || Snake_GameOver==1)
	{
		move_count=0;
		return;
	}

	move_count++;
	if(move_count<SNAKE_MOVE_INTERVAL)
	{
		return;
	}
	move_count=0;

	Snake_Move();

	if(Snake_CheckWall())
	{
		Snake_GameOver=1;
		Snake_GameActive=0;
		return;
	}

	if(Snake_CheckSelf())
	{
		Snake_GameOver=1;
		Snake_GameActive=0;
		return;
	}

	Snake_CheckFood();
}

static void Show_Snake_Border(void)
{
	/* 顶部边框同时分隔分数区和游戏区 */
	OLED_DrawRectangle(
		0,
		SNAKE_MAP_TOP,
		SNAKE_MAP_WIDTH*SNAKE_GRID_SIZE,
		SNAKE_MAP_HEIGHT*SNAKE_GRID_SIZE,
		0
	);
}
void Show_Snake(void)
{
	uint8_t i;
	uint8_t x;
	uint8_t y;

	for(i=0;i<Snake_Length;i++)
	{
		x=snake[i].x*SNAKE_GRID_SIZE;
		y=SNAKE_MAP_TOP+snake[i].y*SNAKE_GRID_SIZE;

		if(i==0)
		{
			/* 蛇头与食物同为4×4像素，使用实心显示 */
			OLED_DrawRectangle(x,y,SNAKE_GRID_SIZE,SNAKE_GRID_SIZE,1);
		}
		else
		{
			/* 蛇身缩小为居中的2×2像素，和蛇头区分 */
			OLED_DrawRectangle(x+1,y+1,2,2,1);
		}
	}
}

void Show_Food(void)
{
	/* 食物与蛇头同为4×4像素，使用空心显示 */
	OLED_DrawRectangle(
		food.x*SNAKE_GRID_SIZE,
		SNAKE_MAP_TOP+food.y*SNAKE_GRID_SIZE,
		SNAKE_GRID_SIZE,
		SNAKE_GRID_SIZE,
		0
	);
}

void Show_Snake_Score(void)
{
	OLED_ShowNum(90,0,Snake_Score,4,OLED_6X8
	);
}


/* ==================================================
   游戏动画
   ================================================== */

int SnakeGame_Animation(void)
{
	while(1)
	{
		/* 读取按键 */
		Snake_Key();

		/* 显示当前画面 */
		OLED_Clear();
		Show_Snake_Score();
		Show_Snake_Border();
		Show_Snake();
		Show_Food();
		OLED_Update();

		/* 游戏结束 */
		if(Snake_GameOver==1)
		{
			OLED_Clear();
			OLED_ShowString(28,24,"Game Over",OLED_8X16);
			OLED_Update();
			ScoreFlash_UpdateSnake(Snake_Score);
Delay_s(1);
			OLED_Clear();
			OLED_Update();
			Snake_GameActive=0;
			return 0;
		}
	}
}