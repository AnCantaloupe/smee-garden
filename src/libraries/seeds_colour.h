#ifndef SEEDS_COLOUR_H
#define SEEDS_COLOUR_H

struct colour {
    
    union {
        v4 argb;
        struct {
            f32 a;
            v3 rgb;
        };
        struct {
            f32 a;
            f32 r;
            f32 g;
            f32 b;
        };
    };
    union {
        u32 hex;
        struct {
            u8 ha;
            u8 hr;
            u8 hg;
            u8 hb;
        };
    };
};

inline v4
U32ToV4Colour(u32 Colour) {
    
    return v4{
        (f32)((Colour >> 24) & 0xFF) / 255.0f,
        (f32)((Colour >> 16) & 0xFF) / 255.0f,
        (f32)((Colour >> 8)  & 0xFF) / 255.0f,
        (f32)((Colour >> 0)  & 0xFF) / 255.0f,
    };
}

inline u32
V4ToU32Colour(v4 Colour) {
    
    return (  ((u8)(Colour.a * 255.0f) << 24)
            | ((u8)(Colour.r * 255.0f) << 16)
            | ((u8)(Colour.g * 255.0f) << 8)
            | ((u8)(Colour.b * 255.0f) << 0));
}

inline colour
Lerp(colour ColourLow, f32 T, colour ColourHigh) {
    
    colour Result = colour{ 
        Lerp(ColourLow.a, T, ColourHigh.a),
        Lerp(ColourLow.r, T, ColourHigh.r),
        Lerp(ColourLow.g, T, ColourHigh.g),
        Lerp(ColourLow.b, T, ColourHigh.b)
    };
    Result.hex = V4ToU32Colour(Result.argb);
    return Result;
}

inline colour
Colour(u32 C) {
    
    colour Result;
    Result.argb = U32ToV4Colour(C);
    Result.hex = C;
    return Result;
}

inline colour
Colour(v4 C) {
    
    colour Result;
    Result.argb = C;
    Result.hex  = V4ToU32Colour(C);
    return Result;
}

inline colour
Colour(f32 A, v3 RGB) {
    
    colour Result;
    Result.argb = { A, RGB.x, RGB.y, RGB.z };
    Result.hex  = V4ToU32Colour(Result.argb);
    return Result;
}

inline colour
Colour(f32 A, f32 R, f32 G, f32 B) {
    
    colour Result;
    Result.argb = { A, R, G, B };
    Result.hex  = V4ToU32Colour(Result.argb);
    return Result;
}

inline colour
Colour(f32 A, u32 RGB) {
    
    colour Result;
    Result.argb = {
        A,
        (f32)((RGB >> 16) & 0xFF) / 255.0f,
        (f32)((RGB >> 8)  & 0xFF) / 255.0f,
        (f32)((RGB >> 0)  & 0xFF) / 255.0f,
    };
    Result.hex = RGB;
    Result.ha  = (u8)(A * 255.0f);
    return Result;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

global const colour CL_WHITE   = Colour(1.0f, 1.0f, 1.0f, 1.0f);
global const colour CL_BLACK   = Colour(1.0f, 0.0f, 0.0f, 0.0f);
global const colour CL_RED     = Colour(1.0f, 1.0f, 0.0f, 0.0f);
global const colour CL_GREEN   = Colour(1.0f, 0.0f, 1.0f, 0.0f);
global const colour CL_BLUE    = Colour(1.0f, 0.0f, 0.0f, 1.0f);
global const colour CL_MAGENTA = Colour(1.0f, 1.0f, 0.0f, 1.0f);
global const colour CL_CYAN    = Colour(1.0f, 0.0f, 1.0f, 1.0f);
global const colour CL_YELLOW  = Colour(1.0f, 1.0f, 1.0f, 0.0f);
global const colour CL_ERROR   = Colour(0xFFB04880);

global const colour EDITOR_COLOUR_XY = Colour(0xFF5050C8);
global const colour EDITOR_COLOUR_XZ = Colour(0xFF50C890);
global const colour EDITOR_COLOUR_YZ = Colour(0xFFC85070);

#endif //SEEDS_COLOUR_H
