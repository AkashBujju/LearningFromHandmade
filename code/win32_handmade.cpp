#include <windows.h>
#include <stdint.h>

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

static bool running;
static BITMAPINFO bitmap_info;
static void *bitmap_memory;
static int bitmap_width;
static int bitmap_height;
static int bytes_per_pixel = 4;

LRESULT CALLBACK win32_main_window_callback(HWND window, UINT message, WPARAM w_param, LPARAM l_param);
void resize_dib_section(int width, int height);
void win32_update_window(HDC device_context, RECT *window_rect, int x, int y, int width, int height);
void render_something(int x, int y);

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR command_line, int show_code) {
	WNDCLASS window_class = {};
	window_class.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
	window_class.lpfnWndProc = win32_main_window_callback;
	window_class.hInstance = instance;
	window_class.lpszClassName = "AkashWindowClass";

	if(RegisterClass(&window_class)) {
		HWND window_handle = CreateWindowEx(0, window_class.lpszClassName, "AkashWindowClass",
																				WS_OVERLAPPEDWINDOW|WS_VISIBLE, CW_USEDEFAULT,
																				CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0,
																				0, instance, 0);

		if(window_handle) {
			int x_offset = 0;
			MSG msg;
			running = true;
			while(running) {
				while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
					if(msg.message == WM_QUIT)
						running = false;

					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}

				render_something(x_offset, 0);

				RECT client_rect;
				GetClientRect(window_handle, &client_rect);
				LONG window_width = client_rect.bottom - client_rect.top;
				LONG window_height = client_rect.right - client_rect.left;
				HDC device_context = GetDC(window_handle);
				win32_update_window(device_context, &client_rect, 0, 0, window_width, window_height);
				ReleaseDC(window_handle, device_context);

				x_offset += 1;
			}
		}
	}
	
	return 0;
}

LRESULT CALLBACK win32_main_window_callback(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
	LRESULT l_result = 0;

	switch(message) 
	{
		case WM_SIZE:
			{
				RECT client_rect;
				GetClientRect(window, &client_rect);
				int width = client_rect.right - client_rect.left;
				int height = client_rect.bottom - client_rect.top;
				resize_dib_section(width, height);
			} 
			break;
		case WM_DESTROY:
			{
				running = false;
			} 
			break;
		case WM_CLOSE:
			{
				running = false;
			}
			break;
		case WM_ACTIVATEAPP:
			{
				OutputDebugStringA("WM_ACTIVATEAPP");
			}
			break;
		case WM_PAINT: 
			{
				PAINTSTRUCT paint;
				HDC device_context = BeginPaint(window, &paint);
				EndPaint(window, &paint);
			}
			break;
		default: 
			{
				OutputDebugStringA("default");
				l_result = DefWindowProc(window, message, w_param, l_param);
			}
	}

	return l_result;
}

void resize_dib_section(int width, int height) {
	if(bitmap_memory) {
		VirtualFree(bitmap_memory, 0, MEM_RELEASE);
	}

	bitmap_width = width;
	bitmap_height = height;

	bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
	bitmap_info.bmiHeader.biWidth = bitmap_width;
	bitmap_info.bmiHeader.biHeight = -bitmap_height;
	bitmap_info.bmiHeader.biPlanes = 1;
	bitmap_info.bmiHeader.biBitCount = 32;
	bitmap_info.bmiHeader.biCompression = BI_RGB;
	bitmap_info.bmiHeader.biSizeImage = 0;
	bitmap_info.bmiHeader.biXPelsPerMeter = 0;
	bitmap_info.bmiHeader.biYPelsPerMeter = 0;
	bitmap_info.bmiHeader.biClrUsed = 0;
	bitmap_info.bmiHeader.biClrImportant = 0;

	int bitmap_memory_size = bitmap_width * bitmap_height * bytes_per_pixel;
	bitmap_memory = VirtualAlloc(0, bitmap_memory_size, MEM_COMMIT, PAGE_READWRITE);
	render_something(0, 0);
}

void win32_update_window(HDC device_context, RECT *window_rect, int x, int y, int width, int height) {
	int window_width = window_rect->right - window_rect->left;
	int window_height = window_rect->bottom- window_rect->top;

	StretchDIBits(device_context, 0, 0, bitmap_width, bitmap_height, 0, 0, window_width, window_height,
			bitmap_memory, &bitmap_info, DIB_RGB_COLORS, SRCCOPY);
}

void render_something(int x, int y) {
	int width = bitmap_width;
	int height = bitmap_height;
	int pitch = width * bytes_per_pixel;

	/* XXRRGGBB */
	uint8* row = (uint8*) bitmap_memory;
	for(int i = 0; i < bitmap_height; ++i) {
		uint8* pixel = (uint8*) row;
		for(int j = 0; j < bitmap_width; ++j) {
			pixel[0] = (uint8)i + y;
			pixel[1] = (uint8)j + x;
			pixel[2] = 0;
			pixel[3] = 0;

			pixel += 4;
		}

		row += pitch;
	}
}
