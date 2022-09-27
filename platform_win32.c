/* 
 * Note: Do not compile this file!
 * It is included in game.c
 * game.c should be the only file you compile
 */

#ifdef _WIN32

#include "game.h"
#include <windows.h>

struct Platform
{
    NB_FLT   framecounter;
    NB_BOOL  closing;
    HWND     window;
    NB_RGB   buffer[NB_WIDTH * NB_HEIGHT];
    HDC      buffer_memory;
    HBITMAP  buffer_bitmap;
    HBRUSH   bg_brush;
} platform;

LRESULT CALLBACK nb_win32_poll(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void nb_win32_run()
{
    INT x, y;
    WNDCLASS wc;
    HDC dc;
    MSG msg;
    BITMAPINFO bminfo;

    /* Background Color is black */
    platform.bg_brush = CreateSolidBrush(RGB(0, 0, 0));
    platform.closing = NB_FALSE;
    platform.framecounter = 0;

    /* Create Window Class */
    wc.lpfnWndProc = DefWindowProc;
    wc.lpszClassName = "NB_WINDOW";
    wc.lpfnWndProc = nb_win32_poll;
    RegisterClass(&wc);

    /* Open Window */
    platform.window = CreateWindow("NB_WINDOW", NB_TITLE, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, NB_WIDTH * 4, NB_HEIGHT * 4, NULL, NULL, NULL, NULL);
    ShowWindow(platform.window, SW_SHOW);

    /* create screen buffer */
    dc = GetDC(platform.window);
    platform.buffer_memory = CreateCompatibleDC(dc);
    platform.buffer_bitmap = CreateCompatibleBitmap(dc, NB_WIDTH, NB_HEIGHT);
    SelectObject(platform.buffer_memory, platform.buffer_bitmap);
    ReleaseDC(platform.window, dc);

    /* run the game */
    nb_init();
    while (!platform.closing)
    {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        nb_step();
        
        /* Copy Palette Screen data to our RGB buffer */
        for (x = 0; x < NB_WIDTH; x ++)
        for (y = 0; y < NB_HEIGHT; y ++)
            platform.buffer[x + y * NB_WIDTH] = nb_game.palette[nb_game.screen[x + y * NB_WIDTH]];

        /* Copy to buffer */
        bminfo.bmiHeader.biSize = sizeof(bminfo.bmiHeader);
        bminfo.bmiHeader.biWidth = NB_WIDTH;
        bminfo.bmiHeader.biHeight = -(NB_INT)NB_HEIGHT;
        bminfo.bmiHeader.biPlanes = 1;
        bminfo.bmiHeader.biBitCount = 32;
        bminfo.bmiHeader.biCompression = BI_RGB;
        bminfo.bmiHeader.biXPelsPerMeter = 1;
        bminfo.bmiHeader.biYPelsPerMeter = 1;
        SetDIBits(platform.buffer_memory, platform.buffer_bitmap, 0, NB_HEIGHT, platform.buffer, &bminfo, 0);

        /* Tell WM_PAINT to run */
        InvalidateRect(platform.window, NULL, FALSE);

        /* Faking V-Sync */
        platform.framecounter += (1000.0 / NB_FRAMERATE);
        Sleep((INT)platform.framecounter);
        platform.framecounter -= (INT)platform.framecounter;
    }
    
    /* shutdown */
    DeleteObject(platform.bg_brush);
    DeleteDC(platform.buffer_memory);
    DeleteObject(platform.buffer_bitmap);
    DestroyWindow(platform.window);
}

LRESULT CALLBACK nb_win32_poll(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC dc;
    FLOAT s;
    RECT size, fill, client;

    switch (msg)
    {
    case WM_CLOSE:
        platform.closing = NB_TRUE;
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_KEYDOWN:
        if (wParam == VK_RIGHT) nb_game.btn[NB_RIGHT] = NB_TRUE;
        if (wParam == VK_LEFT) nb_game.btn[NB_LEFT] = NB_TRUE;
        if (wParam == VK_UP) nb_game.btn[NB_UP] = NB_TRUE;
        if (wParam == VK_DOWN) nb_game.btn[NB_DOWN] = NB_TRUE;
        return 0;
    case WM_KEYUP:
        if (wParam == VK_RIGHT) nb_game.btn[NB_RIGHT] = NB_FALSE;
        if (wParam == VK_LEFT) nb_game.btn[NB_LEFT] = NB_FALSE;
        if (wParam == VK_UP) nb_game.btn[NB_UP] = NB_FALSE;
        if (wParam == VK_DOWN) nb_game.btn[NB_DOWN] = NB_FALSE;
        return 0;
    case WM_PAINT:
        dc = BeginPaint(hwnd, &ps);

        /* Get Screen Size */
        GetClientRect(platform.window, &client);
        s = (INT)min((client.right - client.left) / (FLOAT)NB_WIDTH, (client.bottom - client.top) / (FLOAT)NB_HEIGHT);
        size.left = ((client.right - client.left) - NB_WIDTH * s) / 2;
        size.top = ((client.bottom - client.top) - NB_HEIGHT * s) / 2;
        size.right = size.left + NB_WIDTH * s;
        size.bottom = size.top + NB_HEIGHT * s;

        /* Fill outside with black */
        SetRect(&fill, 0, 0, size.left, client.bottom); FillRect(dc, &fill, platform.bg_brush);
        SetRect(&fill, size.right, 0, client.right, client.bottom); FillRect(dc, &fill, platform.bg_brush);
        SetRect(&fill, size.left, 0, size.right, size.top); FillRect(dc, &fill, platform.bg_brush);
        SetRect(&fill, size.left, size.bottom, size.right, client.bottom); FillRect(dc, &fill, platform.bg_brush);

        /* Display screen on window */
        StretchBlt(dc, size.left, size.top, size.right - size.left, size.bottom - size.top, 
            platform.buffer_memory, 0, 0, NB_WIDTH, NB_HEIGHT, SRCCOPY);

        EndPaint(hwnd, &ps);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

/* without libc wrapping it, _start() is the correct entry point */
void _start(void)
{
    nb_win32_run();
}

#endif /* _WIN32 */