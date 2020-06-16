#include <windows.h>
#include <stdint.h>
#include <xinput.h>
#include <dsound.h>
#include <math.h>

#define PI32 3.1459265359f

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef float real32;
typedef double real64;

static bool running;

typedef struct Win32OffScreenBuffer{
	BITMAPINFO info;
	void *memory;
	int width;
	int height;
	int pitch;
	int bytes_per_pixel;
} Win32OffScreenBuffer;

typedef struct Win32WindowDimension {
	int width;
	int height;
} Win32WindowDimension;

typedef struct Win32SoundOutput {
	int samples_per_second;
	int tone_hz;
	uint32 running_sample_index;
	int wave_period;
	int bytes_per_sample;
	int secondary_buffer_size;
	int tone_volume;
} Win32SoundOutput;


/* XInput GetState*/
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub) {
	return ERROR_DEVICE_NOT_CONNECTED;
}
static x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

/* XInput SetState*/
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub) {
	return ERROR_DEVICE_NOT_CONNECTED;
}
static x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

/* DirectSound */
#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

/* Functions */
static LRESULT CALLBACK win32_main_window_callback(HWND window, UINT message, WPARAM w_param, LPARAM l_param);
static void resize_dib_section(Win32OffScreenBuffer *buffer, int width, int height);
static void win32_update_window(HDC device_context, int window_width, int window_height, Win32OffScreenBuffer *buffer, int x, int y, int width, int height);
static void render_something(Win32OffScreenBuffer *buffer, int x, int y);
static Win32WindowDimension get_window_dimension(HWND window);
static void load_x_input();
static void Win32InitSound(HWND window, int32 samples_per_sec, int32 buffer_size);
static void win32_fill_sound_buffer(Win32SoundOutput *win32_sound_output, DWORD byte_to_lock, DWORD bytes_to_write);
/* Functions */

/* Global variables */
static Win32OffScreenBuffer buffer;
static LPDIRECTSOUNDBUFFER secondary_buffer;
/* Global variables */

static int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR command_line, int show_code) {
	load_x_input();

	WNDCLASSA window_class = {};
	window_class.style = CS_HREDRAW|CS_VREDRAW;
	window_class.lpfnWndProc = win32_main_window_callback;
	window_class.hInstance = instance;
	window_class.lpszClassName = "AkashWindowClass";

	if(RegisterClass(&window_class)) {
		HWND window_handle = CreateWindowEx(0, window_class.lpszClassName, "AkashWindowClass", WS_OVERLAPPEDWINDOW|WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, instance, 0);
		resize_dib_section(&buffer, 800, 600);

		if(window_handle) {
			MSG msg;
			running = true;

			/* Graphics test */
			int x_offset = 0;
			/* Graphics test */

			/* Sound test */
			Win32SoundOutput win32_sound_output = {};
			{
				win32_sound_output.samples_per_second = 48000;
				win32_sound_output.tone_hz = 256;
				win32_sound_output.running_sample_index = 0;
				win32_sound_output.wave_period = win32_sound_output.samples_per_second / win32_sound_output.tone_hz;
				win32_sound_output.bytes_per_sample = sizeof(int16) * 2;
				win32_sound_output.secondary_buffer_size = win32_sound_output.samples_per_second * win32_sound_output.bytes_per_sample;
				win32_sound_output.tone_volume = 16000;
				Win32InitSound(window_handle, win32_sound_output.samples_per_second, win32_sound_output.secondary_buffer_size);
				win32_fill_sound_buffer(&win32_sound_output, 0, win32_sound_output.secondary_buffer_size);
				secondary_buffer->Play(0, 0, DSBPLAY_LOOPING);
			}
			/* Sound test */

			while(running) {
				while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
					if(msg.message == WM_QUIT)
						running = false;

					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}

				for(DWORD controller_index = 0; controller_index < XUSER_MAX_COUNT; ++controller_index) {
					XINPUT_STATE controller_state;
					if(XInputGetState(controller_index, &controller_state) == ERROR_SUCCESS) {
						XINPUT_GAMEPAD *pad = &controller_state.Gamepad;
						bool d_pad_up = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
						bool d_pad_down = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
						bool d_pad_left = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
						bool d_pad_right = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
						bool start = (pad->wButtons & XINPUT_GAMEPAD_START);
						bool back = (pad->wButtons & XINPUT_GAMEPAD_BACK);
						bool left_shoulder = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
						bool right_shoulder = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
						bool a_button = (pad->wButtons & XINPUT_GAMEPAD_A);
						bool b_button = (pad->wButtons & XINPUT_GAMEPAD_B);
						bool x_button = (pad->wButtons & XINPUT_GAMEPAD_X);
						bool y_button = (pad->wButtons & XINPUT_GAMEPAD_Y);

						if(x_button) {
							x_offset += 1;
						}
						else if(a_button) {
							running = false;
						}

						int16 stick_x = pad->sThumbLX;
						int16 stick_y = pad->sThumbLY;
					}
					else {
						// controller not plugged in.
					}
				}

				render_something(&buffer, x_offset, 0);

				/* DirectSound test */
				{
					DWORD play_cursor;
					DWORD write_cursor;
					if(SUCCEEDED(secondary_buffer->GetCurrentPosition(&play_cursor, &write_cursor))) {
						DWORD byte_to_lock = (win32_sound_output.running_sample_index * win32_sound_output.bytes_per_sample) % win32_sound_output.secondary_buffer_size;
						DWORD bytes_to_write;
						if(byte_to_lock == play_cursor) {
							bytes_to_write = 0;
						}
						else if(byte_to_lock > play_cursor) {
							bytes_to_write = win32_sound_output.secondary_buffer_size - byte_to_lock;
							bytes_to_write += play_cursor;
						}
						else {
							bytes_to_write = play_cursor - byte_to_lock;
						}
					
						win32_fill_sound_buffer(&win32_sound_output, byte_to_lock, bytes_to_write);
					}
				}
				/* DirectSound test */

				Win32WindowDimension win32_window_dimension = get_window_dimension(window_handle);
				HDC device_context = GetDC(window_handle);
				win32_update_window(device_context, win32_window_dimension.width, win32_window_dimension.height, &buffer, 0, 0, win32_window_dimension.width, win32_window_dimension.height);
				ReleaseDC(window_handle, device_context);
			}
		}
	}
	
	return 0;
}

