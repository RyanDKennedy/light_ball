#pragma once

#include <string.h>

typedef struct
{
    char *buf;
    int width;
    int height;
    int buf_size;
} Screen;

Screen screen_create(int width, int height);
void screen_destroy(Screen *screen);
void screen_empty(Screen *screen);
void screen_print(Screen *screen);
void screen_write(Screen *screen, int x, int y, char c);
