/*	Making Devices 2021
	https://makingdevices.com
	Rubén García Segovia
	Stop Watch
*/
#include <p18f14k50.h>
#include <delays.h>

 // setup config bits 
 #pragma config PCLKEN=OFF
 #pragma config FOSC = IRC
 #pragma config BOREN = OFF 
 #pragma config WDTEN = OFF 
 #pragma config LVP = OFF  

char digit = 0;
char display = 0;
char const vector[] = {0b00111111,0b00000110,0b01011011,0b01001111,0b01100110,0b01101101,0b01111101,0b00000111,0b01111111,0b01100111,0b01111001};
char number[] = {0,0,0,0,0,0,0,0,0};
char pause = 0; // 0 = run // 1 = paused
char offset = 9; //Variable to calibrate the clock
char clock_mode = 1; //0 = 8 digits. 1 = 6 digits
char deep_sleep = 0;
float reset = 0;
float event_reset = 3000;
float modes = 0;
float event_modes = 3000;
char count_offset = 0;
void InitOSC(void); 
void _low_isr (void);
void _high_isr (void);
void Interrupts_enable (void);
void add_1ms (void);

#pragma code low_vector=0x18 //Low interrupt priority starts at 0x18
void low_interrupt(void)
{
_asm goto _low_isr _endasm
}

#pragma code high_vector=0x08 //High interrupt priority starts at 0x08
void high_interrupt(void)
{
_asm goto _high_isr _endasm
}
#pragma code


#pragma interrupt _low_isr

void _low_isr (void){
	
}

#pragma interrupt _high_isr
void _high_isr (void)
{	
if(INTCONbits.TMR0IF) //Timer0 interrupt
	{
if(PORTAbits.RA1 == 0){  //If we are pressing the button...
	reset++; //We increase reset.
	if (reset>event_reset){ //If we have the button pressed for a while...
		number[0] = 0;   //We reset the count.
		number[1] = 0;
		number[2] = 0;
		number[3] = 0;
		number[4] = 0;   
		number[5] = 0;
		number[6] = 0;
		number[7] = 0;
		reset = 1;
	}
} else{	
	reset = 0; //If we are not pressing nothing, reset = 0.
}

if(PORTAbits.RA0 == 0){  //If we are pressing the button...
	modes++; //We increase reset.
	if (modes>event_modes){ //If we have the button pressed for a while...
		if(clock_mode >0 && clock_mode < 4){
			clock_mode = 0;
		} else if (clock_mode == 0){
			 //PORTAbits.RA4 = 0;
			 clock_mode = 4;
		} else {
			clock_mode = 1;
		}
		modes = 1;
	}
} else{	
	modes = 0; //If we are not pressing nothing, reset = 0.
}

TMR0H = 0xFC;
TMR0L = 0x18;

if (count_offset == offset){
	if(pause==0) add_1ms();
	count_offset = 0;
} else {
	count_offset++;
}
	LATC = 0;
	LATB = 0;
	if(clock_mode < 4) {
		switch(digit){
			case 3:
				LATB = 0b10000000;
				digit = 0;
				display++;
				break;
			case 0:
				LATB = 0b01000000;
				digit = 1;
				break;
			case 1:
				LATB = 0b00100000;
				digit = 2;
				break;
			case 2:
				LATB = 0b00010000;
				digit = 3;
				break;
			default:
				break;
		}
		if (display==1){
			PORTAbits.RA4 = 0;
		} else {
			PORTAbits.RA4 = 1;
			display = 0;
		}
	}
	if(clock_mode >0 && clock_mode < 4){
			clock_mode = 1;
			if(number[4] > 0 || number[5] > 0) clock_mode = 2;
			if(number[6] > 0 || number[7] > 0) clock_mode = 3;
	}
	
	switch (clock_mode){
		case 0:
			if((digit+display*4)==2||(digit+display*4)==4||(digit+display*4)==6){
				LATC = (vector[number[digit+display*4]])|0b10000000;
			} else {
				LATC = vector[number[digit+display*4]];
			}
			break;
		case 3:
			if ((digit+display*4)==0 || (digit+display*4)== 7){
				LATC = 0;
			} else if((digit+display*4)==3||(digit+display*4)==5){
				LATC = (vector[number[digit+1+display*4]])|0b10000000;
			} else {
				LATC = vector[number[digit+1+display*4]];
			}
			break;
		case 2:
			if ((digit+display*4)==0 || (digit+display*4)== 7){
				LATC = 0;
				LATB = 0;
			} else if((digit+display*4)==3||(digit+display*4)==5){
				LATC = (vector[number[digit-1+display*4]])|0b10000000;
			} else {
				LATC = vector[number[digit-1+display*4]];
			}
			break;
		case 1:
			if ((digit+display*4)==0 || (digit+display*4)== 1 || (digit+display*4)== 6 || (digit+display*4)== 7){
				LATC = 0;
				LATB = 0;
			} else if((digit+display*4)==4){
				LATC = (vector[number[digit-2+display*4]])|0b10000000;
			} else {
				LATC = vector[number[digit-2+display*4]];
			}
			break;
		case 4:
			LATC = 0;
			LATB = 0;
			break;
		default:
			break;
	}
	INTCONbits.TMR0IF = 0; // reset overflow bit
	}
 if(INTCONbits.RABIF) 
      { 
		  if(PORTAbits.RA0 == 0){
			if(deep_sleep == 1){
				deep_sleep = 0;
			} else {
				if(clock_mode == 4 && pause == 1) clock_mode = 1;
				pause = 0;
			}
		  }			
		  if(PORTAbits.RA1 == 0){
			if(deep_sleep == 1){
				deep_sleep = 0;
			} else pause = 1;
		  }	
          INTCONbits.RABIF = 0;			//Clear RAB Interrupt Flag 
      } 
}

