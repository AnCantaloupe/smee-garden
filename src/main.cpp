#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include "libraries/stb_truetype.h"

#include "libraries/seeds_render_opengl.h"
#include "libraries/seeds_fonts.h"
//#include "libraries/seeds_fonts.cpp"

typedef struct {
    v3 co;
    f32 Theta;
    f32 Phi;
    f32 iTime;
} state;

typedef struct {
    memory_arena Arena;
    memory_heap  Heap;
    
    GLuint Shader;
    font   Vladivostok120;
    font   Vladivostok100;
    font   Vladivostok80;
} transient_state;

internal GLuint
OGL_ConstructSDFShaderProgram(string HeaderCode, string FragmentCode) {
    
    GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    GLchar *FragmentShaderCode[]    = { HeaderCode.Data, FragmentCode.Data };
    GLint   FragmentShaderLengths[] = { (GLint)HeaderCode.Length, (GLint)FragmentCode.Length };
    glShaderSource(FragmentShader, 2, FragmentShaderCode, FragmentShaderLengths);
    glCompileShader(FragmentShader);
    GLint Compiled = 0;
    glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &Compiled);
    if (!Compiled) {
        GLint MaxLength = 0;
        glGetShaderiv(FragmentShader, GL_INFO_LOG_LENGTH, &MaxLength);
        
        char *InfoLog = (char *)alloca(MaxLength);
        glGetShaderInfoLog(FragmentShader, MaxLength, &MaxLength, InfoLog);
        
        glDeleteShader(FragmentShader);
        Assert(0);
    }
    
    GLuint Program = glCreateProgram();
    glAttachShader(Program, FragmentShader);
    glBindAttribLocation(Program, 0, "fragCoord");
    glLinkProgram(Program);
    GLint Linked = 0;
    glGetProgramiv(Program, GL_LINK_STATUS, &Linked);
    if (!Linked) {
        GLint MaxLength = 0;
        glGetProgramiv(Program, GL_INFO_LOG_LENGTH, &MaxLength);
        
        char *InfoLog = (char *)alloca(MaxLength);
        glGetProgramInfoLog(Program, MaxLength, &MaxLength, InfoLog);
        
        glDeleteProgram(Program);
        Assert(0);
    }
    
    glDeleteShader(FragmentShader);
    return Program;
}
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
    
    string Cursor = String;
    string Line = EatToNextLine(&Cursor);
    f32 Offset = 0.0f;
    switch (Alignment) {
        case SA_Left:   Offset = 0.0f;                                 break;
        case SA_Centre: Offset = LineWidth(Font->PackRange, Line) / 2; break;
        case SA_Right:  Offset = LineWidth(Font->PackRange, Line);     break;
    };
    f32 X = (RealX - Offset );
    stbtt_packedchar oCharData = Font->PackRange.chardata_for_range['o'];
    f32 Y = (RealY - ((FontSize * (NoLines - 1)) - (oCharData.y1 - oCharData.y0)) / 2);
    
    glColor4f(Colour.r, Colour.g, Colour.b, Colour.a);
    
    char Char = String.Data[0];
    for (int Index = 0; Index < String.Length; Index++) {
        char NextChar= String.Data[Index + 1];
        
        stbtt_packedchar CharData = {};
        if (Char > 0) {
            CharData = Font->PackRange.chardata_for_range[Char];
            
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
            Index += 1;
            NextChar = String.Data[Index + 1];
            
            Line = EatToNextLine(&Cursor);
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
            int Kern = 0;
            
            X += (int)(XAdvance + FontSize * Kern);
        }
        
        Char = NextChar;
    }
}

internal void
ResetState(state *State) {
    State->co = {
        -3.2f,
        0.15f,
        0.0f
    };
    State->Theta = 2.35f;
    State->Phi   = 0.0f;
    State->iTime = 0.0f;
}

#include <math.h>