static LRESULT CALLBACK win32_main_window_callback(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
	LRESULT l_result = 0;

	switch(message) {
		case WM_SIZE:
			break;
		case WM_DESTROY:
			running = false;
			break;
		case WM_CLOSE:
			running = false;
			break;
		case WM_ACTIVATEAPP:
			OutputDebugStringA("WM_ACTIVATEAPP");
			break;
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
			{
				uint32 v_key_code = w_param;
				bool was_down = ((l_param & (1 << 30)) != 0);
				bool is_down = ((l_param & (1 << 31)) == 0);

				if(is_down != was_down) {
					if(v_key_code == 27) {
						if(is_down) {
							running = false;
						}
					}
				}
			}
			break;
		case WM_PAINT: 
			{
				PAINTSTRUCT paint;
				HDC device_context = BeginPaint(window, &paint);
				EndPaint(window, &paint);
				break;
			}
		default: 
			l_result = DefWindowProc(window, message, w_param, l_param);
	}

	return l_result;
}

static void resize_dib_section(Win32OffScreenBuffer *buffer, int width, int height) {
	if(buffer->memory) {
		VirtualFree(buffer->memory, 0, MEM_RELEASE);
	}

	buffer->width = width;
	buffer->height = height;

	buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
	buffer->info.bmiHeader.biWidth = buffer->width;
	buffer->info.bmiHeader.biHeight = -buffer->height;
	buffer->info.bmiHeader.biPlanes = 1;
	buffer->info.bmiHeader.biBitCount = 32;
	buffer->info.bmiHeader.biCompression = BI_RGB;
	buffer->info.bmiHeader.biSizeImage = 0;
	buffer->info.bmiHeader.biXPelsPerMeter = 0;
	buffer->info.bmiHeader.biYPelsPerMeter = 0;
	buffer->info.bmiHeader.biClrUsed = 0;
	buffer->info.bmiHeader.biClrImportant = 0;

	buffer->bytes_per_pixel = 4;
	int bitmap_memory_size = buffer->width * buffer->height * buffer->bytes_per_pixel;
	buffer->memory = VirtualAlloc(0, bitmap_memory_size, MEM_COMMIT, PAGE_READWRITE);
	buffer->pitch = width * buffer->bytes_per_pixel;
}

static void win32_update_window(HDC device_context, int window_width, int window_height, Win32OffScreenBuffer *buffer, int x, int y, int width, int height) {
	StretchDIBits(device_context, 0, 0, window_width, window_height, 0, 0, buffer->width, buffer->height, buffer->memory, &buffer->info, DIB_RGB_COLORS, SRCCOPY);
}

static void render_something(Win32OffScreenBuffer *buffer, int x, int y) {
	int width = buffer->width;
	int height = buffer->height;

	// Register: xx RR GG BB 
	// Memory  : BB GG RR xx
	uint8* row = (uint8*) buffer->memory;
	for(int i = 0; i < height; ++i) {
		uint32* pixel = (uint32*) row;
		for(int j = 0; j < width; ++j) {
			uint8 green = j + x;
			uint8 blue = i + y;
			*pixel = ((green << 16) | (blue << 8)) | blue;
			pixel += 1;
		}

		row += buffer->pitch;
	}
}

