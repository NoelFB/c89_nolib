a very simple experiment to see if i can make a game in C89 without the standard library.

Windows example building with GCC:
 - only compile `game.c`, it includes everything else.
 - use the equivalent of `-nostdlib` and `-std=c89`
```
gcc game.c -nostdlib -std=c89 -e _start C:\Windows\System32\user32.dll C:\Windows\System32\Gdi32.dll C:\Windows\System32\kernel32.dll
```

For non-Windows, a different Platform implementation must to be defined, which in turn should call `nb_init` and `nb_step`.