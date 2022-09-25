a very simple experiment to see if i can make a game in C89 without the standard library.

example building:
```
gcc -o game game.c win32.c C:\Windows\System32\user32.dll C:\Windows\System32\Gdi32.dll C:\Windows\System32\kernel32.dll -nostdlib -std=c89 -s -Wall -pedantic
```

for non-windows platforms you'd need to implement the platform functions defined in game.h:
```
void    platform_init(NB_STR name, NB_UINT fps, NB_UINT width, NB_UINT height, NB_COL* screen, NB_BOOL* buttons);
NB_BOOL platform_poll();
void    platform_present();
```

and then call `game_entry()`