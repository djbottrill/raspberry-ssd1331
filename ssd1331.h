/*
 * ssd1331.h
 * 
 * Copyright 2016  David Bottrill
 * 
 * Adapted from the MBED ssd1331 library by Paul Staron
 * https://developer.mbed.org/users/star297/code/ssd1331/
 * 
 */
#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#define countof(x) ( sizeof(x) / sizeof(x[0]) )

// Screen Settings
#define width   				96-1 	// Max X axial direction in screen
#define height  				64-1    // Max Y axial direction in screen
#define Set_Column_Address  	0x15
#define Set_Row_Address     	0x75
#define contrastA   			0x81
#define contrastB    			0x82
#define contrastC    			0x83
#define display_on          	0xAF
#define display_off         	0xAE

// Internal Font size settings
#define normal   				0
#define wide     				1
#define high     				2
#define wh       				3
#define whx36    				4
#define X_width  				6 
#define Y_height 				8 

// GAC hardware acceleration commands
#define GAC_DRAW_line   		0x21    // Draw line
#define GAC_DRAW_RECTANGLE      0x22    // Rectangle
#define GAC_COPY_AREA           0x23    // Copy Area
#define GAC_DIM_window          0x24    // Dim window
#define GAC_CLEAR_window        0x25    // Clear window
#define GAC_FILL_ENABLE_DISABLE 0x26    // Enable Fill
#define SCROLL_SETUP            0x27    // Setup scroll
#define SCROLL_STOP             0x2E    // Scroll Stop
#define SCROLL_START            0x2F    // Scroll Start

// Basic RGB Colour definitions             RED GREEN BLUE values                         

#define Black           		0x0000  //   0,   0,   0 
#define LightGrey       		0xC618  // 192, 192, 192 
#define DarkGrey        		0x7BEF  // 128, 128, 128 
#define Red             		0xF800  // 255,   0,   0 
#define Green           		0x07E0  //   0, 255,   0 
#define Cyan            		0x07FF  //   0, 255, 255 
#define Blue            		0x001F  //   0,   0, 255 
#define Magenta         		0xF81F  // 255,   0, 255 
#define Yellow          		0xFFE0  // 255, 255,   0 
#define White           		0xFFFF  // 255, 255, 255 
		
int Bitmap16RAM(uint8_t x, uint8_t y, unsigned char *Name_BMP); // copy image to RAM, uses up to 12.288k RAM, Fast but need plenty of RAM
int Bitmap16FS(uint8_t x, uint8_t y, unsigned char *Name_BMP); // load from fielsystem, uses 96b RAM, slower, will work on any MCU
// use GIMP to generate images, open image, select image-scale image, set width/height(max 96x64), select File-Export As
// select SSD1331_windows BMP image, Name it with .BMP extension, select export, select Compatability Options- check 'Do Not write colour space'
// select Advanced Options- check '16bit R5 G6 B5', select export. Copy the file to a SD card or to the local file system. 
// image maximum size 96x64, smaller images can be placed anywhere on screen setting x,y co-ords but must fit within boundaries
// example:
// oled.Bitmap16FS(0,0,(unsigned char *)"/sd/image.bmp"); full size 96x64 image
// oled.Bitmap16RAM(20,20,(unsigned char *)"/sd/image.bmp"); place smaller image starting x-20, y-20


// window location
uint8_t _x;
uint8_t _y;
uint8_t _x1;
uint8_t _x2;
uint8_t _y1;
uint8_t _y2;
uint8_t char_x;
uint8_t char_y;
uint8_t chr_size;
uint8_t cwidth;       // character's width
uint8_t cvert;        // character's height
uint8_t externalfont;

//Broadcom GPIO Naming
const int SSD1331_PIN_CS  	= 8;
const int SSD1331_PIN_DC  	= 24;
const int SSD1331_PIN_RST 	= 25;

unsigned int Char_Colour	= 0;
unsigned int BGround_Colour	= 0;
unsigned char* font;

