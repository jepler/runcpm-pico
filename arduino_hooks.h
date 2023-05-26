#pragma once

#include <stdbool.h>
#include <stdint.h>

extern bool (*_kbhit_hook)(void);
extern uint8_t (*_getch_hook)(void);
extern void (*_putch_hook)(uint8_t ch);

