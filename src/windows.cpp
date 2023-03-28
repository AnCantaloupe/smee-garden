#include <windows.h>
#pragma comment (lib, "Shlwapi.lib")
#include <shlwapi.h>

#include "platform.h"

internal void
DEBUGPlatformFreeFileMemory(void *Memory) {
	if (Memory) {
		VirtualFree(Memory, 0, MEM_RELEASE);
	}
}

internal debug_read_file_result
DEBUGPlatformReadEntireFile(char *Filename) {
	debug_read_file_result Result = {};
	
	HANDLE FileHandle = CreateFileA(Filename, GENERIC_READ,
									FILE_SHARE_READ, 0,
									OPEN_EXISTING, 0, 0);
	if (FileHandle != INVALID_HANDLE_VALUE) {
		LARGE_INTEGER FileSize;
		if (GetFileSizeEx(FileHandle, &FileSize)) {
			u32 FileSize32 = SafeTruncateU64(FileSize.QuadPart);
			Result.Contents = VirtualAlloc(0, (size_t)FileSize.QuadPart,
										   MEM_RESERVE | MEM_COMMIT,
										   PAGE_READWRITE);
			if (Result.Contents) {
				DWORD BytesRead;
				if (ReadFile(FileHandle,
							 Result.Contents, FileSize32,
							 &BytesRead, 0)
					&& FileSize32 == BytesRead) {
					// File read successfully
					Result.ContentSize = FileSize32;
				}
				else {
					DEBUGPlatformFreeFileMemory(Result.Contents);
					Result.Contents = 0;
				}
			}
		}
        
		CloseHandle(FileHandle);
	}
    
	return Result;
}

// @Cleanup
inline FILETIME
Win32GetLastWriteTime(char *Filename) {
	
    FILETIME Result = {};
    
	WIN32_FILE_ATTRIBUTE_DATA Data;
	if (GetFileAttributesEx(Filename, GetFileExInfoStandard, &Data)) {
		Result = Data.ftLastWriteTime;
	}
    
	return Result;
}

internal file_time
DEBUGPlatformGetFileWriteTime(char *Filename) {
    
    file_time Result = {};
    
    FILETIME FileLastWrite = Win32GetLastWriteTime(Filename);
    Result.LowDateTime  = FileLastWrite.dwLowDateTime;
    Result.HighDateTime = FileLastWrite.dwHighDateTime;
    
    return Result;
}

////////////////////////////////////////////////////////////////////////////////////////

#define NUM_WINDOWS 1
typedef struct {
    HWND  Window;
    HDC   DeviceContext;
    HGLRC GLContext;
} win32_window_context;

#include <gl/gl.h>

#include "libraries/seeds_maths.h"
#include "libraries/seeds_maths.cpp"
#include "libraries/seeds_strings.h"
#include "libraries/seeds_strings.cpp"
#include "libraries/seeds_colour.h"
#include "libraries/seeds_memory.h"
#include "libraries/seeds_memory.cpp"

global s64    GlobalPerfCountFrequency;
global bool   GlobalRunning;
global win32_window_context GlobalWindow;

global string CONFIG_ComPort;
global string CONFIG_Scene;
global f32 CONFIG_Width;
global f32 CONFIG_Height;

#define SEEDS_OPENGL 1
#include "main.cpp"

internal void
Win32InitOpenGL() {
    
    PIXELFORMATDESCRIPTOR DesiredPixelFormat = {};
    DesiredPixelFormat.nSize      = sizeof(DesiredPixelFormat);
    DesiredPixelFormat.nVersion   = 1;
    DesiredPixelFormat.dwFlags    = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
    DesiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
    DesiredPixelFormat.cColorBits = 32;
    DesiredPixelFormat.cAlphaBits = 8;
    DesiredPixelFormat.iLayerType = PFD_MAIN_PLANE;
    
    int SuggestedPixelFormatIndex = ChoosePixelFormat(GlobalWindow.DeviceContext, &DesiredPixelFormat);
    PIXELFORMATDESCRIPTOR SuggestedPixelFormat;
    DescribePixelFormat(GlobalWindow.DeviceContext, SuggestedPixelFormatIndex,
                        sizeof(SuggestedPixelFormat), &SuggestedPixelFormat);
    SetPixelFormat(GlobalWindow.DeviceContext, SuggestedPixelFormatIndex, &SuggestedPixelFormat);
    
    GlobalWindow.GLContext = wglCreateContext(GlobalWindow.DeviceContext);
    wglMakeCurrent(GlobalWindow.DeviceContext, GlobalWindow.GLContext);
}