static const char font6x8[0x60][6] = {
    { 0x00,0x00,0x00,0x00,0x00,0x00 } , /*SPC */
    { 0x00,0x00,0x5F,0x00,0x00,0x00 } , /* !  */
    { 0x04,0x03,0x04,0x03,0x00,0x00 } , /* "  */
    { 0x28,0x7E,0x14,0x3F,0x0A,0x00 } , /* #  */
    { 0x24,0x2A,0x7F,0x2A,0x12,0x00 } , /* $  */
    { 0x23,0x13,0x08,0x64,0x62,0x00 } , /* %  */
    { 0x30,0x4E,0x59,0x26,0x50,0x00 } , /* &  */
    { 0x00,0x00,0x02,0x01,0x00,0x00 } , /* '  */
    { 0x00,0x00,0x1C,0x22,0x41,0x00 } , /* (  */
    { 0x41,0x22,0x1C,0x00,0x00,0x00 } , /* )  */
    { 0x22,0x14,0x08,0x14,0x22,0x00 } , /* *  */
    { 0x08,0x08,0x3E,0x08,0x08,0x00 } , /* +  */
    { 0x50,0x30,0x00,0x00,0x00,0x00 } , /* ,  */
    { 0x08,0x08,0x08,0x08,0x08,0x00 } , /* -  */
    { 0x60,0x60,0x00,0x00,0x00,0x00 } , /* .  */
    { 0x20,0x10,0x08,0x04,0x02,0x00 } , /* /  */
    { 0x3E,0x51,0x49,0x45,0x3E,0x00 } , /* 0  */
    { 0x00,0x42,0x7F,0x40,0x00,0x00 } , /* 1  */
    { 0x62,0x51,0x49,0x49,0x46,0x00 } , /* 2  */
    { 0x22,0x41,0x49,0x49,0x36,0x00 } , /* 3  */
    { 0x18,0x14,0x12,0x7F,0x10,0x00 } , /* 4  */
    { 0x2F,0x45,0x45,0x45,0x39,0x00 } , /* 5  */
    { 0x3E,0x49,0x49,0x49,0x32,0x00 } , /* 6  */
    { 0x01,0x61,0x19,0x05,0x03,0x00 } , /* 7  */
    { 0x36,0x49,0x49,0x49,0x36,0x00 } , /* 8  */
    { 0x26,0x49,0x49,0x49,0x3E,0x00 } , /* 9  */
    { 0x00,0x36,0x36,0x00,0x00,0x00 } , /* :  */
    { 0x00,0x56,0x36,0x00,0x00,0x00 } , /* ;  */
    { 0x00,0x08,0x14,0x22,0x41,0x00 } , /* <  */
    { 0x14,0x14,0x14,0x14,0x14,0x00 } , /* =  */
    { 0x41,0x22,0x14,0x08,0x00,0x00 } , /* >  */
    { 0x02,0x01,0x59,0x09,0x06,0x00 } , /* ?  */
    { 0x3E,0x41,0x5D,0x55,0x2E,0x00 } , /* @  */
    { 0x60,0x1C,0x13,0x1C,0x60,0x00 } , /* A  */
    { 0x7F,0x49,0x49,0x49,0x36,0x00 } , /* B  */
    { 0x3E,0x41,0x41,0x41,0x22,0x00 } , /* C  */
    { 0x7F,0x41,0x41,0x22,0x1C,0x00 } , /* D  */
    { 0x7F,0x49,0x49,0x49,0x41,0x00 } , /* E  */
    { 0x7F,0x09,0x09,0x09,0x01,0x00 } , /* F  */
    { 0x1C,0x22,0x41,0x49,0x3A,0x00 } , /* G  */
    { 0x7F,0x08,0x08,0x08,0x7F,0x00 } , /* H  */
    { 0x00,0x41,0x7F,0x41,0x00,0x00 } , /* I  */
    { 0x20,0x40,0x40,0x40,0x3F,0x00 } , /* J  */
    { 0x7F,0x08,0x14,0x22,0x41,0x00 } , /* K  */
    { 0x7F,0x40,0x40,0x40,0x00,0x00 } , /* L  */
    { 0x7F,0x04,0x18,0x04,0x7F,0x00 } , /* M  */
    { 0x7F,0x04,0x08,0x10,0x7F,0x00 } , /* N  */
    { 0x3E,0x41,0x41,0x41,0x3E,0x00 } , /* O  */
    { 0x7F,0x09,0x09,0x09,0x06,0x00 } , /* P  */
    { 0x3E,0x41,0x51,0x21,0x5E,0x00 } , /* Q  */
    { 0x7F,0x09,0x19,0x29,0x46,0x00 } , /* R  */
    { 0x26,0x49,0x49,0x49,0x32,0x00 } , /* S  */
    { 0x01,0x01,0x7F,0x01,0x01,0x00 } , /* T  */
    { 0x3F,0x40,0x40,0x40,0x3F,0x00 } , /* U  */
    { 0x03,0x1C,0x60,0x1C,0x03,0x00 } , /* V  */
    { 0x0F,0x70,0x0F,0x70,0x0F,0x00 } , /* W  */
    { 0x41,0x36,0x08,0x36,0x41,0x00 } , /* X  */
    { 0x01,0x06,0x78,0x02,0x01,0x00 } , /* Y  */
    { 0x61,0x51,0x49,0x45,0x43,0x00 } , /* Z  */
    { 0x00,0x00,0x7F,0x41,0x41,0x00 } , /* [  */
    { 0x15,0x16,0x7C,0x16,0x11,0x00 } , /* \  */
    { 0x41,0x41,0x7F,0x00,0x00,0x00 } , /* ]  */
    { 0x00,0x02,0x01,0x02,0x00,0x00 } , /* ^  */
    { 0x40,0x40,0x40,0x40,0x40,0x00 } , /* _  */
    { 0x00,0x01,0x02,0x00,0x00,0x00 } , /* `  */
    { 0x00,0x20,0x54,0x54,0x78,0x00 } , /* a  */
    { 0x00,0x7F,0x44,0x44,0x38,0x00 } , /* b  */
    { 0x00,0x38,0x44,0x44,0x28,0x00 } , /* c  */
    { 0x00,0x38,0x44,0x44,0x7F,0x00 } , /* d  */
    { 0x00,0x38,0x54,0x54,0x18,0x00 } , /* e  */
    { 0x00,0x04,0x3E,0x05,0x01,0x00 } , /* f  */
    { 0x00,0x08,0x54,0x54,0x3C,0x00 } , /* g  */
    { 0x00,0x7F,0x04,0x04,0x78,0x00 } , /* h  */
    { 0x00,0x00,0x7D,0x00,0x00,0x00 } , /* i  */
    { 0x00,0x40,0x40,0x3D,0x00,0x00 } , /* j  */
    { 0x00,0x7F,0x10,0x28,0x44,0x00 } , /* k  */
    { 0x00,0x01,0x7F,0x00,0x00,0x00 } , /* l  */
    { 0x7C,0x04,0x7C,0x04,0x78,0x00 } , /* m  */
    { 0x00,0x7C,0x04,0x04,0x78,0x00 } , /* n  */
    { 0x00,0x38,0x44,0x44,0x38,0x00 } , /* o  */
    { 0x00,0x7C,0x14,0x14,0x08,0x00 } , /* p  */
    { 0x00,0x08,0x14,0x14,0x7C,0x00 } , /* q  */
    { 0x00,0x7C,0x08,0x04,0x04,0x00 } , /* r  */
    { 0x00,0x48,0x54,0x54,0x24,0x00 } , /* s  */
    { 0x00,0x04,0x3E,0x44,0x40,0x00 } , /* t  */
    { 0x00,0x3C,0x40,0x40,0x7C,0x00 } , /* u  */
    { 0x00,0x7C,0x20,0x10,0x0C,0x00 } , /* v  */
    { 0x1C,0x60,0x1C,0x60,0x1C,0x00 } , /* w  */
    { 0x00,0x6C,0x10,0x10,0x6C,0x00 } , /* x  */
    { 0x00,0x4C,0x50,0x30,0x1C,0x00 } , /* y  */
    { 0x00,0x44,0x64,0x54,0x4C,0x00 } , /* z  */
    { 0x00,0x08,0x36,0x41,0x41,0x00 } , /* {  */
    { 0x00,0x00,0x7F,0x00,0x00,0x00 } , /* |  */
    { 0x41,0x41,0x36,0x08,0x00,0x00 } , /* }  */
    { 0x08,0x04,0x08,0x10,0x08,0x00 } , /* ~  */
    { 0x00,0x00,0x00,0x00,0x00,0x00 }   /*null*/
};


