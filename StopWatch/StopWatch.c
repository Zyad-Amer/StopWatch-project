/*
 * StopWatch.c
 *
 *  Created on: Feb 2, 2024
 *      Author: Zyad Montaser
 */

#include<avr/io.h>
#include"util/delay.h"
#include <avr/interrupt.h>

/* Global flag to execute ISR of timer 1 */
unsigned char g_Timer1_interrupt_flag = 0;

/* Enumeration for different the 7-segments  */
enum Seven_Segment
{
	Unit_Of_Seconds_7_Segment,
	Tens_Of_Seconds_7_Segment,
	Unit_Of_Minutes_7_Segment,
	Tens_Of_Minutes_7_Segment,
	Unit_Of_Hours_7_Segment,
	Tens_Of_Hours_7_Segment
};

/* Global enumeration */
enum Seven_Segment Num_Of_7_Segment;

/* Global array to store the digits for each 7-segment */
unsigned char Digits_Of_7Segments[6];

/* Function to enable a specific 7-segment by its number */
void Enable_7_Segment(char Num_Of_7_Segment)
{
	switch (Num_Of_7_Segment)
	{
	case Unit_Of_Seconds_7_Segment:
		/* Enable 7-segment number 0 (units of seconds) and disable the others */
		PORTA = (PORTA & 0X80) | 0X01;
		break;
	case Tens_Of_Seconds_7_Segment:
		/* Enable 7-segment number 1 (tens of seconds) and disable the others */
		PORTA = (PORTA & 0X80) | 0X02;
		break;
	case Unit_Of_Minutes_7_Segment:
		/* Enable 7-segment number 2 (units of minutes) and disable the others */
		PORTA = (PORTA & 0X80) | 0X04;
		break;
	case Tens_Of_Minutes_7_Segment:
		/* Enable 7-segment number 3 (tens of minutes) and disable the others */
		PORTA = (PORTA & 0X80) | 0X08;
		break;
	case Unit_Of_Hours_7_Segment:
		/* Enable 7-segment number 4 (units of hours) and disable the others */
		PORTA = (PORTA & 0X80) | 0X10;
		break;
	case Tens_Of_Hours_7_Segment:
		/* Enable 7-segment number 5 (tens of hours) and disable the others */
		PORTA = (PORTA & 0X80) | 0X20;
		break;
	}
}

/* Function to display a number on the 7-segments */
void Show_Num_On_7_Segments_With_Decoder(char Value_To_Displayed)
{
	PORTC = (PORTC & 0xF0) | Value_To_Displayed;
}

/* Function to pause the stopwatch */
void Pause_StopWatch(void)
{
	/* Disconnect the clock from the timer (No clock source mode) */
	TCCR1B &= (~(1 << CS10) & ~(1 << CS11) & ~(1 << CS12));
}

/* Function to resume the stopwatch */
void Resume_StopWatch(void)
{
	/* Reconnect the clock to the timer */
	TCCR1B |= ((1 << CS10) | (1 << CS12));
}

/* Function to reset the stopwatch */
void Reset_StopWatch(void)
{
	/* Clear all variables */
	Digits_Of_7Segments[0] = 0;
	Digits_Of_7Segments[1] = 0;
	Digits_Of_7Segments[2] = 0;
	Digits_Of_7Segments[3] = 0;
	Digits_Of_7Segments[4] = 0;
	Digits_Of_7Segments[5] = 0;
}

