#include <stdlib.h>
#include <avr\io.h>
#include <avr\interrupt.h>
#include <util/delay.h>
#include <stdio.h>

#define ML 0	//left wheel
#define MR 1	//right wheel
#define BM 2	//both wheels
#define FW 0	//forward
#define BW 1	//backwards


void MotorStep(unsigned char Motor, unsigned short Steps, unsigned char Dir);

volatile unsigned char flag=0;
unsigned char LW_step=7, RW_step=8;
unsigned char step[8] = {0x01, 0x03, 0x02, 0x06, 0x04, 0x0C, 0x08, 0x09};	//step sequence table, half-step
volatile unsigned short i;

int main(void)
{
	unsigned short j;

	PORTB = 0xFF;
	DDRB = 0x02;
	PORTC = 0xF0;
	DDRC = 0x0F;
	PORTD = 0xFF;
	DDRD = 0x3F;

	//init timer1 36kHz
	TCNT1 = 0x0000;
	OCR1A = 111;
	TCCR1A = (1<<COM1A0);
	TCCR1B = (1<<WGM12)|(1<<CS10);
	
	//independent 10ms timer for pseudo-random generation
	TCNT0 = 0x00;
	OCR0A = 0x78;
	TCCR0A = (2<<WGM01);
	TCCR0B = (5<<CS00);
	TIMSK0 = (1<<OCIE0A);


	sei();
	//shut down both motors
	PORTC &= ~(0x0F<<PC0);
	PORTD &= ~(0x0F<<PD2);

	for(;;)
	{
		j = i+100;	//obstacle avoidance factor

		MotorStep(BM,1,FW);		//normally go straight forward
		
		if (!(PINC & (1<<PC5)))	//if there is an obstacle...
		{
			_delay_ms(500);
			MotorStep(BM,300,BW);	//...go backwards...
			_delay_ms(500);
			MotorStep(MR,j,FW);	//...and then turn
			_delay_ms(500);
		}//if
		if (!(PINC & (1<<PC4)))	//same as above but for another sensor
		{
			_delay_ms(500);
			MotorStep(BM,300,BW);
			_delay_ms(500);
			MotorStep(ML,j,FW);
			_delay_ms(500);
		}//if
		
	}//for

}//main

ISR(TIMER0_COMPA_vect)
{
	//calculation of pseudo-random
	i+=10;
	if (i>=300) i=0;
}//ISR

void MotorStep(unsigned char Motor, unsigned short Steps, unsigned char Dir)
{
	unsigned short i;
	for (i=0; i<Steps; i++)
	{
		switch(Motor)
		{
			case 1:
				PORTD &= ~(0x0F<<PD2);
				PORTD |= (step[RW_step]<<PD2);
				if (!Dir)	//forward
				{
					RW_step++;
					if (RW_step==8) RW_step=0;
				}//if
				else		//backwards
				{
					RW_step--;
					if (RW_step==255) RW_step=7;
				}//else
			break;
			case 0:
				PORTC &= ~(0x0F<<PC0);
				PORTC |= (step[LW_step]<<PC0);
				if (!Dir)
				{
					LW_step--;
					if (LW_step==255) LW_step=7;
				}//if
				else
				{
					LW_step--;
					if (LW_step==8) LW_step=0;
				}//else
			break;
			default:	//both motors
				PORTD &= ~(0x0F<<PD2);
				PORTD |= (step[RW_step]<<PD2);
				if (!Dir)	//forward
				{
					RW_step++;
					if (RW_step==8) RW_step=0;
				}//if
				else
				{
					RW_step--;
					if (RW_step==255) RW_step=7;
				}//else

				PORTC &= ~(0x0F<<PC0);
				PORTC |= (step[LW_step]<<PC0);
				if (!Dir)
				{
					LW_step--;
					if (LW_step==255) LW_step=7;
				}//if
				else
				{
					LW_step++;
					if (LW_step==8) LW_step=0;
				}//else
			break;
		}//switch
		_delay_ms(5);	//delay between steps
	}//for
}//MotorStep
