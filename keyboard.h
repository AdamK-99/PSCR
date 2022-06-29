#ifndef KEYBOARD_H_
#define KEYBOARD_H_

extern mqd_t keyboardMQueue;

void set_initial_level();
void init_keyboard();

#endif