void SSD1331_Init(void);
void SSD1331_pixel(uint8_t x,uint8_t y, uint16_t Colour); // place a pixel x,y coordinates, Colour
void SSD1331_rect(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint16_t Colourline); // draw rectangle, start x,y end x,y, Colour
void SSD1331_fillrect(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint16_t Colourline,uint16_t Colourfill); // fill rectangle start x,y, end x,y, outline Colour, fill Colour.
void SSD1331_line(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint16_t Colour); // draw line start x,y, end x,y, Colour
void SSD1331_circle (uint8_t radius, uint8_t x, uint8_t y,uint16_t Colour,uint16_t fill); // draw circle radius, position x,y, Colour
void SSD1331_Fill_Screen(uint16_t Colour); // fill screen with any colour
void SSD1331_foreground(uint16_t Colour); // set text Colour
void SSD1331_background(uint16_t Colour); // set background Colour
void SSD1331_SetFontSize(uint8_t); // set internal font size normal, high, wide, wh (high and wide), whx36 (large 36x36 pixel size)
void SSD1331_on();  // display on
void SSD1331_off(); // display off
void SSD1331_cls(); // clear screen to black screen
void SSD1331_dim();  // flip dim/normal on each call
void SSD1331_contrast(char value); //0~9 low~high
void SSD1331_locate(uint8_t column, uint8_t row); // text start position x,y
uint16_t SSD1331_toRGB(uint16_t R,uint16_t G,uint16_t B);   // get Colour from RGB values 00~FF(0~255)
uint8_t SSD1331_row(); // set row position (in pixels)
uint8_t SSD1331_column(); // set column position (in pixels)
void SSD1331_ScrollSet(int8_t horizontal, int8_t startline, int8_t linecount, int8_t vertical , int8_t frame_interval); // set up scroll function
void SSD1331_Scrollstart(); // start scrolling
void SSD1331_Scrollstop(); // stop scrolling
void SSD1331_Copy(uint8_t src_x1,uint8_t src_y1,uint8_t src_x2,uint8_t src_y2, uint8_t dst_x,uint8_t dst_y); // GAC function to copy/paste area on the screen
void SSD1331_character(uint8_t x, uint8_t y, uint16_t c); // Print single character, x & y pixel co-ords.
void SSD1331_set_font(unsigned char* f); // set external font. Use oled.set_font(NULL) to restore default font
int SSD1331_Bitmap16FS(uint8_t x, uint8_t y, unsigned char *Name_BMP); // load from fielsystem, uses 96b RAM, slower, will work on any MCU
// use GIMP to generate images, open image, select image-scale image, set width/height(max 96x64), select File-Export As
// select Windows BMP image, Name it with .BMP extension, select export, select Compatability Options- check 'Do Not write colour space'
// select Advanced Options- check '16bit R5 G6 B5', select export. Copy the file to a SD card or to the local file system. 
// image maximum size 96x64, smaller images can be placed anywhere on screen setting x,y co-ords but must fit within boundaries
// example:
// oled.Bitmap16FS(0,0,(unsigned char *)"/sd/image.bmp"); full size 96x64 image
// oled.Bitmap16FS(20,20,(unsigned char *)"/sd/image.bmp"); place smaller image starting x-20, y-20
 

