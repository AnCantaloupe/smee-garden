#ifndef SEEDS_MATHS_H

typedef struct v2i {
    
    int x;
    int y;
} v2b;

typedef struct v3i {
    
    int x;
    int y;
    int z;
} v3b;

struct v2 {
    
    f32 x;
    f32 y;
};

struct v3 {
    
    union {
        struct {
            f32 x;
            f32 y;
            f32 z;
        };
        struct {
            v2  xy;
            f32 z;
        };
    };
};

struct v4 {
    
    union {
        union {
            struct {
                v3  xyz;
                f32 w;
            };
            struct {
                f32 x;
                f32 y;
                f32 z;
                f32 w;
            };
        };
        union {
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
    };
};

struct mat2x2 {
    
    union {
        v2  Rows[2];
        
        f32 Elements[4];
        
        struct {
            f32 A00;
            f32 A10;
            f32 A01;
            f32 A11;
        };
    };
};

struct mat3x3 {
    
    union {
        v3  Rows[3];
        
        f32 Elements[9];
        
        struct {
            f32 A00;
            f32 A10;
            f32 A20;
            f32 A01;
            f32 A11;
            f32 A21;
            f32 A02;
            f32 A12;
            f32 A22;
        };
        
        struct {
            f32 a;
            f32 b;
            f32 c;
            f32 d;
            f32 e;
            f32 f;
            f32 g;
            f32 h;
            f32 i;
        };
    };
};

struct mat4x4 {
    
    union {
        v4  Rows[4];
        
        f32 Elements[16];
        
        struct {
            f32 A00;
            f32 A10;
            f32 A20;
            f32 A30;
            
            f32 A01;
            f32 A11;
            f32 A21;
            f32 A31;
            
            f32 A02;
            f32 A12;
            f32 A22;
            f32 A32;
            
            f32 A03;
            f32 A13;
            f32 A23;
            f32 A33;
        };
    };
};

struct biv3 {
    
    f32 xy;
    f32 xz;
    f32 yz;
};

struct rot3 {
    
    f32 a;
    
    union {
        biv3 b;
        
        struct {
            f32 bxy;
            f32 bxz;
            f32 byz;
        };
    };
};

#define SEEDS_MATHS_H
#endif
