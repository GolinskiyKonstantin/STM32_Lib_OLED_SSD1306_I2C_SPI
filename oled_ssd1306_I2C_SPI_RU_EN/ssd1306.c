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

#ifdef SSD1306_64x32
//== Init sequence for 64x48 Micro OLED module ============================
    ssd1306_WriteCommand( SSD1306_DISPLAYOFF );
    ssd1306_WriteCommand( SSD1306_SETDISPLAYCLOCKDIV );
	ssd1306_WriteCommand( 0x80 );  								// the suggested ratio 0x80
    ssd1306_WriteCommand( SSD1306_SETMULTIPLEX );
	ssd1306_WriteCommand( 0x1F );        						// ratio 32 (устанавливаем размер в пикселях по высоте -1 в HEX например 0x27 = 39 ( 40-1 ) )
	ssd1306_WriteCommand( 0xAD ); 
	ssd1306_WriteCommand( 0x30 );
    ssd1306_WriteCommand( SSD1306_SETDISPLAYOFFSET );
	ssd1306_WriteCommand( 0x0 );     							// no offset
    ssd1306_WriteCommand( SSD1306_SETSTARTLINE | 0x00 );       	// line #0
    ssd1306_WriteCommand( SSD1306_CHARGEPUMP );
	ssd1306_WriteCommand( 0x14 );          						// internal vcc
    ssd1306_WriteCommand( SSD1306_NORMALDISPLAY );
    ssd1306_WriteCommand( SSD1306_DISPLAYALLON_RESUME );
    ssd1306_WriteCommand( SSD1306_SEGREMAP | 0x01 );            	// column 127 mapped to SEG0
    ssd1306_WriteCommand( SSD1306_COMSCANDEC );                	// column scan direction reversed
    ssd1306_WriteCommand( SSD1306_SETCOMPINS );
	ssd1306_WriteCommand( 0x02 );          						// 0x12 if height > 32 else 0x02 ( если вісота больше 32 ставим 0х12 , если равно 32 или меньше то 0х02 )
    ssd1306_WriteCommand( SSD1306_SETCONTRAST );
	ssd1306_WriteCommand( 0x7F );         						// contrast level 127
    ssd1306_WriteCommand( SSD1306_SETPRECHARGE );
	ssd1306_WriteCommand( 0xF1 );        						// pre-charge period (1, 15)
    ssd1306_WriteCommand( SSD1306_SETVCOMDETECT );
	ssd1306_WriteCommand( 0x40 );       						// vcomh regulator level
    ssd1306_WriteCommand( SSD1306_DISPLAYON );
//=======================================================================

#elif defined SSD1306_64x48
//== Init sequence for 64x48 Micro OLED module ============================
    ssd1306_WriteCommand( SSD1306_DISPLAYOFF );
    ssd1306_WriteCommand( SSD1306_SETDISPLAYCLOCKDIV );
	ssd1306_WriteCommand( 0x80 );  								// the suggested ratio 0x80
    ssd1306_WriteCommand( SSD1306_SETMULTIPLEX );
	ssd1306_WriteCommand( 0x2F );        						// ratio 32 (устанавливаем размер в пикселях по высоте -1 в HEX например 0x27 = 39 ( 40-1 ) )
	ssd1306_WriteCommand( 0xAD ); 
	ssd1306_WriteCommand( 0x30 );
    ssd1306_WriteCommand( SSD1306_SETDISPLAYOFFSET );
	ssd1306_WriteCommand( 0x0 );     							// no offset
    ssd1306_WriteCommand( SSD1306_SETSTARTLINE | 0x00 );       	// line #0
    ssd1306_WriteCommand( SSD1306_CHARGEPUMP );
	ssd1306_WriteCommand( 0x14 );          						// internal vcc
    ssd1306_WriteCommand( SSD1306_NORMALDISPLAY );
    ssd1306_WriteCommand( SSD1306_DISPLAYALLON_RESUME );
    ssd1306_WriteCommand( SSD1306_SEGREMAP | 0x01 );            	// column 127 mapped to SEG0
    ssd1306_WriteCommand( SSD1306_COMSCANDEC );                	// column scan direction reversed
    ssd1306_WriteCommand( SSD1306_SETCOMPINS );
	ssd1306_WriteCommand( 0x12 );          						// 0x12 if height > 32 else 0x02 ( если вісота больше 32 ставим 0х12 , если равно 32 или меньше то 0х02 )
    ssd1306_WriteCommand( SSD1306_SETCONTRAST );
	ssd1306_WriteCommand( 0x7F );         						// contrast level 127
    ssd1306_WriteCommand( SSD1306_SETPRECHARGE );
	ssd1306_WriteCommand( 0xF1 );        						// pre-charge period (1, 15)
    ssd1306_WriteCommand( SSD1306_SETVCOMDETECT );
	ssd1306_WriteCommand( 0x40 );       						// vcomh regulator level
    ssd1306_WriteCommand( SSD1306_DISPLAYON );