void  SSD1331_RegWrite(unsigned char *Command,  int count)
{
    digitalWrite(SSD1331_PIN_DC, 0);     // Command
    digitalWrite(SSD1331_PIN_CS, 0);     // CS enable
    wiringPiSPIDataRW(0, Command, count) ; 
    digitalWrite(SSD1331_PIN_CS, 1);     // CS dissable
}

void  SSD1331_DataWrite(unsigned char c)
{
	unsigned char buf[1]={};
	buf[0]=c;
    digitalWrite(SSD1331_PIN_DC, 1);     // Data
    digitalWrite(SSD1331_PIN_CS, 0);     // CS enable
    wiringPiSPIDataRW(0, buf, 1) ; 
    digitalWrite(SSD1331_PIN_CS, 1);     // CS dissable

}

void  SSD1331_DataWrite_to(uint16_t Dat)
{
	unsigned char buf[2]={};
	buf[0]=Dat >> 8;
	buf[1]=Dat;
    digitalWrite(SSD1331_PIN_DC, 1);     // Data
    digitalWrite(SSD1331_PIN_CS, 0);     // CS enable
    wiringPiSPIDataRW(0, buf, 2) ;
    digitalWrite(SSD1331_PIN_CS, 1);     // CS dissable   
    
}

void background(uint16_t Colour)
{
    BGround_Colour = Colour;
}

void MaxSSD1331_window()
{    
    unsigned char cmd[7]= {Set_Column_Address,0x00,0x5F,Set_Row_Address,0x00,0x3F};
    SSD1331_RegWrite(cmd, 6);
}

void SSD1331_foreground(uint16_t Colour)
{
    Char_Colour = Colour;
}

void SSD1331_cls(void)
{
    unsigned char cmd[6]= {GAC_CLEAR_window,0,0,width,height};
    SSD1331_RegWrite(cmd,5);
    delayMicroseconds(500);
    MaxSSD1331_window();
    background(0);
}

uint16_t SSD1331_toRGB(uint16_t R,uint16_t G,uint16_t B)
{  
    uint16_t c;
    c = R >> 3;
    c <<= 6;
    c |= G >> 2;
    c <<= 5;
    c |= B >> 3;
    return c;
}

