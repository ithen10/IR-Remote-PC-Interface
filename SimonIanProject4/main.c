/*
 * SimonIanProject4.c
 *
 * Created: 11/9/2018 4:06:18 PM
 * Author : Ian Then & Simon Salami
 * Description: This code is for an IR Remote Receiver Interface.
 * It enables the receiver to receive data from an IR remote. 
 * The data is then sent to the PC and is displayed onto a terminal session. 
 */ 


#include <avr/interrupt.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "defines.h"
#include "uart.h"
#include <avr/io.h>
#include <util/delay.h>

volatile uint8_t pointer = 0;			//pointer used to store the data
volatile uint16_t clock = 0;			//variable used to check the time
volatile uint8_t state;					//variable used to keep track of the state of the code
volatile uint8_t buff[4] ={0} ;			//array used to store the data
volatile uint8_t Repeatbuff[4] ={0};	//array used to store the data for the repeat code
volatile uint8_t Repeatcheck = 0;		//variable used to indicate if a repeat code is sent

#define IDLE 0							//idle at state = 0 
#define LEADING_PULSE 1					//LEADING_PULSE at state = 1
#define LEADING_SPACE 2					//LEADING_SPACE at state = 2
#define Addr_Pulse 3					//Addr_Pulse at state = 3
#define Addr_Space 4					//Addr_Space at state = 4
#define Addr_inv_Pulse 5				//Addr_inv_Pulse at state = 5
#define Addr_inv_Space 6				//Addr_inv_Space at state = 6
#define Comm_Pulse 7					//Comm_Pulse at state = 7
#define Comm_Space 8					//Comm_Space at state = 8
#define Comm_inv_Pulse 9				//Comm_inv_Pulse at state = 9
#define Comm_inv_Space 10				//Comm_inv_Space at state = 10
#define Final_Pulse 11					//Final_Pulse at state = 11

#define now (state/2 - 2)				//now is the index used to store the data depending on the state

#define Lead_P ((clock >= 6000 ) && (clock <= 10000))	//Lead_P = 1 if this condition is true, 0 otherwise 
#define Lead_S ((clock >= 3000 ) && (clock <= 5000))	//Lead_S = 1 if this condition is true, 0 otherwise
#define Log1_P ((clock >= 200 ) && (clock <= 700))		//Log1_P = 1 if this condition is true, 0 otherwise
#define Log0_P ((clock >= 200 ) && (clock <= 700))		//Log0_P = 1 if this condition is true, 0 otherwise
#define Log1_S ((clock >= 1500 ) && (clock <= 1700))	//Log1_S = 1 if this condition is true, 0 otherwise
#define Log0_S ((clock >= 200 ) && (clock <= 700))		//Log0_S = 1 if this condition is true, 0 otherwise
#define Final_P ((clock >= 200 ) && (clock <= 700))		//Final_P = 1 if this condition is true, 0 otherwise
#define Repeat_S ((clock >= 1800 ) && (clock <= 3000))	//Repeat_S = 1 if this condition is true, 0 otherwise

FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);//initialize uart_str as a stream

void invalid ();				//invalid function prototype 
void pulses (bool a ,bool b);	//pulses function prototype 
void spaces (bool c, bool d);	//spaces function prototype 
void display ();				//display function prototype 
void Reset ();					//Reset function prototype 

/*Interrupts*/
ISR(PCINT0_vect){				//interrupt sub routine for the receiver pin
	
	clock = TCNT1;				//copying the value of the counter1 to clock to start comparisms
	TCNT1 = 0;					//reseting the counter1 to 0
	
	if (clock >= 200){			//filtering out noise by disregarding interrupts that occour when clock >= 200
		switch(state){			//switch statement to select the state
			case IDLE:			//if the case is IDLE
				state++;			//increase state by 1
				break;				//break out from this case
			
			case LEADING_PULSE:	//if the case is LEADING_PULSE
				if(Lead_P){			//if Lead_P IS 1
					state++;		//increase state by 1
					
				} else{				//if Lead_P IS 0
					invalid();		//call the invalid function to display the invalid message
				}
				break;				//break out from this case
			
			case LEADING_SPACE:	//if the case is LEADING_SPACE
				if(Lead_S){			//if Lead_S IS 1
					Repeatcheck=0;	//if there is no repeat code repeatcheck = 0
					state++;		//increase state by 1
				} else if(Repeat_S){//if there is a repeat code repeatcheck = 1
					Repeatcheck=1;
					state = Final_Pulse;//state is final pulse
					
				} else{				//if Lead_S = 0
					invalid();		//call the invalid function to display the invalid message
				}
				break;				//break out from this case
						
			case Addr_Pulse:	//if the case is Addr_Pulse
				pulses(Log1_P,Log0_P); //call the pulses function
				break;				//break out from this case
			
			case Addr_Space:	//if the case is Addr_Space
				spaces(Log1_S, Log0_S);//call the spaces function to store the data
				break;				//break out from this case
			
			case Addr_inv_Pulse: //if the case is Addr_inv_Pulse
				pulses(Log1_P,Log0_P);//call the pulses function
				break;					//break out from this case
			
			case Addr_inv_Space: //if the case is Addr_inv_Space
				spaces(Log1_S, Log0_S);//call the spaces function to store the data
				break;					//break out from this case
			
			case Comm_Pulse:	//if the case is Comm_Pulse
				pulses(Log1_P,Log0_P);//call the pulses function 
				break;				//break out from this case
			
			case Comm_Space:	//if the case is Comm_Space
				spaces(Log1_S, Log0_S);//calling the spaces function to store the data
				break;					//break out from this case
			
			case Comm_inv_Pulse: //if the case is Comm_inv_Pulse
				pulses(Log1_P,Log0_P);//call the pulses function 
				break;					//break out from this case
			
			case Comm_inv_Space: //if the case is Comm_inv_Space
				spaces(Log1_S, Log0_S);//call the spaces function to store the data
				break;
			
			case Final_Pulse: //if the case is Final_Pulse
				if (Final_P){		//if Final_P = 1
					if(Repeatcheck){ //if Repeatcheck = 1 ie there is a repeat code
						printf("Address: %x  ,  Data: %x  \r\n", Repeatbuff[0] , Repeatbuff[2]);//print the data in the Repeatbuff
						state=IDLE;//reset the state to IDLE 
					}else{		//if Repeatcheck = 0 ie there is no repeat code
						if (buff[0]+buff[1]==255){ //if the second byte is the inverse of the first 
							if (buff[2]+buff[3]==255){//check if the fourth byte is the inverse of the third 
								display();//call the display function to display the data
							}else{//if the fourth byte is not the inverse of the third 
								invalid();//call the invalid function to display the invalid message
								}
						}else{//if the second byte is not inverse of the first
							invalid();//call the invalid function to display the invalid message
							}
						}
				}else{//if Final_P = 0
					invalid();//call the invalid function to display the invalid message
					}
				break;				//break out from this case
			
			default:	 //if no option is selected, enter default case 
				invalid(); 	//call the invalid function to display the invalid message
			break;  			//break out from this case
		}
	}else{}	
			
}
		
