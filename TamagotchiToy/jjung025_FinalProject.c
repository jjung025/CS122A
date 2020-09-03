/*
 * jjung025_lab6_part2_part1.c
 *
 * Created: 2/11/2016 2:33:32 PM
 *  Author: student
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include "/Users/student/Desktop/keypad.h"
#include "/Users/student/Desktop/io.c"
#include "/Users/student/Desktop/pcd8544.c"

volatile unsigned char TimerFlag = 0;

unsigned char timer_start =1;
unsigned char timer_curr=0;
unsigned char button =0;

void transmit_data(unsigned char data){
	int i;
	for (i=7;i >=0; --i) {
		PORTD = 0x08;
		PORTD |= ((data >> i)& 0x01);
		PORTD |= 0x04;
	}
	PORTD |= 0x02;
	PORTD = 0x00;
}


// unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b){
// 	return (b ? x | (0x01 << k) : x & ~(0x01 << k));
// }
// unsigned char GetBit(unsigned char x, unsigned char k){
// 	return ((x & (0x01 << k)) != 0);
// }
void TimerOn()
{
	TCCR1B = 0x0B; //TCNT1 register will count at 125,000 ticks/s
	OCR1A = 125; //AVR output compare register OCR1A, 1 ms has passed. Thus, we compare to 125.
	TIMSK1 = 0x02; //bit1: OCIE1A -- enables compare match interrupt
	TCNT1=0; ////Initialize avr counter
	
	timer_curr = timer_start; //// TimerISR will be called every _avr_timer_cntcurr milliseconds
	
	SREG |= 0x80; ////Enable global interrupts: 0x80: 1000000
}
void TimerOff()
{
	
	TCCR1B = 0x00; //// bit3bit1bit0=000: timer off
}
void TimerISR() {
	TimerFlag = 1;
}
ISR(TIMER1_COMPA_vect)
{
	
	timer_curr--;
	if (timer_curr == 0)
	{
		TimerISR();
		timer_curr= timer_start;
	}
}
void TimerSet(unsigned long M) //// Set TimerISR() to tick every M ms
{
	timer_start = M;
	timer_curr = timer_start;
}

//We are using PB6 so must disconnect the speaker before programming to the board.

double C_4 = 261.63;
double D_4 = 293.66;
double E_4 = 329.63;

void set_PWM(double frequency) {
	
	
	// Keeps track of the currently set frequency
	// Will only update the registers when the frequency
	// changes, plays music uninterrupted.
	static double current_frequency;
	if (frequency != current_frequency) {

		if (!frequency) TCCR3B &= 0x08; //stops timer/counter
		else TCCR3B |= 0x03; // resumes/continues timer/counter
		
		// prevents OCR3A from overflowing, using prescaler 64
		// 0.954 is smallest frequency that will not result in overflow
		if (frequency < 0.954) OCR3A = 0xFFFF;
		
		// prevents OCR3A from underflowing, using prescaler 64					
		// 31250 is largest frequency that will not result in underflow
		else if (frequency > 31250) OCR3A = 0x0000;
		
		// set OCR3A based on desired frequency
		else OCR3A = (short)(8000000 / (128 * frequency)) - 1;

		TCNT3 = 0; // resets counter
		current_frequency = frequency;
	}
}

void PWM_on() {
	TCCR3A = (1 << COM3A0);
	// COM3A0: Toggle PB6 on compare match between counter and OCR3A
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	// WGM32: When counter (TCNT3) matches OCR3A, reset counter
	// CS31 & CS30: Set a prescaler of 64
	set_PWM(0);
}

void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}

//Music code for dancing

unsigned char input = 0;
unsigned char note = 0;
//unsigned char sequence[22] = {1,9,1,9,1,9,1,9,1,9,1,9,1,9,3,9,8,9,0,9,1,9};
unsigned char sequence[28] = {0,9,0,9,4,9,4,9,5,9,5,9,4,9,3,9,3,9,2,9,2,9,1,9,1,9,0,9};
unsigned char sequence_length = 28;
//unsigned char note_length[22] = {1,1,1,1,3,2,1,1,1,1,3,2,1,1,1,1,1,1,1,1,3,1};
unsigned char note_length[28] = {1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1};
unsigned char time;
unsigned char play;
enum Music_States { Music_Start, Music_Off, Music_Play, Music_On, Music_Restart } Music_State;

double play_note(unsigned char note)
{
	double freq = 0;
	if(note == 0)
	{
		freq = 261.63;
	}
	else if(note == 1)
	{
		freq = 293.66;
	}
	else if(note == 2)
	{
		freq = 329.63;
	}
	else if(note == 3)
	{
		freq = 349.23;
	}
	else if(note == 4)
	{
		freq = 392.00;
	}
	else if(note == 5)
	{
		freq = 440.00;
	}
	else if(note == 6)
	{
		freq = 493.88;
	}
	else if(note == 7)
	{
		freq = 523.25;
	}
	else if(note == 8)
	{
		freq = 246.94;
	}
	else if(note == 9)
	{
		freq =0;
	}
	return freq;
}
	
void Music_Tick() {
	
	static unsigned char i;
	
	switch (Music_State) {
		case Music_Start:
		Music_State = Music_Off;
		break;
		case Music_Off:
		if(!GetBit(input,0)){
			Music_State = Music_Play;
			play = 1;
			i = 0;
		}
		break;
		case Music_Play:
		if(i == sequence_length)
		{
			Music_State = Music_Restart;
			set_PWM(0);
			break;
		}
		else if(i != sequence_length)
		{
			Music_State = Music_On;
			time = 0;
			break;
		}
		
		case Music_On:
		if(time == note_length[i])
		{
			Music_State =Music_Play;
			i++;
			break;
		}
		else
		{
			set_PWM(play_note(sequence[i]));
			time++;
			break;
		}
		
		case Music_Restart:
			Music_State = Music_Play;
			play = 1;
			i = 0;
		break;
		
		default:
		Music_State = Music_Start;
		break;
	}
	
	switch (Music_State) {
		case Music_Start:
		break;
		case Music_Off:
		break;
		case Music_On:
		break;
		case Music_Play:
		break;
		case Music_Restart:
		break;
		default:
		break;
	}
}

//exercise state for screen
unsigned char Ecounter =0;
enum Exer_States { Exer_Start, Exer1_State, Exer_Wait1, Exer2_State, Exer_Wait2} Exer_State;
void Exer_Tick() {
	
	switch (Exer_State){
		case Exer_Start:
			Exer_State = Exer1_State;
		break;
		case Exer1_State:
			LCDBitmap(Exercise1);
			Exer_State = Exer_Wait1;
		break;
		case Exer_Wait1:
			if(Ecounter <3){
				Ecounter++;
				Exer_State = Exer_Wait1;
			}
			else {
				Ecounter =0;
				Exer_State = Exer2_State;
			}
		break;
		case Exer2_State:
			LCDBitmap(Exercise2);
			Exer_State = Exer_Wait2;
		break;
		case Exer_Wait2:
			if(Ecounter <3){
				Ecounter++;
				Exer_State = Exer_Wait2;
			}
			else {
				Ecounter=0;
				Exer_State = Exer1_State;
			}
		break;
	}
	switch (Exer_State){
		case Exer_Start:
		break;
		case Exer1_State:
		break;
		case Exer2_State:
		break;
		case Exer_Wait1:
		break;
		case Exer_Wait2:
		break;
	}
};

//dance screen code 
unsigned char Dcounter =0;
enum Dance_States { Dance_Start, Dance1_State,Dance_Wait1, Dance2_State,Dance_Wait2} Dance_State;
void Dance_Tick() {
	
	switch (Dance_State){
		case Dance_Start:
		Dance_State = Dance1_State;
		break;
		case Dance1_State:
		LCDBitmap(Dance1);
		Dance_State = Dance_Wait1;
		break;
		case Dance_Wait1:
		if(Dcounter <3){
			Dcounter++;
			Dance_State = Dance_Wait1;
		}
		else {
			Dcounter =0;
			Dance_State = Dance2_State;
		}
		break;
		case Dance2_State:
		LCDBitmap(Dance2);
		Dance_State = Dance_Wait2;
		break;
		case Dance_Wait2:
		if(Dcounter <3){
			Dcounter++;
			Dance_State = Dance_Wait2;
		}
		else {
			Dcounter=0;
			Dance_State = Dance1_State;
		}
		break;
	}
	switch (Dance_State){
		case Dance_Start:
		break;
		case Dance1_State:
		break;
		case Dance2_State:
		break;
		case Dance_Wait1:
		break;
		case Dance_Wait2:
		break;
	}
};

//sleep code 
unsigned char Scounter=0;
enum Sleep_States {Sleep1_State, Sleep_Wait1, Sleep2_State, Sleep_Wait2, Sleep3_State, Sleep_Wait3} Sleep_State;
void Sleep_Tick(){
	switch(Sleep_State){
		case Sleep1_State:
			LCDBitmap(Sleep1);
			Sleep_State = Sleep_Wait1;
		break;
		case Sleep_Wait1:
			if (Scounter < 5)
			{
				 Scounter++;
				 Sleep_State = Sleep_Wait1;
			}
			else {
				Scounter =0;
				Sleep_State = Sleep2_State;
			}
		break;
		case Sleep2_State:
			LCDBitmap(Sleep2);
			Sleep_State = Sleep_Wait2;
		break;
		case Sleep_Wait2:
			if (Scounter < 5)
			{
				Scounter++;
				Sleep_State = Sleep_Wait2;
			}
			else {
				Scounter =0;
				Sleep_State = Sleep3_State;
			}
		break;
		case Sleep3_State:
			LCDBitmap(Sleep3);
			Sleep_State = Sleep_Wait3;
		break;
		case Sleep_Wait3:
			if (Scounter < 5)
			{
				Scounter++;
				Sleep_State = Sleep_Wait3;
			}
			else {
				Scounter =0;
				Sleep_State = Sleep1_State;
			}
		break;
	}
	switch(Sleep_State){
		case Sleep1_State:
		break;
		case Sleep_Wait1:
		break;
		case Sleep2_State:
		break;
		case Sleep_Wait2:
		break;
		case Sleep3_State:
		break;
		case Sleep_Wait3:
		break;
	}
};

//displaying score for coin flip 
unsigned char prev_score = 0;
unsigned char coin_score =0;
enum Score_States {Score_Display} Score_State;
void Score_Tick(){
	switch(Score_State){
		case Score_Display: 
			Score_State = Score_Display;
		break;
	}
	switch(Score_State)
	{
		case Score_Display:
			if (coin_score==0)
			transmit_data(0x40);
			else if (coin_score ==1)
			transmit_data( 0x79);
			else if (coin_score ==2)
			transmit_data( 0x24);
			else if (coin_score ==3)
			transmit_data( 0x30);
			else if (coin_score ==4)
			transmit_data( 0x19);
			else if (coin_score ==5)
			transmit_data( 0x12);
			else if (coin_score ==6)
			transmit_data( 0x02);
			else if (coin_score ==7)
			transmit_data( 0xF8);
			else if (coin_score ==8)
			transmit_data(0x00);
			else if (coin_score ==9)
			transmit_data( 0x18);
		break;
	}
};

unsigned char random=6;
unsigned char headsortails =0;
unsigned char choice =0;
int main(void)
{
	unsigned char x;
	
	unsigned char tosscnt =0;
	unsigned char inccnt =0;
	unsigned char inccnt2=0;
	unsigned char rcnt =0;
	
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xF0; PORTC = 0x0F;
	DDRA = 0xFF; PORTA = 0x00;
	DDRD = 0xFF; PORTD = 0x00; 
	PWM_on();
	
	TimerSet(100);
	TimerOn();
	LCDInit();
	LCDBitmap(Original);
	transmit_data(0x40);
	while(1)
	{
		if(random == 0){
			random++;
		}
		else random--;
		x = GetKeypadKey();
		switch (x) {
			// All 5 LEDs on
			case '\0':  break;
			// hex equivalent
			case '1':
			button = 1; // dance
			break;
			case '2':
			button = 2; // exercise 
			break;
			case '3': 
			button = 3; // sleep
			break;
			case '4':
			button =4; // coin flip
			break;
			case 'A':
			button = 5; // win coin toss
			break;
			case '5':
			button = 6;
			break;
			case '6':
			button =7;
			break;
		}
		if(button == 0)
		{}
		else if(button== 1)
		{
			Music_Tick();
			Dance_Tick();
		}
		else if (button == 2){//4
			set_PWM(0);
			Exer_Tick();
		}
		else if (button == 3) //7
		{
			 set_PWM(0);
			 Sleep_Tick();
		}
		else if(button == 5)//*
		{
			set_PWM(0);
			if(tosscnt < 1){
				tosscnt++;
				if(rcnt ==0){
					LCDBitmap(Heads);
					headsortails =1;
				}
				else {
					LCDBitmap(Tails);
					headsortails=2;
				}
			}
			tosscnt =0;
			
		}
		else if (button ==4)//2 // heads button
		{
			if(headsortails==1)
			{
				 coin_score++;
				 Score_Tick();
			}
			button =0;
			headsortails=0;
		}
		else if (button ==6)//5// tails button
		{
			if(headsortails==2)
			{
				coin_score++;
				Score_Tick();
			}
			button =0;
			headsortails=0;
		}
		else if (button ==7)//8// reset score
		{
			set_PWM(0);
			if(inccnt2 < 1){
				inccnt2++;
				coin_score=0;
				Score_Tick();
			}
			inccnt2 =0;
			button =0;
		}
		if(rcnt ==0)
			rcnt =1;
		else rcnt =0;
		while(!TimerFlag);
		TimerFlag = 0;
		
	}
}