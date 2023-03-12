#define SCREEN_WIDTH 1080
#define SCREEN_HEIGHT 1920
#define RENDER_WIDTH 1080
#define RENDER_HEIGHT 1920


global f32    GlobalMaxDepth     = 100000.0f;
global __m128 GlobalMaxDepthWide = _mm_set1_ps(100000.0f);
global f32    DistanceToScreenCoefficient = 5.0f;

#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include "libraries/stb_truetype.h"

#include "libraries/seeds_render_opengl.h"
#include "libraries/seeds_fonts.h"
#include "libraries/seeds_entity.h"

#include "libraries/seeds_fonts.cpp"

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
    font   SMEEFont;
    font   EventFont;
    font   DateFont;
    font   TimeFont;
    font   PlaceFont;
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

inline void
DrawS32Dec(font *Font, s32 Value, f32 RealX, f32 RealY, colour Colour, text_align Alignment = SA_Left) {
    
    char SMemory[64];
    string String = STRING_FROM_ARRAY(SMemory);
    String.Length = S32ToDecString(String, Value).Data - String.Data;
    
    DrawString(Font, String, RealX, RealY, Colour, Alignment);
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
    GLuint &Shader    = TransientState->Shader;
    font   *SMEEFont  = &TransientState->SMEEFont;
    font   *EventFont = &TransientState->EventFont;
    font   *DateFont  = &TransientState->DateFont;
    font   *TimeFont  = &TransientState->TimeFont;
    font   *PlaceFont = &TransientState->PlaceFont;
    
    local_persist GLuint FramebufferName = 0;
    
    ////////////////////////////////////////////////////////////////////////////////////
    // Initialisation
    ////////////////////////////////////////////////////////////////////////////////////
    if (!Memory->IsInitialised) {
        InitialiseArena(Arena,
                        (u8 *)Memory->TransientStorage + sizeof(transient_state),
                        Memory->TransientStorageSize - sizeof(transient_state));
        u32 HeapSize = Megabytes(512);
        InitialiseHeap(Heap, PushArray(Arena, HeapSize, u8), HeapSize);
        
        InitialiseFont(Arena, SMEEFont, STRING_FROM_LITERAL("../data/vladivostok/bold.ttf"), 120*SCREEN_WIDTH/1080.0f, 128, 4096);
        InitialiseFont(Arena, EventFont, STRING_FROM_LITERAL("../data/vladivostok/bold.ttf"), 100*SCREEN_WIDTH/1080.0f, 128, 4096);
        InitialiseFont(Arena, DateFont, STRING_FROM_LITERAL("../data/vladivostok/bold.ttf"), 80*SCREEN_WIDTH/1080.0f, 128, 4096);
#if 0
        InitialiseFont(Arena, TimeFont, STRING_FROM_LITERAL("../data/vladivostok/bold.ttf"), 80*SCREEN_WIDTH/1080.0f, 128, 4096);
        InitialiseFont(Arena, PlaceFont, STRING_FROM_LITERAL("../data/vladivostok/bold.ttf"), 80*SCREEN_WIDTH/1080.0f, 128, 4096);
#else
        *TimeFont  = *DateFont;
        *PlaceFont = *DateFont;
#endif
        
        BindOpenGLFunctions();
        
        debug_read_file_result FragFile = DEBUGPlatformReadEntireFile("../data/garden0.frag");
        string FragCode = { (char *)FragFile.Contents, FragFile.ContentSize };
        Shader = OGL_ConstructSDFShaderProgram(STRING_FROM_LITERAL(""), FragCode);
        DEBUGPlatformFreeFileMemory(FragFile.Contents);
        
        glUseProgram(Shader);
        glUseProgram(0);
        
        ResetState(State);
        
#if 0
        glGenFramebuffers(1, &FramebufferName);
        glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
        
        // The texture we're going to render to
        GLuint renderedTexture;
        glGenTextures(1, &renderedTexture);
        
        // "Bind" the newly created texture : all future texture functions will modify this texture
        glBindTexture(GL_TEXTURE_2D, renderedTexture);
        
        // Give an empty image to OpenGL ( the last "0" )
        glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, RENDER_WIDTH, RENDER_HEIGHT, 0,GL_RGBA, GL_UNSIGNED_BYTE, 0);
        
        // Poor filtering. Needed !
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        
        // Set "renderedTexture" as our colour attachement #0
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);
        
        // Set the list of draw buffers.
        GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers
#endif
        
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
    Theta += (Ard.Right-Ard.Left)*Input->TimeStepFrame;
    //Phi   += MouseSensitivity * Pi32 * MouseDY / 32768.0f;
    if      (Theta < 0.0f)        Theta += 2.0f *  Pi32;
    else if (Theta > 2.0f * Pi32) Theta -= 2.0f * Pi32;
    if      (Phi < -Pi32 / 2.0f)  Phi = -Pi32  / 2.0f;
    else if (Phi >  Pi32 / 2.0f)  Phi = Pi32 / 2.0f;
    
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
    ydir += ButtonDown(Input->Buttons[BUTTON_W]) || ButtonDown(Input->Buttons[BUTTON_Up]);
    ydir -= ButtonDown(Input->Buttons[BUTTON_S]) || ButtonDown(Input->Buttons[BUTTON_Down]);
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
    // We render it to a texture so that we 
    glUseProgram(Shader);
#if 0
    glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
    glViewport(0,0,RENDER_WIDTH,RENDER_HEIGHT); // Render on the whole framebuffer, complete from the lower left corner to the upper right
#endif
    glUniform1i(glGetUniformLocation(Shader, "iChannel0"), 0);
    glUniform1f(glGetUniformLocation(Shader, "iTime"), iTime);
    glUniform3f(glGetUniformLocation(Shader, "iResolution"), RENDER_WIDTH, RENDER_HEIGHT, 1.0f);
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
    
#if 1
    // Text
    glUseProgram(0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    string SMEEText  = STRING_FROM_LITERAL("S M E E");
    string EventText = STRING_FROM_LITERAL("G A R D E N\n \nS A L E\n+ T E A");
    string DateText  = STRING_FROM_LITERAL("2 0   M A R C H\n\
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           M O N   W E E K   4");
    string TimeText  = STRING_FROM_LITERAL("1 1 - 2 P M");
    string PlaceText = STRING_FROM_LITERAL("K E N N E T H   H U N T\n\
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          M E M O R I A L   G A R D E N");
    DrawString(SMEEFont,  SMEEText,  110.0f*SCREEN_WIDTH/1080.0f, 400.0f*SCREEN_HEIGHT/1920.0f, Colour(0xFFD79050));
    DrawString(EventFont, EventText, 110.0f*SCREEN_WIDTH/1080.0f, 450.0f*SCREEN_HEIGHT/1920.0f, CL_BLACK);
    DrawString(TimeFont,  TimeText,  970.0f*SCREEN_WIDTH/1080.0f, 1300.0f*SCREEN_HEIGHT/1920.0f, CL_BLACK, SA_Right);
    DrawString(DateFont,  DateText,  970.0f*SCREEN_WIDTH/1080.0f, 1415.0f*SCREEN_HEIGHT/1920.0f, CL_BLACK, SA_Right);
    DrawString(PlaceFont, PlaceText, 970.0f*SCREEN_WIDTH/1080.0f, 1600.0f*SCREEN_HEIGHT/1920.0f, CL_BLACK, SA_Right);
#endif
}