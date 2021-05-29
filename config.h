#ifndef CONFIG_H
#define CONFIG_H

typedef struct
{
    unsigned int color_background;
    unsigned int color_main_theme;

    //maybe even setting up the source file there?
} config;

extern config global;

#endif