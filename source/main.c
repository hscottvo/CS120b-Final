/*	Author: lab
 *  Partner(s) Name: Scott Vo
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "timer.h"
#include "bit.h"
#include "scheduler.h"
#endif

void set_PWM(double frequency) {
    static double current_frequency;

    if (frequency != current_frequency) {
        if (!frequency) {TCCR3B &= 0x08;}
        else {TCCR3B |= 0x03;}

        if (frequency < 0.954) {OCR3A = 0xFFFF; }

        else if (frequency > 31250) { OCR3A = 0x0000; }

        else {OCR3A = (short)(8000000 / (128 * frequency)) - 1;}

        TCNT3 = 0;
        current_frequency = frequency;
    }
}

void PWM_on() {
    TCCR3A = (1 << COM3A0);

    TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);

    set_PWM(0);
}

void PWM_off() {
    TCCR3A = 0x00;
    TCCR3B = 0x00;
}

double chromatic[36] = {220, 233.1, 246.9, 261.6, 277.2, 293.7, 311.1, 329.6, 349.2, 370,  392,  415.3, 
                      440, 466.2, 493.9, 523.3, 554.4, 587.3, 622.3, 659.3, 698.5, 740,  784,  830.6,
                      880, 932.3, 987.8, 1047,  1109,  1175,  1245,  1319,  1397,  1480, 1568, 1661, 0};

enum notes {a_1, b_flat_1, b_1, c_1, d_flat_1, d_1, e_flat_1, e_1, f_1, g_flat_1, g_1, a_flat_1,
            a_2, b_flat_2, b_2, c_2, d_flat_2, d_2, e_flat_2, e_2, f_2, g_flat_2, g_2, a_flat_2,
            a_3, b_flat_3, b_3, c_3, d_flat_3, d_3, e_flat_3, e_3, f_3, g_flat_3, g_3, a_flat_3, rest};

unsigned short title_melody_period = 333;
unsigned char title_melody_size = 96;
unsigned char title_melody[96] = {a_2, a_1, e_1, a_1, a_2, b_2, c_2, c_2, c_2, c_2, b_2, a_2, 
                                  g_1, g_1, g_1, a_2, g_1, d_1, e_1, e_1, e_1, e_1, e_1, e_1,
                                  a_2, a_1, e_1, a_1, a_2, b_2, c_2, c_2, c_2, b_2, c_2, c_2,
                                  d_2, d_2, c_2, c_2, d_2, g_2, e_2, e_2, e_2, e_2, e_2, rest,
                                  a_2, a_1, e_1, a_1, a_2, b_2, c_2, c_2, c_2, c_2, b_2, a_2,
                                  g_1, g_1, g_1, a_2, g_1, d_1, e_1, e_1, e_1, e_1, e_1, e_1,
                                  f_1, g_1, a_2, a_2, a_2, c_2, b_2, b_2, g_1, g_1, e_1, e_1, 
                                  a_2, a_2, a_2, a_2, e_2, d_2, d_flat_2, d_flat_2, d_flat_2, d_flat_2, d_flat_2, rest};

unsigned char melody_index = 0x00;

enum mus_states {mus_intro, mus_gameplay, mus_over} mus_state;

int music(int state) {
    state = mus_state;
    switch(state) {
        case mus_intro: 
            set_PWM(chromatic[title_melody[melody_index]]);
            melody_index = (melody_index + 1) % title_melody_size;
            break;
        case mus_gameplay:
            break;
        case mus_over:
            break;
        default:
            break;
    }
}

enum disp_states {show_obs, show_player};
int display(int state) {

    // Local Variables
    static unsigned char pattern = 0x00;    // LED pattern - 0: LED off; 1: LED on
    static unsigned char row = 0x00;      // Row(s) displaying pattern. 
                            // 0: display pattern on row
                            // 1: do NOT display pattern on row
    // Transitions
    switch (state) {
        case show_obs:  
            state = show_player;  
            break;
        case show_player:
            state = show_obs;
            break;
        default:    
            state = show_obs;
            break;
    }    
    // Actions
    switch (state) {
        case show_obs:    
            pattern = 0x1B;
            row = 0x80;
            break;
        case show_player:
            pattern = 0x04;
            row = 0x01;
            break;
        default:
    break;
    }
    PORTC = row;    // Pattern to display
    PORTD = ~pattern;        // Row(s) displaying pattern    
    return state;
}


int main(void) {
    /* Insert DDR and PORT initializations */
    DDRB = 0x40; PORTB = 0xBF;
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;

    static task task1, task2;
    task *tasks[] = {&task1, &task2};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
    
    task1.state = show_obs;
    task1.period = 1;
    task1.elapsedTime = task1.period;
    task1.TickFct = &display;

    mus_state = mus_intro;
    task2.state = mus_state;
    task2.period = title_melody_period;
    task2.elapsedTime = task2.period;
    task2.TickFct = &music;

    unsigned long GCD = tasks[0]->period;
    for(unsigned long i = 1; i < numTasks; i++) {
        GCD = findGCD(GCD, tasks[i]->period);
    }

    TimerSet(GCD);
    TimerOn();
    PWM_on();
    /* Insert your solution below */

    while (1) {
        for(unsigned long i = 0; i < numTasks; i++) {
            if(tasks[i]->elapsedTime == tasks[i]->period) {
                tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
                tasks[i]->elapsedTime = 0;
            }
            tasks[i]->elapsedTime += GCD;
        }
        while (!TimerFlag);
        TimerFlag = 0;
    }
    return 1;
}
