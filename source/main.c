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

//--------------------------------------
// LED Matrix Demo SynchSM
// Period: 100 ms
//--------------------------------------
enum Demo_States {show_obs, show_player};
int Demo_Tick(int state) {

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
            pattern = 0x1F;
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
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;

    static task task1;
    task *tasks[] = {&task1};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
    
    task1.state = show_obs;
    task1.period = 1;
    task1.elapsedTime = task1.period;
    task1.TickFct = &Demo_Tick;

    unsigned long GCD = tasks[0]->period;
    for(unsigned long i = 1; i < numTasks; i++) {
        GCD = findGCD(GCD, tasks[i]->period);
    }

    TimerSet(GCD);
    TimerOn();
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
