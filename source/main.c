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
#include <stdio.h>
#include <stdlib.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "timer.h"
#include "bit.h"
#include "scheduler.h"
#include "speaker.h"
#endif

int get_period(int difficulty) {
    switch(difficulty) {
        case 1:
            return 200;
        case 2:
            return 112;
        case 3:
            return 100;
        default:
            return 200;
    }
}

unsigned char tempA = 0x00;
unsigned char obstacle = 0x0B;
unsigned char obstacle_position = 0x80;
unsigned char player = 0x04;
unsigned char difficulty = 0x01;
unsigned char score = 0x00;
unsigned short obs_period = 200;

unsigned char obstacles[8] = {0x0B, 0x19, 0x1E, 0x0D, 0x13, 0x15, 0x17, 0x07};
enum obstacle_states {obs_7, obs_6, obs_5, obs_4, obs_3, obs_2, obs_1, obs_0, obs_wait} obstacle_state;

double chromatic[37] = {220, 233.1, 246.9, 261.6, 277.2, 293.7, 311.1, 329.6, 349.2, 370,  392,  415.3, 
                      440, 466.2, 493.9, 523.3, 554.4, 587.3, 622.3, 659.3, 698.5, 740,  784,  830.6,
                      880, 932.3, 987.8, 1047,  1109,  1175,  1245,  1319,  1397,  1480, 1568, 1661, 0};

enum notes {a_1, b_flat_1, b_1, c_1, d_flat_1, d_1, e_flat_1, e_1, f_1, g_flat_1, g_1, a_flat_1,
            a_2, b_flat_2, b_2, c_2, d_flat_2, d_2, e_flat_2, e_2, f_2, g_flat_2, g_2, a_flat_2,
            a_3, b_flat_3, b_3, c_3, d_flat_3, d_3, e_flat_3, e_3, f_3, g_flat_3, g_3, a_flat_3, rest};

unsigned short title_melody_period = 333; // note = 8th note ; 3/4 time
unsigned char title_melody_size = 96;
unsigned char title_melody[96] = {a_2, a_1, e_1, a_1, a_2, b_2, c_2, c_2, c_2, c_2, b_2, a_2, 
                                  g_1, g_1, g_1, a_2, g_1, d_1, e_1, e_1, e_1, e_1, e_1, e_1,
                                  a_2, a_1, e_1, a_1, a_2, b_2, c_2, c_2, c_2, b_2, c_2, c_2,
                                  d_2, d_2, c_2, c_2, d_2, g_2, e_2, e_2, e_2, e_2, e_2, e_2,
                                  a_2, a_1, e_1, a_1, a_2, b_2, c_2, c_2, c_2, c_2, b_2, a_2,
                                  g_1, g_1, g_1, a_2, g_1, d_1, e_1, e_1, e_1, e_1, e_1, e_1,
                                  f_1, g_1, a_2, a_2, a_2, c_2, b_2, b_2, g_1, g_1, e_1, e_1, 
                                  a_2, a_2, a_2, a_2, e_2, d_2, d_flat_2, d_flat_2, d_flat_2, d_flat_2, d_flat_2, rest};

unsigned short gameplay_melody_period = 83; // note = 16th note; 4/4 time
unsigned char gameplay_melody_size = 128;
unsigned char gameplay_melody[128] = {a_flat_1, rest, a_flat_1, rest, rest, rest, a_flat_1, rest, rest, rest, a_flat_1, rest, rest, rest, a_flat_1, rest, 
                                   rest, rest, a_flat_1, rest, rest, rest, a_flat_1, rest, a_2, a_2, a_flat_1, a_flat_1, a_2, a_2, a_flat_1, rest,
                                   a_flat_1, rest, a_flat_1, rest, rest, rest, a_flat_1, rest, rest, rest, a_flat_1, rest, rest, rest, a_flat_1, rest,
                                   g_flat_1, g_flat_1, g_flat_1, g_flat_1, e_flat_1, e_flat_1, e_flat_1, e_flat_1, a_flat_1, a_flat_1, a_flat_1, a_flat_1, a_flat_1, a_flat_1, a_flat_1, rest,
                                   a_flat_1, rest, a_flat_1, rest, rest, rest, a_flat_1, rest, rest, rest, a_flat_1, rest, rest, rest, a_flat_1, rest, 
                                   rest, rest, a_flat_1, rest, rest, rest, a_flat_1, rest, a_2, a_2, a_flat_1, a_flat_1, a_2, a_2, a_flat_1, rest,
                                   a_flat_1, rest, a_flat_1, rest, rest, rest, a_flat_1, rest, rest, rest, a_flat_1, rest, rest, rest, a_flat_1, rest,
                                   a_2, a_2, a_2, a_2, b_flat_2, b_flat_2, b_flat_2, b_flat_2, b_2, b_2, b_2, b_2, c_2, c_2, c_2, rest};

