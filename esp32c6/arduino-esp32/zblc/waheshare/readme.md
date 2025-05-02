https://docs.lvgl.io/master/details/integration/framework/arduino.html

После установки библиотеки TFT_eSPI необходимо отредактировать файл \Documents\Arduino\libraries\TFT_eSPI\User_Setup_Select.h:
- закомментировать строку #include <User_Setup.h> 
- раскомментировать строку #include <User_Setups/Setup72_ESP32_ST7789_172x320.h>
