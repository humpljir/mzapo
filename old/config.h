#ifndef CONFIG_H
#define CONFIG_H

typedef struct
{
    //global settings
    unsigned int color_background;
    unsigned int color_main_theme;

    //maybe add even selected source file here?
} config;

extern config global;

#endif