/* Function to increase the time */
void Increase_Time(void)
{
	Digits_Of_7Segments[0]++;
	if (Digits_Of_7Segments[0] == 10)
	{
		Digits_Of_7Segments[1]++;
		Digits_Of_7Segments[0] = 0;
	}
	if (Digits_Of_7Segments[1] == 6)
	{
		Digits_Of_7Segments[2]++;
		Digits_Of_7Segments[1] = 0;
		Digits_Of_7Segments[0] = 0;
	}
	if (Digits_Of_7Segments[2] == 10)
	{
		Digits_Of_7Segments[3]++;
		Digits_Of_7Segments[2] = 0;
		Digits_Of_7Segments[1] = 0;
		Digits_Of_7Segments[0] = 0;
	}
	if (Digits_Of_7Segments[3] == 6)
	{
		Digits_Of_7Segments[4]++;
		Digits_Of_7Segments[3] = 0;
		Digits_Of_7Segments[2] = 0;
		Digits_Of_7Segments[1] = 0;
		Digits_Of_7Segments[0] = 0;
	}
	if (Digits_Of_7Segments[4] == 10)
	{
		Digits_Of_7Segments[5]++;
		Digits_Of_7Segments[4] = 0;
		Digits_Of_7Segments[3] = 0;
		Digits_Of_7Segments[2] = 0;
		Digits_Of_7Segments[1] = 0;
		Digits_Of_7Segments[0] = 0;
	}
	if (Digits_Of_7Segments[5] == 10)
	{
		Reset_StopWatch();
	}
}

/* Function to initialize external interrupts INT0, INT1, and INT2 */
void INT_0_1_2_Intialization(void)
{
	/* Configure interrupt triggers */
	MCUCR |= (1 << ISC10) | (1 << ISC11) | (1 << ISC01);
	MCUCR &= ~(1 << ISC00); /* Trigger INT0 with the falling edge & INT1 with the rising edge */
	MCUCSR &= ~(1 << ISC2); /* Trigger INT2 with the falling edge */
	/* Enable external interrupt pins of INT0, INT1, and INT2 */
	GICR |= (1 << INT0) | (1 << INT1) | (1 << INT2);
}

/* Interrupt Service Routines (ISRs) for external interrupts */
ISR(INT0_vect)
{
	Reset_StopWatch(); /* Reset stopwatch on INT0 trigger */
}

ISR(INT1_vect)
{
	Pause_StopWatch(); /* Pause stopwatch on INT1 trigger */
}

ISR(INT2_vect)
{
	Resume_StopWatch(); /* Resume stopwatch on INT2 trigger */
}

/* Function to initialize Timer 1 for CTC mode */
void Timer1_CTC_Intialization(void)
{
	TCCR1A = (1 << FOC1A); /* Enable for non-PWM mode */
	TCCR1B = (1 << WGM12) | (1 << CS10) | (1 << CS12); /* Enable compare mode and set prescaler to 1024 */
	TCNT1 = 0; /* Set the initial value */
	OCR1A = 1000; /* Set the compare value */
	TIMSK |= (1 << OCIE1A); /* Enable compare match interrupt bit */
}

/* ISR for Timer 1 compare match */
ISR(TIMER1_COMPA_vect)
{
	g_Timer1_interrupt_flag = 1; /* Set flag to indicate timer interrupt */
}

int main()
{
	DDRA |= 0X1F; /* Configure pins as output from PA0 --> PA5 */
	DDRC |= 0X0F; /* Configure pins as output from PC0 --> PC3 */
	DDRB &= ~(1 << PB2); /* Configure pin 2 in PORTB as input pin */
	DDRD &= (~(1 << PD2) & ~(1 << PD3)); /* Configure pin 2 & 3 in PORTD as input pins */
	PORTD |= (1 << PD2); /* Activate internal pull-up resistor at pin 2 in PORTD */
	SREG |= (1 << 7); /* Enable global interrupt bit */

	Reset_StopWatch(); /* Initialize all variables to 0 */

	Timer1_CTC_Intialization(); /* Start Timer 1 for counting */

	INT_0_1_2_Intialization(); /* Initialize external interrupts */

	while (1)
	{
		/* Check if Timer 1 interrupt flag is set */
		if (g_Timer1_interrupt_flag == 1)
		{
			Increase_Time(); /* Increment time */
			g_Timer1_interrupt_flag = 0; /* Reset interrupt flag */
		}

		/* Using multiplexing technique to display all the 7 segments at the same time */
		for (Num_Of_7_Segment = 0; Num_Of_7_Segment < 6; Num_Of_7_Segment++)
		{
			Enable_7_Segment(Num_Of_7_Segment); /* Enable current segment */
			Show_Num_On_7_Segments_With_Decoder(Digits_Of_7Segments[Num_Of_7_Segment]); /* Display digit */
			_delay_ms(2); /* Delay for stable display */
		}
	}
}
