internal void
__TwoD() {
    
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

internal void
__ThreeD() {
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW);
    
    glDisable(GL_BLEND); // TODO(canta)
    
    glDepthRangef(0.1f, GlobalMaxDepth);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

internal void
OGL_ClearColourBuffer(colour C) {
    
    glClearColor(C.r, C.g, C.b, C.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

inline void
OGL_SetViewport(v2i Dimensions) {
    
    glViewport(0, 0, Dimensions.x, Dimensions.y);
}

inline void
OGL_SetViewportCentredInContainer(v2i Inner, v2i Outer) {
    
    glViewport((Outer.x - Inner.x) / 2, (Outer.y - Inner.y) / 2, Inner.x, Inner.y);
}

inline void
OGL_SetViewportScaledFixedAspectRatio(v2i Inner, v2i Outer) {
    
    int DrawWidth, DrawHeight;
    f32 Comp = ((f32)Outer.x / Inner.x) / ((f32)Outer.y / Inner.y);
    f32 AspectRatio = (f32)Inner.x / (f32)Inner.y;
    if (Comp > 1)      { DrawWidth  = (int)(AspectRatio * (f32)Outer.y); DrawHeight = (int)Outer.y; }
    else if (Comp < 1) { DrawWidth  = (int)Outer.x; DrawHeight = (int)((f32)Outer.x / AspectRatio); }
    else               { DrawWidth  = Outer.x; DrawHeight = Outer.y; }
    int XOffset = (Outer.x - DrawWidth)  / 2;
    int YOffset = (Outer.y - DrawHeight) / 2;
    
    glViewport(XOffset, YOffset, DrawWidth, DrawHeight);
}

internal b32
OGL_ShaderStatus(GLuint Shader, GLenum StatusCheck) {
    
    GLint Status;
    glGetShaderiv(Shader, StatusCheck, &Status);
    switch (Status) {
        case GL_TRUE: {
            // TODO(canta): Logging
            return 1;
        } break;
        
        case GL_FALSE: {
            return 0;
            // TODO(canta): Logging
        } break;
        
        case GL_INVALID_VALUE: {
            return 0;
            // TODO(canta): Logging
        } break;
        
        case GL_INVALID_OPERATION: {
            return 0;
            // TODO(canta): Logging
        } break;
        
        case GL_INVALID_ENUM: {
            return 0;
            // TODO(canta): Logging
        } break;
    };
    return 0;
}

internal b32
OGL_ProgramStatus(GLuint Program, GLenum StatusCheck) {
    
    GLint Status;
    glGetProgramiv(Program, StatusCheck, &Status);
    switch (Status) {
        case GL_TRUE: {
            // TODO(canta): Logging
            return 1;
        } break;
        
        case GL_FALSE: {
            return 0;
            // TODO(canta): Logging
        } break;
        
        case GL_INVALID_VALUE: {
            // TODO(canta): Logging
        } break;
        
        case GL_INVALID_OPERATION: {
            // TODO(canta): Logging
        } break;
        
        case GL_INVALID_ENUM: {
            // TODO(canta): Logging
        } break;
    };
    return 0;
}

internal GLuint
OGL_ConstructShaderProgram(string HeaderCode, string VertexCode, string FragmentCode) {
    
    GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLchar *VertexShaderCode[]    = { HeaderCode.Data, VertexCode.Data };
    GLint   VertexShaderLengths[] = { (GLint)HeaderCode.Length, (GLint)VertexCode.Length };
    glShaderSource(VertexShader, 2, VertexShaderCode, VertexShaderLengths);
    glCompileShader(VertexShader);
    Assert(OGL_ShaderStatus(VertexShader, GL_COMPILE_STATUS));
    
    GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    GLchar *FragmentShaderCode[]    = { HeaderCode.Data, FragmentCode.Data };
    GLint   FragmentShaderLengths[] = { (GLint)HeaderCode.Length, (GLint)FragmentCode.Length };
    glShaderSource(FragmentShader, 2, FragmentShaderCode, FragmentShaderLengths);
    glCompileShader(FragmentShader);
    Assert(OGL_ShaderStatus(FragmentShader, GL_COMPILE_STATUS));
    
    GLuint Program = glCreateProgram();
    glAttachShader(Program, VertexShader);
    glAttachShader(Program, FragmentShader);
    glBindAttribLocation(Program, 0, "i_Position");
    glLinkProgram(Program);
    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);
    Assert(OGL_ProgramStatus(Program, GL_LINK_STATUS));
    
    return Program;
}

#ifdef SEEDS_ASSETS_H
// @Memory(canta)
internal ogl_render_context
OGL_ConstructRenderContext(mesh *Mesh) {
    
    ogl_render_context Context = {};
    
    // TODO(canta): Restructure mesh so that we just have a index array?
    v3     *Vertices  = (v3 *)alloca(Mesh->NoTriangles * 3 * sizeof(v3));
    v2     *TexCoords = (v2 *)alloca(Mesh->NoTriangles * 3 * sizeof(v2));
    v3     *Colours   = (v3 *)alloca(Mesh->NoTriangles * 3 * sizeof(v3));
    GLuint *Indices   = (GLuint *)alloca(Mesh->NoTriangles * 3 * sizeof(GLuint));
    for (int I = 0; I < Mesh->NoTriangles; I++) {
        for (int J = 0; J < 3; J++) {
            int VI = Mesh->Triangles[I].Vertices[J];
            int TI = Mesh->Triangles[I].VerticesUV[J];
            Vertices[3 * I + J]  = Mesh->Vertices[VI];
            TexCoords[3 * I + J] = Mesh->UVs[TI];
            Colours[3 * I + J]   = CL_WHITE.rgb;
            Indices[3 * I + J]   = 3 * I + J;
        }
    }
    
    glGenVertexArrays(1, &Context.VAO);
    glBindVertexArray(Context.VAO);
    
    glGenBuffers(3, Context.VertexBuffers);
    
    glBindBuffer(GL_ARRAY_BUFFER, Context.VertexBuffers[0]);
    glBufferData(GL_ARRAY_BUFFER, Mesh->NoTriangles * 3 * sizeof(v3), Vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, Context.VertexBuffers[1]);
    glBufferData(GL_ARRAY_BUFFER, Mesh->NoTriangles * 2 * sizeof(v2), TexCoords, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, Context.VertexBuffers[2]);
    glBufferData(GL_ARRAY_BUFFER, Mesh->NoTriangles * 3 * sizeof(v3), Colours, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    glGenBuffers(1, &Context.IndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Context.IndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Mesh->NoTriangles * 3 * sizeof(GLuint), Indices, GL_STATIC_DRAW);
    
    Context.Textures = (GLuint *)malloc(Mesh->NoMaterials * sizeof(GLuint));
    glGenTextures(Mesh->NoMaterials, Context.Textures);
    for (int MI = 0; MI < Mesh->NoMaterials; MI++) {
        loaded_bitmap *Bitmap = Mesh->Materials[MI].Bitmap;
        if (Bitmap) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, Context.Textures[MI]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Bitmap->Width, Bitmap->Pitch / BYTES_PER_PIXEL, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, Bitmap->Memory);
        }
    }
    
    Mesh->VAO      = Context.VAO;
    Mesh->Textures = Context.Textures;
    
    return Context;
}
#endif

// @Memory(canta)
internal ogl_render_context
OGL_ConstructRenderContext(tri_mesh Mesh) {
    
    ogl_render_context Context = {};
    
    // TODO(canta): Restructure tri_mesh so that we just have a index array?
    GLuint *Indices = (GLuint *)alloca(Mesh.NoTriangles * 3 * sizeof(GLuint));
    for (int I = 0; I < Mesh.NoTriangles; I++) {
        for (int J = 0; J < 3; J++) {
            Indices[3 * I + J] = Mesh.Triangles[I].Vertices[J];
        }
    }
    
    glGenVertexArrays(1, &Context.VAO);
    glBindVertexArray(Context.VAO);
    
    glGenBuffers(1, Context.VertexBuffers);
    glBindBuffer(GL_ARRAY_BUFFER, Context.VertexBuffers[0]);
    glBufferData(GL_ARRAY_BUFFER, Mesh.NoVertices * sizeof(v3), Mesh.Vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    glGenBuffers(1, &Context.IndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Context.IndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Mesh.NoTriangles * 3 * sizeof(GLuint), Indices, GL_STATIC_DRAW);
    
    return Context;
}

// @Memory(canta)
internal ogl_render_context
OGL_ConstructDiscRenderContext(f32 Radius) {
    
    ogl_render_context Context = {};
    
    f32 Error = 1.0f;
    f32 ThetaDeg = (f32)acos(2 * ((1 - Error) / (Radius * Radius)) - 1);
    f32 ThetaRad = ThetaDeg * Pi32 / 180;
    int NoPoints = (int)(2 * Pi32 / ThetaRad);
    
    v3 *Vertices = (v3 *)alloca((NoPoints + 1) * sizeof(v3));
    Vertices[0] = { 0, 0, 0 };
    f32 ThetaCum = 0;
    for (int I = 1; I < NoPoints + 1; I++) {
        Vertices[I] = Radius * v3{ cosf(ThetaCum), sinf(ThetaCum), 0 };
        ThetaCum += ThetaRad;
    }
    GLuint *Indices = (GLuint *)alloca((NoPoints + 2) * sizeof(GLuint));
    for (int I = 0; I < NoPoints + 1; I++) Indices[I] = I;
    Indices[NoPoints + 1] = 1;
    
    glGenVertexArrays(1, &Context.VAO);
    glBindVertexArray(Context.VAO);
    
    glGenBuffers(1, Context.VertexBuffers);
    glBindBuffer(GL_ARRAY_BUFFER, Context.VertexBuffers[0]);
    glBufferData(GL_ARRAY_BUFFER, (NoPoints + 1) * sizeof(v3), Vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    glGenBuffers(1, &Context.IndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Context.IndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (NoPoints + 2) * sizeof(GLuint), Indices, GL_STATIC_DRAW);
    
    return Context;
}

// @Cleanup(canta)
internal mat4x4
ComputeProjectionMatrix(f32 DistanceToScreen, f32 AspectRatio, f32 ZNear, f32 ZFar) {
    
    f32 ZAdd  = ZFar + ZNear;
    f32 ZSub  = ZFar - ZNear;
    return mat4x4{
        DistanceToScreen / AspectRatio, 0, 0, 0,
        0, -ZAdd / ZSub, 0, -2 * ZFar * ZNear / ZSub,
        0, 0, DistanceToScreen, 0,
        0, -1, 0, 0
    };
}

internal mat4x4
ComputeViewMatrix(entity C) {
    
    return mat4x4{
        C.UnitX.x, C.UnitX.y, C.UnitX.z, -Dot(C.UnitX, C.Origin),
        C.UnitY.x, C.UnitY.y, C.UnitY.z, -Dot(C.UnitY, C.Origin),
        C.UnitZ.x, C.UnitZ.y, C.UnitZ.z, -Dot(C.UnitZ, C.Origin),
        0, 0, 0, 1
    };
}

internal mat4x4
ComputeModelMatrix(entity E) {
    return mat4x4{
        E.Scale.x * E.UnitX.x, E.Scale.y * E.UnitY.x, E.Scale.z * E.UnitZ.x, E.Origin.x,
        E.Scale.x * E.UnitX.y, E.Scale.y * E.UnitY.y, E.Scale.z * E.UnitZ.y, E.Origin.y,
        E.Scale.x * E.UnitX.z, E.Scale.y * E.UnitY.z, E.Scale.z * E.UnitZ.z, E.Origin.z,
        0, 0, 0, 1
    };
}

internal void
OGL_RenderSeedsMesh(GLuint Program, GLuint VAO, tri_mesh Mesh, entity Entity, entity Camera, mat4x4 View, mat4x4 Projection, colour CL, colour CH, f32 NormDist) {
    
    __ThreeD();
    glUseProgram(Program);
    glBindVertexArray(VAO);
    
    mat4x4 Model = ComputeModelMatrix(Entity);
    mat4x4 ProjViewModel = Projection * View * Model;
    glUniformMatrix4fv(glGetUniformLocation(Program, "ProjViewModel"),  1, 0, (GLfloat *)&ProjViewModel);
    
    glUniform4f(glGetUniformLocation(Program, "ColourLow"),   CL.r, CL.g, CL.b, CL.a);
    glUniform4f(glGetUniformLocation(Program, "ColourHigh"),  CH.r, CH.g, CH.b, CH.a);
    glUniform1f(glGetUniformLocation(Program, "NormDist"),    NormDist);
    glUniform1f(glGetUniformLocation(Program, "EntityDepth"), Length(Entity.Origin - Camera.Origin));
    glUniform1f(glGetUniformLocation(Program, "ColourMethod"), 1);
    
    glDrawElements(GL_TRIANGLES, 3 * Mesh.NoTriangles, GL_UNSIGNED_INT, 0);
    
    glUseProgram(0);
}

#ifdef SEEDS_ASSETS_H
#ifdef SEEDS_ENTITY_H
internal void
OGL_RenderEntities(GLuint Program, entity_manager *EntityManager, mesh_library *MeshLibrary, int CameraIndex, f32 CameraFOV) {
    
    __ThreeD();
    glUseProgram(Program);
    
    entity Camera = EntityManager->Entities[CameraIndex];
    mat4x4 View       = ComputeViewMatrix(Camera);
    
    f32 DistanceToScreen = 5.0f / tanf(CameraFOV * 0.5f);
    f32 Aspect = 2; // @Cleanup(isuru): Hardcoded
    f32 ZFar  = GlobalMaxDepth;
    f32 ZNear = 0.1f;
    mat4x4 Projection = ComputeProjectionMatrix(DistanceToScreen, Aspect, ZNear, ZFar);
    
    for (int EI = 0; EI < ArrayCount(EntityManager->Entities); EI++) {
        if (EI == CameraIndex)          continue;
        if (!EntityManager->Exists[EI]) continue;
        
        entity Entity = EntityManager->Entities[EI];
        
        entity_colour_info ColourInfo = EntityManager->ColourInfo[EI];
        colour CL = ColourInfo.Low;
        colour CH = ColourInfo.High;
        
        mat4x4 Model         = ComputeModelMatrix(Entity);
        mat4x4 ProjViewModel = Projection * View * Model;
        glUniformMatrix4fv(glGetUniformLocation(Program, "ProjViewModel"),  1, 0, (GLfloat *)&ProjViewModel);
        
        glUniform4f(glGetUniformLocation(Program, "ColourLow"),   CL.r, CL.g, CL.b, CL.a);
        glUniform4f(glGetUniformLocation(Program, "ColourHigh"),  CH.r, CH.g, CH.b, CH.a);
        glUniform1f(glGetUniformLocation(Program, "NormDist"),    -ColourInfo.NormDist);
        glUniform1f(glGetUniformLocation(Program, "EntityDepth"), Length(Entity.Origin - Camera.Origin));
        glUniform1f(glGetUniformLocation(Program, "ColourMethod"), 1);
        
        mesh Mesh = MeshLibrary->Meshes[EntityManager->MeshIndices[EI]];
        glBindVertexArray(Mesh.VAO);
        glDrawElements(GL_TRIANGLES, 3 * Mesh.NoTriangles, GL_UNSIGNED_INT, 0);
    }
    
    glUseProgram(0);
}
#endif
#endif

