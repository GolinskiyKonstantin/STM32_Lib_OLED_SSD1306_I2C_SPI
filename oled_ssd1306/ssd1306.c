/*

  ******************************************************************************
  * @file 			( фаил ):   ssd1306.c
  * @brief 		( описание ):  	
  ******************************************************************************
  * @attention 	( внимание ):
  ******************************************************************************
  
*/
 
#include "ssd1306.h"

#if defined(SSD1306_USE_I2C)
	extern I2C_HandleTypeDef SSD1306_I2C_PORT;
#elif defined(SSD1306_USE_SPI)
	extern SPI_HandleTypeDef SSD1306_SPI_PORT;
#else
#error "You should define SSD1306_USE_SPI or SSD1306_USE_I2C macro"
#endif


/* Absolute value */
#define ABS(x)   ((x) > 0 ? (x) : -(x))

/* SSD1306 data buffer */
static uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];


//----------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
/* Private SSD1306 structure */
typedef struct {
	uint16_t CurrentX;
	uint16_t CurrentY;
	uint8_t Inverted;
	uint8_t Initialized;
} SSD1306_t;

/* Private variable */
static SSD1306_t SSD1306;

//----------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
void SSD1306_ScrollRight(uint8_t start_row, uint8_t end_row)
{
  ssd1306_WriteCommand (SSD1306_RIGHT_HORIZONTAL_SCROLL);  // send 0x26
  ssd1306_WriteCommand (0x00);  // send dummy
  ssd1306_WriteCommand (start_row);  // start page address
  ssd1306_WriteCommand (0X00);  // time interval 5 frames
  ssd1306_WriteCommand (end_row);  // end page address
  ssd1306_WriteCommand (0X00);
  ssd1306_WriteCommand (0XFF);
  ssd1306_WriteCommand (SSD1306_ACTIVATE_SCROLL); // start scroll
}

//----------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
void SSD1306_ScrollLeft(uint8_t start_row, uint8_t end_row)
{
  ssd1306_WriteCommand (SSD1306_LEFT_HORIZONTAL_SCROLL);  // send 0x26
  ssd1306_WriteCommand (0x00);  // send dummy
  ssd1306_WriteCommand (start_row);  // start page address
  ssd1306_WriteCommand (0X00);  // time interval 5 frames
  ssd1306_WriteCommand (end_row);  // end page address
  ssd1306_WriteCommand (0X00);
  ssd1306_WriteCommand( 0XFF);
  ssd1306_WriteCommand (SSD1306_ACTIVATE_SCROLL); // start scroll
}

//----------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
void SSD1306_Scrolldiagright(uint8_t start_row, uint8_t end_row)
{
  ssd1306_WriteCommand (SSD1306_SET_VERTICAL_SCROLL_AREA);  // sect the area
  ssd1306_WriteCommand (0x00);   // write dummy
  ssd1306_WriteCommand( SSD1306_HEIGHT);

  ssd1306_WriteCommand (SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL);
  ssd1306_WriteCommand (0x00);
  ssd1306_WriteCommand (start_row);
  ssd1306_WriteCommand (0X00);
  ssd1306_WriteCommand (end_row);
  ssd1306_WriteCommand (0x01);
  ssd1306_WriteCommand (SSD1306_ACTIVATE_SCROLL);
}

//----------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
void SSD1306_Scrolldiagleft(uint8_t start_row, uint8_t end_row)
{
  ssd1306_WriteCommand (SSD1306_SET_VERTICAL_SCROLL_AREA);  // sect the area
  ssd1306_WriteCommand (0x00);   // write dummy
  ssd1306_WriteCommand (SSD1306_HEIGHT);

  ssd1306_WriteCommand (SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL);
  ssd1306_WriteCommand (0x00);
  ssd1306_WriteCommand (start_row);
  ssd1306_WriteCommand (0X00);
  ssd1306_WriteCommand (end_row);
  ssd1306_WriteCommand (0x01);
  ssd1306_WriteCommand (SSD1306_ACTIVATE_SCROLL);
}

//----------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
void SSD1306_Stopscroll(void)
{
	ssd1306_WriteCommand(SSD1306_DEACTIVATE_SCROLL);
}


//----------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
void SSD1306_InvertDisplay (int i)
{
  if (i) ssd1306_WriteCommand (SSD1306_INVERTDISPLAY);

  else ssd1306_WriteCommand (SSD1306_NORMALDISPLAY);

}

