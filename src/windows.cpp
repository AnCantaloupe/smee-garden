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

internal bool
DEBUGPlatformWriteEntireFile(char *Filename, void *Memory, u32 MemorySize) {
	bool Result = false;
	
	HANDLE FileHandle = CreateFileA(Filename, GENERIC_WRITE, 0,
									0, CREATE_ALWAYS, 0, 0);
	if (FileHandle != INVALID_HANDLE_VALUE) {
		DWORD BytesWritten;
		if (WriteFile(FileHandle,
					  Memory, MemorySize,
					  &BytesWritten, 0)) {
			// File written successfully
			Result = (BytesWritten == MemorySize);
		}
        CloseHandle(FileHandle);
	}
    
	return Result;
}

internal b32
DEBUGPlatformDirectoryEmpty(char *Directory) {
    
    return PathIsDirectoryEmptyA(Directory);
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

#include <gl/gl.h>

// TODO(canta): Stop including this!
// We are only using sprintf() at the moment.
#include <stdio.h>

#include "libraries/seeds_maths.h"
#include "libraries/seeds_maths.cpp"
#include "libraries/seeds_strings.h"
#include "libraries/seeds_strings.cpp"
#include "libraries/seeds_string_conversions.cpp"
#include "libraries/seeds_colour.h"
#include "libraries/seeds_simd.h"
#include "libraries/seeds_memory.h"
#include "libraries/seeds_memory.cpp"

#define SEEDS_OPENGL 1
#include "main.cpp"

#define NUM_WINDOWS 1
typedef struct {
    HWND  Window;
    HDC   DeviceContext;
    HGLRC GLContext;
} win32_window_context;

global s64    GlobalPerfCountFrequency;
global bool   GlobalRunning;
global bool   GlobalPause;
global win32_window_context GlobalWindow;

internal void
Win32InitOpenGL() {
    
    PIXELFORMATDESCRIPTOR DesiredPixelFormat = {};
    DesiredPixelFormat.nSize      = sizeof(DesiredPixelFormat);
    DesiredPixelFormat.nVersion   = 1;
    DesiredPixelFormat.dwFlags    = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
    DesiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
    DesiredPixelFormat.cColorBits = 32; // TODO(canta): Not consistent with documentation.
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

global_variable WINDOWPLACEMENT GlobalWindowPosition = { sizeof(GlobalWindowPosition) };
internal void
ToggleFullscreen(HWND Window) {
    
    // NOTE(canta):
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
        //SetWindowPos(Window, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
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
            // TODO(casey): Handle this with a message to the user?
            GlobalRunning = false;
        } break;
        
        case WM_SETCURSOR:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
        
        case WM_DESTROY:
        {
            // TODO(casey): Handle this as an error - recreate window?
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
                    
#if SEEDS_INTERNAL
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
#endif
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

internal void
HandleDebugCycleCounters(memory *Memory) {
#if SEEDS_INTERNAL
    
#if PERFORMANCE_DIAGNOSTICS
    OutputDebugStringA("DEBUG CYCLE COUNTS:\n");
#endif
    for(int I = 0; I < ArrayCount(Memory->Counters); I++) {
        debug_cycle_counter *Counter = Memory->Counters + I;
        
        if (Counter->HitCount) {
#if PERFORMANCE_DIAGNOSTICS
            char PrintBuffer[512];
            sprintf_s(PrintBuffer, sizeof(PrintBuffer),
                      "  %3d: %10I64ucy\t%4I32uh\t%10I64ucy/h\t%fms\t%s\n",
                      I,
                      Counter->CycleCount,
                      Counter->HitCount,
                      Counter->CycleCount / Counter->HitCount,
                      (f32)Counter->CycleCount / (f32)GlobalPerfCountFrequency,
                      DebugCycleCounterNames[I]);
            OutputDebugStringA(PrintBuffer);
#endif
            Counter->CycleCount = 0;
            Counter->HitCount = 0;
        }
    }
    
#endif
}

int CALLBACK
WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode) {
    
    LARGE_INTEGER PerfCounterFrequencyResult;
    QueryPerformanceFrequency(&PerfCounterFrequencyResult);
    GlobalPerfCountFrequency = PerfCounterFrequencyResult.QuadPart;
    
    UINT DesiredSchedulerMS = 1;
    b32 SleepIsGranular = (timeBeginPeriod(DesiredSchedulerMS) != TIMERR_NOCANDO);
    
    WNDCLASSA WindowClass1 = {};
    WindowClass1.style         = (CS_OWNDC | CS_HREDRAW | CS_VREDRAW);
    WindowClass1.lpfnWndProc   = Win32MainWindowCallback1;
    WindowClass1.hInstance     = Instance;
    WindowClass1.hCursor       = LoadCursor(0, IDC_ARROW);
    //WindowClass1.hIcon       = 0;
    WindowClass1.lpszClassName = "SMEE_Garden_Main";
    
    if (RegisterClass(&WindowClass1)) {
        DWORD WindowStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
        
        RECT DesiredWindowDimensions = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
        b32 AdjustWindowRectSuccess = AdjustWindowRect(&DesiredWindowDimensions, WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0);
        
        GlobalWindow.Window = CreateWindowEx(0, WindowClass1.lpszClassName,
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
            
            f32 GameUpdateHz = (f32)(MonitorRefreshHz);
            f32 TargetSecondsPerFrame = (1.0f / GameUpdateHz);
            
            GlobalRunning = true;
            
#if SEEDS_INTERNAL
            LPVOID BaseAddress = (LPVOID) Terabytes((u64)2);
#else
            LPVOID BaseAddress = 0;
#endif
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
                
                // Process config file
                char PortBuffer[8]; // Needs to accomodate "COMXXX:"
                string ComPort = STRING_FROM_ARRAY(PortBuffer);
                {
                    debug_read_file_result ConfigFile = DEBUGPlatformReadEntireFile("../data/config");
                    string Cursor = { (char *)ConfigFile.Contents, ConfigFile.ContentSize };
                    while (Cursor.Length) {
                        string Line = EatToNextLine(&Cursor);
                        string Key = EatToNextToken(&Line);
                        if (StringsIdentical(Key, STRING_FROM_LITERAL("PORT"))) {
                            CopyStringNull(ComPort, Line);
                            ComPort.Length = StringLength(ComPort);
                        }
                    }
                }
                
                // Setting up Arduino serial comm
                // https://learn.microsoft.com/en-us/previous-versions/ff802693(v=msdn.10)
                HANDLE ArduinoPort = CreateFile(ComPort.Data, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
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
#if SEEDS_INTERNAL
                    {
                        tracked_files *TrackedFiles = &Memory.TrackedFiles;
                        for (int FileIndex = 0; FileIndex < NO_TRACKED_FILES; FileIndex++) {
                            debug_read_file_result *File = TrackedFiles->Files + FileIndex;
                            
                            if (File->ContentSize) {
                                char      *Filename      = TrackedFiles->Names[FileIndex];
                                file_time *LastWriteTime = TrackedFiles->LastWriteTime + FileIndex;
                                
                                FILETIME NewWriteTime = Win32GetLastWriteTime(Filename);
                                FILETIME OldWriteTime = {};
                                OldWriteTime.dwLowDateTime = LastWriteTime->LowDateTime;
                                OldWriteTime.dwHighDateTime = LastWriteTime->HighDateTime; 
                                
                                if (CompareFileTime(&NewWriteTime, (FILETIME *)TrackedFiles->LastWriteTime + FileIndex) != 0) {
                                    
                                    DEBUGPlatformFreeFileMemory(File->Contents);
                                    *File = DEBUGPlatformReadEntireFile(Filename);
                                    
                                    LastWriteTime->LowDateTime  = NewWriteTime.dwLowDateTime;
                                    LastWriteTime->HighDateTime = NewWriteTime.dwHighDateTime;
                                    
                                    TrackedFiles->Updated       [FileIndex] = 1;
                                    
                                    {
                                        char   PMemory[256];
                                        string PrintBuffer = STRING_FROM_ARRAY(PMemory);
                                        string Cursor = Print(PrintBuffer, STRING_FROM_LITERAL("File updated: %s\n"),
                                                              Filename, StringLength({Filename, MAX_PATH}));
                                        Cursor.Data[0] = '\0';
                                        PrintBuffer.Length = Cursor.Data - PrintBuffer.Data;
                                        OutputDebugStringA(PrintBuffer.Data);
                                    }
                                }
                            }
                        }
                    }
#endif
                    
                    // NOTE: Arduino input
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
                    
                    // NOTE: Keyboard input
                    for (int ButtonIndex = 0; ButtonIndex < ArrayCount(NewInput->Buttons); ButtonIndex++) {
                        NewInput->Buttons[ButtonIndex].HalfTransitionCount = 0;
                        NewInput->Buttons[ButtonIndex].EndedDown =
                            OldInput->Buttons[ButtonIndex].EndedDown;
                    }
                    
                    Win32ProcessPendingMessages(NewInput);
                    
                    // NOTE: Mouse input
                    GlobalWindowInformation = Win32GetWindowInformation(GlobalWindow.Window);
                    
                    POINT MouseP;
                    GetCursorPos(&MouseP);
                    // TODO(canta): Keep this as a float? Normalise it?
                    NewInput->MouseX = MouseP.x;
                    NewInput->MouseY = MouseP.y;
                    NewInput->MouseZ = 0; // TODO(canta): Support mousewheel?
                    
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
                    
                    if (!GlobalPause) {
                        Main(&Memory, NewInput, GlobalWindowInformation);
                        
                        HandleDebugCycleCounters(&Memory);
                        
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
                            
                            if (FrameSecondsElapsed < TargetSecondsPerFrameSleep) {
                                //Log("Missed sleep");
                            }
                            
                            while (FrameSecondsElapsed < TargetSecondsPerFrame) {
                                FrameSecondsElapsed = Win32GetSecondsElapsed(LastCounter, Win32GetWallClock());
                            }
                            
                        }
                        else {
                            //Log("Missed frame rate");
                        }
                        
                        LARGE_INTEGER EndCounter = Win32GetWallClock();
                        f32 FrameTimeMS = 1000.0f * Win32GetSecondsElapsed(LastCounter, EndCounter);
                        f32 FramesPerSecond = 1000.0f / FrameTimeMS;
                        LastCounter = EndCounter;
                        
                        SwapBuffers(GlobalWindow.DeviceContext);
                        FlipWallClock = Win32GetWallClock();
                        
                        input *Temp = NewInput;
                        NewInput = OldInput;
                        OldInput = Temp;
                        
                        u64 EndCycleCount = __rdtsc();
                        f32 CyclesElapsed = (f32)(EndCycleCount - LastCycleCount);
                        LastCycleCount = EndCycleCount;
                        f32 MegaCyclesPerFrame = (f32)CyclesElapsed / (1000.0f * 1000.0f);
#if SEEDS_INTERNAL && PERFORMANCE_DIAGNOSTICS
                        {
                            char PMemory[256];
                            string PrintBuffer = STRING_FROM_ARRAY(PMemory);
                            string Cursor = Print(PrintBuffer, STRING_FROM_LITERAL("%fms, %fFPS, %fMc\n\n"),
                                                  FrameTimeMS, FramesPerSecond, MegaCyclesPerFrame);
                            Cursor.Data[0] = '\0';
                            PrintBuffer.Length = Cursor.Data - PrintBuffer.Data;
                            OutputDebugStringA(PrintBuffer.Data);
                        }
#endif
                    }
                }
            }
        }
    }
    
    return 0;
}
