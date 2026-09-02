/***************************************************************************************
  * STM32手表UI部分程序由火禾实验室创建并免费开源共享
  * 你可以任意查看、使用和修改，并应用到自己的项目之中
  * 程序版权归火禾实验室所有，任何人或组织不得将其据为己有
  * 
  * 程序名称：				stm32可编程多功能手表UI
  * 程序创建时间：			2025.5.27
  * 当前程序版本：			V1.0
  * 当前版本发布时间：	2025.7.10
  ****************************************************************************************/

#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "MyRTC.h"                                                                                                                                                                                                     
#include "Key.h"
#include "LED.h"
#include "SetTime.h"
#include "menu.h"
#include "MPU6050.h"
#include "Delay.h"
#include <math.h>
#include "dino.h"
#include "snake.h"
#include "ScoreFlash.h"

#include "AD.h"
#include "PWM.h"

uint8_t KeyNum;//用于存储按键值

/**
  * 函    数：外设初始化函数
  * 参    数：无
  * 返 回 值：无
  * 说    明：在这里放置手表所需外设的初始化函数，
	            然后再将这个函数放在主函数中对所有的外设进行初始化
  */
void Peripheral_Init(void)
{
	MyRTC_Init();
	Key_Init();
	LED_Init();
	PWM_Init();
	MPU6050_Init();
	AD_Init();
	ScoreFlash_Init();
}



/*----------------------------------首页时钟-------------------------------------*/

uint16_t ADValue;
float VBAT;
int Battery_Capacity;

//显示电池图标和电量
void Show_Battery(void)
{
	int sum;
	for(int i=0;i<3000;i++)
	{
		ADValue=AD_GetValue();
		sum+=ADValue;
		
	}
	ADValue=sum/3000;
	VBAT=(float)ADValue/4095*3.3;
	
	Battery_Capacity=ADValue*100/4095;//模拟adc电量转换，测试使用这个
//	Battery_Capacity=(ADValue-3276)*100/819;//电池电量转换，实际外接电池使用这个，具体电池数据也要变
	
	if(Battery_Capacity<0)Battery_Capacity=0;
	
	//OLED_ShowNum(64,0,ADValue,4,OLED_6X8);
	//OLED_Printf(64,8,OLED_6X8,"VBAT:%.2f",VBAT);
	OLED_ShowNum(85,4,Battery_Capacity,3,OLED_6X8);
	OLED_ShowChar(103,4,'%',OLED_6X8);
	
	if(Battery_Capacity==100)OLED_ShowImage(110,0,16,16,Battery);
	else if(Battery_Capacity>=10&&Battery_Capacity<100)
	{
		OLED_ShowImage(110,0,16,16,Battery);
		OLED_ClearArea((112+Battery_Capacity/10),5,(10-Battery_Capacity/10),6);
		OLED_ClearArea(85,4,6,8);
	}
	
	else
	{
		OLED_ShowImage(110,0,16,16,Battery);
		OLED_ClearArea(112,5,10,6);
		OLED_ClearArea(85,4,12,8);
	}
};


//显示首页时钟UI
void Show_Clock_UI(void)
{
	Show_Battery();
	MyRTC_ReadTime();
	OLED_Printf(0,0,OLED_6X8,"%d-%d-%d",MyRTC_Time[0],MyRTC_Time[1],MyRTC_Time[2]);
	OLED_Printf(16,16,OLED_12X24,"%02d:%02d:%02d",MyRTC_Time[3],MyRTC_Time[4],MyRTC_Time[5]);
	OLED_ShowString(0,48,"菜单",OLED_8X16);
	OLED_ShowString(96,48,"设置",OLED_8X16);
}

int clkflag=1;//首页时钟按键标志位，赋初值为1，光标一进来停在第一项

//控制光标在首页时钟移动的函数
int First_Page_Clock(void)
{
	while(1)
	{
		KeyNum=Key_GetNum();

		if(KeyNum==1)//上一项
		{
			clkflag--;
			if(clkflag<=0)clkflag=2;
		}
		else if(KeyNum==2)//下一项
		{
			clkflag++;
			if(clkflag>=3)clkflag=1;
		}
		else if(KeyNum==3)//确认
		{
			OLED_Clear();
			OLED_Update();
			return clkflag;
		}
		
		else if(KeyNum==4)
		{
			GPIO_ResetBits(GPIOB, GPIO_Pin_13);//长按Key3,KeyNum等于4，拉低CTL引脚（PB13),单片机关机
			GPIO_SetBits(GPIOB, GPIO_Pin_12);//拉高BAT_ADC_EN引脚（PB12),ADC检测电路断开
		};
		switch(clkflag)
		{
			case 1:
				Show_Clock_UI();
				OLED_ReverseArea(0,48,32,16);
				OLED_Update();
				break;
			
			case 2:
				Show_Clock_UI();
				OLED_ReverseArea(96,48,32,16);
				OLED_Update();
				break;
		}
	}
}