//=======================================================================

#elif defined SSD1306_72x40	
//== Init sequence for SSD1306 72x40 OLED module ========================
    ssd1306_WriteCommand( SSD1306_DISPLAYOFF );
    ssd1306_WriteCommand( SSD1306_SETDISPLAYCLOCKDIV );
	ssd1306_WriteCommand( 0x80 );  								// the suggested ratio 0x80
    ssd1306_WriteCommand( SSD1306_SETMULTIPLEX );
	ssd1306_WriteCommand( 0x27 );								// ratio 32 (устанавливаем размер в пикселях по высоте -1 в HEX например 0x27 = 39 ( 40-1 ) )
	ssd1306_WriteCommand( 0xAD ); 
	ssd1306_WriteCommand( 0x30 );	
    ssd1306_WriteCommand( SSD1306_SETDISPLAYOFFSET );
	ssd1306_WriteCommand( 0x0 );     							// no offset
    ssd1306_WriteCommand( SSD1306_SETSTARTLINE | 0x00 );        	// line #0
    ssd1306_WriteCommand( SSD1306_CHARGEPUMP );
	ssd1306_WriteCommand( 0x14 );          						// internal vcc
    ssd1306_WriteCommand( SSD1306_MEMORYMODE );
	ssd1306_WriteCommand( 0x02 );          						// page mode
    ssd1306_WriteCommand( SSD1306_SEGREMAP | 0x01 );            	// column 127 mapped to SEG0
    ssd1306_WriteCommand( SSD1306_COMSCANDEC );                	// column scan direction reversed
    ssd1306_WriteCommand( SSD1306_SETCOMPINS );
	ssd1306_WriteCommand( 0x12 );          						// 0x12 if height > 32 else 0x02 ( если вісота больше 32 ставим 0х12 , если равно 32 или меньше то 0х02 )
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