//----------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
void SSD1306_DrawBitmap(int16_t x, int16_t y, const unsigned char* bitmap, int16_t w, int16_t h, SSD1306_COLOR_t color)
{

    int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t byte = 0;

    for(int16_t j=0; j<h; j++, y++)
    {
        for(int16_t i=0; i<w; i++)
        {
            if(i & 7)
            {
               byte <<= 1;
            }
            else
            {
               byte = (*(const unsigned char *)(&bitmap[j * byteWidth + i / 8]));
            }
            if(byte & 0x80) SSD1306_DrawPixel(x+i, y, color);
        }
    }
}







//----------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
uint8_t SSD1306_Init(void) {
	
	ssd1306_Reset();
	
	#if defined(SSD1306_USE_I2C)
	// Init I2C ------------------------------------------------------------------
	ssd1306_I2C_Init();
	
	/* Check if LCD connected to I2C */
	if (HAL_I2C_IsDeviceReady(&SSD1306_I2C_PORT, SSD1306_I2C_ADDR, 1, 20000) != HAL_OK) {
		/* Return false */
		return 0;
	}
	//----------------------------------------------------------------------------
	
	#elif defined(SSD1306_USE_SPI)
	
	
	
	
	#else
	#error "You should define SSD1306_USE_SPI or SSD1306_USE_I2C macro"
	#endif
	
	
	/* A little delay */
	uint32_t p = 2500;
	while(p>0)
		p--;
	
	/* Init LCD */

#ifdef SSD1306_64x48
//== Init sequence for 64x48 Micro OLED module ============================
    ssd1306_WriteCommand( SSD1306_DISPLAYOFF );
    ssd1306_WriteCommand( SSD1306_SETDISPLAYCLOCKDIV );
	ssd1306_WriteCommand( 0x80 );  								// the suggested ratio 0x80
    ssd1306_WriteCommand( SSD1306_SETMULTIPLEX );
	ssd1306_WriteCommand( 0x2F );        
    ssd1306_WriteCommand( SSD1306_SETDISPLAYOFFSET );
	ssd1306_WriteCommand( 0x0 );     							// no offset
    ssd1306_WriteCommand( SSD1306_SETSTARTLINE | 0x0 );       	// line #0
    ssd1306_WriteCommand( SSD1306_CHARGEPUMP );
	ssd1306_WriteCommand( 0x14 );          						// internal vcc
    ssd1306_WriteCommand( SSD1306_NORMALDISPLAY );
    ssd1306_WriteCommand( SSD1306_DISPLAYALLON_RESUME );
    ssd1306_WriteCommand( SSD1306_SEGREMAP | 0x1 );            	// column 127 mapped to SEG0
    ssd1306_WriteCommand( SSD1306_COMSCANDEC );                	// column scan direction reversed
    ssd1306_WriteCommand( SSD1306_SETCOMPINS );
	ssd1306_WriteCommand( 0x12 );          						// 0x12 if height > 32 else 0x02
    ssd1306_WriteCommand( SSD1306_SETCONTRAST );
	ssd1306_WriteCommand( 0x7F );         						// contrast level 127
    ssd1306_WriteCommand( SSD1306_SETPRECHARGE );
	ssd1306_WriteCommand( 0xF1 );        						// pre-charge period (1, 15)
    ssd1306_WriteCommand( SSD1306_SETVCOMDETECT );
	ssd1306_WriteCommand( 0x40 );       						// vcomh regulator level
    ssd1306_WriteCommand( SSD1306_DISPLAYON );
//=======================================================================

#elif defined SSD1306_96x16	
//== Init sequence for SSD1306 96x16 OLED module ========================
    ssd1306_WriteCommand( SSD1306_DISPLAYOFF );
    ssd1306_WriteCommand( SSD1306_SETDISPLAYCLOCKDIV );
	ssd1306_WriteCommand( 0x80 );  								// clock divide ratio and osc frequency
    ssd1306_WriteCommand( SSD1306_SETMULTIPLEX );
	ssd1306_WriteCommand( 0x0F );        						// multiplex ratio
    ssd1306_WriteCommand( SSD1306_SETDISPLAYOFFSET );
	ssd1306_WriteCommand( 0x0 );     							// display offset zero
    ssd1306_WriteCommand( SSD1306_SETSTARTLINE | 0x0 );        	// set display start line to 0
    ssd1306_WriteCommand( SSD1306_CHARGEPUMP );
	ssd1306_WriteCommand( 0x14 );          						// charge pump setting enable
    ssd1306_WriteCommand( SSD1306_MEMORYMODE );
	ssd1306_WriteCommand( 0x00 );          						// page addressing mode
    ssd1306_WriteCommand( SSD1306_SEGREMAP | 0xA1 );           	// segment remap
    ssd1306_WriteCommand( SSD1306_COMSCANDEC );                	// scan dir reverse
    ssd1306_WriteCommand( SSD1306_SETCOMPINS );
	ssd1306_WriteCommand( 0x02 );          						// com pin HW config
    ssd1306_WriteCommand( SSD1306_SETCONTRAST );
	ssd1306_WriteCommand( 0xAF );         						// set contrast level 0xaf
    ssd1306_WriteCommand( SSD1306_SETPRECHARGE );
	ssd1306_WriteCommand( 0xF1 );        						// pre-charge period 0x0f1
    ssd1306_WriteCommand( SSD1306_SETVCOMDETECT );
	ssd1306_WriteCommand( 0x20 );       						// vcomh deselect level
    ssd1306_WriteCommand( SSD1306_DEACTIVATE_SCROLL );         	//  Deactivate scroll
    ssd1306_WriteCommand( SSD1306_DISPLAYALLON_RESUME );
    ssd1306_WriteCommand( SSD1306_NORMALDISPLAY );
    ssd1306_WriteCommand( SSD1306_DISPLAYON );
//=======================================================================	

#elif defined SSD1306_128x32
//== Init sequence for SSD1306 128x32 OLED module ======================
    ssd1306_WriteCommand( SSD1306_DISPLAYOFF );
    ssd1306_WriteCommand( SSD1306_SETDISPLAYCLOCKDIV );
	ssd1306_WriteCommand( 0x80 );  								// the suggested ratio 0x80
    ssd1306_WriteCommand( SSD1306_SETMULTIPLEX );
	ssd1306_WriteCommand( 0x1F );        						// ratio 32
    ssd1306_WriteCommand( SSD1306_SETDISPLAYOFFSET );
	ssd1306_WriteCommand( 0x0 );     							// no offset
    ssd1306_WriteCommand( SSD1306_SETSTARTLINE | 0x0 );        	// line #0
    ssd1306_WriteCommand( SSD1306_CHARGEPUMP );
	ssd1306_WriteCommand( 0x14 );          						// internal vcc
    ssd1306_WriteCommand( SSD1306_MEMORYMODE );
	ssd1306_WriteCommand( 0x02 );          						// page mode
    ssd1306_WriteCommand( SSD1306_SEGREMAP | 0x1 );            	// column 127 mapped to SEG0
    ssd1306_WriteCommand( SSD1306_COMSCANDEC );                	// column scan direction reversed
    ssd1306_WriteCommand( SSD1306_SETCOMPINS );
	ssd1306_WriteCommand( 0x02 );          						// sequential COM pins, disable remap
    ssd1306_WriteCommand( SSD1306_SETCONTRAST );
	ssd1306_WriteCommand( 0x7F );         						// contrast level 127
    ssd1306_WriteCommand( SSD1306_SETPRECHARGE );
	ssd1306_WriteCommand( 0xF1 );        						// pre-charge period (1, 15)
    ssd1306_WriteCommand( SSD1306_SETVCOMDETECT );
	ssd1306_WriteCommand( 0x40 );       						// vcomh regulator level
    ssd1306_WriteCommand( SSD1306_DISPLAYALLON_RESUME );
    ssd1306_WriteCommand( SSD1306_NORMALDISPLAY );
    ssd1306_WriteCommand( SSD1306_DISPLAYON );
//=======================================================================	

#elif defined SSD1306_128x64
//== Init sequence for SSD1306 128x64 OLED module ======================
    ssd1306_WriteCommand( SSD1306_DISPLAYOFF );
    ssd1306_WriteCommand( SSD1306_SETDISPLAYCLOCKDIV );
	ssd1306_WriteCommand( 0x80 );  								// the suggested ratio 0x80 
    ssd1306_WriteCommand( SSD1306_SETMULTIPLEX );
	ssd1306_WriteCommand( 0x3F );        						// ratio 64
    ssd1306_WriteCommand( SSD1306_SETDISPLAYOFFSET );
	ssd1306_WriteCommand( 0x0 );     							// no offset
    ssd1306_WriteCommand( SSD1306_SETSTARTLINE | 0x0 );        	// line #0	// подымаем картинку вверх
    ssd1306_WriteCommand( SSD1306_CHARGEPUMP );
	ssd1306_WriteCommand( 0x14 );          						// internal vcc
    ssd1306_WriteCommand( SSD1306_MEMORYMODE );
	ssd1306_WriteCommand( 0x02 );          						// page mode
    ssd1306_WriteCommand( SSD1306_SEGREMAP | 0x1 );            	// column 127 mapped to SEG0	// двигаем картинку вправо
    ssd1306_WriteCommand( SSD1306_COMSCANDEC );                	// column scan direction reversed
    ssd1306_WriteCommand( SSD1306_SETCOMPINS );
	ssd1306_WriteCommand( 0x12 );          						// alt COM pins, disable remap
    ssd1306_WriteCommand( SSD1306_SETCONTRAST );
	ssd1306_WriteCommand( 0x7F );         						// contrast level 127
    ssd1306_WriteCommand( SSD1306_SETPRECHARGE );
	ssd1306_WriteCommand( 0xF1 );        						// pre-charge period (1, 15)
    ssd1306_WriteCommand( SSD1306_SETVCOMDETECT );
	ssd1306_WriteCommand( 0x40 );       						// vcomh regulator level
    ssd1306_WriteCommand( SSD1306_DISPLAYALLON_RESUME );
    ssd1306_WriteCommand( SSD1306_NORMALDISPLAY );
    ssd1306_WriteCommand( SSD1306_DISPLAYON );
//=======================================================================	

#elif defined SH1106_128x64
//== Init sequence for SH1106 128x64 OLED module ======================

  ssd1306_WriteCommand( SSD1306_DISPLAYOFF );
  ssd1306_WriteCommand( SSD1306_SETSTARTPAGE | 0X0 );            	// set page address  
  ssd1306_WriteCommand( SSD1306_SETCONTRAST ); 
  ssd1306_WriteCommand( 0x80 );             						// 128
  ssd1306_WriteCommand( SSD1306_SEGREMAP | 0X1 );                	// set segment remap
  ssd1306_WriteCommand( SSD1306_NORMALDISPLAY );                 	// normal / reverse
  ssd1306_WriteCommand( SSD1306_SETMULTIPLEX ); 
  ssd1306_WriteCommand( 0x3F );            							// ratio 64
  ssd1306_WriteCommand( SH1106_SET_PUMP_MODE );
  ssd1306_WriteCommand( SH1106_PUMP_ON );  							// set charge pump enable
  ssd1306_WriteCommand( SH1106_SET_PUMP_VOLTAGE | 0X2 );         	// 8.0 volts
  ssd1306_WriteCommand( SSD1306_COMSCANDEC );                    	// Com scan direction
  ssd1306_WriteCommand( SSD1306_SETDISPLAYOFFSET ); 
  ssd1306_WriteCommand( 0X00 );        								// set display offset
  ssd1306_WriteCommand( SSD1306_SETDISPLAYCLOCKDIV ); 
  ssd1306_WriteCommand( 0X80 );      								// set osc division
  ssd1306_WriteCommand( SSD1306_SETPRECHARGE ); 
  ssd1306_WriteCommand( 0X1F );            							// set pre-charge period
  ssd1306_WriteCommand( SSD1306_SETCOMPINS ); 
  ssd1306_WriteCommand( 0X12 );              						// set COM pins
  ssd1306_WriteCommand( SSD1306_SETVCOMDETECT );  
  ssd1306_WriteCommand( 0x40 );          							// set vcomh
  ssd1306_WriteCommand( SSD1306_DISPLAYON );
//=======================================================================	
#endif	  
  
	ssd1306_WriteCommand(SSD1306_DEACTIVATE_SCROLL);

	/* Clear screen */
	SSD1306_Fill(SSD1306_COLOR_BLACK);
	
	/* Update screen */
	SSD1306_UpdateScreen();
	
	/* Set default values */
	SSD1306.CurrentX = 0;
	SSD1306.CurrentY = 0;
	
	/* Initialized OK */
	SSD1306.Initialized = 1;
	
	/* Return OK */
	return 1;
}
//----------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
void SSD1306_UpdateScreen(void) {
	uint8_t m;
	
	for (m = 0; m < 8; m++) {
		ssd1306_WriteCommand(0xB0 + m);
		
		#if defined (SH1106_128x64)
			// экран SH1106 имеет больше разрешение 132x64 страниц а сам экран 128 поетому смещаем на 2 
			ssd1306_WriteCommand(SSD1306_SETLOWCOLUMN + ( 2 & 0x0f ) );
			ssd1306_WriteCommand(SSD1306_SETHIGHCOLUMN + ( 2 >> 4 ) );
		#else
			ssd1306_WriteCommand(SSD1306_SETLOWCOLUMN );
			ssd1306_WriteCommand(SSD1306_SETHIGHCOLUMN );
		#endif
			
		/* Write multi data */
		ssd1306_WriteData(&SSD1306_Buffer[SSD1306_WIDTH * m], SSD1306_WIDTH);
	}
}
//----------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
void SSD1306_ToggleInvert(void) {
	uint16_t i;
	
	/* Toggle invert */
	SSD1306.Inverted = !SSD1306.Inverted;
	
	/* Do memory toggle */
	for (i = 0; i < sizeof(SSD1306_Buffer); i++) {
		SSD1306_Buffer[i] = ~SSD1306_Buffer[i];
	}
}
//----------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
void SSD1306_Fill(SSD1306_COLOR_t color) {
	/* Set memory */
	memset(SSD1306_Buffer, (color == SSD1306_COLOR_BLACK) ? 0x00 : 0xFF, sizeof(SSD1306_Buffer));
}
//----------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
void SSD1306_DrawPixel(uint16_t x, uint16_t y, SSD1306_COLOR_t color) {
	if (
		x >= SSD1306_WIDTH ||
		y >= SSD1306_HEIGHT
	) {
		/* Error */
		return;
	}
	
	/* Check if pixels are inverted */
	if (SSD1306.Inverted) {
		color = (SSD1306_COLOR_t)!color;
	}
	
	/* Set color */
	if (color == SSD1306_COLOR_WHITE) {
		SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
	} else {
		SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
	}
}
//----------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
void SSD1306_GotoXY(uint16_t x, uint16_t y) {
	/* Set write pointers */
	SSD1306.CurrentX = x;
	SSD1306.CurrentY = y;
}
//----------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
char SSD1306_Putc(char ch, FontDef_t* Font, SSD1306_COLOR_t color) {
	uint32_t i, b, j;
	
	/* Check available space in LCD */
	if (
		SSD1306_WIDTH <= (SSD1306.CurrentX + Font->FontWidth) ||
		SSD1306_HEIGHT <= (SSD1306.CurrentY + Font->FontHeight)
	) {
		/* Error */
		return 0;
	}
	
	/* Go through font */
	for (i = 0; i < Font->FontHeight; i++) {
		
		if( ch < 127 ){			
			b = Font->data[(ch - 32) * Font->FontHeight + i];
		}
		
		else if( (uint8_t) ch > 191 ){
			// +96 это так как латинские символы и знаки в шрифтах занимают 96 позиций
			// и если в шрифте который содержит сперва латиницу и спец символы и потом 
			// только кирилицу то нужно добавлять 95 если шрифт 
			// содержит только кирилицу то +96 не нужно
			b = Font->data[((ch - 192) + 96) * Font->FontHeight + i];
		}
		
		else if( (uint8_t) ch == 168 ){	// 168 символ по ASCII - Ё
			// 160 эллемент ( символ Ё ) 
			b = Font->data[( 160 ) * Font->FontHeight + i];
		}
		
		else if( (uint8_t) ch == 184 ){	// 184 символ по ASCII - ё
			// 161 эллемент  ( символ ё ) 
			b = Font->data[( 161 ) * Font->FontHeight + i];
		}
		
		
		
		for (j = 0; j < Font->FontWidth; j++) {
			if ((b << j) & 0x8000) {
				SSD1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR_t) color);
			} else {
				SSD1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR_t)!color);
			}
		}
	}
	
	/* Increase pointer */
	SSD1306.CurrentX += Font->FontWidth;
	
	/* Return character written */
	return ch;
}
//----------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
char SSD1306_Puts(char* str, FontDef_t* Font, SSD1306_COLOR_t color) {
	/* Write characters */
	while (*str) {
		/* Write character by character */
		if (SSD1306_Putc(*str, Font, color) != *str) {
			/* Return error */
			return *str;
		}
		
		/* Increase string pointer */
		str++;
	}
	
	/* Everything OK, zero should be returned */
	return *str;
}
 
