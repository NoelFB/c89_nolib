#ifndef NB_GAME_H
#define NB_GAME_H

/* Types */
#define NB_FLT   float
#define NB_INT   int
#define NB_UINT  unsigned int
#define NB_COL   unsigned int
#define NB_BOOL  int
#define NB_TRUE  1
#define NB_FALSE 0
#define NB_STR   const char*
#define NB_NULL  0

/* Controls */
#define BTN_LEFT  0
#define BTN_RIGHT 1
#define BTN_UP    2
#define BTN_DOWN  3
#define BTN_A     4
#define BTN_B     5
#define BTN_COUNT 6

/* Game Entry function, called via the Platform */
void    game_entry();

/* Platform API */
void    platform_init(NB_STR name, NB_UINT fps, NB_UINT width, NB_UINT height, NB_COL* screen, NB_BOOL* buttons);
NB_BOOL platform_poll();
void    platform_present();

#endif