unsigned short game_over_period = 133;
unsigned char game_over_size = 24;
unsigned char game_over_melody[24] = {a_flat_1, rest, g_1, g_flat_1, rest, f_1, e_1, rest, b_1, b_1, b_1, e_1, 
                                    e_flat_1, rest, rest, rest, rest, rest, e_flat_2, rest, rest, rest, rest, rest };

unsigned char melody_index = 0x00;
unsigned short melody_period = 100;

enum mus_states {mus_intro, mus_gameplay, mus_over} mus_state;
enum game_states {game_wait, game_start, game_playing, game_reset, game_over} game_state;

int music(int state) {
    state = mus_state;
    switch(state) {
        case mus_intro: 
            set_PWM(chromatic[title_melody[melody_index]]);
            melody_index = (melody_index + 1) % title_melody_size;
            break;
        case mus_gameplay:
            set_PWM(chromatic[gameplay_melody[melody_index]]);
            melody_index = (melody_index + 1) % gameplay_melody_size;
            break;
        case mus_over:
            if(melody_index >= game_over_size) {
                game_state = game_wait;
                melody_index = 0x00;
                mus_state = mus_intro;
                melody_period = title_melody_period;
                set_PWM(0);
            }
            else {
                set_PWM(chromatic[game_over_melody[melody_index]]);
                melody_index = melody_index + 1;
            }

            break;
        default:
            break;
    }
    return state;
}

enum disp_states {show_obs, show_player};
int display(int state) {

    // Local Variables
    static unsigned char pattern = 0x00;    // LED pattern - 0: LED off; 1: LED on
    static unsigned char row = 0x00;      // Row(s) displaying pattern. 
                            // 0: display pattern on row
                            // 1: do NOT display pattern on row

    if (game_state != game_playing) {
        if (game_state == game_wait){
            unsigned char diff_led = 0x00;
            for(unsigned char i = 0; i < difficulty; ++i) {
                diff_led = diff_led << 1;
                diff_led |= 0x01;
            }
            PORTA = (PORTA & 0x07) | (diff_led << 3);
            PORTD = 0x1F;
        } else {
            PORTA = (PORTA & 0x07) | (score << 3);
        }
        PORTC = 0x00;
        PORTD = (score & 0xE0) | 0x1F;
        return state;
    } else {
        PORTA = (PORTA & 0x07) | (score << 3);
        PORTD = (PORTD & 0x1F) | (score & 0xE0);
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
                pattern = obstacle;
                row = obstacle_position;
                break;
            case show_player:
                pattern = player;
                row = 0x01;
                break;
            default:
        break;
        }
        PORTC = row;    // Pattern to display
        PORTD = (PORTD & 0xE0) | (~pattern & 0x1F);        // Row(s) displaying pattern    
    }
    return state;
}

// enum game_states {game_wait, game_start, game_playing, game_reset, game_over} game_state; This is above, here just for reference
int game(int state) {
    tempA = ~PINA & 0x07;
    state = game_state;
    switch(state) {
        case game_wait: 
            if ((tempA & 0x07) == 0x02) {
                state = game_start;      // press middle button: start game
                srand(melody_index * 7);
                melody_index = 0x00;
                mus_state = mus_gameplay;
                melody_period = gameplay_melody_period;
                set_PWM(0);
                obstacle = obstacles[(rand() % 8)];
                obstacle_position = 0x80;
                obstacle_state = obs_0;
                obs_period = get_period(difficulty);
                score = 0;
            }
            break;
        case game_start: 
            if ((tempA & 0x07) == 0x02) state = game_start; 
            else state = game_playing;
            break;
        case game_playing: 
            if ((tempA & 0x07) == 0x02) state = game_reset;
            else if ((obstacle_position == 0x01) && ((obstacle & player) != 0x00 )){
                state = game_over;
                mus_state = mus_over;
                obstacle_state = obs_wait;
                melody_index = 0x00;
                melody_period = game_over_period;
                set_PWM(0);
            }
            else state = game_playing;
            break;
        case game_reset:
            if ((tempA & 0x07) == 0x02) state = game_reset;
            else {
                state = game_wait;
                melody_index = 0x00;
                mus_state = mus_intro;
                melody_period = title_melody_period;
                set_PWM(0);
            }
            break;
        case game_over:
            state = game_over;
            break;
        default: 
            state = game_wait;
            break;
    }
    game_state = state;
    return state;
}