static Win32WindowDimension get_window_dimension(HWND window) {
	RECT client_rect;
	GetClientRect(window, &client_rect);
	Win32WindowDimension win32_window_dimension;
	win32_window_dimension.width = client_rect.right - client_rect.left;
	win32_window_dimension.height = client_rect.bottom - client_rect.top;

	return win32_window_dimension;
}

static void load_x_input() {
	HMODULE x_input_library = LoadLibraryA("xinput1_4.dll");
	if(!x_input_library) {
		x_input_library = LoadLibraryA("xinput1_3.dll");
	}

	if(x_input_library) {
		XInputGetState = (x_input_get_state*) GetProcAddress(x_input_library, "XInputGetState");
		XInputSetState = (x_input_set_state*) GetProcAddress(x_input_library, "XInputSetState");
	}
}

static void Win32InitSound(HWND window, int32 samples_per_sec, int32 buffer_size) {
	/* Load the library. */
	HMODULE d_sound_library = LoadLibraryA("dsound.dll");

	if(d_sound_library) {
		/* Get the direct sound object. */
		direct_sound_create *DirectSoundCreate = (direct_sound_create*) GetProcAddress(d_sound_library, "DirectSoundCreate");

		LPDIRECTSOUND direct_sound;
		if(DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &direct_sound, 0))) {
			WAVEFORMATEX wave_format = {};
			wave_format.wFormatTag = WAVE_FORMAT_PCM;
			wave_format.nChannels = 2;
			wave_format.nSamplesPerSec = samples_per_sec;
			wave_format.wBitsPerSample = 16;
			wave_format.nBlockAlign = (wave_format.nChannels * wave_format.wBitsPerSample) / 8;
			wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;
			wave_format.cbSize = 0;

			if(SUCCEEDED(direct_sound->SetCooperativeLevel(window, DSSCL_PRIORITY))) {
				DSBUFFERDESC buffer_description = {};
				buffer_description.dwSize = sizeof(buffer_description);
				buffer_description.dwFlags = DSBCAPS_PRIMARYBUFFER;

				/* Create a primary buffer. */
				LPDIRECTSOUNDBUFFER primary_buffer;
				if(SUCCEEDED(direct_sound->CreateSoundBuffer(&buffer_description, &primary_buffer, 0))) {
					if(SUCCEEDED(primary_buffer->SetFormat(&wave_format))) {
					}
					else {
						/* Could not set format. */
					}
				}
			}
			else {
				/* Could not set cooperative_level. */
			}

			/* Create a secondary primary buffer. */
			DSBUFFERDESC buffer_description = {};
			buffer_description.dwSize = sizeof(buffer_description);
			buffer_description.dwFlags = 0;
			buffer_description.dwBufferBytes = buffer_size;
			buffer_description.lpwfxFormat = &wave_format;
			if(SUCCEEDED(direct_sound->CreateSoundBuffer(&buffer_description, &secondary_buffer, 0))) {
				/* Start playing it! */
			}
		}
		else {
			/* Could not create DirectSound object! */
		}
	}
	else {
		/* Could not load the d_sound_library. */
	}
}

static void win32_fill_sound_buffer(Win32SoundOutput *win32_sound_output, DWORD byte_to_lock, DWORD bytes_to_write) {
	void *region_1, *region_2;
	DWORD region_1_size, region_2_size;

	if(SUCCEEDED(secondary_buffer->Lock(byte_to_lock, bytes_to_write, &region_1, &region_1_size, &region_2, &region_2_size, 0)))
	{
		int16* sample_out;

		sample_out = (int16*) region_1;
		DWORD region_1_sample_count = region_1_size / win32_sound_output->bytes_per_sample;
		for(DWORD sample_index = 0; sample_index < region_1_sample_count; ++sample_index) {
			real32 t = 2.0f * PI32 * (real32)win32_sound_output->running_sample_index / (real32)win32_sound_output->wave_period;
			real32 sine_value = sinf(t);
			int16 sample_value = (int16)(sine_value * win32_sound_output->tone_volume);
			*sample_out++ = sample_value; 
			*sample_out++ = sample_value; 
			win32_sound_output->running_sample_index += 1;
		}

		DWORD region_2_sample_count = region_2_size / win32_sound_output->bytes_per_sample;
		sample_out = (int16*) region_2;
		for(DWORD sample_index = 0; sample_index < region_2_sample_count; ++sample_index) {
			real32 t = 2.0f * PI32 * (real32)win32_sound_output->running_sample_index / (real32)win32_sound_output->wave_period;
			real32 sine_value = sinf(t);
			int16 sample_value = (int16)(sine_value * win32_sound_output->tone_volume);
			*sample_out++ = sample_value; 
			*sample_out++ = sample_value;
			win32_sound_output->running_sample_index += 1;
		}

		secondary_buffer->Unlock(region_1, region_1_size, region_2, region_2_size);
	}
}
