#ifndef SEEDS_SIMD_H

#include <intrin.h>

struct v3_wide {
    
    __m128 x;
    __m128 y;
    __m128 z;
};

struct v4_wide {
    
    __m128 x;
    __m128 y;
    __m128 z;
    __m128 w;
};

struct mat3x3_wide {
    
    union {
        v3_wide Rows[3];
        
        __m128  Elements[9];
        
        struct {
            __m128 A00;
            __m128 A10;
            __m128 A20;
            __m128 A01;
            __m128 A11;
            __m128 A21;
            __m128 A02;
            __m128 A12;
            __m128 A22;
        };
    };
};

struct mat4x4_wide {
    
    union {
        v4_wide  Rows[4];
        
        __m128 Elements[16];
        
        struct {
            __m128 A00;
            __m128 A10;
            __m128 A20;
            __m128 A30;
            
            __m128 A01;
            __m128 A11;
            __m128 A21;
            __m128 A31;
            
            __m128 A02;
            __m128 A12;
            __m128 A22;
            __m128 A32;
            
            __m128 A03;
            __m128 A13;
            __m128 A23;
            __m128 A33;
        };
    };
};

//// NOTE: s32
inline __m128i
operator+(__m128i A, __m128i B) {
    
    return _mm_add_epi32(A, B);
}

inline void
operator+=(__m128i &A, __m128i B) {
    
    A = A + B;
}

inline __m128i
operator-(__m128i A, __m128i B) {
    
    return _mm_sub_epi32(A, B);
}

inline void
operator-=(__m128i &A, __m128i B) {
    
    A = A - B;
}

inline __m128i
operator*(__m128i A, __m128i B) {
    
    return _mm_mul_epi32(A, B);
}

inline __m128i
operator<(__m128i A, __m128i B) {
    
    return _mm_cmplt_epi32(A, B);
}

inline __m128i
operator>(__m128i A, __m128i B) {
    
    return _mm_cmpgt_epi32(A, B);
}

inline __m128i
operator<=(__m128i A, __m128i B) {
    
    __m128i Ander = _mm_set1_epi32(0xFFFFFFFF);
    return _mm_andnot_si128(_mm_cmpgt_epi32(A, B), Ander);
}

inline __m128i
operator>=(__m128i A, __m128i B) {
    
    __m128i Ander = _mm_set1_epi32(0xFFFFFFFF);
    return _mm_andnot_si128(_mm_cmplt_epi32(A, B), Ander);
}

inline __m128i
operator&&(__m128i A, __m128i B) {
    
    return _mm_and_si128(A, B);
}

//// NOTE: f32
inline __m128
operator+(__m128 A, __m128 B) {
    
    return _mm_add_ps(A, B);
}

inline void
operator+=(__m128 &A, __m128 B) {
    
    A = A + B;
}

inline __m128
operator-(__m128 A, __m128 B) {
    
    return _mm_sub_ps(A, B);
}

inline void
operator-=(__m128 &A, __m128 B) {
    
    A = A - B;
}

inline __m128
operator-(__m128 Value) {
    
    return _mm_setzero_ps() - Value;
}

inline __m128
operator*(__m128 A, __m128 B) {
    
    return _mm_mul_ps(A, B);
}

inline __m128
operator/(__m128 A, __m128 B) {
    
    return _mm_div_ps(A, B);
}

inline __m128
operator==(__m128 A, __m128 B) {
    
    return _mm_cmpeq_ps(A, B);
}

inline __m128
operator<(__m128 A, __m128 B) {
    
    return _mm_cmplt_ps(A, B);
}

inline __m128
operator>(__m128 A, __m128 B) {
    
    return _mm_cmpgt_ps(A, B);
}

inline __m128
operator<=(__m128 A, __m128 B) {
    
    return _mm_cmple_ps(A, B);
}

inline __m128
operator>=(__m128 A, __m128 B) {
    
    return _mm_cmpge_ps(A, B);
}

inline __m128
operator&(__m128 A, __m128 B) {
    
    return _mm_and_ps(A, B);
}

#define SEEDS_SIMD_H
#endif
