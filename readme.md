a very simple experiment to see if i can make a game in C89 without the standard library.

Windows example building with GCC:
 - only compile `game.c`, it includes everything else.
 - use the equivalent of `-nostdlib` and `-std=c89`
```
gcc game.c -nostdlib -std=c89 C:\Windows\System32\user32.dll C:\Windows\System32\Gdi32.dll C:\Windows\System32\kernel32.dll
```

for non-windows platforms you'd need to implement the platform functions defined in `game.h`:
```
void     nb_platform_init();
NB_BOOL  nb_platform_poll();
void     nb_platform_present();
```

and then call `nb_run()`