internal void
Main(memory *Memory, input *Input, window_information WindowInformation) {
    
    ////////////////////////////////////////////////////////////////////////////////////
    // Setup
    ////////////////////////////////////////////////////////////////////////////////////
    state *State = (state *)Memory->PermanentStorage;
    v3  &co    = State->co;
    f32 &Theta = State->Theta;
    f32 &Phi   = State->Phi;
    f32 &iTime = State->iTime;
    
    transient_state *TransientState = (transient_state *)Memory->TransientStorage;
    memory_arena    *Arena = &TransientState->Arena;
    memory_heap     *Heap  = &TransientState->Heap;
    GLuint &Shader = TransientState->Shader;
    // TODO(isuru): Probably not worth doing this but wtv
    font   *Vladivostok120 = &TransientState->Vladivostok120;
    font   *Vladivostok100 = &TransientState->Vladivostok100;
    font   *Vladivostok80  = &TransientState->Vladivostok80;
    
    ////////////////////////////////////////////////////////////////////////////////////
    // Initialisation
    ////////////////////////////////////////////////////////////////////////////////////
    if (!Memory->IsInitialised) {
        InitialiseArena(Arena,
                        (u8 *)Memory->TransientStorage + sizeof(transient_state),
                        Memory->TransientStorageSize - sizeof(transient_state));
        u32 HeapSize = Megabytes(512);
        InitialiseHeap(Heap, PushArray(Arena, HeapSize, u8), HeapSize);
        
        InitialiseFont(Arena, Vladivostok120, STRING_FROM_LITERAL("../data/vladivostok_bold.ttf"), 120*CONFIG_Width/1080.0f, 128, 4096);
        InitialiseFont(Arena, Vladivostok100, STRING_FROM_LITERAL("../data/vladivostok_bold.ttf"), 100*CONFIG_Width/1080.0f, 128, 4096);
        InitialiseFont(Arena, Vladivostok80, STRING_FROM_LITERAL("../data/vladivostok_bold.ttf"), 80*CONFIG_Width/1080.0f, 128, 4096);
        
        BindOpenGLFunctions();
        
        debug_read_file_result FragFile = DEBUGPlatformReadEntireFile("../data/garden0.frag");
        string FragCode = { (char *)FragFile.Contents, FragFile.ContentSize };
        Shader = OGL_ConstructSDFShaderProgram(STRING_FROM_LITERAL(""), FragCode);
        DEBUGPlatformFreeFileMemory(FragFile.Contents);
        
        glUseProgram(Shader);
        glUseProgram(0);
        
        ResetState(State);
        
        Memory->IsInitialised = true;
    }
    
    ////////////////////////////////////////////////////////////////////////////////////
    // Input
    ////////////////////////////////////////////////////////////////////////////////////
    State->iTime += Input->TimeStepFrame;
    
    arduino Ard = Input->Arduino;
    
    // Looking around
    s32 MouseDX = 0;
    s32 MouseDY = 0;
    if (ButtonDownNotPressed(Input->MouseButtonLeft)) {
        MouseDX = Input->MouseX - Input->LastMouseX;
        MouseDY = Input->MouseY - Input->LastMouseY;
    }
    Input->LastMouseX = Input->MouseX;
    Input->LastMouseY = Input->MouseY;
    f32 MouseSensitivity = 2.0f;
    Theta += MouseSensitivity * Pi32 * MouseDX / 32768.0f;
    Theta += 0.05f * Input->TimeStepFrame * (ButtonDown(Input->Buttons[BUTTON_Left]) - ButtonDown(Input->Buttons[BUTTON_Right]));
    Theta += (Ard.Right-Ard.Left)*Input->TimeStepFrame*0.2f;
    if      (Theta < 0.0f)        Theta += 2.0f *  Pi32;
    else if (Theta > 2.0f * Pi32) Theta -= 2.0f * Pi32;
    
    v3 cz = {
        cosf(Theta)*cosf(Phi),
        sinf(Theta)*cosf(Phi),
        -sinf(Phi),
    };
    v3 cx = Cross(cz, v3{0.0, 0.0, 1.0});
    v3 cy = Cross(cx, cz);
    
    // Walking
    int xdir = 0;
    int ydir = 0;
    xdir += ButtonDown(Input->Buttons[BUTTON_D]);
    xdir -= ButtonDown(Input->Buttons[BUTTON_A]);
    ydir += ButtonDown(Input->Buttons[BUTTON_W]);
    ydir += ButtonDown(Input->Buttons[BUTTON_Up]);
    ydir -= ButtonDown(Input->Buttons[BUTTON_S]);
    ydir -= ButtonDown(Input->Buttons[BUTTON_Down]);
    ydir += 2*(Ard.Forward - Ard.Back);
    v3 dx = (f32)xdir*cx*Input->TimeStepFrame*0.5f;
    v3 dy = (f32)ydir*cz*Input->TimeStepFrame*0.5f;
    dx.z = 0.0f;
    dy.z = 0.0f;
    co += dx;
    co += dy;
    
    // Utilities
    if (ButtonReleased(Input->Buttons[BUTTON_R])) ResetState(State);
    
    ////////////////////////////////////////////////////////////////////////////////////
    // Rendering
    ////////////////////////////////////////////////////////////////////////////////////
    // Garden shader
    glUseProgram(Shader);
    glUniform1i(glGetUniformLocation(Shader, "iChannel0"), 0);
    glUniform1f(glGetUniformLocation(Shader, "iTime"), iTime);
    glUniform3f(glGetUniformLocation(Shader, "iResolution"), CONFIG_Width, CONFIG_Height, 1.0f);
    glUniform3f(glGetUniformLocation(Shader, "co"), co.x, co.z, co.y);
    glUniform3f(glGetUniformLocation(Shader, "cx"), cx.x, cx.z, cx.y);
    glUniform3f(glGetUniformLocation(Shader, "cy"), cy.x, cy.z, cy.y);
    glUniform3f(glGetUniformLocation(Shader, "cz"), cz.x, cz.z, cz.y);
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(1.0f, -1.0f);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(-1.0f, 1.0f);
    glEnd();
    
    // Text
    glUseProgram(0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glBindTexture(GL_TEXTURE_2D, Vladivostok120->Baked);
    glBegin(GL_QUADS);
    DrawString(Vladivostok120, 120.0f/120.0f,
               110.0f*CONFIG_Width/1080.0f, 400.0f*CONFIG_Height/1920.0f, Colour(0xFFD79050), SA_Left,
               STRING_FROM_LITERAL("S M E E"));
    
#if 0    
    string Today = STRING_FROM_LITERAL("T O D A Y");
    colour TodayColour = Colour(fmodf((float)rand(), 20.0f)/100.0f+0.8f, sinf(1.2f*iTime), cosf(iTime), sinf(-iTime));
    TodayColour.r += fmodf((float)rand(), 10.0f)/100.0f;
    TodayColour.g += fmodf((float)rand(), 10.0f)/100.0f;
    TodayColour.b += fmodf((float)rand(), 10.0f)/100.0f;
    DrawString(Vladivostok120, 120.0f/120.0f,
               540.0f*CONFIG_Width/1080.0f, 750.0f*CONFIG_Height/1920.0f, TodayColour, SA_Centre,
               Today);
    DrawString(Vladivostok120, 120.0f/120.0f,
               540.0f*CONFIG_Width/1080.0f, 950.0f*CONFIG_Height/1920.0f, TodayColour, SA_Centre,
               Today);
    DrawString(Vladivostok120, 120.0f/120.0f,
               540.0f*CONFIG_Width/1080.0f, 1150.0f*CONFIG_Height/1920.0f, TodayColour, SA_Centre,
               Today);
#endif
    
    glEnd();
    
    glBindTexture(GL_TEXTURE_2D, Vladivostok100->Baked);
    glBegin(GL_QUADS);
    DrawString(Vladivostok100, 100.0f/120.0f,
               110.0f*CONFIG_Width/1080.0f, 450.0f*CONFIG_Height/1920.0f, CL_BLACK, SA_Left,
               STRING_FROM_LITERAL("G A R D E N\n \nS A L E\n+ T E A"));
    glEnd();
    
    glBindTexture(GL_TEXTURE_2D, Vladivostok80->Baked);
    glBegin(GL_QUADS);
    DrawString(Vladivostok80, 80.0f/120.0f,
               970.0f*CONFIG_Width/1080.0f, 1300.0f*CONFIG_Height/1920.0f, CL_BLACK, SA_Right,
               STRING_FROM_LITERAL("1 1 A M - 2 P M"));
    DrawString(Vladivostok80, 80.0f/120.0f,
               970.0f*CONFIG_Width/1080.0f, 1415.0f*CONFIG_Height/1920.0f, CL_BLACK, SA_Right,
               STRING_FROM_LITERAL("2 0   M A R C H\nM O N   W E E K   4"));
    DrawString(Vladivostok80, 80.0f/120.0f,
               970.0f*CONFIG_Width/1080.0f, 1600.0f*CONFIG_Height/1920.0f, CL_BLACK, SA_Right,
               STRING_FROM_LITERAL("K E N N E T H   H U N T\nM E M O R I A L   G A R D E N"));
    glEnd();
}
