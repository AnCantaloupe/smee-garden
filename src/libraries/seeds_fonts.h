#ifndef SEEDS_FONTS_H

enum text_align {
    SA_Left,
    SA_Right,
    SA_Centre,
    
    SA_Count
};

struct font {
    
    stbtt_pack_range  PackRange;
    
#if SEEDS_OPENGL
    GLfloat Width;
    GLfloat Height;
    GLuint  Baked;
#else
    loaded_bitmap     Baked;
#endif
};


#define SEEDS_FONTS_H
#endif
