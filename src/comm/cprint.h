#ifndef __COMM__CPRINT_H
#define __COMM__CPRINT_H
/*
 * color print
 */
#include <stdarg.h>
/*
 * 背景色：
 * 	40:黑 41:深红 42:绿 43:黄色 44:蓝色 45:紫色 46:深绿 47:白色
 * 前景色：
 * 	30:黑 31:红 32:绿 33:黄 34:蓝色 35:紫色 36:深绿 37:白色
 */
/*
 * foreground color
 */
typedef enum{
	BLACK_F 	= 30,	//black
	RED_F 		= 31,	//red
	GREEN_F		= 32,	//green
	YELLOW_F 	= 33,	//yellow
	BLUE_F		= 34,	//blue
	PURPLE_F 	= 35,	//purple
	DARKBLUE_F	= 36,	//dark blue
	WHITE_F		= 37,	//white
	UNSET_F		= -1	//do not set the foreground color
}FgColor;

/*
 * background color
 */
typedef enum{
	BLACK_B		=40,	//black
	DARKRED_B	=41,	//dark red
	GREEN_B		=42,	//green
	YELLOW_B	=43,	//yellow
	BLUE_B		=44,	//blue
	PURPLE_B	=45,	//purple
	DARKGREEN_B	=46,	//dark green
	WHITE_B		=47,	//white
	UNSET_B		= -1	//do not set the background color
}BgColor;

/*
 * The type of the string.
 * 	\033[1m       设置高亮度
 * 	\033[4m       下划线
 * 	\033[5m       闪烁
 * 	\033[7m       反显
 * 	\033[8m       消隐
 */

typedef enum{
	HIGHLIGHT	= 1,	//highlight
	UNDERLINE	= 4,	//under line
	BLINKING	= 5,	//blink
	REVERSE		= 7,	//reverse
	HIDE  		= 8,	//hide 消隐
	UNSET_T		= -1	//do not set the print type
}Ptype;

/*
 * The same as printf.
 * It can print color string on the console.
 */
int cprintf(FgColor fc, BgColor bc, Ptype t
		, const char *fmt, ...);

int fvcprintf(int fd, FgColor fc, BgColor bc, Ptype t
		, const char *fmt, va_list ap);
		
int fcprintf(int fd, FgColor fc, BgColor bc, Ptype t
		, const char *fmt,...);

#endif
