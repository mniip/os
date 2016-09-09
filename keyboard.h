#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

extern uint8_t key_state[0x80];
extern int poll_event(uint32_t *);
extern int poll_char(char *);
extern int poll_string(char *);

#define EVENT_KEY(e) ((e) & 0xFFFFFF7F)
#define EVENT_CODE1(e) ((e) & 0x7F)
#define EVENT_CODE2(e) ((e) >> 8)
#define EVENT_RELEASED(e) ((e) & 0x80)
#define KEY_DOWN(e) (key_state[(e) & 0x80] & (1 << ((e) >> 8)))

#endif