enum control_states {control_wait, diff_left, diff_right, move_left, move_right};
int control_tick(int state) {
    tempA = ~PINA & 0x07;
    switch(state){ 
        case control_wait: 
            if(game_state == game_wait) {
                if (tempA == 0x01) {
                    state = diff_right;
                    if (difficulty > 1) {
                        difficulty -= 1; 
                    } 
                } else if (tempA == 0x04) {
                    state = diff_left;
                    if (difficulty < 3) {
                        difficulty += 1;
                    }
                }
            } else if (game_state == game_playing) {
                if (tempA == 0x01) {
                    state = move_right;
                    player = player == 1? player: player >> 1;
                } else if (tempA == 0x04) {
                    state = move_left;
                    player = player == 16? player: player << 1;
                }
            } else state = control_wait;
            break;
        case diff_left: 
            if (tempA == 0x00) state = control_wait;
            else state = diff_left;
            break;
        case diff_right:
            if (tempA == 0x00) state = control_wait;
            else state = diff_right;
            break;
        case move_left: 
            if (tempA == 0x00) state = control_wait;
            else state = move_left;
            break;
        case move_right: 
            if (tempA == 0x00) state = control_wait;
            else state = move_right;
            break;
        default: 
            state = control_wait;
    }
    return state;
}

// enum obstacle_states {obs_7, obs_6, obs_5, obs_4, obs_3, obs_2, obs_1, obs_0} obstacle_state;
int obstacle_tick(int state) {
    switch(obstacle_state){
        case obs_7:
            state = obs_6;
            obstacle_position = 0x40;
            break;
        case obs_6:
            state = obs_5;
            obstacle_position = 0x20;
            break;
        case obs_5:
            state = obs_4;
            obstacle_position = 0x10;
            break;
        case obs_4:
            state = obs_3;
            obstacle_position = 0x08;
            break;
        case obs_3:
            state = obs_2;
            obstacle_position = 0x04;
            break;
        case obs_2:
            state = obs_1;
            obstacle_position = 0x02;
            break;
        case obs_1:
            state = obs_0;
            obstacle_position = 0x01;
            break;
        case obs_0:
            state = obs_7;
            obstacle_position = 0x80;
            srand(melody_index * 7);
            obstacle = obstacles[rand() % 8];
            score += 1;
            break;
        case obs_wait:
            state = obs_wait;
    }
    obstacle_state = state;
    return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0xF8; PORTA = 0x07;
    DDRB = 0xFF; PORTB = 0x00;
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;

    static task task1, task2, task3, task4, task5;
    task *tasks[] = {&task1, &task2, &task3, &task4, &task5};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
    
    game_state = game_wait;
    task1.state = game_state;
    task1.period = 10;
    task1.elapsedTime = task1.period;
    task1.TickFct = &game;

    mus_state = mus_intro;
    melody_period = title_melody_period;
    task2.state = mus_state;
    task2.period = melody_period;
    task2.elapsedTime = task2.period;
    task2.TickFct = &music;

    obstacle_state = obs_7;
    task3.state = obstacle_state;
    task3.period = 200;
    task3.elapsedTime = task3.period;
    task3.TickFct = &obstacle_tick;

    task4.state = control_wait;
    task4.period = 5;
    task4.elapsedTime = task4.period;
    task4.TickFct = &control_tick;

    task5.state = show_obs;
    task5.period = 1;
    task5.elapsedTime = task5.period;
    task5.TickFct = &display;

    TimerSet(1);
    TimerOn();
    PWM_on();
    /* Insert your solution below */

    while (1) {
        // task3.state = game_state;
        task2.period = melody_period;
        task3.period = obs_period;
        for(unsigned long i = 0; i < numTasks; i++) {
            if(tasks[i]->elapsedTime >= tasks[i]->period) {
                tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
                tasks[i]->elapsedTime = 0;
            }
            tasks[i]->elapsedTime += 1;
        }
        while (!TimerFlag);
        TimerFlag = 0;
    }
    return 1;
}