/*----------------------------------设置界面-------------------------------------*/

//显示设置界面UI
void Show_SettingPage_UI(void)
{
	OLED_ShowImage(0,0,16,16,Return);
	OLED_ShowString(0,16,"日期时间设置",OLED_8X16);
}

int setflag=1;
//控制光标在设置界面移动的函数
int SettingPage(void)
{
	while(1)
	{
		KeyNum=Key_GetNum();
		uint8_t setflag_temp=0;
		if(KeyNum==1)//上一项
		{
			setflag--;
			if(setflag<=0)setflag=2;
		}
		else if(KeyNum==2)//下一项
		{
			setflag++;
			if(setflag>=3)setflag=1;
		}
		else if(KeyNum==3)//确认
		{
			OLED_Clear();
			OLED_Update();
			setflag_temp=setflag;
		}
		
		if(setflag_temp==1){return 0;}
		else if(setflag_temp==2){SetTime();}//跳转到日期时间设置界面
		
		switch(setflag)
		{
			case 1:
				Show_SettingPage_UI();
				OLED_ReverseArea(0,0,16,16);
				OLED_Update();
				break;
			
			case 2:
				Show_SettingPage_UI();
				OLED_ReverseArea(0,16,96,16);
				OLED_Update();
				break;
		}
	}
}

/*----------------------------------滑动菜单界面-------------------------------------*/

uint8_t pre_selection;//上次选择的选项
uint8_t target_selection;//目标选项
uint8_t x_pre=48;//上次选项的x坐标
uint8_t Speed=4;//速度
uint8_t move_flag;//开始移动的标志位，1表示开始移动，0表示停止移动


//滑动菜单动画函数
void Menu_Animation(void)
{
	OLED_Clear();
	OLED_ShowImage(42,10,44,44,Frame);
	
	if(pre_selection<target_selection)
	{
		x_pre-=Speed;
		if(x_pre==0)
		{
			pre_selection++;
			move_flag=0;
			x_pre=48;
		}
	}
	
	if(pre_selection>target_selection)
	{
		x_pre+=Speed;
		if(x_pre==96)
		{
			pre_selection--;
			move_flag=0;
			x_pre=48;
		}
	}
	
	if(pre_selection>=1)
	{
		OLED_ShowImage(x_pre-48,16,32,32,Menu_Graph[pre_selection-1]);
	}
	
	if(pre_selection>=2)
	{
		OLED_ShowImage(x_pre-96,16,32,32,Menu_Graph[pre_selection-2]);
	}
	
	OLED_ShowImage(x_pre,16,32,32,Menu_Graph[pre_selection]);
	OLED_ShowImage(x_pre+48,16,32,32,Menu_Graph[pre_selection+1]);
	OLED_ShowImage(x_pre+96,16,32,32,Menu_Graph[pre_selection+2]);
	
	OLED_Update();
}


//选择移动方向的函数
void Set_Selection(uint8_t move_flag,uint8_t Pre_Selection,uint8_t Target_Selection)
{
	if(move_flag==1)
	{
		pre_selection=Pre_Selection;
		target_selection=Target_Selection;
		
	}
	Menu_Animation();
}

//滑动菜单到具体功能的转场函数（下移转场）
void MenuToFunction(void)
{
	for(uint8_t i=0;i<=6;i++)//每个循环向下移8格，六个循环过后完全移出屏幕
	{
		OLED_Clear();
			if(pre_selection>=1)
		{
			OLED_ShowImage(x_pre-48,16+8*i,32,32,Menu_Graph[pre_selection-1]);
		}
		
		
		OLED_ShowImage(x_pre,16+8*i,32,32,Menu_Graph[pre_selection]);
		OLED_ShowImage(x_pre+48,16+8*i,32,32,Menu_Graph[pre_selection+1]);
		
		OLED_Update();
	}
	
}