// ==============================================================================
// OLED Initialisation 
// Setup GPIO pins using Broadcom numbering	
void SSD1331_init(void)
{
	wiringPiSetupGpio () ;		
	pinMode(SSD1331_PIN_CS, OUTPUT);
	pinMode(SSD1331_PIN_DC, OUTPUT);
	pinMode(SSD1331_PIN_RST,OUTPUT);
	wiringPiSPISetup(0, 32000000);			//Maximum speed for Raspberry Pi SPI
    // reset
    delay(200);
    digitalWrite(SSD1331_PIN_RST, 0);       //Reset active
    delay(200);
    digitalWrite(SSD1331_PIN_RST, 1);

    // initialize sequence
    unsigned char cmd[1]= { 0 };
    cmd[0]=(0xAE);    						// OLED display OFF
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0x75);    						// Set Row Address 
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0x00);    						// Start = 0 
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0x3F);    						// End = 63 
    SSD1331_RegWrite(cmd,1);	
    cmd[0]=(0x15);    						// Set Column Address 
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0x00);    						// Start = 0 
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0x5F);    						// End = 95 
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0xA0);    						// Set remap & data format 0111 0000
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0x72);    						// RGB colour
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0xA1);    						// set display start row RAM
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0x00);
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0xA2);    						// set dispaly offset
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0x00);
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0xA4);    						// Set Display Mode
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0xA8);    						// Set Multiplex Ratio
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0x3F);
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0xAD);    						// Set Master Configuration
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0x8F);    						// (External VCC Supply Selected)
    SSD1331_RegWrite(cmd,1);	
    cmd[0]=(0xB0);    						// Set Power Saving Mode
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0x1A);
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0xB1);    						// Set Phase 1 & 2 Period Adjustment
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0x74);
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0xB3);    						// Set Display Clock Divide Ratio / Oscillator Frequency
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0xD0);
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0x8A);    						// Set Second Pre-charge Speed of Colour A
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0x81);
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0x8B);    						// Set Second Pre-charge Speed of Colour B
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0x82);
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0x8C);    						// Set Second Pre-charge Speed of Colour C
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0x83);
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0xBB);    						// Set Pre-charge Level
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0x3E);
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0xBE);    						// Set VCOMH
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0x3E);
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0x87);    						// Set Master Current Control
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0x0F);
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0x81);    						// Set contrast Control for Colour &#129;gA&#129;h
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0x80);
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0x82);    						// Set contrast Control for Colour &#129;gB&#129;h
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0x80);
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0x83);    						// Set contrast Control for Colour &#129;gC&#129;h
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0x80);
    SSD1331_RegWrite(cmd,1);
    cmd[0]=(0xAF);    						//display ON
    SSD1331_RegWrite(cmd,1);

    chr_size = normal;
    SSD1331_cls();
}

void SSD1331_pixel(uint8_t x,uint8_t y,uint16_t Colour)
{
    unsigned char cmd[7]= {Set_Column_Address,0x00,0x00,Set_Row_Address,0x00,0x00};
    if ((x>width)||(y>height)) return ;
    cmd[1] = (unsigned char)x;
    cmd[2] = (unsigned char)x;
    cmd[4] = (unsigned char)y;
    cmd[5] = (unsigned char)y;
    SSD1331_RegWrite(cmd, 6);
    SSD1331_DataWrite_to(Colour);
}

void SSD1331_putp(int colour)
{
    SSD1331_pixel(_x, _y, colour);
    _x++;
    if(_x > _x2) {
        _x = _x1;
        _y++;
        if(_y > _y2) {
            _y = _y1;
        }
    }
}

void SSD1331_window(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
    _x = x;
    _y = y;
    // window settings
    _x1 = x;            // start y
    _x2 = x + w - 1;    // end x
    _y1 = y;            // start y
    _y2 = y + h - 1;    // end y
    unsigned char cmd[7]= {Set_Column_Address,0x00,0x00,Set_Row_Address,0x00,0x00};
    SSD1331_RegWrite(cmd, 6);
    unsigned char cmd2[7]= {Set_Column_Address,_x1,_x2,Set_Row_Address,_y1,_y2};
    SSD1331_RegWrite(cmd2, 6);
}

void SSD1331_rect(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint16_t Colour_line)
{
    if  ( x1 > width ) x1 = width;
    if  ( y1 > height ) y1 = height;
    if  ( x2 > width ) x2 = width;
    if  ( y2 > height ) y2 = height;

    unsigned char cmd[11]= { 0 };
    cmd[0] = GAC_FILL_ENABLE_DISABLE;
    cmd[1] = 0;      // fill 1, empty 0
    SSD1331_RegWrite(cmd, 2);
    cmd[0] = GAC_DRAW_RECTANGLE;
    cmd[1] = (unsigned char)x1;
    cmd[2] = (unsigned char)y1;
    cmd[3] = (unsigned char)x2;
    cmd[4] = (unsigned char)y2;
    cmd[5] = (unsigned char)((Colour_line>> 11) << 1);    // OutSSD1331_line Blue
    cmd[6] = (unsigned char)((Colour_line>> 5 ) & 0x3F);  // OutSSD1331_line Green
    cmd[7] = (unsigned char)((Colour_line<< 1 ) & 0x3F);  // OutSSD1331_line Red
    cmd[8] = (0);
    cmd[9] = (0);
    cmd[10]= (0);
    SSD1331_RegWrite(cmd, 11);
    delayMicroseconds(500);
}

