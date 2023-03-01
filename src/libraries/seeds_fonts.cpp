// TODO(canta): Do something better than fixed size rendering + rescaling on hardware

internal int
LineWidth(stbtt_pack_range PackRange, string Line) {
    
    int Result = 0;
    
    for (int Index = 0; Index < Line.Length; Index++) {
        stbtt_packedchar CharData = PackRange.chardata_for_range[Line.Data[Index]];
        Result += (int)CharData.xadvance;
    }
    
    return Result;
}

internal void
InitialiseFont(memory_arena *Arena, font *Font, string FontFullPath,
               f32 FontSizePixels, int NoCharacters = 128, int BakedDim = 4096) {
    
    stbtt_fontinfo FontInfo;
    
    debug_read_file_result FileRead = DEBUGPlatformReadEntireFile(FontFullPath.Data);
    if (FileRead.ContentSize) {
        
        int FontIndex = 0;
        int FontOffset = stbtt_GetFontOffsetForIndex((u8 *)FileRead.Contents, FontIndex);
        stbtt_InitFont(&FontInfo, (u8 *)FileRead.Contents, FontOffset);
        Font->PackRange = { FontSizePixels, 0, 0, NoCharacters, PushArray(Arena, NoCharacters, stbtt_packedchar), 0, 0 };
        
        void *BakedMemory = PushArray(Arena, Sq(BakedDim), u8);
        
#if SEEDS_OPENGL
        Font->Width  = (GLfloat)BakedDim;
        Font->Height = (GLfloat)BakedDim;
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#else
        Font->Baked  = { BakedDim, BakedDim, BakedDim, BakedMemory};
#endif
        
        stbtt_pack_context PackContext;
        stbtt_PackBegin(&PackContext, (u8 *)BakedMemory, BakedDim, BakedDim, BakedDim, 1, 0);
        stbtt_PackFontRanges(&PackContext, FontInfo.data, 0, &Font->PackRange, 1);
        stbtt_PackEnd(&PackContext);
        
#if SEEDS_OPENGL
        glGenTextures(1, &Font->Baked);
        glBindTexture(GL_TEXTURE_2D, Font->Baked);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, BakedDim, BakedDim, 0, GL_ALPHA, GL_UNSIGNED_BYTE, BakedMemory);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        
        PopArray(Arena, Sq(BakedDim), u8);
#endif
        
        DEBUGPlatformFreeFileMemory(FileRead.Contents);
    }
    else {
        // TODO(canta): Logging
    }
}

#if SEEDS_OPENGL

internal void
DrawString(font *Font, string String, f32 RealX, f32 RealY, colour Colour, text_align Alignment = SA_Left) {
    
    BEGIN_TIMED_BLOCK(DrawString);
    
    int X = (int)RealX;
    int Y = (int)RealY;
    
    int NoLines = CountLines(String);
    
    int FontSize = (int)Font->PackRange.font_size;
    
    // @Cleanup(canta)
    string StringCopy = String;
    string Line = EatToNextLine(&StringCopy);
    switch (Alignment) {
        case SA_Left:   X = X;                                        break;
        case SA_Centre: X = X - LineWidth(Font->PackRange, Line) / 2; break;
        case SA_Right:  X = X - LineWidth(Font->PackRange, Line);     break;
    };
    // @Incomplete(canta): Make this better!
    stbtt_packedchar oCharData = Font->PackRange.chardata_for_range['o'];
    Y -= ((FontSize * (NoLines - 1)) - (oCharData.y1 - oCharData.y0)) / 2;
    
    f32 HalfWidth  = SCREEN_WIDTH  / 2;
    f32 HalfHeight = SCREEN_HEIGHT / 2;
    
    glColor4f(Colour.r, Colour.g, Colour.b, Colour.a);
    
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, Font->Baked);
    glBegin(GL_QUADS);
    
    char Char = String.Data[0];
    for (int Index = 0; Index < String.Length; Index++) {
        char NextChar= String.Data[Index + 1];
        
        stbtt_packedchar CharData = {};
        if (Char > 0) {
            CharData = Font->PackRange.chardata_for_range[Char];
            
            // TODO(canta): Wtfrickkkkk
            f32 u0 = CharData.x0 / Font->Width;
            f32 v0 = CharData.y0 / Font->Height;
            f32 u1 = CharData.x1 / Font->Width;
            f32 v1 = CharData.y1 / Font->Height;
            
            // @Cleanup(canta): I can't remember the point of this
            int YG = Y + (int)(Font->PackRange.font_size);
            YG = Y;
            
            f32 x0 = (X + CharData.xoff)  / HalfWidth - 1;
            f32 x1 = (X + CharData.xoff2) / HalfWidth - 1;
            f32 y0 = 1 - (YG + CharData.yoff)  / HalfHeight;
            f32 y1 = 1 - (YG + CharData.yoff2) / HalfHeight;
            
            glTexCoord2f(u0, v1); glVertex2f(x0, y1);
            glTexCoord2f(u1, v1); glVertex2f(x1, y1);
            glTexCoord2f(u1, v0); glVertex2f(x1, y0);
            glTexCoord2f(u0, v0); glVertex2f(x0, y0);
        }
        
        if (NextChar == '\n') {
            // TODO(canta): This is a total fucking mess.
            Index += 1;
            NextChar = String.Data[Index + 1];
            
            // @Cleanup(canta)
            string StringNC = string{ String.Data + Index + 1, String.Length - Index - 1};
            Line = EatToNextLine(&StringNC);
            
            switch (Alignment) {
                case SA_Left:   X = (int)RealX;                                             break;
                case SA_Centre: X = (int)RealX - LineWidth(Font->PackRange, Line) / 2; break;
                case SA_Right:  X = (int)RealX - LineWidth(Font->PackRange, Line);     break;
            };
            Y += (int)FontSize;
            
        }
        else  {
            int XAdvance = (int)CharData.xadvance;
            // TODO(canta): Necessary? More efficient way?
            //int Kern = stbtt_GetCodepointKernAdvance(FontInfo, Char, NextChar);
            int Kern = 0;
            
            X += (int)(XAdvance + FontSize * Kern);
        }
        
        Char = NextChar;
    }
    
    glEnd();
    
    END_TIMED_BLOCK(DrawString);
}

