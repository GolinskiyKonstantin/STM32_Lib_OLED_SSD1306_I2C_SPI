/*
  ******************************************************************************
  * @file 			( фаил ):   ssd1306.h
  * @brief 		( описание ):  	
  ******************************************************************************
  * @attention 	( внимание ):
  ******************************************************************************
  
 */
 
 
/*

		ВНИМАНИЕ!!! Если на екране справа артефакты на два пикселя или изображение смещенно на два пикселя влево
		то ето драйвер не SSD1306_128x64, а драйвер SH1106_132x64 он имеет разрешение 
		больше вместо 128 имеет 132 по 2 пикселя больше по краям, но сам экран имеет 128 отсюда и сдвиг
		поэтому указываем драйвер SH1106_132x64 и все будет ОК
		Чаще всего это экраны диагональю 1.3 дюйма
		
*/
 
#ifndef _SSD1306_H
#define _SSD1306_H

/* C++ detection */
#ifdef __cplusplus
extern C {
#endif

#include "main.h"

#include "fonts.h"

#include "stdlib.h"
#include "string.h"
#include "math.h"

//#######  SETUP  ############################################################################################################


			//=== SETUP настройки экрана =========================================================
			
			// выбираем тип подключения I2C or SPI
				#define	SSD1306_USE_I2C
			//	#define	SSD1306_USE_SPI

			// выбираем разрешение экрана ( нужное раскомментировать ) ---------------
			
			//	#define	SSD1306_64x32
			//	#define	SSD1306_64x48
			//	#define	SSD1306_72x40
			//	#define	SSD1306_96x16
			//	#define	SSD1306_128x32
				#define	SSD1306_128x64
			//	#define	SH1106_128x64
			//------------------------------------------------------------------
			
			
			// I2C config - указываем номер шины I2C	--------
			#if defined(SSD1306_USE_I2C)
				#ifndef SSD1306_I2C_PORT
					#define SSD1306_I2C_PORT		hi2c1
				#endif

				// указываем I2C address -------------
				#ifndef SSD1306_I2C_ADDR
					#define SSD1306_I2C_ADDR         0x78
					//#define SSD1306_I2C_ADDR       0x7A
				#endif

			//---------------------------------------------------
			
			
			// SPI config - указываем номер SPI и пины ----------
			#elif defined(SSD1306_USE_SPI)
				#ifndef SSD1306_SPI_PORT
					#define SSD1306_SPI_PORT				hspi1
				#endif
				
				#ifndef SSD1306_CS_GPIO_Port
					#define SSD1306_CS_GPIO_Port         	GPIOA
				#endif
				#ifndef SSD1306_CS_Pin
					#define SSD1306_CS_Pin          		GPIO_PIN_12
				#endif

				#ifndef SSD1306_DC_GPIO_Port
					#define SSD1306_DC_GPIO_Port         	GPIOA
				#endif
				#ifndef SSD1306_DC_Pin
					#define SSD1306_DC_Pin          		GPIO_PIN_14
				#endif

				#ifndef SSD1306_RESET_GPIO_Port
					#define SSD1306_RESET_GPIO_Port      	GPIOB
				#endif
				#ifndef SSD1306_RESET_Pin
					#define SSD1306_RESET_Pin       		GPIO_PIN_8
				#endif
			
			//---------------------------------------------------
			#else
				#error "You should define SSD1306_USE_SPI or SSD1306_USE_I2C macro"
			#endif
			
			//====================================================================================
			



//#########################################################################################################################

#ifdef SSD1306_64x32
/* SSD1306 width in pixels */
#ifndef SSD1306_WIDTH
	#define SSD1306_WIDTH	64
#endif
/* SSD1306 LCD height in pixels */
#ifndef SSD1306_HEIGHT
	#define SSD1306_HEIGHT	32
#endif

#elif defined SSD1306_64x48
/* SSD1306 width in pixels */
#ifndef SSD1306_WIDTH
	#define SSD1306_WIDTH	64
#endif
/* SSD1306 LCD height in pixels */
#ifndef SSD1306_HEIGHT
	#define SSD1306_HEIGHT	48
#endif

#elif defined SSD1306_72x40
/* SSD1306 width in pixels */
#ifndef SSD1306_WIDTH
	#define SSD1306_WIDTH	72
#endif
/* SSD1306 LCD height in pixels */
#ifndef SSD1306_HEIGHT
	#define SSD1306_HEIGHT	40
#endif

#elif defined SSD1306_96x16
/* SSD1306 width in pixels */
#ifndef SSD1306_WIDTH
	#define SSD1306_WIDTH	96
#endif
/* SSD1306 LCD height in pixels */
#ifndef SSD1306_HEIGHT
	#define SSD1306_HEIGHT	16
#endif

#elif defined SSD1306_128x32
/* SSD1306 width in pixels */
#ifndef SSD1306_WIDTH
	#define SSD1306_WIDTH	128
#endif
/* SSD1306 LCD height in pixels */
#ifndef SSD1306_HEIGHT
	#define SSD1306_HEIGHT	32
#endif

#elif defined SSD1306_128x64
/* SSD1306 width in pixels */
#ifndef SSD1306_WIDTH
	#define SSD1306_WIDTH	128
#endif
/* SSD1306 LCD height in pixels */
#ifndef SSD1306_HEIGHT
	#define SSD1306_HEIGHT	64
#endif

#elif defined SH1106_128x64
/* SH1106 width in pixels */
#ifndef SSD1306_WIDTH
	#define SSD1306_WIDTH	128
#endif
/* SH1106 LCD height in pixels */
#ifndef SSD1306_HEIGHT
	#define SSD1306_HEIGHT	64
#endif


#endif


#define PI 	3.14159265

//--------------------------------------------------------------------------------------

#define SSD1306_RIGHT_HORIZONTAL_SCROLL              0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL               0x27
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL  0x2A
#define SSD1306_DEACTIVATE_SCROLL                    0x2E 	// Stop scroll
#define SSD1306_ACTIVATE_SCROLL                      0x2F 	// Start scroll
#define SSD1306_SET_VERTICAL_SCROLL_AREA             0xA3 	// Set scroll range


//---  SSD1306  ------------------------------------------------------------------------------------------------------------

#define SSD1306_SETLOWCOLUMN 					0x00				/** Set Lower Column Start Address for Page Addressing Mode. */
#define SSD1306_SETHIGHCOLUMN 				0x10				/** Set Higher Column Start Address for Page Addressing Mode. */
#define SSD1306_MEMORYMODE 						0x20				/** Set Memory Addressing Mode. */
#define SSD1306_SETSTARTLINE 					0x40				/** Set display RAM display start line register from 0 - 63. */
#define SSD1306_SETCONTRAST 					0x81				/** Set Display Contrast to one of 256 steps. */
#define SSD1306_CHARGEPUMP 						0x8D				/** Enable or disable charge pump.  Follow with 0X14 enable, 0X10 disable. */
#define SSD1306_SEGREMAP 							0xA0				/** Set Segment Re-map between data column and the segment driver. */
#define SSD1306_DISPLAYALLON_RESUME 	0xA4				/** Resume display from GRAM content. */
#define SSD1306_DISPLAYALLON 					0xA5				/** Force display on regardless of GRAM content. */
#define SSD1306_NORMALDISPLAY 				0xA6				/** Set Normal Display. */
#define SSD1306_INVERTDISPLAY 				0xA7				/** Set Inverse Display. */
#define SSD1306_SETMULTIPLEX 					0xA8				/** Set Multiplex Ratio from 16 to 63. */
#define SSD1306_DISPLAYOFF 						0xAE				/** Set Display off. */
#define SSD1306_DISPLAYON 						0xAF				/** Set Display on. */
#define SSD1306_SETSTARTPAGE 					0XB0				/**Set GDDRAM Page Start Address. */
#define SSD1306_COMSCANINC 						0xC0				/** Set COM output scan direction normal. */
#define SSD1306_COMSCANDEC 						0xC8				/** Set COM output scan direction reversed. */
#define SSD1306_SETDISPLAYOFFSET 			0xD3				/** Set Display Offset. */
#define SSD1306_SETCOMPINS 						0xDA				/** Sets COM signals pin configuration to match the OLED panel layout. */
#define SSD1306_SETVCOMDETECT 				0xDB				/** This command adjusts the VCOMH regulator output. */
#define SSD1306_SETDISPLAYCLOCKDIV 		0xD5				/** Set Display Clock Divide Ratio/ Oscillator Frequency. */
#define SSD1306_SETPRECHARGE 					0xD9				/** Set Pre-charge Period */
#define SSD1306_DEACTIVATE_SCROLL 		0x2E				/** Deactivate scroll */
#define SSD1306_NOP 									0XE3				/** No Operation Command. */

//---------------------------------------------------------------------------------------------------------------------------

//--  SH1106 128x64 ---------------------------------------------------------------------------------------------------------

#define SH1106_SET_PUMP_VOLTAGE 		0X30				/** Set Pump voltage value: (30H~33H) 6.4, 7.4, 8.0 (POR), 9.0. */
#define SH1106_SET_PUMP_MODE 				0XAD				/** First byte of set charge pump mode */
#define SH1106_PUMP_ON 							0X8B				/** Second byte charge pump on. */
#define SH1106_PUMP_OFF 						0X8A				/** Second byte charge pump off. */
  
//---------------------------------------------------------------------------------------------------------------------------

/**
 * @brief  SSD1306 color enumeration
 */
typedef enum {
	SSD1306_COLOR_BLACK = 0x00, /*!< Black color, no pixel */
	SSD1306_COLOR_WHITE = 0x01  /*!< Pixel is set. Color depends on LCD */
} SSD1306_COLOR_t;



void ssd1306_Reset(void);
uint8_t SSD1306_Init(void);
void SSD1306_UpdateScreen(void);
void SSD1306_ToggleInvert(void);
void SSD1306_Fill(SSD1306_COLOR_t Color);
void SSD1306_DrawPixel(uint16_t x, uint16_t y, SSD1306_COLOR_t color);
void SSD1306_GotoXY(uint16_t x, uint16_t y);
void SSD1306_Putc(char ch, FontDef_t* Font, SSD1306_COLOR_t color);
void SSD1306_Puts(char* str, FontDef_t* Font, SSD1306_COLOR_t color);
void SSD1306_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, SSD1306_COLOR_t c);
void SSD1306_DrawRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, SSD1306_COLOR_t c);
void SSD1306_DrawFilledRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, SSD1306_COLOR_t c);
void SSD1306_DrawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, SSD1306_COLOR_t color);
void SSD1306_DrawCircle(int16_t x0, int16_t y0, int16_t r, SSD1306_COLOR_t c);
void SSD1306_DrawFilledCircle(int16_t x0, int16_t y0, int16_t r, SSD1306_COLOR_t c);
void ssd1306_WriteCommand(uint8_t data);
void ssd1306_WriteData(uint8_t *data, uint16_t count);
void SSD1306_DrawBitmap(int16_t x, int16_t y, const unsigned char* bitmap, int16_t w, int16_t h, SSD1306_COLOR_t color);
void SSD1306_ScrollRight(uint8_t start_row, uint8_t end_row);
void SSD1306_ScrollLeft(uint8_t start_row, uint8_t end_row);
void SSD1306_Scrolldiagright(uint8_t start_row, uint8_t end_row);
void SSD1306_Scrolldiagleft(uint8_t start_row, uint8_t end_row);
void SSD1306_Stopscroll(void);
void SSD1306_InvertDisplay (int i);
void SSD1306_Clear (void);
void SSD1306_Contrast(uint8_t value );
void SSD1306_MirrorReflection(uint8_t value );
void SSD1306_ScreenRotation(uint8_t value );
void SSD1306_DrawCircleHelper(int16_t x0, int16_t y0, int16_t radius, int8_t quadrantMask, SSD1306_COLOR_t color);
void SSD1306_DrawFillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t corners, int16_t delta, SSD1306_COLOR_t color);
void SSD1306_DrawFillRoundRect(int16_t x, int16_t y, uint16_t width, uint16_t height, int16_t cornerRadius, SSD1306_COLOR_t color);
void SSD1306_DrawRoundRect(int16_t x, int16_t y, uint16_t width, uint16_t height, int16_t cornerRadius, SSD1306_COLOR_t color);
void SSD1306_DrawLineThick(int16_t x1, int16_t y1, int16_t x2, int16_t y2, SSD1306_COLOR_t color, uint8_t thick) ;
void SSD1306_DrawArc(int16_t x0, int16_t y0, int16_t radius, int16_t startAngle, int16_t endAngle, SSD1306_COLOR_t color, uint8_t thick);
void SSD1306_DrawLineThickWithAngle(int16_t x, int16_t y, int16_t length, double angle_degrees, SSD1306_COLOR_t color, uint8_t thick);
void SSD1306_DrawBitmapWithAngle(int16_t x, int16_t y, const unsigned char* bitmap, int16_t w, int16_t h, SSD1306_COLOR_t color, double angle_degrees);
void SSD1306_PutcWithAngle(char ch, FontDef_t* Font, SSD1306_COLOR_t color, double angle_degrees);
void SSD1306_PutsWithAngle(char* str, FontDef_t* Font, SSD1306_COLOR_t color, double angle_degrees);
void SSD1306_DrawEllipse(int16_t x0, int16_t y0, int16_t radiusX, int16_t radiusY, SSD1306_COLOR_t color);
void SSD1306_DrawEllipseWithAngle(int16_t x0, int16_t y0, int16_t radiusX, int16_t radiusY, float angle_degrees, SSD1306_COLOR_t color);
void SSD1306_DrawEllipseFilled(int16_t x0, int16_t y0, int16_t radiusX, int16_t radiusY, SSD1306_COLOR_t color);
void SSD1306_DrawEllipseFilledWithAngle(int16_t x0, int16_t y0, int16_t radiusX, int16_t radiusY, float angle_degrees, SSD1306_COLOR_t color);
void SSD1306_DrawLineWithAngle(int16_t x, int16_t y, uint16_t length, double angle_degrees, SSD1306_COLOR_t color);

/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif	/*	_SSD1306_H */

/************************ (C) COPYRIGHT GKP *****END OF FILE****/
