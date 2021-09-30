/* Whac-A-Mole Game Main Source File

 Developer: Gopal Agarwal
 Created: May 12, 2021

Hardware Resources:
 - TM4C123GH6PM Board
 - Switches and LED's

Software Used:
 - IAR Embedded Workbench

*/

#include "TM4C123GH6PM.h"              
#include<stdlib.h>                  
#include<stdbool.h>  
#include<time.h>
#include "DEC_TO_HEX.h"                 

// Functions used in programm
void Check_switch(unsigned int value1, unsigned int value2);
void Random_LED_On(void);
void SysTick_Handler(void);

bool S_STATUS = false;            
bool GET_OUT;                     // Used to get out of infinite loop
int COUNT = 0;                    // To count the times the led blinks

// This function is  used to generate any random value in between 0-7 and the corresponding LED will turn ON 
void Random_LED_On(void)
{             
       unsigned int index1 = rand() % 7;                // Generating a random value between 0-7
       unsigned int index2 = rand() % 7;
       unsigned int value2 = 0x00;                      // At 1st stage value2 is 0x00
       unsigned int value1 = dec_to_hex(index1);        // Converting random generated decimal value into hexadecimal using dec_to_hex user defined library
       if(COUNT >= 10)                                  // "COUNT" counts the turns and as 10 turns completes 2nd stage start
       value2 = dec_to_hex(index2);                     // At 2nd stage two LED's simultaneously gets ON and value2 stores for 2nd LED
       Check_switch(value1, value2);            
}

// This function is used to check the input response of the user
void Check_switch(unsigned int value1, unsigned int value2)          
{
	GPIOE -> DATA = value1 | value2;                // Turn ON the random LED         
	SysTick -> LOAD |= 15999999;                    // Systick timer starts for 1sec
	SysTick -> VAL = 0;
	SysTick -> CTRL = 7; 
	
  while(1)                                    
  {
      if( (!(GPIOB -> DATA & value1)) )         // If correct switch pressed for 1st LED           
      {
        S_STATUS = true;	
        GPIOE -> DATA &= ~(value1);             // Make random LED OFF                    
        GPIOF -> DATA = 0X08;	                // Make Green LED ON to show user win 		
      }
      if(COUNT >= 10)                         // Start 2nd stage after 10 turns
      {
        if( (!(GPIOB -> DATA & value2)) )       // If correct switch pressed for 2nd LED when 2nd stage starts        
        {
          S_STATUS = true;	
          GPIOE -> DATA &= ~(value2);                     
          GPIOF -> DATA = 0X08;			
        }
      }
      if(GET_OUT == false)                 // Tick is used to come out from this infinite while loop after every turn
      {
        GET_OUT = true;
        break;
      } 
  }
}

// Control will transfer here when Systick interrupt generates SYSTICK_HANDLER Interrupt Function 
void SysTick_Handler(void)                      
{
      
      GPIOE -> DATA = 0x00;
      SysTick -> CTRL = 0x00;
      if(S_STATUS == false)            // If no switch or false switch pressed
          GPIOF -> DATA = 0x02;        // Red LED turns ON to show user Loose   
      
      SysTick -> LOAD = 15999999;       //1sec delay before next event    
      SysTick -> VAL = 0;             
      SysTick -> CTRL = 5; 
      while(!(SysTick -> CTRL == 0x10000)); 
    
      GPIOF -> DATA = 0X00;           // Make all LED's Off
      S_STATUS = false;               // Make status again false  
      GET_OUT = false;
}

int main()                                     
{   
  // RCGCGPIO Register Enables Clock Refer page:- 340
      SYSCTL -> RCGCGPIO |= 0x32;         //0b0011 0010 Ports B, E, F clock Enabled                   
 
  /* GPIO E will be Connected with LED's. 
     One random LED will get ON user need to press Corresponding Button to Switch it off befor time gets over */ 
      GPIOE -> DEN |= 0xFF;               //Port E digitally Enabled
      GPIOE -> DIR |= 0xFF;               // 0b1111 1111 Port E pins set as output
      
  // GPIO B will be connected with switches to turn off the LED's   
      GPIOB -> LOCK |= 0x4C4F434B;        //Commit register unlocked for port B
      GPIOB -> CR |= 0x0C;                //Pins 2 and 3 are unlocked 
      GPIOB -> PUR |= 0xFF;               //Internal Pull Up resistor enabled
      GPIOB -> DIR |= 0x00;               //Port B set as input
      GPIOB -> DEN |= 0xFF;               //Port B digitally Enabled
        
  //CODE FOR GPIO F
      GPIOF -> DIR |= 0x1A;               // Green and Red LED set as output, Switch 1 set as Input
      GPIOF -> DEN |= 0x1A;               // Green, Red LED and Switch 1 are digitally Enabled
      GPIOF -> PUR |= 0x10;               // Internal Pull-up resistor enabled on Switch 1
      
      srand(time(0));
      
      while( (GPIOF -> DATA == 0x10) );           // To turn ON the game press Switch 1
      
      while(COUNT < 20)
      {
       Random_LED_On();         // Function called to make random led on
       COUNT++;
      }
}