#else

internal void
DrawCharacter(game_offscreen_buffer *DisplayBuffer, loaded_bitmap *Bitmap, f32 FMinX, f32 FMinY, f32 FMaxX, f32 FMaxY, colour Colour) {
    
    int MinX = (int)FMinX;
    int MinY = (int)FMinY;
    int MaxX = (int)FMaxX;
    int MaxY = (int)FMaxY;
    
    int ClampedMinX = (MinX >= 0) ? MinX : 0;
    int ClampedMinY = (MinY >= 0) ? MinY : 0;
    int ClampedMaxX = (MaxX < DisplayBuffer->Width)  ? MaxX : DisplayBuffer->Width;
    int ClampedMaxY = (MaxY < DisplayBuffer->Height) ? MaxY : DisplayBuffer->Height;
    
    u32 ColourU32 = (  (u8)(Colour.a * 255.0f) << 24
                     | (u8)(Colour.r * 255.0f) << 16
                     | (u8)(Colour.g * 255.0f) <<  8
                     | (u8)(Colour.b * 255.0f) <<  0);
    
    u8 *SourceRow = ((u8 *)Bitmap->Memory
                     + (ClampedMinY - MinY) * Bitmap->Pitch
                     + (ClampedMinX - MinX) * BYTES_PER_PIXEL);
    u8 *DestRow   = ((u8 *)DisplayBuffer->Memory
                     + ClampedMinY * DisplayBuffer->Pitch
                     + ClampedMinX * BYTES_PER_PIXEL);
    for (int YI = ClampedMinY; YI < ClampedMaxY; YI++) {
        
        u8  *Source = (u8 *)SourceRow;
        u32 *Dest   = (u32 *)DestRow;
        for (int XI = ClampedMinX; XI < ClampedMaxX; XI++) {
            
            int AOffset = 24;
            int ROffset = 16;
            int GOffset = 8;
            int BOffset = 0;
            
            f32 SourceAlpha = Colour.a * ((f32)(*Source) / 255.0f);
            if (SourceAlpha == 0.0f) {
            }
            else if (SourceAlpha == 1.0f) {
                *Dest = ColourU32;
            }
            else {
                // @Alpha
                f32 SourceRed   = SourceAlpha * Colour.r;
                f32 SourceGreen = SourceAlpha * Colour.g;
                f32 SourceBlue  = SourceAlpha * Colour.b;
                
                f32 DestAlpha   = (((*Dest >> AOffset) & 0xFF) / 255.0f);
                f32 DestRed     = (((*Dest >> ROffset) & 0xFF) / 255.0f);
                f32 DestGreen   = (((*Dest >> GOffset) & 0xFF) / 255.0f);
                f32 DestBlue    = (((*Dest >> BOffset) & 0xFF) / 255.0f);
                
                f32 Multiplier = 1 - SourceAlpha;
                f32 RedF32      = DestRed   * Multiplier + SourceRed;
                f32 GreenF32    = DestGreen * Multiplier + SourceGreen;
                f32 BlueF32     = DestBlue  * Multiplier + SourceBlue;
                
                u8 R = (u8)(RedF32   * 255.0f);
                u8 G = (u8)(GreenF32 * 255.0f);
                u8 B = (u8)(BlueF32  * 255.0f);
                
                *Dest = (  0xFF << AOffset
                         | (R << ROffset)
                         | (G << GOffset)
                         | (B << BOffset)  );
            }
            
            Source++;
            Dest++;
        }
        
        SourceRow += Bitmap->Pitch;
        DestRow   += DisplayBuffer->Pitch;
    }
}