//----------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
void SSD1306_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, SSD1306_COLOR_t c) {
	int16_t dx, dy, sx, sy, err, e2, i, tmp; 
	
	/* Check for overflow */
	if (x0 >= SSD1306_WIDTH) {
		x0 = SSD1306_WIDTH - 1;
	}
	if (x1 >= SSD1306_WIDTH) {
		x1 = SSD1306_WIDTH - 1;
	}
	if (y0 >= SSD1306_HEIGHT) {
		y0 = SSD1306_HEIGHT - 1;
	}
	if (y1 >= SSD1306_HEIGHT) {
		y1 = SSD1306_HEIGHT - 1;
	}
	
	dx = (x0 < x1) ? (x1 - x0) : (x0 - x1); 
	dy = (y0 < y1) ? (y1 - y0) : (y0 - y1); 
	sx = (x0 < x1) ? 1 : -1; 
	sy = (y0 < y1) ? 1 : -1; 
	err = ((dx > dy) ? dx : -dy) / 2; 

	if (dx == 0) {
		if (y1 < y0) {
			tmp = y1;
			y1 = y0;
			y0 = tmp;
		}
		
		if (x1 < x0) {
			tmp = x1;
			x1 = x0;
			x0 = tmp;
		}
		
		/* Vertical line */
		for (i = y0; i <= y1; i++) {
			SSD1306_DrawPixel(x0, i, c);
		}
		
		/* Return from function */
		return;
	}
	
	if (dy == 0) {
		if (y1 < y0) {
			tmp = y1;
			y1 = y0;
			y0 = tmp;
		}
		
		if (x1 < x0) {
			tmp = x1;
			x1 = x0;
			x0 = tmp;
		}
		
		/* Horizontal line */
		for (i = x0; i <= x1; i++) {
			SSD1306_DrawPixel(i, y0, c);
		}
		
		/* Return from function */
		return;
	}
	
	while (1) {
		SSD1306_DrawPixel(x0, y0, c);
		if (x0 == x1 && y0 == y1) {
			break;
		}
		e2 = err; 
		if (e2 > -dx) {
			err -= dy;
			x0 += sx;
		} 
		if (e2 < dy) {
			err += dx;
			y0 += sy;
		} 
	}
}
//----------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
void SSD1306_DrawRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, SSD1306_COLOR_t c) {
	/* Check input parameters */
	if (
		x >= SSD1306_WIDTH ||
		y >= SSD1306_HEIGHT
	) {
		/* Return error */
		return;
	}
	
	/* Check width and height */
	if ((x + w) >= SSD1306_WIDTH) {
		w = SSD1306_WIDTH - x;
	}
	if ((y + h) >= SSD1306_HEIGHT) {
		h = SSD1306_HEIGHT - y;
	}
	
	/* Draw 4 lines */
	SSD1306_DrawLine(x, y, x + w, y, c);         /* Top line */
	SSD1306_DrawLine(x, y + h, x + w, y + h, c); /* Bottom line */
	SSD1306_DrawLine(x, y, x, y + h, c);         /* Left line */
	SSD1306_DrawLine(x + w, y, x + w, y + h, c); /* Right line */
}
//----------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
void SSD1306_DrawFilledRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, SSD1306_COLOR_t c) {
	uint8_t i;
	
	/* Check input parameters */
	if (
		x >= SSD1306_WIDTH ||
		y >= SSD1306_HEIGHT
	) {
		/* Return error */
		return;
	}
	
	/* Check width and height */
	if ((x + w) >= SSD1306_WIDTH) {
		w = SSD1306_WIDTH - x;
	}
	if ((y + h) >= SSD1306_HEIGHT) {
		h = SSD1306_HEIGHT - y;
	}
	
	/* Draw lines */
	for (i = 0; i <= h; i++) {
		/* Draw lines */
		SSD1306_DrawLine(x, y + i, x + w, y + i, c);
	}
}
//----------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
void SSD1306_DrawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, SSD1306_COLOR_t color) {
	/* Draw lines */
	SSD1306_DrawLine(x1, y1, x2, y2, color);
	SSD1306_DrawLine(x2, y2, x3, y3, color);
	SSD1306_DrawLine(x3, y3, x1, y1, color);
}

