#include "arduino_hooks.h"

bool (*_kbhit_hook)(void);
uint8_t (*_getch_hook)(void);
void (*_putch_hook)(uint8_t ch);

