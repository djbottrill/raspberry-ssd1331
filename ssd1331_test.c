/*
 * ssd1331.h
 * 
 * Copyright 2016  David Bottrill
 * 
 
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include "ssd1331.h"

void gettime();
char Time[50],Date[50];

int main(void)
{
	SSD1331_init();	
	
    int y=0;
    char buf[255]="";
    unsigned char buf1[255]="";
    
	while(1){
	    SSD1331_contrast(9); // set SSD1331_contrast to maximum
        SSD1331_Fill_Screen(SSD1331_toRGB(255,0,0)); //red
        delay(500);
        SSD1331_Fill_Screen(SSD1331_toRGB(0,255,0)); //green
        delay(500);
        SSD1331_Fill_Screen(SSD1331_toRGB(0,0,255)); //blue
        delay(500);
        SSD1331_Fill_Screen(SSD1331_toRGB(255,255,255)); //white
        delay(500);	
		SSD1331_cls();
		delay(500);	
        SSD1331_circle (20, 40, 30 ,SSD1331_toRGB(0,0,255) , 1);      					//fill SSD1331_circle
        SSD1331_circle (20, 50, 35 ,SSD1331_toRGB(255,255,255) , 0);  					//SSD1331_circle 
        SSD1331_circle (20, 60, 40 ,SSD1331_toRGB(255,0,0) , 0);      					//SSD1331_circle	
        SSD1331_line( 0, 0, width, height, SSD1331_toRGB(0,255,255)); 					//SSD1331_line
        SSD1331_line( width, 0, 0, height, SSD1331_toRGB(255,0,255)); 					//SSD1331_line 
        SSD1331_rect(10,10,90,60,SSD1331_toRGB(255,255,0));      						//rectangle
        SSD1331_fillrect(20,20,40,40,SSD1331_toRGB(255,255,255),SSD1331_toRGB(0,255,0));//fillrectangle       
        delay(2000);		
        

        for(y = 9; y >= 0; y--) {
			SSD1331_contrast(y);  // set SSD1331_contrast level
			SSD1331_foreground(SSD1331_toRGB(255,255,255)); // set text colour
			SSD1331_locate(1, 10); // set text start location			
			sprintf(buf,"Contrast: %d\nline 2",y);
			SSD1331_print(buf);
			delay(300);
        }
        delay(2000);
        SSD1331_cls();

        SSD1331_SetFontSize(high); // set tall font
        SSD1331_foreground(SSD1331_toRGB(0,255,0)); // set text colour
        SSD1331_contrast(9);  // set SSD1331_contrast to Max
        
        SSD1331_locate(0, 10);
		SSD1331_print( "HIGH 12345");
		
		SSD1331_SetFontSize(wide); // set text to wide
        SSD1331_foreground(SSD1331_toRGB(0,0,255));
        SSD1331_locate(0, 28);
        SSD1331_print( "WIDE 123");
		
		
        SSD1331_SetFontSize(wh); // set text to wide and tall
        SSD1331_foreground(SSD1331_toRGB(255,0,0));
        SSD1331_locate(0, 40);
        SSD1331_print( "WH 123");
		
		
        SSD1331_SetFontSize(normal); // set text to normal
        SSD1331_foreground(SSD1331_toRGB(255,255,255));  		
		
		
		SSD1331_ScrollSet(0,8,18,1,0); // set scroll function
        SSD1331_Scrollstart(); // start scroll
       
        gettime();delay(1000);gettime();delay(1000);gettime();delay(1000);
              
        SSD1331_ScrollSet(0,8,18,-2,0);
        SSD1331_Scrollstart();       
        gettime();delay(1000);gettime();delay(1000);gettime();delay(1000);
        
        
        SSD1331_ScrollSet(0,8,18,3,0);
        SSD1331_Scrollstart();
        
        gettime();delay(1000);gettime();delay(1000);gettime();delay(1000);
        
        
        SSD1331_ScrollSet(0,8,18,-4,0);
        SSD1331_Scrollstart();
       
        gettime();delay(1000);gettime();delay(1000);gettime();delay(1000);    
        
        SSD1331_Scrollstop(); // stop scroll

		strcpy( (char*) buf1, "balloon.bmp" );
		SSD1331_Bitmap16FS(0, 0, buf1);
        delay(5000);
	}			
}	
void gettime()
{    
    time_t seconds = time(NULL);
    strftime(Time,40,"%H:%M:%S %a", localtime(&seconds));
    strftime(Date,40,"%d-%b-%Y", localtime(&seconds));
    SSD1331_locate(0, 0);
    SSD1331_print(Time); 
}
