#include "game.h"
#include <windows.h>

struct Platform
{
	NB_STR   name;
	NB_UINT  fps;
	NB_UINT  width;
	NB_UINT  height;
	NB_COL*  screen;
	NB_BOOL* buttons;
	NB_FLT   framecounter;
	NB_BOOL  closing;
	HWND     window;
    HDC      buffer_memory;
    HBITMAP  buffer_bitmap;
} platform;

/* without libc wrapping it, _start() is the correct entry point */
void _start(void)
{
	/* run the game */
    game_entry();
	
	/* shutdown */
    DeleteDC(platform.buffer_memory);
    DeleteObject(platform.buffer_bitmap);
	DestroyWindow(platform.window);
}

LRESULT CALLBACK perform_poll(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
		if (wParam == VK_RIGHT) platform.buttons[BTN_RIGHT] = NB_TRUE;
		if (wParam == VK_LEFT) platform.buttons[BTN_LEFT] = NB_TRUE;
		if (wParam == VK_UP) platform.buttons[BTN_UP] = NB_TRUE;
		if (wParam == VK_DOWN) platform.buttons[BTN_DOWN] = NB_TRUE;
		return 0;
	case WM_KEYUP:
		if (wParam == VK_RIGHT) platform.buttons[BTN_RIGHT] = NB_FALSE;
		if (wParam == VK_LEFT) platform.buttons[BTN_LEFT] = NB_FALSE;
		if (wParam == VK_UP) platform.buttons[BTN_UP] = NB_FALSE;
		if (wParam == VK_DOWN) platform.buttons[BTN_DOWN] = NB_FALSE;
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void platform_init(NB_STR name, NB_UINT fps, NB_UINT width, NB_UINT height, NB_COL* screen, NB_BOOL* buttons)
{
	WNDCLASS wc;
	HDC dc;

	platform.name = name;
	platform.fps = fps;
	platform.width = width;
	platform.height = height;
	platform.screen = screen;
	platform.closing = NB_FALSE;
	platform.buttons = buttons;
	platform.framecounter = 0;

	/* Create Window Class */
	wc.lpfnWndProc = DefWindowProc;
	wc.lpszClassName = "NB_WINDOW";
	wc.lpfnWndProc = perform_poll;
	wc.hIcon = NULL;
	wc.lpszMenuName = NULL;
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	RegisterClass(&wc);

	/* Open Window */
	platform.window = CreateWindow("NB_WINDOW", name, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, width * 4, height * 4, NULL, NULL, NULL, NULL);
	ShowWindow(platform.window, SW_SHOW);

	/* create screen buffer */
	dc = GetDC(platform.window);
	platform.buffer_memory = CreateCompatibleDC(dc);
    platform.buffer_bitmap = CreateCompatibleBitmap(dc, platform.width, platform.height);
    SelectObject(platform.buffer_memory, platform.buffer_bitmap);
	ReleaseDC(platform.window, dc);
}

NB_BOOL platform_poll()
{
	MSG msg;

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (platform.closing)
		return NB_FALSE;

	return NB_TRUE;
}

void platform_present()
{
	HDC dc;
	FLOAT w, h, s;
	RECT size;
	HBRUSH bg;
    BITMAPINFO bminfo;
	
	dc = GetDC(platform.window);

	/* Get Screen Size */
	GetClientRect(platform.window, &size);
	s = min((size.right - size.left) / (FLOAT)platform.width, (size.bottom - size.top) / (FLOAT)platform.height);
	w = platform.width * s;
	h = platform.height * s;

	/* Draw Viewport Edges */
	bg = CreateSolidBrush(RGB(0, 0, 0));
	FillRect(dc, &size, bg);
	DeleteObject(bg);

	/* Print Screen */
	bminfo.bmiHeader.biSize = sizeof(bminfo.bmiHeader);
    bminfo.bmiHeader.biWidth = platform.width;
    bminfo.bmiHeader.biHeight = -(NB_INT)platform.height;
    bminfo.bmiHeader.biPlanes = 1;
    bminfo.bmiHeader.biBitCount = 32;
    bminfo.bmiHeader.biCompression = BI_RGB;
    bminfo.bmiHeader.biSizeImage = 0;
    bminfo.bmiHeader.biXPelsPerMeter = 1;
    bminfo.bmiHeader.biYPelsPerMeter = 1;
    bminfo.bmiHeader.biClrUsed = 0;
    bminfo.bmiHeader.biClrImportant = 0;
    SetDIBits(platform.buffer_memory, platform.buffer_bitmap, 0, platform.height, platform.screen, &bminfo, 0);
	StretchBlt(dc, 
		((size.right - size.left) - w) / 2, ((size.bottom - size.top) - h) / 2, w, h, 
		platform.buffer_memory, 0, 0, platform.width, platform.height, SRCCOPY);

	/* Finish */
	ReleaseDC(platform.window, dc);

	/* Faking V-Sync */
	platform.framecounter += (1000.0 / platform.fps);
	Sleep((INT)platform.framecounter);
	platform.framecounter -= (INT)platform.framecounter;
}
