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
    HDC      buffer_memory;
    HBITMAP  buffer_bitmap;
} platform;

void nb_win32_run();
void nb_win32_present();
LRESULT CALLBACK nb_win32_poll(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void nb_win32_run()
{
	WNDCLASS wc;
	HDC dc;
	MSG msg;

	/* Create Window Class */
	wc.lpfnWndProc = DefWindowProc;
	wc.lpszClassName = "NB_WINDOW";
	wc.lpfnWndProc = nb_win32_poll;
	wc.hIcon = NULL;
	wc.lpszMenuName = NULL;
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
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
	platform.closing = NB_FALSE;
	platform.framecounter = 0;

	nb_init();

	while (1)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (platform.closing)
			break;

		nb_step();
		nb_win32_present();

		/* Faking V-Sync */
		platform.framecounter += (1000.0 / NB_FRAMERATE);
		Sleep((INT)platform.framecounter);
		platform.framecounter -= (INT)platform.framecounter;
	}
	
	/* shutdown */
    DeleteDC(platform.buffer_memory);
    DeleteObject(platform.buffer_bitmap);
	DestroyWindow(platform.window);
}

LRESULT CALLBACK nb_win32_poll(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
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
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void nb_win32_present()
{
	HDC dc;
	FLOAT w, h, s;
	RECT size;
	HBRUSH bg;
    BITMAPINFO bminfo;
	
	dc = GetDC(platform.window);

	/* Get Screen Size */
	GetClientRect(platform.window, &size);
	s = min((size.right - size.left) / (FLOAT)NB_WIDTH, (size.bottom - size.top) / (FLOAT)NB_HEIGHT);
	w = NB_WIDTH * s;
	h = NB_HEIGHT * s;

	/* Draw Viewport Edges */
	bg = CreateSolidBrush(RGB(0, 0, 0));
	FillRect(dc, &size, bg);
	DeleteObject(bg);

	/* Print Screen */
	bminfo.bmiHeader.biSize = sizeof(bminfo.bmiHeader);
    bminfo.bmiHeader.biWidth = NB_WIDTH;
    bminfo.bmiHeader.biHeight = -(NB_INT)NB_HEIGHT;
    bminfo.bmiHeader.biPlanes = 1;
    bminfo.bmiHeader.biBitCount = 32;
    bminfo.bmiHeader.biCompression = BI_RGB;
    bminfo.bmiHeader.biXPelsPerMeter = 1;
    bminfo.bmiHeader.biYPelsPerMeter = 1;
    SetDIBits(platform.buffer_memory, platform.buffer_bitmap, 0, NB_HEIGHT, nb_game.screen, &bminfo, 0);
	StretchBlt(dc, 
		((size.right - size.left) - w) / 2, ((size.bottom - size.top) - h) / 2, w, h, 
		platform.buffer_memory, 0, 0, NB_WIDTH, NB_HEIGHT, SRCCOPY);

	/* Finish */
	ReleaseDC(platform.window, dc);
}

/* without libc wrapping it, _start() is the correct entry point */
void _start(void)
{
	nb_win32_run();
}

#endif /* _WIN32 */