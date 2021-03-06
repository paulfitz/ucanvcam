/*
 * EffecTV - Realtime Digital Video Effector
 * Copyright (C) 2001-2006 FUKUCHI Kentaro
 *
 * screen.h: header for screen manager
 *
 */

#ifndef __SCREEN_H__
#define __SCREEN_H__

#ifdef SDL_IS_AVAILABLE
#  include <SDL/SDL.h>
#else 
typedef void *SDL_Event;
#endif

#if defined(WIN32) || defined(WIN64)
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif /* Def WIN32 or Def WIN64 */

//extern SDL_Surface *screen; /* display surface */  //PFHIT

struct Screeny {
  void *pixels;
};

extern struct Screeny *screen;

extern int stretch;         /* flag for stretching requirement */
extern int doublebuf;       /* flag for double buffering */
extern int fullscreen;      /* flag for fullscreen mode */
extern int hwsurface;       /* flag for hardware surface */
extern int screen_width;
extern int screen_height;
extern int screen_scale;

/* description of these functions are in screen.c */
int screen_init(int w, int h, int s);
void screen_quit(void);
int screen_bpp(void);
void screen_setcaption(const char *str);
void screen_clear(int color);
void screen_fullscreen(void);
int screen_lock(void);
void screen_unlock(void);

#ifndef RGB_BGR_CONVERSION
#define screen_getaddress() (screen->pixels)
#else
unsigned char *screen_getaddress(void);
#endif

#ifndef RGB_BGR_CONVERSION
#define screen_update() (SDL_Flip(screen))
#else
int screen_update(void);
#endif

#endif /* __SCREEN_H__ */