ISR(TIMER1_OVF_vect){					//interrupt sub routine for timer overflow 1
	state = IDLE;						//reset the state to IDLE 
}

/*Main*/
int main(void)
{	
	stdout = &uart_str;					//stdout is a pointer to uart_str
	uart_init();						//initialize the uart

	PORTB &= ~(1<<PORTB6);				//put 0 into pb6(IR receiver pin)
	DDRB &= ~(1<<PORTB6);				//input low
	PCICR |= (1<<PCIE0);				//Enable Group 0 interrupts
	PCMSK0 |= (1<<PCINT6);				//Enable specific PCINT6 interrupt
	TIMSK1 |= (1<<TOIE1);				//enable timer overflow 1 interrupt
	sei();								//Global interrupt enabled
	TCCR1B |= (1<<CS10);				//TIMER1 WITH NO PRESCALER
	
	while (1) 
    {
    }
}

/*functions*/

/*invalid function definition*/
/*function to print invalid message when wrong data is received*/
void invalid(){
	//print invalid message//
	printf("invalid state:%u clock:%u buff0:%u buff1:%u buff2:%u buff3:%u\r\n",state, clock, buff[0], (uint8_t)~buff[1], buff[2],(uint8_t)~buff[3]);
	Reset();				//go to reset function
}
	
/*pulses function definition*/
/*function to check if signal pulse logical 1s or 0s are true*/
void pulses (bool a,bool b){//a is log1_p b is log0_p
	if(a || b){			//if  log1_p or log0_p is true
		state++;		//increase state
	} else{				//if log1_p and log1_s are false
		invalid();		//go to invalid function
		}
}

/*spaces function definition*/
/*function to input signal space logical 1s or 0s bit-by-bit into buff[i] for i from 0 to 3*/
void spaces (bool c, bool d){ //a is log1_s b is log0_s	
	if(c || d){				//if log1_s or log 0_s is true
		buff[now] |= (c << pointer); //puts bit 1 or 0 into buff[now] byte at 'pointer' position
		
		if(pointer == 7){	//if the MSB of the byte has already been inputted
			state++;		//go to next state
			pointer = 0;	//reset pointer value to 0
			} else{			//else if MSB has not been inputted yet
			state--;		//decrease state value by 1
			pointer++;		//increment pointer value by 1
			}
		} else{				//if log1_s and log 0_s are both false	
			invalid();		//go to invalid function
			}
}

/*Display function definition*/
/*function to display Address and Data codes*/
void display(){
	state=IDLE;					//reset state value to IDLE
	for (int i = 0 ; i < 4 ; i++)	//for loop to copy values in buff[] to Repeatbuff[]
	{
		Repeatbuff[i] = buff[i];
	}

/*print address code and data code to terminal session*/
	printf("Address: %x  ,  Data: %x%s\r\n", buff[0] , buff[2],"  "); //the string of exactly 2 spaces makes it work. No idea why, but it works
	
	for (int i = 0 ; i < 4 ; i++)	//for loop to clear buff[]
	{
		buff[i] = 0;
	}
}

/*Reset function definition*/
/*function to reset the buffers ‘buff[i]’ and ‘Repeatbuff[i]’ values to 0 for i from 0 to 3*/
void Reset (){				
	for (int i = 0 ; i < 4 ; i++)	//for loop to reset buff[i]
	{
		buff[i] = 0;
	}
	for (int i = 0 ; i < 4 ; i++)	//for loop to reset Repeatbuff[i]
	{
		Repeatbuff[i] = 0;
	}
	state = IDLE;					//reset state value to IDLE
	pointer = 0;					//reset pointer used for placing values in buff[] to 0
}