uint8_t menu_flag=1;
//控制图标在滑动菜单界面移动的函数
int Menu(void)
{
	move_flag=1;
	uint8_t DirectFlag=2;//置1：移动到上一项；置2：移动到下一项
	while(1)
	{
		KeyNum=Key_GetNum();
		uint8_t menu_flag_temp=0;
		if(KeyNum==1)//上一项
		{
			DirectFlag=1;
			move_flag=1;
			menu_flag--;
			if(menu_flag<=0)menu_flag=7;
		}
		else if(KeyNum==2)//下一项
		{
			DirectFlag=2;
			move_flag=1;
			menu_flag++;
			if(menu_flag>=8)menu_flag=1;
		}
		else if(KeyNum==3)//确认
		{
			OLED_Clear();
			OLED_Update();
			menu_flag_temp=menu_flag;
		}
		
		if(menu_flag_temp==1){return 0;}
		else if(menu_flag_temp==2){MenuToFunction();StopWatch();}//跳转到秒表界面
		else if(menu_flag_temp==3){MenuToFunction();LED();}//跳转到手电筒界面
	  else if(menu_flag_temp==4){MenuToFunction();MPU6050();}//跳转到MPU6050界面
		else if(menu_flag_temp==5){MenuToFunction();Game();}//跳转到游戏界面
		else if(menu_flag_temp==6){MenuToFunction();Emoji();}//跳转到动态表情包界面
		else if(menu_flag_temp==7){MenuToFunction();Gradienter();}//跳转到水平仪界面
			

			if(menu_flag==1)
			{
				if(DirectFlag==1)Set_Selection(move_flag,1,0);
				else if(DirectFlag==2)Set_Selection(move_flag,0,0);
			}
			
			else
			{
				if(DirectFlag==1)Set_Selection(move_flag,menu_flag,menu_flag-1);
				else if(DirectFlag==2)Set_Selection(move_flag,menu_flag-2,menu_flag-1);
			}
	}
}

/*----------------------------------秒表-------------------------------------*/

uint8_t hour,min,sec;
//显示秒表界面的函数
void Show_StopWatch_UI(void)
{
	OLED_ShowImage(0,0,16,16,Return);
	OLED_Printf(32,20,OLED_8X16,"%02d:%02d:%02d",hour,min,sec);
	OLED_ShowString(8,44,"开始",OLED_8X16);
	OLED_ShowString(48,44,"停止",OLED_8X16);
	OLED_ShowString(88,44,"清除",OLED_8X16);
}

uint8_t start_timing_flag;//1：开始，0：停止

//显示计时器的函数
void StopWatch_Tick(void)
{
	static uint16_t Count;
	
	if(start_timing_flag==1)
	{
		Count++;
		
		if(Count>=1000)	
		{	
			Count=0;
			sec++;
			if(sec>=60)
			{
				sec=0;
				min++;
				if(min>=60)
				{
					min=0;
					hour++;
					if(hour>99)hour=0;
				}
			}
		}
	}
	
}


uint8_t stopwatch_flag=1;
//控制光标在秒表界面移动的函数
int StopWatch(void)
{
	while(1)
	{
		KeyNum=Key_GetNum();
		uint8_t stopwatch_flag_temp=0;
		if(KeyNum==1)//上一项
		{
			stopwatch_flag--;
			if(stopwatch_flag<=0)stopwatch_flag=4;
		}
		else if(KeyNum==2)//下一项
		{
			stopwatch_flag++;
			if(stopwatch_flag>=5)stopwatch_flag=1;
		}
		else if(KeyNum==3)//确认
		{
			OLED_Clear();
			OLED_Update();
			stopwatch_flag_temp=stopwatch_flag;
		}
		
		if(stopwatch_flag_temp==1){return 0;}
		
		
		else if(stopwatch_flag_temp==2){start_timing_flag=1;}
		else if(stopwatch_flag_temp==3){start_timing_flag=0;}
		else if(stopwatch_flag_temp==4){start_timing_flag=0;hour=min=sec=0;}
		
		
		switch(stopwatch_flag)
		{
			case 1://返回
				Show_StopWatch_UI();
				OLED_ReverseArea(0,0,16,16);
				OLED_Update();
				break;
			
			case 2://开始计时
				Show_StopWatch_UI();
				
				OLED_ReverseArea(8,44,32,16);
				OLED_Update();
				break;
			
			case 3://停止计时
				Show_StopWatch_UI();
				
				OLED_ReverseArea(48,44,32,16);
				OLED_Update();
				break;
			
			case 4://清零
				Show_StopWatch_UI();
				
				
				OLED_ReverseArea(88,44,32,16);
				OLED_Update();
				break;
		}
	}
}

