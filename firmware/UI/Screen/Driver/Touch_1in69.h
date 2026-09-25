/*****************************************************************************
* | File      	:   Touch_1IN69.h
* | Author      :   Waveshare team
* | Function    :   Hardware underlying interface
* | Info        :
*                Used to shield the underlying layers of each master 
*                and enhance portability
*----------------
* |	This version:   V1.0
* | Date        :   2022-12-02
* | Info        :   Basic version
*
******************************************************************************/
#ifndef __Touch_1IN69_H
#define __Touch_1IN69_H	
#include "DEV_Config.h"
#include <stdlib.h>		//itoa()
#include <stdio.h>

#define address   0x15  //从机地址

#define GESTUREID 0x01 //手势码
#define None 0x00 		//无手势
#define Down 0x01 		//下滑
#define UP 0x02 		//上滑
#define LEFT 0x03		//左滑
#define RIGHT 0x04		//右滑
#define CLICK 0x05		//单击
#define DOUBLE_CLICK 0x0B//双击
#define LONG_PRESS 0x0C	//长按

#define FingerNum 0X02
#define XposH 0x03 //X坐标高4位
#define XposL 0x04 //X坐标低8位
#define YposH 0x05 //Y坐标高4位
#define YposL 0x06 //Y坐标低8位

#define BPC0H 0xB0 //BPC0值的高8位
#define BPC0L 0xB1 //BPC0值的低8位
#define BPC1H 0xB2 //BPC1值的高8位
#define BPC1L 0xB3 //BPC1值的低8位
#define ChipID 0xA7 //芯片型号
#define ProjID 0xA8 //工程编号

#define FwVersion 0xA9 //软件版本号

#define MotionMask 0xEC 
#define EnConLR  0x04 //使能连续左右滑动的动作
#define EnConUD  0x02 //使能连续上下滑动的动作
#define EnDClick 0x01 //使能双击的动作

#define IrqPluseWidth 0xED //中断低脉冲输出宽度  单位0.1ms，可选值：1～200 默认值为10

#define NorScanPer 0xEE //正常快速检测周期 单位10ms，可选值：1～30 默认值为1

#define MotionSlAngle 0xEF //手势检测滑动分区角度控制 Angle=tan(c)*10
							//c为以x轴正方向为基准的角
#define LpScanRaw1H 0xF0 //低功耗扫描1号通道的基准值的高8位
#define LpScanRaw1L 0xF1 //低功耗扫描1号通道的基准值的低8位
#define LpScanRaw2H 0xF2 //低功耗扫描1号通道的基准值的高8位
#define LpScanRaw2L 0xF3 //低功耗扫描1号通道的基准值的低8位

#define LpAutoWakeTime 0xF4 //低功耗时自动重校正周期 单位1分钟，可选值：1～5。默认值为5
#define LpScanTH 0xF5 //低功耗扫描唤醒门限。越小越灵敏 可选值：1～255。默认值为48
#define LpScanWin 0xF6 //低功耗扫描量程。越大越灵敏，功耗越高 可选值：0，1，2，3。默认值为3
#define LpScanFreq 0xF7 //低功耗扫描频率 越小越灵敏 可选值：1～255。默认值为7
#define LpScanIdac 0xF8 //低功耗扫描电流 越小越灵敏 可选值：1～255
#define AutoSleepTime 0xF9 //x秒内无触摸时 自动进入低功耗模式 单位1S，默认值为2S

#define IrqCtl 0xFA 
#define EnTest 0x80 //中断引脚测试，使能后自动周期性发出低脉冲
#define EnTouch 0x40 //检测到触摸时，周期性发出低脉冲
#define EnChange 0x20 //检测到触摸状态变化时，发出低脉冲
#define EnMotion 0x10 //检测到手势时，发出低脉冲
#define OnceWLP 0x00 //长按手势只发出一个低脉冲信号

#define AutoReset 0xFB //x秒内有触摸但无有效手势时 自动复位 单位1S 为0时不启用此功能 默认为5
#define LongPressTime 0xFC //长按x秒后自动复位 单位1S 为0时不启用此功能 默认为10
#define IOCtl 0xFD //
#define SOFT_RST 0x04 //使能软复位
#define IIC_OD 0x02 //OD
#define En1v8  0x01 //1.8V

#define DisAutoSleep 0xFE //默认为0 使能自动进入低功耗模式 为非0值时 禁止自动进入低功耗模式
 


typedef struct{
	UBYTE mode;
	UBYTE Gesture;
	UWORD color;
	UWORD x_point;
	UWORD y_point;

}Touch_1IN69_XY;

extern Touch_1IN69_XY XY;

typedef enum {
    TOUCH_INIT  = 0,
    TOUCH_IRQ,
    TOUCH_FUNCTION,
	TOUCH_DRAW,
	TOUCH_OUT_GESTURE,
	TOUCH_NO_DRAW,
} Touch_STATE;


UBYTE Touch_1IN69_init(UBYTE mode);
Touch_1IN69_XY Touch_1IN69_Get_Point();

#endif