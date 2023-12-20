# STM32_Lib_OLED_SSD1306_I2C_SPI
STM32 Lib OLED SSD1306 I2C SPI ( 64x32 64x48 72x40 96x16 128x32 128x64 ) SH1106 128x64 EN UA RUS

Данная библиотека для работы с дисплеем SSD1306.

Работает с разрешением экрана:

			// выбираем тип подключения I2C or SPI
				#define	SSD1306_USE_I2C
				#define	SSD1306_USE_SPI

			// выбираем разрешение экрана ( нужное раскомментировать )

				#define	SSD1306_64x32
			 	#define	SSD1306_64x48
			 	#define	SSD1306_72x40
			 	#define	SSD1306_96x16
			 	#define	SSD1306_128x32
				#define	SSD1306_128x64
				#define	SH1106_128x64

Данная библиотека поддерживает кириллицу.

Конфигурация в файле ssd1306.h

Инициализация и примеры работы в файле example.txt

LCDAssistant - конвертор изображения в массив.

По всем вопросам пишите: golinskiy.konstantin@gmail.com