void SSD1331_fillrect(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint16_t Colour_line,uint16_t Colourfill)
{
    if  ( x1 > width ) x1 = width;
    if  ( y1 > height ) y1 = height;
    if  ( x2 > width ) x2 = width;
    if  ( y2 > height ) y2 = height;

    unsigned char cmd[11]= { 0 };
    cmd[0] = GAC_FILL_ENABLE_DISABLE;
    cmd[1] = 1;      									// fill 1, empty 0
    SSD1331_RegWrite(cmd, 2);
    cmd[0] = GAC_DRAW_RECTANGLE;
    cmd[1] = (unsigned char)x1;
    cmd[2] = (unsigned char)y1;
    cmd[3] = (unsigned char)x2;
    cmd[4] = (unsigned char)y2;
    cmd[5] = (unsigned char)((Colour_line>> 11) << 1);   // Outline Blue
    cmd[6] = (unsigned char)((Colour_line>> 5 ) & 0x3F); // Outline Green
    cmd[7] = (unsigned char)((Colour_line<< 1 ) & 0x3F); // Outline Red
    cmd[8] = (unsigned char)((Colourfill>> 11) << 1);    // fill Blue
    cmd[9] = (unsigned char)((Colourfill>> 5 ) & 0x3F);  // fill Green
    cmd[10]= (unsigned char)((Colourfill<< 1 ) & 0x3F);  // fill Red
    SSD1331_RegWrite(cmd, 11);
    delayMicroseconds(500);
}

void SSD1331_Fill_Screen(uint16_t Colour)
{
    BGround_Colour = Colour;
    SSD1331_fillrect(0,0,width,height,Colour,Colour);
}

void SSD1331_line(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint16_t Colour)
{
    if  ( x1 > width ) x1 = width;
    if  ( y1 > height ) y1 = height;
    if  ( x2 > width ) x2 = width;
    if  ( y2 > height ) y2 = height;

    unsigned char cmd[11]= { 0 };
    cmd[0] = GAC_FILL_ENABLE_DISABLE;
    cmd[1] = 0;      									// fill 0, empty 0
    SSD1331_RegWrite(cmd, 2);
    cmd[0] = GAC_DRAW_line;
    cmd[1] = (unsigned char)x1;
    cmd[2] = (unsigned char)y1;
    cmd[3] = (unsigned char)x2;
    cmd[4] = (unsigned char)y2;
    cmd[5] = (unsigned char)(((Colour>>11)&0x1F)<<1);    // Blue
    cmd[6] = (unsigned char)((Colour>>5)&0x3F);          // Green
    cmd[7] = (unsigned char)((Colour&0x1F)<<1);          // Red
    SSD1331_RegWrite(cmd, 8);
    delayMicroseconds(500);
}

void SSD1331_circle (uint8_t radius, uint8_t x, uint8_t y , uint16_t col, uint16_t fill)
{
    if  ( x > width ) x = width;
    if  ( y > height ) y = height;
    
    int16_t  cx, cy, d;
    d = 3 - 2 * radius;
    cy = radius;
    SSD1331_pixel(x, radius+y, col);
    SSD1331_pixel(x, -radius+y, col);
    SSD1331_pixel(radius+x, y, col);
    SSD1331_pixel(-radius+x, y, col);
    if(fill) {
        SSD1331_line(x,radius+y,x,-radius+y,col);
        SSD1331_line( radius+x,y,-radius+x,y,col);
    }

    for (cx = 0; cx <= cy; cx++) {
        if(d>=0) {
            d+=10+4*cx-4*cy;
            cy--;
        } else {
            d+=6+4*cx;
        }
        SSD1331_pixel(cy+x, cx+y, col);
        SSD1331_pixel(cx+x, cy+y, col);
        SSD1331_pixel(-cx+x, cy+y, col);
        SSD1331_pixel(-cy+x, cx+y, col);
        SSD1331_pixel(-cy+x, -cx+y, col);
        SSD1331_pixel(-cx+x, -cy+y, col);
        SSD1331_pixel(cx+x, -cy+y, col);
        SSD1331_pixel(cy+x, -cx+y, col);
        if(fill) {
            SSD1331_line(cy+x, cx+y, cy+x, -cx+y, col);
            SSD1331_line(cx+x, cy+y, cx+x, -cy + y, col);
            SSD1331_line(-cx+x, cy+y, -cx+x, cy+y, col);
            SSD1331_line(-cy+x, cx+y, -cy+x, cx+y, col);
            SSD1331_line(-cy+x, -cx+y, -cy+x, cx+y, col);
            SSD1331_line(-cx+x, -cy+y, -cx+x, cy+y, col);
            SSD1331_line(cx+x, -cy+y, cx+x, cy+y, col);
            SSD1331_line(cy+x, -cx+y, cy+x, cx+y, col);
        }
    }
}

void SSD1331_contrast(char value)
{
    int v = value*20;
    if(v>180) {
        v=180;
    }
    unsigned char cmd[7];
    cmd[0] = contrastA;
    cmd[1] = v;
    cmd[2] = contrastB ;
    cmd[3] = v;
    cmd[4] = contrastC ;
    cmd[5] = v;
    SSD1331_RegWrite(cmd, 6);
}

void SSD1331_locate(uint8_t column, uint8_t row)
{
    char_x  = column;
    char_y = row;
}


void SSD1331_SetFontSize(uint8_t Csize)
{
    chr_size = Csize;
}