/*----------------------------------手电筒-------------------------------------*/

//显示手电筒界面的函数
void Show_LED_UI(void)
{
	OLED_ShowImage(0,0,16,16,Return);
	OLED_ShowString(20,20,"OFF",OLED_12X24);
	OLED_ShowString(72,20,"ON",OLED_12X24);
}

uint8_t led_flag=1,last_led_flag=0;//last_led_flag上次标志位记录，解决重复执行问题
uint8_t brightness=100;

//灯条变化函数
void Show_Brightness_Bar(void)
{
    uint8_t width;

    width = (uint16_t)brightness * 100 / 99;

    OLED_ShowImage(19, 57, 100, 8, light);

    if(width < 100)
    {
        OLED_ReverseArea(19 + width, 57, 100 - width, 8);
    }
}

//控制光标在手电筒界面移动的函数
int LED(void)
{
	while(1)
	{
		KeyNum=Key_GetNum();

		if(KeyNum==1)//0FF
		{
			led_flag=2;
		}
		else if(KeyNum==2)//ON
		{
			led_flag=3;
		}
		else if(KeyNum==3)//返回
		{
			led_flag=1;
			OLED_Clear();
			OLED_Update();
			return 0;
		}
		
		
		else if(KeyNum == 5)       // 长按减
		{
			OLED_ShowNum(0, 57, brightness, 3, OLED_6X8);//调试使用
			OLED_Update();
			if(brightness > 0)
			{
				brightness--;
			}

			if(led_flag == 3)  // 当前灯是开的
			{
				PWM_SetCompare3(brightness);
			}
		}
		else if(KeyNum == 6)  // 长按加
		{
			OLED_ShowNum(0, 57, brightness, 3, OLED_6X8);
			OLED_Update();
			if(brightness < 100)
			{
				brightness++;
			}
		
			if(led_flag == 3)
			{
				PWM_SetCompare3(brightness);
			}
		}
		
		
		//引入当前状态和上次状态，防止重复执行
		if(led_flag!=last_led_flag)
		{
		
		switch(led_flag)
		{
			case 1:

				break;
			
			case 2:
				LED_OFF();
				break;
			
			case 3:
				LED_ON();
				break;
			
		
		}
			last_led_flag = led_flag;
		}
		
		
		
		switch(led_flag)
		{
			case 1:
				
				Show_LED_UI();
				OLED_ReverseArea(0,0,16,16);
				OLED_Update();
				break;
			
			case 2:
				
				Show_LED_UI();
				OLED_ReverseArea(20,20,36,24);
				OLED_Update();
				break;
			
			case 3:
				
				Show_LED_UI();
				OLED_ReverseArea(72,20,24,24);
				OLED_Update();
				break;
			
		
		}
		OLED_ShowNum(0, 57, brightness, 3, OLED_6X8);
		OLED_ShowImage(19,57,100,8,light);
		Show_Brightness_Bar();
	}
}

/*----------------------------------MPU6050-------------------------------------*/

int16_t ax,ay,az,gx,gy,gz;//MPU6050测得的三轴加速度和角速度
float roll_g,pitch_g,yaw_g;//陀螺仪解算的欧拉角
float roll_a,pitch_a;//加速度计解算的欧拉角
float Roll,Pitch,Yaw;//互补滤波后的欧拉角
float a=0.98f;//互补滤波器系数
float Delta_t=0.005f;//采样周期约5ms
double pi=3.1415927;

/* MPU6050 ±2000°/s量程的灵敏度：16.4 LSB/(°/s) */
int32_t gyro_x_bias,gyro_y_bias;
float roll_zero,pitch_zero;
uint8_t MPU6050_Calibrated;

//上电时校准陀螺仪零偏和当前放平姿态
void MPU6050_Calibrate(void)
{
	int32_t sum_gx=0;
	int32_t sum_gy=0;
	float sum_roll=0;
	float sum_pitch=0;
	uint8_t i;

	for(i=0;i<50;i++)
	{
		MPU6050_GetData(&ax,&ay,&az,&gx,&gy,&gz);
		sum_gx+=gx;
		sum_gy+=gy;
		sum_roll+=atan2((double)ay,(double)az)*180.0/pi;
		sum_pitch+=atan2((double)(-ax),
						 sqrt((double)ay*ay+(double)az*az))*180.0/pi;
		Delay_ms(5);
	}

	gyro_x_bias=sum_gx/50;
	gyro_y_bias=sum_gy/50;
	roll_zero=sum_roll/50.0f;
	pitch_zero=sum_pitch/50.0f;
	Roll=0;
	Pitch=0;
	MPU6050_Calibrated=1;
}

