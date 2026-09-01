#ifndef __SNAKE_H
#define __SNAKE_H

#include "stm32f10x.h"

/* =========================
   游戏地图
   ========================= */

#define SNAKE_MAP_WIDTH     32
#define SNAKE_MAP_HEIGHT    14
#define SNAKE_MAP_TOP       8

/* 每一个格子的像素大小 */
#define SNAKE_GRID_SIZE     4

/* 蛇最大长度 */
#define SNAKE_MAX_LENGTH    50
#define SNAKE_MOVE_INTERVAL 150


/* =========================
   蛇的方向
   ========================= */

#define SNAKE_UP            0
#define SNAKE_DOWN          1
#define SNAKE_LEFT          2
#define SNAKE_RIGHT         3


/* =========================
   蛇节点
   ========================= */

struct Snake_Position
{
	uint8_t x;
	uint8_t y;
};


/* =========================
   对外函数
   ========================= */

/* 初始化游戏 */
void Snake_Init(void);

/* 游戏动画 */
int SnakeGame_Animation(void);

/* 游戏定时更新 */
void Snake_Tick(void);

/* 按键处理 */
void Snake_Key(void);

/* 显示蛇 */
void Show_Snake(void);

/* 显示食物 */
void Show_Food(void);

/* 显示分数 */
void Show_Snake_Score(void);

#endif