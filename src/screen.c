#include "screen.h"

#include <stdio.h>
#include <stdlib.h>

Screen screen_create(int width, int height)
{
    Screen result;
    result.width = width;
    result.height = height;
    result.buf_size = result.width * result.height, + 1;
    result.buf = calloc(result.buf_size, sizeof(char));
    screen_empty(&result);
    return result;
}

void screen_destroy(Screen *screen)
{
    free(screen->buf);
}

void screen_empty(Screen *screen)
{
    memset(screen->buf, ' ', screen->width * screen->height * sizeof(char));
}

void screen_print(Screen *screen)
{
    for (int i = 0; i < screen->height; ++i)
    {
	printf("%.*s\n", screen->width, screen->buf + i * screen->width);
    }

    printf("\033[%dF\n", screen->height + 1);
}

void screen_write(Screen *screen, int x, int y, char c)
{
    screen->buf[(screen->height - y - 1) * screen->width + x] = c;
}


