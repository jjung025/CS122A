#include "pcd8544.h"
//This code was derived from the sample code on www.sparkfun.com for the nokia 5110 screen

void LCDWrite(uint8_t dc, uint8_t data) {
	// Check if write is command or data
	if (dc)
		LCD_DC_HI;    // Data
	else
		LCD_DC_LO;    // Command
	
	// Signal LCD to start receiving byte
	LCD_SCE_LO;
	for (uint8_t i = 0; i < 8; i++) {
		// Determine if bit to send is 1 or 0
		if (data&(1<<7))
			LCD_DN_HI;
		else
			LCD_DN_LO;
		
		// Oscillate clock for next bit
		LCD_SCLK_LO;
		LCD_SCLK_HI;
		
		// Shift to next bit
		data <<= 1;
	}
	
	// Signal LCD to end receiving
	LCD_SCE_HI;
	LCD_DN_LO;
}

void gotoXY(unsigned char x, unsigned char y) {
	LCDWrite(0, 0x80 | x);  
	LCDWrite(0, 0x40 | y);  
}

void LCDClear(void) {
	// Clear all pixels on LCD
	for (int i = 0; i < LCD_WIDTH*LCD_HEIGHT/8; i++)
		LCDWrite(LCD_D, 0x00);
}


void LCDInit(void) {
	
	// configure the LCD pins as outputs 
	// NOTE: Configured in main
	
	// initialize the LCD pins
	LCD_SCE_HI;
	//LCD_BACKLIGHT_HI;
	LCD_RST_LO;
	LCD_RST_HI;
	
	// Set of commands
	LCDWrite(LCD_C, 0x21);  // extended instruction set is coming
	LCDWrite(LCD_C, 0xB1);	// Set CONTRAST: set Vop to 5V
	LCDWrite(LCD_C, 0x04);	// set the temperature coefficient
	LCDWrite(LCD_C, 0x15);	// set the bias system
	// active mode, horizontal addressing, basic instruction set
	LCDWrite(LCD_C, 0x20);
	LCDWrite(LCD_C, 0x0C);  // normal mode
	LCDClear();             // clear the LCD
}

//used to display out my picutres 
void LCDBitmap(const unsigned char my_array[]) {
	for (unsigned short index = 0 ; index < (LCD_WIDTH * LCD_HEIGHT / 8) ; index++)
		LCDWrite(LCD_D, my_array[index]);
}



