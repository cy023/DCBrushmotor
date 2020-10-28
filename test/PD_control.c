#define F_CPU 11059200UL
#define PI 3.1415926
#define DUTY_SET(A) OCR1A = (uint16_t)(A * 0.86);

#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

#include "../src/uart.h"
#include "../src/pid.h"

volatile uint32_t ADC_divid = 0;
volatile uint32_t SIN_divid = 0;
volatile double sinData[64] = {0};
volatile uint16_t ADCresult = 0;
volatile uint16_t ADCresult_lest = 0;
volatile uint16_t initial = 0;

PID_t PIDStr_t;

void PWM_set()
{
    TCCR1A |= (0 << WGM10) | (0 << WGM11); 				/* Phase Correct Mode Select [mode:8] */
    TCCR1A |= (1 << COM1A0) | (1 << COM1A1);
    TCCR1B |= (0 << WGM12) | (1 << WGM13); 				/* Phase Correct Mode Select */
    TCCR1B |= (1 << CS10 ) | (1 << CS11) | (0 << CS12); /* Prescaler : 64 */
    DDRB |= (1 << PB5);
    ICR1 = 86;
    TCNT1 = 0;
    TIMSK |= (1 << TICIE1);
}

void ADC_set()
{
    ADMUX |= (0 << REFS1) | (0 << REFS0); 					/* ADC_REF_AREF */
    ADCSRA |= (0 << ADPS0)| (0 << ADPS1) | (0 << ADPS2);	/* Prescaler : 2 */
    ADCSRA |= (0 << ADFR);  								/* ADC_CONVMODE_TRIGERED */
    ADCSRA |= (1 << ADEN); 									/* ADC Enable. */
    ADCSRA |= (1 << ADIE);  								/* ADC Interrupt Enable. */
    ADCSRA |= (1 << ADSC);  								/* ADC start conversion. */
}

int main(void)
{
    UART_init();
    PWM_set();
    ADC_set();

    PIDStr_t.PID_lesterror.d_lerr=0;
    PIDStr_t.PID_gain.Kd = 1;

    DDRD |= (1 << DDD6);
    PORTD |= (1 << PD6);
    // DUTY_SET(100);

    // for (int i = 0; i < 64; i++)
    //     sinData[i] = (sin(((double)i/64)*2*PI) + 1) / 2;
    
    printf("plz scan cmd\n");
    scanf("%f", &(PIDStr_t.Cmd));
    printf("PIDStr_t.Cmd = %f\n", PIDStr_t.Cmd);
    
    sei();

    while (1) { 
        PIDStr_t.PID_lesterror.d_lerr = (float)(ADCresult-ADCresult_lest)/0.064;
        PIDStr_t.PID_error.d_err =  (PIDStr_t.Cmd  - PIDStr_t.PID_lesterror.d_lerr);
        if (PIDStr_t.Cmd >= 0)
            PIDStr_t.FB = 40 - (PIDStr_t.PID_gain.Kd  * PIDStr_t.PID_error.d_err) / 5;
        else
            PIDStr_t.FB = 60 - (PIDStr_t.PID_gain.Kd  * PIDStr_t.PID_error.d_err) / 5;
        ADCresult_lest = ADCresult;
        PIDStr_t.FB = PIDStr_t.PID_error.d_err * PIDStr_t.PID_gain.Kd;
        DUTY_SET(PIDStr_t.FB);
        printf("%f\n",PIDStr_t.FB);
    }
    return 0;
}

ISR (TIMER1_CAPT_vect)
{
    ADC_divid++;
    // if (initial && !(ADC_divid % 32)) {
    //     SIN_divid++;
    //     DUTY_SET(sinData[SIN_divid%64] * 100);
    // }
    if (!(ADC_divid % 64))
        ADCSRA |= (1 << ADSC);  /* ADC conversion */
}

ISR (ADC_vect)
{
    uint8_t resultH = 0;
    uint8_t resultL = 0;
    resultL = ADCL;
    resultH = ADCH;
    ADCresult = (resultH << 8) + resultL;
    printf("%d\n", (resultH << 8) + resultL);
    // ADCbuffer[batchIndex] = (resultH << 8) + resultL;
    // batchIndex++;
}