#elif defined SSD1306_96x16	
//== Init sequence for SSD1306 96x16 OLED module ========================
    ssd1306_WriteCommand( SSD1306_DISPLAYOFF );
    ssd1306_WriteCommand( SSD1306_SETDISPLAYCLOCKDIV );
	ssd1306_WriteCommand( 0x80 );  								// clock divide ratio and osc frequency
    ssd1306_WriteCommand( SSD1306_SETMULTIPLEX );
	ssd1306_WriteCommand( 0x0F );        						// multiplex ratio ratio 32 (устанавливаем размер в пикселях по высоте -1 в HEX например 0x27 = 39 ( 40-1 ) )
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
	ssd1306_WriteCommand( 0x02 );          						// 0x12 if height > 32 else 0x02 ( если вісота больше 32 ставим 0х12 , если равно 32 или меньше то 0х02 )
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
	ssd1306_WriteCommand( 0x1F );        						// ratio 32 ratio 32 (устанавливаем размер в пикселях по высоте -1 в HEX например 0x27 = 39 ( 40-1 ) )
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
	ssd1306_WriteCommand( 0x02 );          						// 0x12 if height > 32 else 0x02 ( если вісота больше 32 ставим 0х12 , если равно 32 или меньше то 0х02 )
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
	ssd1306_WriteCommand( 0x3F );        						// ratio 64 ratio 32 (устанавливаем размер в пикселях по высоте -1 в HEX например 0x27 = 39 ( 40-1 ) )
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
	ssd1306_WriteCommand( 0x12 );          						// 0x12 if height > 32 else 0x02 ( если вісота больше 32 ставим 0х12 , если равно 32 или меньше то 0х02 )
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
  ssd1306_WriteCommand( 0x80 );             						// 128 ratio 32 (устанавливаем размер в пикселях по высоте -1 в HEX например 0x27 = 39 ( 40-1 ) )
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
  ssd1306_WriteCommand( 0X12 );              						// 0x12 if height > 32 else 0x02 ( если вісота больше 32 ставим 0х12 , если равно 32 или меньше то 0х02 )
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
		#elif defined SSD1306_72x40	
			// экран SSD1306_72x40 имеет смещение в 28 пикселей, поетому смещаем на 28 и выравниваем картинку
			ssd1306_WriteCommand(SSD1306_SETLOWCOLUMN + ( 28 & 0x0f ) );
			ssd1306_WriteCommand(SSD1306_SETHIGHCOLUMN + ( 28 >> 4 ) );
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
void SSD1306_Putc(char ch, FontDef_t* Font, SSD1306_COLOR_t color) {
	uint32_t i, b, j;
	
	/* Check available space in LCD */
	if (
		SSD1306_WIDTH > (SSD1306.CurrentX + Font->FontWidth) ||
		SSD1306_HEIGHT > (SSD1306.CurrentY + Font->FontHeight)
	) {
	
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
			
			//----  Украинская раскладка ----------------------------------------------------
			else if( (uint8_t) ch == 170 ){	// 168 символ по ASCII - Є
				// 162 эллемент ( символ Є )
				b = Font->data[( 162 ) * Font->FontHeight + i];
			}
			else if( (uint8_t) ch == 175 ){	// 184 символ по ASCII - Ї
				// 163 эллемент  ( символ Ї )
				b = Font->data[( 163 ) * Font->FontHeight + i];
			}
			else if( (uint8_t) ch == 178 ){	// 168 символ по ASCII - І
				// 164 эллемент ( символ І )
				b = Font->data[( 164 ) * Font->FontHeight + i];
			}
			else if( (uint8_t) ch == 179 ){	// 184 символ по ASCII - і
				// 165 эллемент  ( символ і )
				b = Font->data[( 165 ) * Font->FontHeight + i];
			}
			else if( (uint8_t) ch == 186 ){	// 184 символ по ASCII - є
				// 166 эллемент  ( символ є )
				b = Font->data[( 166 ) * Font->FontHeight + i];
			}
			else if( (uint8_t) ch == 191 ){	// 168 символ по ASCII - ї
				// 167 эллемент ( символ ї )
				b = Font->data[( 167 ) * Font->FontHeight + i];
			}
			//-----------------------------------------------------------------------------
			
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
void SSD1306_Puts(char* str, FontDef_t* Font, SSD1306_COLOR_t color) {
	
	unsigned char buff_char;
	
	uint16_t len = strlen(str);
	
	while (len--) {
		
		//---------------------------------------------------------------------
		// проверка на кириллицу UTF-8, если латиница то пропускаем if
		// Расширенные символы ASCII Win-1251 кириллица (код символа 128-255)
		// проверяем первый байт из двух ( так как UTF-8 ето два байта )
		// если он больше либо равен 0xC0 ( первый байт в кириллеце будет равен 0xD0 либо 0xD1 именно в алфавите )
		if ( (uint8_t)*str >= 0xC0 ){	// код 0xC0 соответствует символу кириллица 'A' по ASCII Win-1251
			
			// проверяем какой именно байт первый 0xD0 либо 0xD1---------------------------------------------
			switch ((uint8_t)*str) {
				case 0xD0: {
					// увеличиваем массив так как нам нужен второй байт
					str++;
					// проверяем второй байт там сам символ
					if ((uint8_t)*str >= 0x90 && (uint8_t)*str <= 0xBF){ buff_char = (*str) + 0x30; }	// байт символов А...Я а...п  делаем здвиг на +48
					else if ((uint8_t)*str == 0x81) { buff_char = 0xA8; break; }		// байт символа Ё ( если нужнф еще символы добавляем тут и в функции DrawChar() )
					else if ((uint8_t)*str == 0x84) { buff_char = 0xAA; break; }		// байт символа Є ( если нужнф еще символы добавляем тут и в функции DrawChar() )
					else if ((uint8_t)*str == 0x86) { buff_char = 0xB2; break; }		// байт символа І ( если нужнф еще символы добавляем тут и в функции DrawChar() )
					else if ((uint8_t)*str == 0x87) { buff_char = 0xAF; break; }		// байт символа Ї ( если нужнф еще символы добавляем тут и в функции DrawChar() )
					break;
				}
				case 0xD1: {
					// увеличиваем массив так как нам нужен второй байт
					str++;
					// проверяем второй байт там сам символ
					if ((uint8_t)*str >= 0x80 && (uint8_t)*str <= 0x8F){ buff_char = (*str) + 0x70; }	// байт символов п...я	елаем здвиг на +112
					else if ((uint8_t)*str == 0x91) { buff_char = 0xB8; break; }		// байт символа ё ( если нужнф еще символы добавляем тут и в функции DrawChar() )
					else if ((uint8_t)*str == 0x94) { buff_char = 0xBA; break; }		// байт символа є ( если нужнф еще символы добавляем тут и в функции DrawChar() )
					else if ((uint8_t)*str == 0x96) { buff_char = 0xB3; break; }		// байт символа і ( если нужнф еще символы добавляем тут и в функции DrawChar() )
					else if ((uint8_t)*str == 0x97) { buff_char = 0xBF; break; }		// байт символа ї ( если нужнф еще символы добавляем тут и в функции DrawChar() )
					break;
				}
			}
			//------------------------------------------------------------------------------------------------
			// уменьшаем еще переменную так как израсходывали 2 байта для кириллицы
			len--;
			
			SSD1306_Putc(buff_char, Font, color);
		}
		//---------------------------------------------------------------------
		else{
			SSD1306_Putc(*str, Font, color);
		}
		
		/* Increase string pointer */
		str++;
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


//==============================================================================
// Процедура рисования прямоугольник с закругленніми краями ( заполненый )
//==============================================================================
void SSD1306_DrawFillRoundRect(int16_t x, int16_t y, uint16_t width, uint16_t height, int16_t cornerRadius, SSD1306_COLOR_t color) {
	
	int16_t max_radius = ((width < height) ? width : height) / 2; // 1/2 minor axis
  if (cornerRadius > max_radius){
    cornerRadius = max_radius;
	}
	
  SSD1306_DrawRectangle(x + cornerRadius, y, x + cornerRadius + width - 2 * cornerRadius, y + height, color);
  // draw four corners
  SSD1306_DrawFillCircleHelper(x + width - cornerRadius - 1, y + cornerRadius, cornerRadius, 1, height - 2 * cornerRadius - 1, color);
  SSD1306_DrawFillCircleHelper(x + cornerRadius, y + cornerRadius, cornerRadius, 2, height - 2 * cornerRadius - 1, color);
}
//==============================================================================

//==============================================================================
// Процедура рисования половины окружности ( правая или левая ) ( заполненый )
//==============================================================================
void SSD1306_DrawFillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t corners, int16_t delta, SSD1306_COLOR_t color) {

  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;
  int16_t px = x;
  int16_t py = y;

  delta++; // Avoid some +1's in the loop

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    if (x < (y + 1)) {
      if (corners & 1){
        SSD1306_DrawLine(x0 + x, y0 - y, x0 + x, y0 - y - 1 + 2 * y + delta, color);
		}
      if (corners & 2){
        SSD1306_DrawLine(x0 - x, y0 - y, x0 - x, y0 - y - 1 + 2 * y + delta, color);
		}
    }
    if (y != py) {
      if (corners & 1){
        SSD1306_DrawLine(x0 + py, y0 - px, x0 + py, y0 - px - 1 + 2 * px + delta, color);
		}
      if (corners & 2){
        SSD1306_DrawLine(x0 - py, y0 - px, x0 - py, y0 - px - 1 + 2 * px + delta, color);
		}
		py = y;
    }
    px = x;
  }
}
//==============================================================================																		

//==============================================================================
// Процедура рисования четверти окружности (закругление, дуга) ( ширина 1 пиксель)
//==============================================================================
void SSD1306_DrawCircleHelper(int16_t x0, int16_t y0, int16_t radius, int8_t quadrantMask, SSD1306_COLOR_t color)
{
    int16_t f = 1 - radius ;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * radius;
    int16_t x = 0;
    int16_t y = radius;

    while (x <= y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
				
        x++;
        ddF_x += 2;
        f += ddF_x;

        if (quadrantMask & 0x4) {
            SSD1306_DrawPixel(x0 + x, y0 + y, color);
            SSD1306_DrawPixel(x0 + y, y0 + x, color);;
        }
        if (quadrantMask & 0x2) {
			SSD1306_DrawPixel(x0 + x, y0 - y, color);
            SSD1306_DrawPixel(x0 + y, y0 - x, color);
        }
        if (quadrantMask & 0x8) {
			SSD1306_DrawPixel(x0 - y, y0 + x, color);
            SSD1306_DrawPixel(x0 - x, y0 + y, color);
        }
        if (quadrantMask & 0x1) {
            SSD1306_DrawPixel(x0 - y, y0 - x, color);
            SSD1306_DrawPixel(x0 - x, y0 - y, color);
        }
    }
}
//==============================================================================		

//==============================================================================
// Процедура рисования прямоугольник с закругленніми краями ( пустотелый )
//==============================================================================
void SSD1306_DrawRoundRect(int16_t x, int16_t y, uint16_t width, uint16_t height, int16_t cornerRadius, SSD1306_COLOR_t color) {
	
	int16_t max_radius = ((width < height) ? width : height) / 2; // 1/2 minor axis
  if (cornerRadius > max_radius){
    cornerRadius = max_radius;
	}
	
  SSD1306_DrawLine(x + cornerRadius, y, x + cornerRadius + width -1 - 2 * cornerRadius, y, color);         // Top
  SSD1306_DrawLine(x + cornerRadius, y + height - 1, x + cornerRadius + width - 1 - 2 * cornerRadius, y + height - 1, color); // Bottom
  SSD1306_DrawLine(x, y + cornerRadius, x, y + cornerRadius + height - 1 - 2 * cornerRadius, color);         // Left
  SSD1306_DrawLine(x + width - 1, y + cornerRadius, x + width - 1, y + cornerRadius + height - 1 - 2 * cornerRadius, color); // Right
	
  // draw four corners
  SSD1306_DrawCircleHelper(x + cornerRadius, y + cornerRadius, cornerRadius, 1, color);
  SSD1306_DrawCircleHelper(x + width - cornerRadius - 1, y + cornerRadius, cornerRadius, 2, color);
  SSD1306_DrawCircleHelper(x + width - cornerRadius - 1, y + height - cornerRadius - 1, cornerRadius, 4, color);
  SSD1306_DrawCircleHelper(x + cornerRadius, y + height - cornerRadius - 1, cornerRadius, 8, color);
}
//==============================================================================

//==============================================================================
// Процедура рисования линия толстая ( последний параметр толщина )
//==============================================================================
void SSD1306_DrawLineThick(int16_t x1, int16_t y1, int16_t x2, int16_t y2, SSD1306_COLOR_t color, uint8_t thick) {
	const int16_t deltaX = abs(x2 - x1);
	const int16_t deltaY = abs(y2 - y1);
	const int16_t signX = x1 < x2 ? 1 : -1;
	const int16_t signY = y1 < y2 ? 1 : -1;

	int16_t error = deltaX - deltaY;

	if (thick > 1){
		SSD1306_DrawCircle(x2, y2, thick >> 1, color);
	}
	else{
		SSD1306_DrawPixel(x2, y2, color);
	}

	while (x1 != x2 || y1 != y2) {
		if (thick > 1){
			SSD1306_DrawCircle(x1, y1, thick >> 1, color);
		}
		else{
			SSD1306_DrawPixel(x1, y1, color);
		}

		const int16_t error2 = error * 2;
		if (error2 > -deltaY) {
			error -= deltaY;
			x1 += signX;
		}
		if (error2 < deltaX) {
			error += deltaX;
			y1 += signY;
		}
	}
}
//==============================================================================		

//==============================================================================
// Процедура рисования дуга толстая ( часть круга )
//==============================================================================
void SSD1306_DrawArc(int16_t x0, int16_t y0, int16_t radius, int16_t startAngle, int16_t endAngle, SSD1306_COLOR_t color, uint8_t thick) {
	
	int16_t xLast = -1, yLast = -1;

    if (startAngle > endAngle) {
        // Рисование первой части дуги от startAngle до 360 градусов
        for (int16_t angle = startAngle; angle <= 360; angle += 2) {
            float angleRad = (float)(360 - angle) * PI / 180;
            int x = cos(angleRad) * radius + x0;
            int y = sin(angleRad) * radius + y0;

            if (xLast != -1 && yLast != -1) {
                if (thick > 1) {
                    SSD1306_DrawLineThick(xLast, yLast, x, y, color, thick);
                } else {
                    SSD1306_DrawLine(xLast, yLast, x, y, color);
                }
            }

            xLast = x;
            yLast = y;
        }

        // Рисование второй части дуги от 0 до endAngle
        for (int16_t angle = 0; angle <= endAngle; angle += 2) {
            float angleRad = (float)(360 - angle) * PI / 180;
            int x = cos(angleRad) * radius + x0;
            int y = sin(angleRad) * radius + y0;

            if (xLast != -1 && yLast != -1) {
                if (thick > 1) {
                    SSD1306_DrawLineThick(xLast, yLast, x, y, color, thick);
                } else {
                    SSD1306_DrawLine(xLast, yLast, x, y, color);
                }
            }

            xLast = x;
            yLast = y;
        }
    } else {
        // Рисование дуги от startAngle до endAngle
        for (int16_t angle = startAngle; angle <= endAngle; angle += 2) {
            float angleRad = (float)(360 - angle) * PI / 180;
            int x = cos(angleRad) * radius + x0;
            int y = sin(angleRad) * radius + y0;

            if (xLast != -1 && yLast != -1) {
                if (thick > 1) {
                    SSD1306_DrawLineThick(xLast, yLast, x, y, color, thick);
                } else {
                    SSD1306_DrawLine(xLast, yLast, x, y, color);
                }
            }

            xLast = x;
            yLast = y;
        }
    }
}
//==============================================================================


//==============================================================================
// линия толстая нужной длины и указаным углом поворота (0-360) ( последний параметр толшина )
//==============================================================================
void SSD1306_DrawLineThickWithAngle(int16_t x, int16_t y, int16_t length, double angle_degrees, SSD1306_COLOR_t color, uint8_t thick) {
    double angleRad = (360.0 - angle_degrees) * PI / 180.0;
    int16_t x2 = x + (int16_t)(cos(angleRad) * length) + 0.5;
    int16_t y2 = y + (int16_t)(sin(angleRad) * length) + 0.5;

    SSD1306_DrawLineThick(x, y, x2, y2, color, thick);
}
//==============================================================================


//==============================================================================
// Процедура рисования иконки монохромной с указаным углом
//==============================================================================
void SSD1306_DrawBitmapWithAngle(int16_t x, int16_t y, const unsigned char* bitmap, int16_t w, int16_t h, SSD1306_COLOR_t color, double angle_degrees) {
    // Преобразование угла в радианы
    double angle_radians = (360.0 - angle_degrees) * PI / 180.0;

    // Вычисление матрицы поворота
    double cosTheta = cos(angle_radians);
    double sinTheta = sin(angle_radians);

    // Ширина и высота повернутого изображения
    int16_t rotatedW = round(fabs(w * cosTheta) + fabs(h * sinTheta));
    int16_t rotatedH = round(fabs(h * cosTheta) + fabs(w * sinTheta));

    // Вычисление центральных координат повернутого изображения
    int16_t centerX = x + w / 2;
    int16_t centerY = y + h / 2;

    // Проходим по каждому пикселю изображения и рисуем его повернутым
    for (int16_t j = 0; j < h; j++) {
        for (int16_t i = 0; i < w; i++) {
            // Вычисление смещения от центра
            int16_t offsetX = i - w / 2;
            int16_t offsetY = j - h / 2;

            // Применение матрицы поворота
            int16_t rotatedX = round(centerX + offsetX * cosTheta - offsetY * sinTheta);
            int16_t rotatedY = round(centerY + offsetX * sinTheta + offsetY * cosTheta);

            // Проверка находится ли пиксель в пределах экрана
            if (rotatedX >= 0 && rotatedX < SSD1306_WIDTH && rotatedY >= 0 && rotatedY < SSD1306_HEIGHT) {
                // Получение цвета пикселя из исходного изображения
                uint8_t byteWidth = (w + 7) / 8;
                uint8_t byte = (*(const unsigned char*)(&bitmap[j * byteWidth + i / 8]));
                if (byte & (0x80 >> (i & 7))) {
                    // Рисование пикселя на экране
                    SSD1306_DrawPixel(rotatedX, rotatedY, color);
                }
            }
        }
    }
}
//==============================================================================



//==============================================================================
// Процедура рисования символа с указаным углом ( 1 буква или знак )
//==============================================================================
void SSD1306_PutcWithAngle(char ch, FontDef_t* Font, SSD1306_COLOR_t color, double angle_degrees){
	
	uint32_t i, b, j;
  
	// Преобразуем угол в радианы
	double radians = (360.0 - angle_degrees) * PI / 180.0;

	// Вычисляем матрицу поворота
	double cosAngle = cos(radians);
	double sinAngle = sin(radians);

	// Переменные для преобразованных координат
	double newX, newY;
	

		/* Check available space in LCD */
	if (
		SSD1306_WIDTH > (SSD1306.CurrentX + Font->FontWidth) ||
		SSD1306_HEIGHT > (SSD1306.CurrentY + Font->FontHeight)
	) {
	
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
			
			//----  Украинская раскладка ----------------------------------------------------
			else if( (uint8_t) ch == 170 ){	// 168 символ по ASCII - Є
				// 162 эллемент ( символ Є )
				b = Font->data[( 162 ) * Font->FontHeight + i];
			}
			else if( (uint8_t) ch == 175 ){	// 184 символ по ASCII - Ї
				// 163 эллемент  ( символ Ї )
				b = Font->data[( 163 ) * Font->FontHeight + i];
			}
			else if( (uint8_t) ch == 178 ){	// 168 символ по ASCII - І
				// 164 эллемент ( символ І )
				b = Font->data[( 164 ) * Font->FontHeight + i];
			}
			else if( (uint8_t) ch == 179 ){	// 184 символ по ASCII - і
				// 165 эллемент  ( символ і )
				b = Font->data[( 165 ) * Font->FontHeight + i];
			}
			else if( (uint8_t) ch == 186 ){	// 184 символ по ASCII - є
				// 166 эллемент  ( символ є )
				b = Font->data[( 166 ) * Font->FontHeight + i];
			}
			else if( (uint8_t) ch == 191 ){	// 168 символ по ASCII - ї
				// 167 эллемент ( символ ї )
				b = Font->data[( 167 ) * Font->FontHeight + i];
			}
			//-----------------------------------------------------------------------------
			
			for (j = 0; j < Font->FontWidth; j++) {
				// Применяем поворот к координатам
				newX = SSD1306.CurrentX + cosAngle * j - sinAngle * i + 0.5;
				newY = SSD1306.CurrentY + sinAngle * j + cosAngle * i + 0.5;
				
				if ((b << j) & 0x8000) {
					SSD1306_DrawPixel(newX, newY, (SSD1306_COLOR_t) color);
				} else {
					SSD1306_DrawPixel(newX, newY, (SSD1306_COLOR_t)!color);
				}
			}
		}
	}
}
//==============================================================================


//==============================================================================
// Процедура рисования строки с указаным углом
//==============================================================================
void SSD1306_PutsWithAngle(char* str, FontDef_t* Font, SSD1306_COLOR_t color, double angle_degrees){	

	unsigned char buff_char;
	
	uint16_t len = strlen(str);
	
	while (len--) {
		
		//---------------------------------------------------------------------
		// проверка на кириллицу UTF-8, если латиница то пропускаем if
		// Расширенные символы ASCII Win-1251 кириллица (код символа 128-255)
		// проверяем первый байт из двух ( так как UTF-8 ето два байта )
		// если он больше либо равен 0xC0 ( первый байт в кириллеце будет равен 0xD0 либо 0xD1 именно в алфавите )
		if ( (uint8_t)*str >= 0xC0 ){	// код 0xC0 соответствует символу кириллица 'A' по ASCII Win-1251
			
			// проверяем какой именно байт первый 0xD0 либо 0xD1---------------------------------------------
			switch ((uint8_t)*str) {
				case 0xD0: {
					// увеличиваем массив так как нам нужен второй байт
					str++;
					// проверяем второй байт там сам символ
					if ((uint8_t)*str >= 0x90 && (uint8_t)*str <= 0xBF){ buff_char = (*str) + 0x30; }	// байт символов А...Я а...п  делаем здвиг на +48
					else if ((uint8_t)*str == 0x81) { buff_char = 0xA8; break; }		// байт символа Ё ( если нужнф еще символы добавляем тут и в функции DrawChar() )
					else if ((uint8_t)*str == 0x84) { buff_char = 0xAA; break; }		// байт символа Є ( если нужнф еще символы добавляем тут и в функции DrawChar() )
					else if ((uint8_t)*str == 0x86) { buff_char = 0xB2; break; }		// байт символа І ( если нужнф еще символы добавляем тут и в функции DrawChar() )
					else if ((uint8_t)*str == 0x87) { buff_char = 0xAF; break; }		// байт символа Ї ( если нужнф еще символы добавляем тут и в функции DrawChar() )
					break;
				}
				case 0xD1: {
					// увеличиваем массив так как нам нужен второй байт
					str++;
					// проверяем второй байт там сам символ
					if ((uint8_t)*str >= 0x80 && (uint8_t)*str <= 0x8F){ buff_char = (*str) + 0x70; }	// байт символов п...я	елаем здвиг на +112
					else if ((uint8_t)*str == 0x91) { buff_char = 0xB8; break; }		// байт символа ё ( если нужнф еще символы добавляем тут и в функции DrawChar() )
					else if ((uint8_t)*str == 0x94) { buff_char = 0xBA; break; }		// байт символа є ( если нужнф еще символы добавляем тут и в функции DrawChar() )
					else if ((uint8_t)*str == 0x96) { buff_char = 0xB3; break; }		// байт символа і ( если нужнф еще символы добавляем тут и в функции DrawChar() )
					else if ((uint8_t)*str == 0x97) { buff_char = 0xBF; break; }		// байт символа ї ( если нужнф еще символы добавляем тут и в функции DrawChar() )
					break;
				}
			}
			//------------------------------------------------------------------------------------------------
			// уменьшаем еще переменную так как израсходывали 2 байта для кириллицы
			len--;
			
			SSD1306_PutcWithAngle(buff_char, Font, color, angle_degrees);
		}
		//---------------------------------------------------------------------
		else{
			SSD1306_PutcWithAngle(*str, Font, color, angle_degrees);
		}
		// Смещаем начальные координаты с каждым символом с учетом угла
		SSD1306.CurrentX += (Font->FontWidth * cos((360.0 - angle_degrees) * PI / 180.0) + 0.5);
		SSD1306.CurrentY += (Font->FontWidth * sin((360.0 - angle_degrees) * PI / 180.0) + 0.5);

		// Increase string pointer
		str++;
	}
}
//==============================================================================


//==============================================================================
// рисуем элипс
//==============================================================================
void SSD1306_DrawEllipse(int16_t x0, int16_t y0, int16_t radiusX, int16_t radiusY, SSD1306_COLOR_t color) {
    int x, y;
    for (float angle = 0; angle <= 360; angle += 0.1) {
        x = x0 + radiusX * cos(angle * PI / 180);
        y = y0 + radiusY * sin(angle * PI / 180);
        SSD1306_DrawPixel(x, y, color);
    }
}
//==============================================================================


//==============================================================================
// рисуем элипс под указаным углом наклона
//==============================================================================
void SSD1306_DrawEllipseWithAngle(int16_t x0, int16_t y0, int16_t radiusX, int16_t radiusY, float angle_degrees, SSD1306_COLOR_t color) {
    float cosAngle = cos((360.0 - angle_degrees) * PI / 180);
    float sinAngle = sin((360.0 - angle_degrees) * PI / 180);

    for (int16_t t = 0; t <= 360; t++) {
        float radians = t * PI / 180.0;
        int16_t x = radiusX * cos(radians);
        int16_t y = radiusY * sin(radians);

        int16_t xTransformed = x0 + cosAngle * x - sinAngle * y;
        int16_t yTransformed = y0 + sinAngle * x + cosAngle * y;

        SSD1306_DrawPixel(xTransformed, yTransformed, color);
    }
}
//==============================================================================


//==============================================================================
// рисуем элипс закрашенный
//==============================================================================
void SSD1306_DrawEllipseFilled(int16_t x0, int16_t y0, int16_t radiusX, int16_t radiusY, SSD1306_COLOR_t color) {
	int x, y;

	for (y = -radiusY; y <= radiusY; y++) {
			for (x = -radiusX; x <= radiusX; x++) {
					if ((x * x * radiusY * radiusY + y * y * radiusX * radiusX) <= (radiusX * radiusX * radiusY * radiusY)) {
							SSD1306_DrawPixel(x0 + x, y0 + y, color);
					}
			}
	}
}
//==============================================================================


//==============================================================================
// рисуем элипс закрашенный под указаным углом наклона
//==============================================================================
void SSD1306_DrawEllipseFilledWithAngle(int16_t x0, int16_t y0, int16_t radiusX, int16_t radiusY, float angle_degrees, SSD1306_COLOR_t color) {
   float cosAngle = cos((360.0 - angle_degrees) * PI / 180.0);
    float sinAngle = sin((360.0 - angle_degrees) * PI / 180.0);

    for (int16_t y = -radiusY; y <= radiusY; y++) {
        for (int16_t x = -radiusX; x <= radiusX; x++) {
          float xTransformed = cosAngle * x - sinAngle * y;
          float yTransformed = sinAngle * x + cosAngle * y;

					if ((x * x * radiusY * radiusY + y * y * radiusX * radiusX) <= (radiusX * radiusX * radiusY * radiusY)){
             SSD1306_DrawPixel(x0 + xTransformed, y0  + yTransformed, color);
          }
        }
    }
}
//==============================================================================


//==============================================================================
// Процедура рисования линии с указаным углом и длиной
//==============================================================================
void SSD1306_DrawLineWithAngle(int16_t x, int16_t y, uint16_t length, double angle_degrees, SSD1306_COLOR_t color) {
    // Преобразование угла в радианы
    double angle_radians = (360.0 - angle_degrees) * PI / 180.0;

    // Вычисление конечных координат
    int16_t x2 = x + length * cos(angle_radians) + 0.5;
    int16_t y2 = y + length * sin(angle_radians) + 0.5;

    // Используем существующую функцию для рисования линии
    SSD1306_DrawLine(x, y, x2, y2, color);
}
//==============================================================================




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