//----------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
void SSD1306_DrawFilledTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, SSD1306_COLOR_t color) {
	int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0, 
	yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0, 
	curpixel = 0;
	
	deltax = ABS(x2 - x1);
	deltay = ABS(y2 - y1);
	x = x1;
	y = y1;

	if (x2 >= x1) {
		xinc1 = 1;
		xinc2 = 1;
	} else {
		xinc1 = -1;
		xinc2 = -1;
	}

	if (y2 >= y1) {
		yinc1 = 1;
		yinc2 = 1;
	} else {
		yinc1 = -1;
		yinc2 = -1;
	}

	if (deltax >= deltay){
		xinc1 = 0;
		yinc2 = 0;
		den = deltax;
		num = deltax / 2;
		numadd = deltay;
		numpixels = deltax;
	} else {
		xinc2 = 0;
		yinc1 = 0;
		den = deltay;
		num = deltay / 2;
		numadd = deltax;
		numpixels = deltay;
	}

	for (curpixel = 0; curpixel <= numpixels; curpixel++) {
		SSD1306_DrawLine(x, y, x3, y3, color);

		num += numadd;
		if (num >= den) {
			num -= den;
			x += xinc1;
			y += yinc1;
		}
		x += xinc2;
		y += yinc2;
	}
}

void SSD1306_DrawCircle(int16_t x0, int16_t y0, int16_t r, SSD1306_COLOR_t c) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

    SSD1306_DrawPixel(x0, y0 + r, c);
    SSD1306_DrawPixel(x0, y0 - r, c);
    SSD1306_DrawPixel(x0 + r, y0, c);
    SSD1306_DrawPixel(x0 - r, y0, c);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        SSD1306_DrawPixel(x0 + x, y0 + y, c);
        SSD1306_DrawPixel(x0 - x, y0 + y, c);
        SSD1306_DrawPixel(x0 + x, y0 - y, c);
        SSD1306_DrawPixel(x0 - x, y0 - y, c);

        SSD1306_DrawPixel(x0 + y, y0 + x, c);
        SSD1306_DrawPixel(x0 - y, y0 + x, c);
        SSD1306_DrawPixel(x0 + y, y0 - x, c);
        SSD1306_DrawPixel(x0 - y, y0 - x, c);
    }
}

