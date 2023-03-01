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
#pragma intrinsic(_BitScanForward)
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
#define local_persist   static
#define global_variable static

#include <limits.h>
#include <float.h>
#define U8_MIN  (UCHAR_MIN)
#define U16_MIN (USHRT_MIN)
#define U32_MIN (ULONG_MIN)
#define U64_MIN (ULLONG_MIN)
#define U8_MAX  (UCHAR_MAX)
#define U16_MAX (USHRT_MAX)
#define U32_MAX (ULONG_MAX)
#define U64_MAX (ULLONG_MAX)
#define S8_MIN  (CHAR_MIN)
#define S16_MIN (SHRT_MIN)
#define S32_MIN (LONG_MIN)
#define S64_MIN (LLONG_MIN)
#define S8_MAX  (CHAR_MAX)
#define S16_MAX (SHRT_MAX)
#define S32_MAX (LONG_MAX)
#define S64_MAX (LLONG_MAX)
#define F32_MIN (FLT_MIN)
#define F64_MIN (DBL_MIN)
#define F32_MAX (FLT_MAX)
#define F64_MAX (DBL_MAX)

#define Pi32 3.14159265359f

#define Assert(Expression) if (!(Expression)) { *(int *)0 = 0; }
#define Warning(Expression) (OutputDebugStringA("WARNING: " Expression "\n"))
#define Log(Expression) (OutputDebugStringA(Expression "\n"))

#define Kilobytes(Value) ((Value) * 1024)
#define Megabytes(Value) (Kilobytes(Value) * 1024)
#define Gigabytes(Value) (Megabytes(Value) * 1024)
#define Terabytes(Value) (Gigabytes(Value) * 1024)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

inline u32 SafeTruncateU64(u64 Value) {
    
    Assert(Value <= U64_MAX);
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

#define NO_TRACKED_FILES 1
typedef struct tracked_files {
    debug_read_file_result Files [NO_TRACKED_FILES];
    char Names                   [NO_TRACKED_FILES][MAX_PATH];
    b32  Updated                 [NO_TRACKED_FILES];
    file_time LastWriteTime      [NO_TRACKED_FILES];
    int  NoFiles;
} tracked_files;

char *DebugCycleCounterNames[] = {
    "SaySMEE",
    "DrawString",
};

enum {
    DebugCycleCounter_SaySMEE,
    DebugCycleCounter_DrawString,
    DebugCycleCounter_Count
};

typedef struct debug_cycle_counter {
    u64 CycleCount;
    u32 HitCount;
} debug_cycle_counter;

#if _MSC_VER
#define BEGIN_TIMED_BLOCK(ID) u64 StartCycleCount##ID = __rdtsc();
#define END_TIMED_BLOCK(ID)   DEBUGGlobalMemory->Counters[DebugCycleCounter_##ID].CycleCount += __rdtsc() - StartCycleCount##ID; DEBUGGlobalMemory->Counters[DebugCycleCounter_##ID].HitCount++
#else
#define BEGIN_TIMED_BLOCK(ID)
#define END_TIMED_BLOCK(ID)
#endif

typedef struct {
    
    bool IsInitialised;
    
    u64  PermanentStorageSize;
    void *PermanentStorage; // NOTE: REQUIRED to be cleared to zero at startup
    
    u64  TransientStorageSize;
    void *TransientStorage; // NOTE: REQUIRED to be cleared to zero at startup
    
#if SEEDS_INTERNAL
    tracked_files TrackedFiles;
    debug_cycle_counter Counters[DebugCycleCounter_Count];
#endif
} memory;

internal debug_read_file_result DEBUGPlatformReadEntireFile(char *Filename);
internal file_time DEBUGPlatformGetFileWriteTime(char *Filename);
internal void PlatformSetCurrentGLContext(int Index);

internal b32 TrackFile(memory *Memory, char *Filename) {
    
    tracked_files *TrackedFiles = &Memory->TrackedFiles;
    
    // Storing name
    // @Cleanup(canta): CopyStringNull this fella
    int   CharIndex = 0;
    char *Source = Filename;
    char *Dest   = TrackedFiles->Names[0];
    while (*Source && CharIndex < MAX_PATH) *Dest++ = *Source++;
    
    // Getting file
    debug_read_file_result *File = TrackedFiles->Files;
    *File = DEBUGPlatformReadEntireFile(Filename);
    if (File->ContentSize) {
        TrackedFiles->LastWriteTime [0] = (file_time)DEBUGPlatformGetFileWriteTime(Filename);
        TrackedFiles->Updated       [0] = 1;
        
        TrackedFiles->NoFiles = 1;
        
        return 1;
    }
    else {
        Warning("File empty");
        return 0;
    }
}

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