//通过MPU6050的数据进行姿态解算的函数
void MPU6050_Calculation(void)
{
	float gyro_x_dps;
	float gyro_y_dps;
	float current_roll;
	float current_pitch;

	if(MPU6050_Calibrated==0)
	{
		MPU6050_Calibrate();
	}

	MPU6050_GetData(&ax,&ay,&az,&gx,&gy,&gz);

	/* 先将陀螺仪原始值换算为°/s，再进行积分 */
	gyro_x_dps=(float)(gx-gyro_x_bias)/16.4f;
	gyro_y_dps=(float)(gy-gyro_y_bias)/16.4f;
	roll_g=Roll+gyro_x_dps*Delta_t;
	pitch_g=Pitch+gyro_y_dps*Delta_t;

	/* 加速度计同时使用三个轴，减小另一方向倾斜带来的耦合 */
	current_roll=atan2((double)ay,(double)az)*180.0/pi-roll_zero;
	current_pitch=atan2((double)(-ax),
						 sqrt((double)ay*ay+(double)az*az))*180.0/pi-pitch_zero;

	roll_a=current_roll;
	pitch_a=current_pitch;

	/* 通过互补滤波融合陀螺仪和加速度计 */
	Roll=a*roll_g+(1-a)*roll_a;
	Pitch=a*pitch_g+(1-a)*pitch_a;

	/* Yaw保持原有逻辑不变 */
	yaw_g=Yaw+(float)gz*Delta_t;
	Yaw=a*yaw_g;
}
//显示MPU6050界面的UI
void Show_MPU6050_UI(void)
{
	OLED_ShowImage(0,0,16,16,Return);
	OLED_Printf(0,16,OLED_8X16,"Roll: %.2f",Roll);
	OLED_Printf(0,32,OLED_8X16,"Pitch:%.2f",Pitch);
	OLED_Printf(0,48,OLED_8X16,"Yaw:  %.2f",Yaw);
}

//控制光标在MPU6050界面移动的函数
int MPU6050(void)
{
	while(1)
	{
		KeyNum=Key_GetNum();
		if(KeyNum==3)
		{
			OLED_Clear();
			OLED_Update();
			return 0;
		}
		
		OLED_Clear();
		MPU6050_Calculation();
		Show_MPU6050_UI();
		OLED_ReverseArea(0,0,16,16);
		OLED_Update();
	}
}

/*----------------------------------游戏-------------------------------------*/

void Show_Game_UI(void)
{
	OLED_ShowImage(0,0,16,16,Return);
	OLED_ShowString(80,0,"高分",OLED_8X16);
	OLED_ShowString(0,16,"谷歌小恐龙",OLED_8X16);
	OLED_ShowString(0,32,"贪吃蛇",OLED_8X16);
}

void Show_HighScore_UI(void)
{
	OLED_ShowImage(0,0,16,16,Return);
	OLED_ShowString(24,0,"历史最高分",OLED_8X16);
	OLED_ShowString(0,20,"小恐龙",OLED_8X16);
	OLED_ShowNum(88,20,ScoreFlash_GetDinoHigh(),5,OLED_8X16);
	OLED_ShowString(0,40,"贪吃蛇",OLED_8X16);
	OLED_ShowNum(88,40,ScoreFlash_GetSnakeHigh(),5,OLED_8X16);
}