void SSD1306_DrawFilledCircle(int16_t x0, int16_t y0, int16_t r, SSD1306_COLOR_t c) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

    SSD1306_DrawPixel(x0, y0 + r, c);
    SSD1306_DrawPixel(x0, y0 - r, c);
    SSD1306_DrawPixel(x0 + r, y0, c);
    SSD1306_DrawPixel(x0 - r, y0, c);
    SSD1306_DrawLine(x0 - r, y0, x0 + r, y0, c);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        SSD1306_DrawLine(x0 - x, y0 + y, x0 + x, y0 + y, c);
        SSD1306_DrawLine(x0 + x, y0 - y, x0 - x, y0 - y, c);

        SSD1306_DrawLine(x0 + y, y0 + x, x0 - y, y0 + x, c);
        SSD1306_DrawLine(x0 + y, y0 - x, x0 - y, y0 - x, c);
    }
}
 

//----------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
void SSD1306_Clear (void)
{
	SSD1306_Fill (SSD1306_COLOR_BLACK);
    SSD1306_UpdateScreen();
}
//----------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
void SSD1306_ON(void) {
	ssd1306_WriteCommand(0x8D);  
	ssd1306_WriteCommand(0x14);  
	ssd1306_WriteCommand(0xAF);  
}
//----------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
void SSD1306_OFF(void) {
	ssd1306_WriteCommand(0x8D);  
	ssd1306_WriteCommand(0x10);
	ssd1306_WriteCommand(0xAE);  
}
//----------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
void SSD1306_Contrast(uint8_t value ){
	ssd1306_WriteCommand(0x81);
	ssd1306_WriteCommand(value);
}
//----------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
void SSD1306_MirrorReflection(uint8_t value ){
	if(value){
		ssd1306_WriteCommand(0xA0); 		// вкл отзеркаливание
	}
	else{
		ssd1306_WriteCommand(0xA0 | 0x1 ); // выкл отзеркаливание
	}
}
//----------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
void SSD1306_ScreenRotation(uint8_t value ){
	if(value){// поварачиваем экран на 180
		ssd1306_WriteCommand(0xA0);	
		ssd1306_WriteCommand(0xC0);
	}
	else{// поварачиваем экран на 0 ( по умолчанию )
		ssd1306_WriteCommand(0xA0 | 0x1);	
		ssd1306_WriteCommand(0xC8);
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
//  _____ ___   _____ 
// |_   _|__ \ / ____|
//   | |    ) | |     
//   | |   / /| |     
//  _| |_ / /_| |____ 
// |_____|____|\_____|
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
void ssd1306_I2C_Init() {
	//MX_I2C1_Init();
	uint32_t p = 250000;
	while(p>0)
		p--;
	//HAL_I2C_DeInit(&SSD1306_I2C_PORT);
	//p = 250000;
	//while(p>0)
	//	p--;
	//MX_I2C1_Init();
}
//----------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
void ssd1306_Reset(void) {
	
	#if defined(SSD1306_USE_I2C)
		/* for I2C - do nothing */
	
	#elif defined(SSD1306_USE_SPI)
		// CS = High (not selected)
		HAL_GPIO_WritePin(SSD1306_CS_GPIO_Port, SSD1306_CS_Pin, GPIO_PIN_SET);

		// Reset the OLED
		HAL_GPIO_WritePin(SSD1306_RESET_GPIO_Port, SSD1306_RESET_Pin, GPIO_PIN_RESET);
		HAL_Delay(10);
		HAL_GPIO_WritePin(SSD1306_RESET_GPIO_Port, SSD1306_RESET_Pin, GPIO_PIN_SET);
		HAL_Delay(10);
	
	#else
	#error "You should define SSD1306_USE_SPI or SSD1306_USE_I2C macro"
	#endif
}

/*
	******************************************************************************
	* @brief	 ( описание ):  
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
void ssd1306_WriteCommand(uint8_t data) {
	
	#if defined(SSD1306_USE_I2C)
		uint8_t dt[2];
		dt[0] = 0x00;
		dt[1] = data;	
		HAL_I2C_Master_Transmit(&SSD1306_I2C_PORT, SSD1306_I2C_ADDR, dt, 2, 10);
	
	#elif defined(SSD1306_USE_SPI)
		HAL_GPIO_WritePin(SSD1306_CS_GPIO_Port, SSD1306_CS_Pin, GPIO_PIN_RESET); // select OLED
		HAL_GPIO_WritePin(SSD1306_DC_GPIO_Port, SSD1306_DC_Pin, GPIO_PIN_RESET); // command
		
		HAL_SPI_Transmit(&SSD1306_SPI_PORT, (uint8_t *) &data, 1, HAL_MAX_DELAY);
		/*
			// отправка по SMSIS
			SPI1->DR = data;
			while(( SPI1->SR & SPI_SR_BSY) ){};	// ждем когда освободится шина ( означает что передача закончилась )
				
		*/
		HAL_GPIO_WritePin(SSD1306_CS_GPIO_Port, SSD1306_CS_Pin, GPIO_PIN_SET); // un-select OLED
	
	#else
	#error "You should define SSD1306_USE_SPI or SSD1306_USE_I2C macro"
	#endif
}



//----------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
void ssd1306_WriteData(uint8_t* data, uint16_t count) {
	
	#if defined(SSD1306_USE_I2C)
		uint8_t dt[256];
		dt[0] = 0x40;
		uint8_t i;
		for(i = 0; i < count; i++)
		dt[i+1] = data[i];	
		HAL_I2C_Master_Transmit(&SSD1306_I2C_PORT, SSD1306_I2C_ADDR, dt, count+1, 10);
	
	#elif defined(SSD1306_USE_SPI)
		HAL_GPIO_WritePin(SSD1306_CS_GPIO_Port, SSD1306_CS_Pin, GPIO_PIN_RESET); // select OLED
		HAL_GPIO_WritePin(SSD1306_DC_GPIO_Port, SSD1306_DC_Pin, GPIO_PIN_SET); // data
		
		HAL_SPI_Transmit(&SSD1306_SPI_PORT, data, count, HAL_MAX_DELAY);
		/*
			// отправка по SMSIS
			SPI1->DR = data;
			while(( SPI1->SR & SPI_SR_BSY) ){};	// ждем когда освободится шина ( означает что передача закончилась )
				
		*/
		HAL_GPIO_WritePin(SSD1306_CS_GPIO_Port, SSD1306_CS_Pin, GPIO_PIN_SET); // un-select OLED
	
	#else
	#error "You should define SSD1306_USE_SPI or SSD1306_USE_I2C macro"
	#endif
}
//----------------------------------------------------------------------------------


/*

	Передача по SPI DMA CMSIS:
	
	в коде для работы с DMA сперва вызываем функцию ssd1306RunDisplayUPD()
	потом передаем данные на дисплей вункциями любыми текст, рисуем линию и т.д 
	по окончании передачи выключаем DMA вызвав функцию ssd1306StopDispayUPD()
	
	void ssd1306RunDisplayUPD()
	{
		GPIOA->BSRR|=GPIO_BSRR_BS3;	// data
		DMA1_Channel3->CCR&=~(DMA_CCR1_EN);//Выключить DMA
		DMA1_Channel3->CPAR=(uint32_t)(&SPI1->DR);//Занесем в DMA адрес регистра данных SPI1
		DMA1_Channel3->CMAR=(uint32_t)&displayBuff;//Адрес данных
		DMA1_Channel3->CNDTR=sizeof(displayBuff);//Размер данных
		DMA1->IFCR&=~(DMA_IFCR_CGIF3);
		GPIOA->BSRR|=GPIO_BSRR_BR2;// CS_RES Выбор устройства на шине
		DMA1_Channel3->CCR|=DMA_CCR1_CIRC;//Циклический режим DMA
		DMA1_Channel3->CCR|=DMA_CCR1_EN;//Включить DMA
	}

	void ssd1306StopDispayUPD()
	{
		GPIOA->BSRR|=GPIO_BSRR_BS2;// CS_SET Дезактивация устройства на шине
		DMA1_Channel3->CCR&=~(DMA_CCR1_EN);//Выключить DMA
		DMA1_Channel3->CCR&=~DMA_CCR1_CIRC;//Выключить циклический режим
	}

*/

/************************ (C) COPYRIGHT GKP *****END OF FILE****/