internal void
DrawString(game_offscreen_buffer *DisplayBuffer, font *Font, string String, f32 RealX, f32 RealY, colour Colour, text_align Alignment = SA_Left) {
    
    BEGIN_TIMED_BLOCK(DrawString);
    
    int X = (int)RealX;
    int Y = (int)RealY;
    
    int NoLines = CountLines(String);
    
    int FontSize = (int)Font->PackRange.font_size;
    
    // @Cleanup(canta)
    string StringCopy = String;
    string Line = EatToNextLine(&StringCopy);
    switch (Alignment) {
        case SA_Left:   X = X;                                        break;
        case SA_Centre: X = X - LineWidth(Font->PackRange, Line) / 2; break;
        case SA_Right:  X = X - LineWidth(Font->PackRange, Line);     break;
    };
    // @Incomplete(canta): Make this better!
    stbtt_packedchar oCharData = Font->PackRange.chardata_for_range['o'];
    Y -= ((FontSize * (NoLines - 1)) - (oCharData.y1 - oCharData.y0)) / 2;
    
    char Char = String.Data[0];
    for (int Index = 0; Index < String.Length; Index++) {
        char NextChar= String.Data[Index + 1];
        
        stbtt_packedchar CharData = {};
        if (Char > 0) {
            CharData = Font->PackRange.chardata_for_range[Char];
            
            loaded_bitmap Bitmap;
            Bitmap.Width  = (int)(CharData.x1 - CharData.x0);
            Bitmap.Height = (int)(CharData.y1 - CharData.y0);
            Bitmap.Pitch  = Font->Baked.Pitch;
            Bitmap.Memory = ((u8 *)Font->Baked.Memory
                             + (int)CharData.y0 * Font->Baked.Pitch
                             + (int)CharData.x0);
            
            DrawCharacter(DisplayBuffer, &Bitmap, X + CharData.xoff, Y + CharData.yoff, X + CharData.xoff2, Y + CharData.yoff2, Colour);
        }
        
        if (NextChar == '\n') {
            // TODO(canta): This is a total fucking mess.
            Index += 1;
            NextChar = String.Data[Index + 1];
            
            // @Cleanup(canta)
            string StringNC = string{ String.Data + Index + 1, String.Length - Index - 1};
            Line = EatToNextLine(&StringNC);
            
            switch (Alignment) {
                case SA_Left:   X = (int)RealX;                                             break;
                case SA_Centre: X = (int)RealX - LineWidth(Font->PackRange, Line) / 2; break;
                case SA_Right:  X = (int)RealX - LineWidth(Font->PackRange, Line);     break;
            };
            Y += (int)FontSize;
            
        }
        else  {
            int XAdvance = (int)CharData.xadvance;
            // TODO(canta): Necessary? More efficient way?
            //int Kern = stbtt_GetCodepointKernAdvance(FontInfo, Char, NextChar);
            int Kern = 0;
            
            X += (int)(XAdvance + FontSize * Kern);
        }
        
        Char = NextChar;
    }
    
    END_TIMED_BLOCK(DrawString);
}

inline void
DrawS32Dec(game_offscreen_buffer *DisplayBuffer, font *Font, s32 Value, f32 RealX, f32 RealY, colour Colour, text_align Alignment = SA_Left) {
    
    char SMemory[64];
    string String = STRING_FROM_ARRAY(SMemory);
    String.Length = S32ToDecString(String, Value).Data - String.Data;
    
    DrawString(DisplayBuffer, Font, String, RealX, RealY, Colour, Alignment);
}

inline void
DrawU32Dec(game_offscreen_buffer *DisplayBuffer, font *Font, u32 Value, f32 RealX, f32 RealY, colour Colour,  text_align Alignment = SA_Left) {
    
    char SMemory[64];
    string String = STRING_FROM_ARRAY(SMemory);
    String.Length = U32ToDecString(String, Value).Data - String.Data;
    
    DrawString(DisplayBuffer, Font, String, RealX, RealY, Colour, Alignment);
}

inline void
DrawF32Dec(game_offscreen_buffer *DisplayBuffer, font *Font, f32 Value, f32 RealX, f32 RealY, colour Colour,  text_align Alignment = SA_Left) {
    
    char SMemory[64];
    string String = STRING_FROM_ARRAY(SMemory);
    String.Length = F32ToDecString(String, Value).Data - String.Data;
    
    DrawString(DisplayBuffer, Font, String, RealX, RealY, Colour, Alignment);
}

#endif
