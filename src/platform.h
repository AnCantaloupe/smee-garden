#ifndef PLATFORM_H

#if !defined(COMPILER_MSVC)
#define COMPILER_MSVC 0
#endif

#if !COMPILER_MSVC
#if _MSC_VER
#undef COMPILER_MSVC
#define COMPILER_MSVC 1
#endif
#endif

#if COMPILER_MSVC
#include <intrin.h>
#endif

#include <stdint.h> // TODO(canta)
typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;
typedef          char      s8;
typedef          short     s16;
typedef          int       s32;
typedef          long long s64;
typedef          size_t    memory_index;
typedef          float     f32;
typedef          double    f64;
typedef          int       b32;
typedef          void *    HANDLE;

#define internal static
#define local    static
#define global   static

#define Pi32 3.14159265359f

#define Assert(Expression) if (!(Expression)) { *(int *)0 = 0; }
#define Warning(Expression) (OutputDebugStringA("WARNING: " (Expression) "\n"))
#define Log(Expression) (OutputDebugStringA((Expression) "\n"))

#define Kilobytes(Value) ((Value) * 1024)
#define Megabytes(Value) (Kilobytes(Value) * 1024)
#define Gigabytes(Value) (Megabytes(Value) * 1024)
#define Terabytes(Value) (Gigabytes(Value) * 1024)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

inline u32 SafeTruncateU64(u64 Value) {
    
    Assert(Value <= 0xFFFFFFFFFFFFFFFF);
    u32 Result = (u32)Value;
    
    return Result;
}

#define MAX_PATH 260

typedef struct debug_read_file_result {
    u32  ContentSize;
    void *Contents;
} debug_read_file_result;

typedef struct file_time {
    u32 LowDateTime;
    u32 HighDateTime;
} file_time;

typedef struct {
    
    bool IsInitialised;
    
    u64  PermanentStorageSize;
    void *PermanentStorage; // NOTE: REQUIRED to be cleared to zero at startup
    
    u64  TransientStorageSize;
    void *TransientStorage; // NOTE: REQUIRED to be cleared to zero at startup
} memory;

internal debug_read_file_result DEBUGPlatformReadEntireFile(char *Filename);
internal file_time DEBUGPlatformGetFileWriteTime(char *Filename);
internal void PlatformSetCurrentGLContext(int Index);

enum {
    BUTTON_A,
    BUTTON_D,
    BUTTON_R,
    BUTTON_S,
    BUTTON_W,
    BUTTON_Left,
    BUTTON_Right,
    BUTTON_Up,
    BUTTON_Down,
    BUTTON_Count
};

typedef struct {
    int HalfTransitionCount;
    b32 EndedDown;
} button_state;

typedef struct {
    int Steps;
    int Left;
    int Right;
    int Forward;
    int Back;
} arduino;

typedef struct {
    union {
        button_state MouseButtons[3];
        struct {
            button_state MouseButtonLeft;
            button_state MouseButtonRight;
            button_state MouseButtonMiddle;
        };
    };
    
    s32 MouseX;
    s32 MouseY;
    s32 MouseZ; // (canta): This is the mousewheel!
    s32 LastMouseX;
    s32 LastMouseY;
    
    arduino Arduino;
    
    button_state Buttons[BUTTON_Count];
    
    f32 TimeStepFrame;
} input;

inline b32 ButtonDown(button_state Button) {
    return Button.EndedDown;
}

inline b32 ButtonDownNotPressed(button_state Button) {
    return (Button.EndedDown && !Button.HalfTransitionCount);
}

inline b32 ButtonPressed(button_state Button) {
    return Button.EndedDown && Button.HalfTransitionCount;
}

inline b32 ButtonReleased(button_state Button) {
    return !Button.EndedDown && Button.HalfTransitionCount;
}

typedef struct {
    b32 Active;
    int X;
    int Y;
    int W;
    int H;
} window_information;
global window_information GlobalWindowInformation;
global memory *DEBUGGlobalMemory = {};

#define PLATFORM_H
#endif