int HighScorePage(void)
{
	while(1)
	{
		KeyNum=Key_GetNum();
		if(KeyNum==3)
		{
			OLED_Clear();
			OLED_Update();
			return 0;
		}

		OLED_Clear();
		Show_HighScore_UI();
		OLED_Update();
	}
}
uint8_t game_flag=1;
//控制光标在游戏选择界面移动的函数
int Game(void)
{
	while(1)
	{
		KeyNum=Key_GetNum();
		uint8_t game_flag_temp=0;
		if(KeyNum==1)//上一项
		{
			game_flag--;
			if(game_flag<=0)game_flag=4;
		}
		else if(KeyNum==2)//下一项
		{
			game_flag++;
			if(game_flag>=5)game_flag=1;
		}
		else if(KeyNum==3)//确认
		{
			OLED_Clear();
			OLED_Update();
			game_flag_temp=game_flag;
		}
		
		if(game_flag_temp==1){return 0;}
		else if(game_flag_temp==2){DinoGame_Pos_Init();DinoGame_Animation();}//恐龙游戏
		else if(game_flag_temp==3){Snake_Init();SnakeGame_Animation();}//贪吃蛇
		else if(game_flag_temp==4){HighScorePage();}//历史高分
		
		switch(game_flag)
		{
			case 1:
				Show_Game_UI();
				OLED_ReverseArea(0,0,16,16);
				OLED_Update();
				break;
			
			case 2:
				Show_Game_UI();
				LED_OFF();
				OLED_ReverseArea(0,16,80,16);
				OLED_Update();
				break;
			
			case 3:
				Show_Game_UI();
				LED_OFF();
				OLED_ReverseArea(0,32,48,16);
				OLED_Update();
				break;
			
			
		
			case 4:
				Show_Game_UI();
				OLED_ReverseArea(80,0,32,16);
				OLED_Update();
				break;
					}
	}
}

/*----------------------------------动态表情包-------------------------------------*/

uint8_t animation_flag=1;
uint8_t emoji_frame;
uint8_t emoji_pause_count;
uint8_t cat_frame;

//显示原有表情动画的一帧
void Show_Emoji_UI(void)
{
	uint8_t frame=emoji_frame;

	/* 原动画最后一帧保持500ms，恢复原来的眨眼节奏 */
	if(emoji_pause_count>0)
	{
		frame=5;
	}

	OLED_Clear();
	if(frame<3)
	{
		OLED_ShowImage(30,10+frame,16,16,Eyebrow[0]);//左眉毛
		OLED_ShowImage(82,10+frame,16,16,Eyebrow[1]);//右眉毛
		OLED_DrawEllipse(40,32,6,6-frame,1);//左眼
		OLED_DrawEllipse(88,32,6,6-frame,1);//右眼
	}
	else
	{
		frame-=3;
		OLED_ShowImage(30,12-frame,16,16,Eyebrow[0]);//左眉毛
		OLED_ShowImage(82,12-frame,16,16,Eyebrow[1]);//右眉毛
		OLED_DrawEllipse(40,32,6,4+frame,1);//左眼
		OLED_DrawEllipse(88,32,6,4+frame,1);//右眼
	}
	OLED_ShowImage(54,40,20,20,Mouth);
	OLED_Update();

	if(emoji_pause_count>0)
	{
		emoji_pause_count++;
		if(emoji_pause_count>=5)
		{
			emoji_pause_count=0;
			emoji_frame=0;
		}
	}
	else
	{
		emoji_frame++;
		if(emoji_frame>=6)
		{
			emoji_pause_count=1;
		}
	}
	Delay_ms(100);
}
//显示“我很臭吗”图片动画的一帧
void Show_Cat_Video(void)
{
	OLED_Clear();
	OLED_ShowImage(32,0,64,64,cat_vedio[cat_frame]);
	OLED_Update();

	cat_frame++;
	if(cat_frame>=30)
	{
		cat_frame=0;
	}
	Delay_ms(50);
}

//用按键控制动画切换和退出动态表情包界面
int Emoji(void)
{
	animation_flag=1;
	emoji_frame=0;
	emoji_pause_count=0;
	cat_frame=0;

	while(1)
	{
		KeyNum=Key_GetNum();
		if(KeyNum==1)
		{
			if(animation_flag==1) animation_flag=2;
			else animation_flag=1;
		}
		else if(KeyNum==2)
		{
			if(animation_flag==2) animation_flag=1;
			else animation_flag=2;
		}
		else if(KeyNum==3)
		{
			OLED_Clear();
			OLED_Update();
			return 0;
		}

		if(animation_flag==1)
		{
			Show_Emoji_UI();
		}
		else
		{
			Show_Cat_Video();
		}
	}
}
/*----------------------------------水平仪-------------------------------------*/
//显示水平仪
void Show_Gradienter_UI(void)
{
	MPU6050_Calculation();
	OLED_DrawCircle(64,32,30,0);
	OLED_DrawCircle(64-Roll,32+Pitch,4,1);
}

//用按键控制退出水平仪界面的函数
int Gradienter(void)
{
	while(1)
	{
		KeyNum=Key_GetNum();
		if(KeyNum==3)
		{
			OLED_Clear();
			OLED_Update();
			return 0;
		}
		OLED_Clear();
		Show_Gradienter_UI();
		OLED_Update();
	}
}