#ifndef SEEDS_RENDER_OPENGL_H

#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_STATIC_DRAW                    0x88E4
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_INFO_LOG_LENGTH                0x8B84


#define GL_FRAMEBUFFER                    0x8D40
#define GL_COLOR_ATTACHMENT0              0x8CE0


typedef char     GLchar;
typedef intptr_t GLintptr;
typedef size_t   GLsizeiptr; // TODO(canta): Is this correct??

typedef GLuint gl_create_shader(GLenum type);
typedef void   gl_shader_source(GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
typedef void   gl_compile_shader(GLuint shader);
typedef void   gl_attach_shader(GLuint program, GLuint shader);
typedef void   gl_get_shader_iv(GLuint shader, GLenum pname, GLint *params);
typedef void   gl_get_shader_info_log(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void   gl_delete_shader(GLuint shader);
typedef GLuint gl_create_program(void);
typedef void   gl_link_program(GLuint program);
typedef void   gl_use_program(GLuint program);
typedef void   gl_get_program_iv(GLuint program, GLenum pname, GLint *params);
typedef void   gl_delete_program (GLuint program);
typedef void   gl_get_program_info_log(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);

typedef void   gl_active_texture(GLenum texture);
typedef void   gl_gen_samplers(GLsizei n, GLuint *samplers);
typedef void   gl_bind_sampler(GLuint unit, GLuint sampler);

typedef void gl_bind_vertex_array(GLuint array);
typedef void gl_gen_vertex_arrays(GLsizei n, GLuint *arrays);
typedef void gl_bind_buffer(GLenum target, GLuint buffer);
typedef void gl_gen_buffers(GLsizei n, GLuint *buffers);
typedef void gl_buffer_data(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void gl_buffer_sub_data(GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
typedef void gl_get_buffer_sub_data(GLenum target, GLintptr offset, GLsizeiptr size, void *data);
typedef void gl_enable_vertex_attrib_array(GLuint index);
typedef void gl_bind_attrib_location(GLuint program, GLuint index, const GLchar *name);
typedef void gl_vertex_attrib_pointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);

typedef GLint gl_get_uniform_location(GLuint program, const GLchar *name);
typedef void  gl_uniform_1ui(GLint location, GLuint V0);
typedef void  gl_uniform_1i(GLint location, GLint V0);
typedef void  gl_uniform_1f(GLint location, GLfloat V0);
typedef void  gl_uniform_3f(GLint location, GLfloat V0, GLfloat V1, GLfloat V2);
typedef void  gl_uniform_4f(GLint location, GLfloat V0, GLfloat V1, GLfloat V2, GLfloat V3);
typedef void  gl_uniform_3fv(GLint location, GLfloat V0, GLfloat V1, GLfloat V2);
typedef void  gl_uniform_matrix_4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

typedef void gl_depth_range_f(GLclampf zNear, GLclampf zFar);

typedef void gl_gen_framebuffers(GLsizei n, GLuint *ids);
typedef void gl_bind_framebuffer(GLenum target, GLuint framebuffer);
typedef void gl_framebuffer_texture(GLenum target, GLenum attachment, GLuint texture, GLint level);
typedef void gl_draw_buffers(GLsizei n, const GLenum * bufs);



global gl_create_shader  *glCreateShader;
global gl_shader_source  *glShaderSource;
global gl_compile_shader *glCompileShader;
global gl_get_shader_iv  *glGetShaderiv;
global gl_attach_shader  *glAttachShader;
global gl_delete_shader  *glDeleteShader;
global gl_create_program *glCreateProgram;
global gl_link_program   *glLinkProgram;
global gl_get_program_iv *glGetProgramiv;
global gl_use_program    *glUseProgram;
global gl_delete_program    *glDeleteProgram;
global gl_get_shader_info_log *glGetShaderInfoLog;
global gl_get_program_info_log *glGetProgramInfoLog;

global gl_active_texture              *glActiveTexture;
global gl_gen_samplers               *glGenSamplers;
global gl_bind_sampler               *glBindSampler;

global gl_bind_vertex_array          *glBindVertexArray;
global gl_gen_vertex_arrays          *glGenVertexArrays;
global gl_bind_buffer                *glBindBuffer;
global gl_gen_buffers                *glGenBuffers;
global gl_buffer_data                *glBufferData;
global gl_buffer_sub_data            *glBufferSubData;
global gl_get_buffer_sub_data        *glGetBufferSubData;
global gl_enable_vertex_attrib_array *glEnableVertexAttribArray;
global gl_bind_attrib_location       *glBindAttribLocation;
global gl_vertex_attrib_pointer      *glVertexAttribPointer;

global gl_get_uniform_location *glGetUniformLocation;
global gl_uniform_1ui          *glUniform1ui;
global gl_uniform_1i           *glUniform1i;
global gl_uniform_1f           *glUniform1f;
global gl_uniform_3f           *glUniform3f;
global gl_uniform_4f           *glUniform4f;
global gl_uniform_3fv          *glUniform3fv;
global gl_uniform_matrix_4fv   *glUniformMatrix4fv;

global gl_depth_range_f *glDepthRangef;

global gl_gen_framebuffers *glGenFramebuffers;
global gl_bind_framebuffer *glBindFramebuffer;
global gl_framebuffer_texture *glFramebufferTexture;
global gl_draw_buffers *glDrawBuffers;



internal void
BindOpenGLFunctions() {
    
    glCreateShader  = (gl_create_shader  *)wglGetProcAddress("glCreateShader");
    glShaderSource  = (gl_shader_source  *)wglGetProcAddress("glShaderSource");
    glCompileShader = (gl_compile_shader *)wglGetProcAddress("glCompileShader");
    glGetShaderiv   = (gl_get_shader_iv  *)wglGetProcAddress("glGetShaderiv");
    glAttachShader  = (gl_attach_shader  *)wglGetProcAddress("glAttachShader");
    glDeleteShader  = (gl_delete_shader  *)wglGetProcAddress("glDeleteShader");
    glCreateProgram = (gl_create_program *)wglGetProcAddress("glCreateProgram");
    glLinkProgram   = (gl_link_program   *)wglGetProcAddress("glLinkProgram");
    glGetProgramiv  = (gl_get_program_iv *)wglGetProcAddress("glGetProgramiv");
    glUseProgram    = (gl_use_program    *)wglGetProcAddress("glUseProgram");
    glDeleteProgram = (gl_delete_program    *)wglGetProcAddress("glDeleteProgram");
    glGetShaderInfoLog = (gl_get_program_info_log *)wglGetProcAddress("glGetShaderInfoLog");
    glGetProgramInfoLog = (gl_get_program_info_log *)wglGetProcAddress("glGetProgramInfoLog");
    
    glActiveTexture = (gl_active_texture      *)wglGetProcAddress("glActiveTexture");
    glGenSamplers = (gl_gen_samplers          *)wglGetProcAddress("glGenSamplers");
    glBindSampler = (gl_bind_sampler          *)wglGetProcAddress("glBindSampler");
    
    glBindVertexArray         = (gl_bind_vertex_array          *)wglGetProcAddress("glBindVertexArray");
    glGenVertexArrays         = (gl_gen_vertex_arrays          *)wglGetProcAddress("glGenVertexArrays");
    glBindBuffer              = (gl_bind_buffer                *)wglGetProcAddress("glBindBuffer");
    glGenBuffers              = (gl_gen_buffers                *)wglGetProcAddress("glGenBuffers");
    glBufferData              = (gl_buffer_data                *)wglGetProcAddress("glBufferData");
    glBufferSubData           = (gl_buffer_sub_data            *)wglGetProcAddress("glBufferSubData");
    glGetBufferSubData        = (gl_get_buffer_sub_data        *)wglGetProcAddress("glGetBufferSubData");
    glEnableVertexAttribArray = (gl_enable_vertex_attrib_array *)wglGetProcAddress("glEnableVertexAttribArray");
    glBindAttribLocation      = (gl_bind_attrib_location       *)wglGetProcAddress("glBindAttribLocation");
    glVertexAttribPointer     = (gl_vertex_attrib_pointer      *)wglGetProcAddress("glVertexAttribPointer");
    
    glGetUniformLocation = (gl_get_uniform_location *)wglGetProcAddress("glGetUniformLocation");
    glUniform1ui         = (gl_uniform_1ui          *)wglGetProcAddress("glUniform1ui");
    glUniform1i          = (gl_uniform_1i           *)wglGetProcAddress("glUniform1i");
    glUniform1f          = (gl_uniform_1f           *)wglGetProcAddress("glUniform1f");
    glUniform3f          = (gl_uniform_3f           *)wglGetProcAddress("glUniform3f");
    glUniform4f          = (gl_uniform_4f           *)wglGetProcAddress("glUniform4f");
    glUniform3fv         = (gl_uniform_3fv          *)wglGetProcAddress("glUniform3fv");
    glUniformMatrix4fv   = (gl_uniform_matrix_4fv   *)wglGetProcAddress("glUniformMatrix4fv");
    
    glDepthRangef = (gl_depth_range_f *)wglGetProcAddress("glDepthRangef");
    
    glGenFramebuffers = ( gl_gen_framebuffers *)wglGetProcAddress("glGenFramebuffers");
    glBindFramebuffer = ( gl_bind_framebuffer *)wglGetProcAddress("glBindFramebuffer");
    glFramebufferTexture = (gl_framebuffer_texture *)wglGetProcAddress("glFramebufferTexture");
    glDrawBuffers = (gl_draw_buffers *)wglGetProcAddress("glDrawBuffers");
}

#define SEEDS_RENDER_OPENGL_H
#endif