void SSD1331_set_font(unsigned char* f)
{
    font = f;
    if (font==NULL) {
        externalfont=0;   // set display.font
    } else {
        externalfont=1;
    }
}

void SSD1331_FontSizeConvert(int *lpx,int *lpy)
{
    switch( chr_size ) {
        case wide:
            *lpx=2;
            *lpy=1;
            break;
        case high:
            *lpx=1;
            *lpy=2;
            break;
        case wh  :
            *lpx=2;
            *lpy=2;
            break;
        case whx36  :
            *lpx=6;
            *lpy=6;
            break;
        case normal:
        default:
            *lpx=1;
            *lpy=1;
            break;
    }
}

void SSD1331_PutChar(uint8_t column,uint8_t row, int value)
{
    if(externalfont) { // external font
        uint8_t hor,vert,offset,bpl,j,i,b;
        const unsigned char* sign;
        unsigned char z,w;
        // read font parameter from start of array
        offset = font[0];                    				// bytes / char
        hor = font[1];                       				// get hor size of font
        vert = font[2];                      				// get vert size of font
        bpl = font[3];                       				// bytes per SSD1331_line
        if(value == '\n') {
            char_x = 0;
            char_y = char_y + vert;
        }
        if ((value < 31) || (value > 127)) return;   		// test char range
        if (char_x + hor > 95) {
            char_x = 0;
            char_y = char_y + vert;
            if (char_y >= 63 - font[2]) {
                char_y = 0;
            }
        }
        SSD1331_window(char_x, char_y,hor,vert); 
        sign = &font[((value -32) * offset) + 4];
        w = sign[0];
        for (j=0; j<vert; j++) {
            for (i=0; i<hor; i++) {
                z =  sign[bpl * i + ((j & 0xF8) >> 3)+1];
                b = 1 << (j & 0x07);
                if (( z & b ) == 0x00) {
                    SSD1331_putp(BGround_Colour);
                } else {
                    SSD1331_putp(Char_Colour);
                }
            }
        }
        if ((w + 2) < hor) {    							// x offset to next char
            char_x += w + 2;
        } else char_x += hor;
        
    } else {
        // internal font
        if(value == '\n') {
            char_x = 0;
            char_y = char_y + Y_height;
        }
        if ((value < 31) || (value > 127)) return;   		// test char range
        if (char_x + X_width > width) {
            char_x = 0;
            char_y = char_y + Y_height;
            if (char_y >= height - Y_height) {
                char_y = 0;
            }
        }
        int i,j,w,lpx,lpy,k,l,xw;
        unsigned char Temp=0;
        j = 0; i = 0;
        w = X_width;
        SSD1331_FontSizeConvert(&lpx, &lpy);
        xw = X_width;
        
        for(i=0; i<xw; i++) {
            for ( l=0; l<lpx; l++) {
                Temp = font6x8[value-32][i];
                for(j=Y_height-1; j>=0; j--) {
                    for (k=0; k<lpy; k++) {
                        SSD1331_pixel(char_x+(i*lpx)+l, char_y+(((j+1)*lpy)-1)-k,  ((Temp & 0x80)==0x80) ? Char_Colour : BGround_Colour);
                    }
                    Temp = Temp << 1;
                }
            }
        }
        SSD1331_FontSizeConvert(&lpx, &lpy);
        char_x += (w*lpx);
    }
}

void SSD1331_putc( int c )
{    
    SSD1331_PutChar( char_x , char_y ,c);
}

void SSD1331_ScrollSet(int8_t horizontal, int8_t start_line, int8_t SSD1331_linecount, int8_t vertical , int8_t frame_interval)
{
    unsigned char cmd[7];
    if((start_line>height+1)||((start_line+SSD1331_linecount)>height+1)) return ;
    if ( frame_interval > 3 ) frame_interval = 3;
    cmd[0] = SCROLL_SETUP;      
    cmd[1] = horizontal;
    cmd[2] = start_line;
    cmd[3] = SSD1331_linecount;
    cmd[4] = vertical;
    cmd[5] = frame_interval;
    SSD1331_RegWrite(cmd,6);
}

void SSD1331_Scrollstart()
{
	unsigned char buf[1]={};
	buf[0]=SCROLL_START;
    SSD1331_RegWrite(buf,1);
}

void SSD1331_Scrollstop()
{
	unsigned char buf[1]={};
	buf[0]=SCROLL_STOP;
    SSD1331_RegWrite(buf,1);
}

void SSD1331_print( char message[])
{
	int ii=0;
  for (ii=0 ; ii<strlen(message); ii++){
    SSD1331_putc(message[ii]);
	}
}

void SSD1331_Maxwindow()
{    
    unsigned char cmd[7]= {Set_Column_Address,0x00,0x5F,Set_Row_Address,0x00,0x3F};
    SSD1331_RegWrite(cmd, 6);
}
	
