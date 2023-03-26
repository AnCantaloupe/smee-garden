// TODO(canta): Do something better than fixed size rendering + rescaling on hardware

internal f32
LineWidth(stbtt_pack_range PackRange, string Line) {
    
    f32 Result = 0;
    
    for (int Index = 0; Index < Line.Length; Index++) {
        stbtt_packedchar CharData = PackRange.chardata_for_range[Line.Data[Index]];
        Result += CharData.xadvance;
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
        
        Font->Width  = (GLfloat)BakedDim;
        Font->Height = (GLfloat)BakedDim;
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        stbtt_pack_context PackContext;
        stbtt_PackBegin(&PackContext, (u8 *)BakedMemory, BakedDim, BakedDim, BakedDim, 1, 0);
        stbtt_PackFontRanges(&PackContext, FontInfo.data, 0, &Font->PackRange, 1);
        stbtt_PackEnd(&PackContext);
        
        glGenTextures(1, &Font->Baked);
        glBindTexture(GL_TEXTURE_2D, Font->Baked);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, BakedDim, BakedDim, 0, GL_ALPHA, GL_UNSIGNED_BYTE, BakedMemory);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        
        PopArray(Arena, Sq(BakedDim), u8);
        
        DEBUGPlatformFreeFileMemory(FileRead.Contents);
    }
}

internal void
DrawString(font *Font, f32 Scale, f32 RealX, f32 RealY, colour Colour, text_align Alignment, string String) {
    Scale = 1.0f;
    int NoLines = CountLines(String);
    
    f32 FontSize = Font->PackRange.font_size ;
    
    // @Cleanup(canta)
    string StringCopy = String;
    string Line = EatToNextLine(&StringCopy);
    f32 Offset = 0.0f;
    switch (Alignment) {
        case SA_Left:   Offset = 0.0f;                                 break;
        case SA_Centre: Offset = LineWidth(Font->PackRange, Line) / 2; break;
        case SA_Right:  Offset = LineWidth(Font->PackRange, Line);     break;
    };
    f32 X = (RealX - Offset );
    // @Incomplete(canta): Make this better!
    stbtt_packedchar oCharData = Font->PackRange.chardata_for_range['o'];
    f32 Y = (RealY - ((FontSize * (NoLines - 1)) - (oCharData.y1 - oCharData.y0)) / 2);
    
    glColor4f(Colour.r, Colour.g, Colour.b, Colour.a);
    
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
            
            f32 x0 = (X + CharData.xoff )/CONFIG_Width*2.0f - 1;
            f32 x1 = (X + CharData.xoff2 )/CONFIG_Width*2.0f - 1;
            f32 y0 = 1 - (Y + CharData.yoff )/CONFIG_Height*2.0f;
            f32 y1 = 1 - (Y + CharData.yoff2 )/CONFIG_Height*2.0f;
            
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
                case SA_Left:   Offset = 0.0f;                                 break;
                case SA_Centre: Offset = LineWidth(Font->PackRange, Line) / 2; break;
                case SA_Right:  Offset = LineWidth(Font->PackRange, Line);     break;
            };
            X = RealX - Offset ;
            Y += FontSize;
        }
        else  {
            int XAdvance = (int)(CharData.xadvance );
            // TODO(canta): Necessary? More efficient way?
            //int Kern = stbtt_GetCodepointKernAdvance(FontInfo, Char, NextChar);
            int Kern = 0;
            
            X += (int)(XAdvance + FontSize * Kern);
        }
        
        Char = NextChar;
    }
}