internal window_information
Win32GetWindowInformation(HWND Window) {
    window_information Result;
    
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Active = Window == GetActiveWindow();
    Result.X = ClientRect.left;
    Result.Y = ClientRect.top;
    Result.W = ClientRect.right - ClientRect.left;
    Result.H = ClientRect.bottom - ClientRect.top;
    
    return Result;
}

global WINDOWPLACEMENT GlobalWindowPosition = { sizeof(GlobalWindowPosition) };
internal void
ToggleFullscreen(HWND Window) {
    
    // This follows Raymond Chen's implementation of fullscreen toggling
    // https://devblogs.microsoft.com/oldnewthing/20100412-00/?p=14353
    DWORD Style = GetWindowLong(Window, GWL_STYLE);
    if (Style & WS_OVERLAPPEDWINDOW) {
        MONITORINFO MonitorInfo = { sizeof(MonitorInfo) };
        if (GetWindowPlacement(Window, &GlobalWindowPosition) &&
            GetMonitorInfo(MonitorFromWindow(Window,
                                             MONITOR_DEFAULTTOPRIMARY), &MonitorInfo)) {
            SetWindowLong(Window, GWL_STYLE,
                          Style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(Window, HWND_TOP,
                         MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
                         MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
                         MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else {
        SetWindowLong(Window, GWL_STYLE, Style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(Window, &GlobalWindowPosition);
        SetWindowPos(Window, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

LRESULT CALLBACK
Win32MainWindowCallback1(HWND   Window,
                         UINT   Message,
                         WPARAM WParam,
                         LPARAM LParam) {
    LRESULT Result = 0;
    
    switch (Message) {
        case WM_SIZE: break;
        
        case WM_CLOSE:
        {
            GlobalRunning = false;
        } break;
        
        case WM_SETCURSOR:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
        
        case WM_DESTROY:
        {
            GlobalRunning = false;
        } break;
        
        case WM_ACTIVATEAPP: break;
        
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            Assert(!"Keyboard input came in through a non-dispatch message");		
        } break;
        
        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
            
            GlobalWindowInformation = Win32GetWindowInformation(Window);
            SwapBuffers(DeviceContext);
            
            EndPaint(Window, &Paint);
        } break;
        
        default:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam);
        }break;
    }
    
    return Result;
};

internal void
Win32ProcessKeyboardMessage(button_state *NewState, b32 IsDown) {
    
    if (NewState->EndedDown != IsDown) {
        NewState->EndedDown = IsDown;
        NewState->HalfTransitionCount++;
    }
}

internal void
Win32ProcessPendingMessages(input *Input) {
    
    MSG Message;
    while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {
        switch (Message.message) {
            case WM_QUIT: {
                GlobalRunning = false;
            } break;
            
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP: {
                u32 VKCode = (u32)Message.wParam;
                bool WasDown = ((Message.lParam & (1 << 30)) != 0);
                bool IsDown = ((Message.lParam & (1 << 31)) == 0);
                
                if (IsDown != WasDown) {
                    if      (VKCode == 'A') {
                        Win32ProcessKeyboardMessage(&Input->Buttons[BUTTON_A], IsDown);
                    }
                    else if (VKCode == 'D') {
                        Win32ProcessKeyboardMessage(&Input->Buttons[BUTTON_D], IsDown);
                    }
                    else if (VKCode == 'S') {
                        Win32ProcessKeyboardMessage(&Input->Buttons[BUTTON_S], IsDown);
                    }
                    else if (VKCode == 'W') {
                        Win32ProcessKeyboardMessage(&Input->Buttons[BUTTON_W], IsDown);
                    }
                    else if (VKCode == 'R') {
                        Win32ProcessKeyboardMessage(&Input->Buttons[BUTTON_R], IsDown);
                    }
                    
                    else if (VKCode == VK_LEFT) {
                        Win32ProcessKeyboardMessage(&Input->Buttons[BUTTON_Left], IsDown);
                    }
                    else if (VKCode == VK_RIGHT) {
                        Win32ProcessKeyboardMessage(&Input->Buttons[BUTTON_Right], IsDown);
                    }
                    else if (VKCode == VK_UP) {
                        Win32ProcessKeyboardMessage(&Input->Buttons[BUTTON_Up], IsDown);
                    }
                    else if (VKCode == VK_DOWN) {
                        Win32ProcessKeyboardMessage(&Input->Buttons[BUTTON_Down], IsDown);
                    }
                    
                    if (IsDown) {
                        bool AltKeyWasDown = ((Message.lParam & (1 << 29)) != 0);
                        if ((VKCode == VK_ESCAPE) || ((VKCode == VK_F4) && AltKeyWasDown)) {
                            GlobalRunning = false;
                        }
                        if ((VKCode == VK_RETURN) && AltKeyWasDown) {
                            if (Message.hwnd) {
                                ToggleFullscreen(Message.hwnd);
                            }
                        }
                    }
                }
            } break;
            
            default: {
                TranslateMessage(&Message);
                DispatchMessage(&Message);
            } break;
        }
    }
}

inline LARGE_INTEGER
Win32GetWallClock(void) {
    
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return Result;
}

inline f32
Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End) {
    
    return(f32)(End.QuadPart - Start.QuadPart) / (f32)GlobalPerfCountFrequency;
}

int CALLBACK
WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode) {
    
    LARGE_INTEGER PerfCounterFrequencyResult;
    QueryPerformanceFrequency(&PerfCounterFrequencyResult);
    GlobalPerfCountFrequency = PerfCounterFrequencyResult.QuadPart;
    
    UINT DesiredSchedulerMS = 1;
    b32 SleepIsGranular = (timeBeginPeriod(DesiredSchedulerMS) != TIMERR_NOCANDO);
    
    WNDCLASSA WindowClass = {};
    WindowClass.style         = (CS_OWNDC | CS_HREDRAW | CS_VREDRAW);
    WindowClass.lpfnWndProc   = Win32MainWindowCallback1;
    WindowClass.hInstance     = Instance;
    WindowClass.hCursor       = LoadCursor(0, IDC_ARROW);
    WindowClass.lpszClassName = "SMEE_Garden_Main";
    
    if (RegisterClass(&WindowClass)) {
        // Process config file
        char PortBuffer[11]; // Needs to accomodate "\\.\COMXXX"
        char SceneBuffer[MAX_PATH];
        {
            debug_read_file_result ConfigFile = DEBUGPlatformReadEntireFile("../data/config");
            string Cursor = { (char *)ConfigFile.Contents, ConfigFile.ContentSize };
            while (Cursor.Length) {
                string Line = EatToNextLine(&Cursor);
                string Key = EatToNextToken(&Line);
                if (StringsIdentical(Key, STRING_FROM_LITERAL("PORT"))) {
                    CONFIG_ComPort = STRING_FROM_ARRAY(PortBuffer);
                    string Rem = CopyStringNull(CONFIG_ComPort, Line);
                    CONFIG_ComPort.Length = Rem.Data - CONFIG_ComPort.Data;
                }
                else if (StringsIdentical(Key, STRING_FROM_LITERAL("SCENE"))) {
                    CONFIG_Scene = STRING_FROM_ARRAY(SceneBuffer);
                    string Rem = CatStringsNull(2, CONFIG_Scene, STRING_FROM_LITERAL("../data/"), Line);
                    CONFIG_Scene.Length = Rem.Data - CONFIG_Scene.Data;
                }
                else if (StringsIdentical(Key, STRING_FROM_LITERAL("WIDTH"))) {
                    CONFIG_Width = (f32)S32FromDecString(Line);
                }
                else if (StringsIdentical(Key, STRING_FROM_LITERAL("HEIGHT"))) {
                    CONFIG_Height = (f32)S32FromDecString(Line);
                }
            }
            
            DEBUGPlatformFreeFileMemory(ConfigFile.Contents);
        }
        
        DWORD WindowStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
        RECT DesiredWindowDimensions = { 0, 0, (int)CONFIG_Width, (int)CONFIG_Height };
        b32 AdjustWindowRectSuccess = AdjustWindowRect(&DesiredWindowDimensions, WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0);
        GlobalWindow.Window = CreateWindowEx(0, WindowClass.lpszClassName,
                                             "",
                                             WindowStyle,
                                             CW_USEDEFAULT, CW_USEDEFAULT,
                                             DesiredWindowDimensions.right  - DesiredWindowDimensions.left,
                                             DesiredWindowDimensions.bottom - DesiredWindowDimensions.top,
                                             0, 0, Instance, 0);
        
        if (GlobalWindow.Window) {
            GlobalWindow.DeviceContext = GetDC(GlobalWindow.Window);
            Win32InitOpenGL();
            
            int MonitorRefreshHz = 1;
            int Win32RefreshRate = GetDeviceCaps(GlobalWindow.DeviceContext, VREFRESH);
            if (Win32RefreshRate > 1) {
                MonitorRefreshHz = Win32RefreshRate;
            }
            
            f32 TargetSecondsPerFrame = (1.0f / (f32)(MonitorRefreshHz));
            
            GlobalRunning = true;
            
            LPVOID BaseAddress = 0;
            memory Memory = {};
            DEBUGGlobalMemory = &Memory;
            Memory.PermanentStorageSize = Megabytes(64);
            Memory.TransientStorageSize = Gigabytes((u64)1);
            
            size_t TotalMemorySize = (size_t)(Memory.PermanentStorageSize + Memory.TransientStorageSize);
            Memory.PermanentStorage =  VirtualAlloc(BaseAddress, TotalMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            Memory.TransientStorage = ((u8 *)Memory.PermanentStorage + Memory.PermanentStorageSize);
            
            if (   Memory.PermanentStorage
                && Memory.TransientStorage) {
                
                input Input[2] = {};
                input *NewInput = &Input[0];
                input *OldInput = &Input[1];
                
                LARGE_INTEGER LastCounter = Win32GetWallClock();
                LARGE_INTEGER FlipWallClock = Win32GetWallClock();
                s64 LastCycleCount = __rdtsc();
                
                // Setting up Arduino serial comm
                // https://learn.microsoft.com/en-us/previous-versions/ff802693(v=msdn.10)
                HANDLE ArduinoPort = CreateFile(CONFIG_ComPort.Data, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
                COMMTIMEOUTS CommTimeouts;
                GetCommTimeouts(ArduinoPort, &CommTimeouts);
                CommTimeouts.ReadIntervalTimeout = MAXDWORD;
                CommTimeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
                CommTimeouts.ReadTotalTimeoutConstant = 0;
                SetCommTimeouts(ArduinoPort, &CommTimeouts);
                OVERLAPPED Reader = {};
                Reader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
                
                while (GlobalRunning) {
                    NewInput->TimeStepFrame = TargetSecondsPerFrame;
                    
                    {   // Arduino input
                        char Buffer[256];
                        DWORD EventMask = 0;
                        DWORD NoBytesRead;
                        ReadFile(ArduinoPort, Buffer, 255, &NoBytesRead, &Reader);
                        Buffer[NoBytesRead] = '\0';
                        if (NoBytesRead > 0) {
                            arduino *Ard = &Input->Arduino;
                            *Ard = {};
                            Ard->Steps = NoBytesRead;
                            for (DWORD I = 0; I < NoBytesRead; I++) {
                                char Value = Buffer[I] - '0';
                                Ard->Left    += (bool)(Value & 8);
                                Ard->Right   += (bool)(Value & 4);
                                Ard->Forward += (bool)(Value & 2);
                                Ard->Back    += (bool)(Value & 1);
                            }
                            OutputDebugStringA(Buffer);
                        }
                    }
                    
                    {   // Keyboard input
                        for (int ButtonIndex = 0; ButtonIndex < ArrayCount(NewInput->Buttons); ButtonIndex++) {
                            NewInput->Buttons[ButtonIndex].HalfTransitionCount = 0;
                            NewInput->Buttons[ButtonIndex].EndedDown =
                                OldInput->Buttons[ButtonIndex].EndedDown;
                        }
                        Win32ProcessPendingMessages(NewInput);
                    }
                    
                    {   // Mouse input
                        GlobalWindowInformation = Win32GetWindowInformation(GlobalWindow.Window);
                        
                        POINT MouseP;
                        GetCursorPos(&MouseP);
                        // TODO(canta): Keep this as a float? Normalise it?
                        NewInput->MouseX = MouseP.x;
                        NewInput->MouseY = MouseP.y;
                        
                        NewInput->MouseButtons[0].HalfTransitionCount = 0;
                        Win32ProcessKeyboardMessage(&NewInput->MouseButtons[0],
                                                    GetKeyState(VK_LBUTTON) & (1 << 15));
                        OldInput->MouseButtons[0].EndedDown = NewInput->MouseButtons[0].EndedDown;
                        
                        NewInput->MouseButtons[1].HalfTransitionCount = 0;
                        Win32ProcessKeyboardMessage(&NewInput->MouseButtons[1],
                                                    GetKeyState(VK_RBUTTON) & (1 << 15));
                        OldInput->MouseButtons[1].EndedDown = NewInput->MouseButtons[1].EndedDown;
                        
                        NewInput->MouseButtons[2].HalfTransitionCount = 0;
                        Win32ProcessKeyboardMessage(&NewInput->MouseButtons[2],
                                                    GetKeyState(VK_MBUTTON) & (1 << 15));
                        OldInput->MouseButtons[2].EndedDown = NewInput->MouseButtons[2].EndedDown;
                    }
                    
                    Main(&Memory, NewInput, GlobalWindowInformation);
                    
                    input *Temp = NewInput;
                    NewInput = OldInput;
                    OldInput = Temp;
                    
                    LARGE_INTEGER WorkCounter = Win32GetWallClock();
                    f32 WorkSecondsElapsed = Win32GetSecondsElapsed(LastCounter, WorkCounter);
                    
                    f32 TargetSecondsPerFrameSleep = TargetSecondsPerFrame - 0.001f;
                    f32 FrameSecondsElapsed = WorkSecondsElapsed;
                    if (FrameSecondsElapsed < TargetSecondsPerFrameSleep) {
                        if (SleepIsGranular) {
                            DWORD SleepMS = (DWORD)(1000.0f * (TargetSecondsPerFrameSleep
                                                               - FrameSecondsElapsed));
                            if (SleepMS > 0) Sleep(SleepMS);
                        }
                        
                        while (FrameSecondsElapsed < TargetSecondsPerFrame) {
                            FrameSecondsElapsed = Win32GetSecondsElapsed(LastCounter, Win32GetWallClock());
                        }
                        
                    }
                    
                    LARGE_INTEGER EndCounter = Win32GetWallClock();
                    f32 FrameTimeMS = 1000.0f * Win32GetSecondsElapsed(LastCounter, EndCounter);
                    f32 FramesPerSecond = 1000.0f / FrameTimeMS;
                    LastCounter = EndCounter;
                    
                    SwapBuffers(GlobalWindow.DeviceContext);
                    FlipWallClock = Win32GetWallClock();
                    
                    u64 EndCycleCount = __rdtsc();
                    f32 CyclesElapsed = (f32)(EndCycleCount - LastCycleCount);
                    LastCycleCount = EndCycleCount;
                    f32 MegaCyclesPerFrame = (f32)CyclesElapsed / (1000.0f * 1000.0f);
                    {
                        char PMemory[256];
                        string PrintBuffer = STRING_FROM_ARRAY(PMemory);
                        string Cursor = Print(PrintBuffer, STRING_FROM_LITERAL("%fms, %fFPS, %fMc\n\n"),
                                              FrameTimeMS, FramesPerSecond, MegaCyclesPerFrame);
                        Cursor.Data[0] = '\0';
                        PrintBuffer.Length = Cursor.Data - PrintBuffer.Data;
                        OutputDebugStringA(PrintBuffer.Data);
                    }
                    
                }
            }
        }
    }
    
    return 0;
}