void sSSD1331_Copy(uint8_t src_x1,uint8_t src_y1,uint8_t src_x2,uint8_t src_y2,uint8_t dst_x,uint8_t dst_y)
{
    unsigned char cmd[8]= { 0 };
    if ((src_x1>width)||(src_y1>height)||(src_x2>width)||(src_y2>height)) return;
    if ((dst_x>width)||(dst_y>height))return;
    cmd[0] = GAC_COPY_AREA;      //
    cmd[1] = (unsigned char)src_x1;
    cmd[2] = (unsigned char)src_y1;
    cmd[3] = (unsigned char)src_x2;
    cmd[4] = (unsigned char)src_y2;
    cmd[5] = (unsigned char)dst_x;
    cmd[6] = (unsigned char)dst_y;
    SSD1331_RegWrite(cmd,7);
}

int SSD1331_Bitmap16FS(uint8_t x, uint8_t y, unsigned char *Name_BMP) {
#define OffsetPixelWidth    18
#define OffsetPixelHeigh    22
#define OffsetFileSize      34
#define OffsetPixData       10
#define OffsetBPP           28
    char filename[50];
    unsigned char BMP_Header[54];
    unsigned short BPP_t;
    unsigned int PixelWidth,PixelHeigh,start_data;
    unsigned int    i,off;
    int padd,j;
    unsigned short *line;
    
    i=0;
    while (*Name_BMP!='\0') {
        filename[i++]=*Name_BMP++;
    } 
  
    filename[i] = 0;  
    FILE *Image = fopen((const char *)&filename[0], "rb");  // open the bmp file
 
    if (!Image) {
        return(0);      									// error file not found !
    }

    fread(&BMP_Header[0],1,54,Image);      					// get the BMP Header

    if (BMP_Header[0] != 0x42 || BMP_Header[1] != 0x4D) {  	// check magic byte
        fclose(Image);
        return(-1);     									// error no BMP file
    }
 
    BPP_t = BMP_Header[OffsetBPP] + (BMP_Header[OffsetBPP + 1] << 8);
    if (BPP_t != 0x0010) {
        fclose(Image);
        printf("error no 16 bit BMP\n"); 
        return(-2);     									// error no 16 bit BMP
    }
   
    PixelHeigh = BMP_Header[OffsetPixelHeigh] + (BMP_Header[OffsetPixelHeigh + 1] << 8) + (BMP_Header[OffsetPixelHeigh + 2] << 16) + (BMP_Header[OffsetPixelHeigh + 3] << 24);
    PixelWidth = BMP_Header[OffsetPixelWidth] + (BMP_Header[OffsetPixelWidth + 1] << 8) + (BMP_Header[OffsetPixelWidth + 2] << 16) + (BMP_Header[OffsetPixelWidth + 3] << 24);
    if (PixelHeigh > height+1 + y || PixelWidth > width+1 + x) {
        fclose(Image);
        return(-3);      									// to big
    }

    start_data = BMP_Header[OffsetPixData] + (BMP_Header[OffsetPixData + 1] << 8) + (BMP_Header[OffsetPixData + 2] << 16) + (BMP_Header[OffsetPixData + 3] << 24);

    line = (unsigned short *) malloc (PixelWidth*2); // we need a buffer for a line (x2 as the buffer seemed to be too small)

    if (line == NULL) {
        return(-4);         								// error no memory
    }

    // the lines are padded to multiple of 4 bytes
    padd = -1;
    do {
        padd ++;
    } while ((PixelWidth * 2 + padd)%4 != 0);
      
    SSD1331_window(x, y,PixelWidth,PixelHeigh);
   
    for (j = PixelHeigh - 1; j >= 0; j--) {               	//Lines bottom up
        off = j * (PixelWidth * 2 + padd) + start_data;   	// start of line
        fseek(Image, off ,SEEK_SET);
        fread(line,1,PixelWidth * 2,Image);       			// read a line - slow !
        for (i = 0; i < PixelWidth; i++) {        			// copy pixel data to TFT
             SSD1331_DataWrite_to(line[i]);
        } 
    } 
    free (line);
    fclose(Image);
    SSD1331_Maxwindow();
    return(PixelWidth);
}

void SSD1331_on()
{
	unsigned char cmd[1]={};
    cmd[0]=0xaf;    			
    SSD1331_RegWrite(cmd,1);
}

void SSD1331_off()
{
	unsigned char cmd[1]={};  
    cmd[0]=0xae;   
    SSD1331_RegWrite(cmd,1);
}

void SSD1331_dim()
{
    unsigned char cmd[5]= {};
    cmd[0]=GAC_DIM_window;
    cmd[1]=0;
    cmd[2]=0;
    cmd[3]=width;
    cmd[5]=height;
    SSD1331_RegWrite(cmd,5);
}