/* ****************** MAIN ****************** */
char a = 0;
void main(void)
{
	if(a==0){
		a = 1;
		InitOSC();     //Internal OSC 16MHz 
    	Interrupts_enable(); //Enable RA0 & RA1 interrupts
		TRISB =0;
		TRISC =0;
		TRISAbits.RA4 = 0;
		TRISAbits.RA5 = 0;
		PORTAbits.RA5 = 0;
		PORTAbits.RA4 = 1;
		OSCCONbits.IDLEN = 1;
	}
	Sleep();
}

 void InitOSC(void) 
 {    
      OSCCON  = 0b01010111;   //Internal 16MHz    
 } 
 void Interrupts_enable(void) 
 {    
	TRISA = 0xFF; //A port as input
    INTCON2bits.RABPU = 0; // Enable Pull-UP on port A-B
    INTCON2bits.RABIP = 1; // Change on port = High priority
	IOCA  = 0b000000011;        //Enable Interrupt on Change for Pin 4 of Port A 
    INTCONbits.RABIE = 1;      //Enable RA and RB Port Change Interrupt 

T0CONbits.T08BIT = 0; //16 bit
T0CONbits.T0CS = 0; //Internal Instruction Cycle Clock
T0CONbits.PSA = 1; 

T0CONbits.T08BIT = 0;
T0CONbits.T0CS = 0;
T0CONbits.PSA = 1;
TMR0H = 0xFC;
TMR0L = 0x18;
T0CONbits.TMR0ON = 1;

	RCONbits.IPEN       = 1;    //Enable Interrupt Priorities
    INTCONbits.GIEL     = 1;    //Enable Low Priority Interrupt
    INTCONbits.GIEH     = 1;    //Enable high priority interrupts        
    INTCONbits.TMR0IE   = 1;    //Enable Timer0 Interrupt
    INTCON2bits.TMR0IP  = 1;    //TMR0 set to low Priority Interrupt
    INTCONbits.TMR0IF = 0;  // T0 int flag bit cleared before starting
    T0CONbits.TMR0ON = 1;   // timer0 START
	INTCONbits.GIE = 1; 		   //Enable all unmasked interrupts   
 } 
 
void add_1ms(void){
char i = 0;
number[0]++;
	for(i=0; i<7;i++){
			if(number[3]>5){
				number[3] = 0;
				number[4]++;
			}
			if(number[5]>5){
				number[5] = 0;
				number[6]++;
			}
			if(number[i]>9){
				number[i+1]++;
				number[i] = 0;
			}
			if(number[i]<0){
				number[i+1]--;
				number[i] = 9;
			}
	}
}


