#pragma once

extern bool (*_kbhit_hook)(void);
extern uint8_t (*_getch_hook)(void);
extern void (*_putch_hook)(uint8_t ch);

