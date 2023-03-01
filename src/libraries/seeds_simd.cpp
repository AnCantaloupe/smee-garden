inline v3_wide
operator+(v3_wide A, v3_wide B) {
    
    return v3_wide{
        A.x + B.x,
        A.y + B.y,
        A.z + B.z
    };
}

inline v3_wide
operator+(v3_wide Vector, __m128 Scalar) {
    
    return v3_wide{
        Vector.x + Scalar,
        Vector.y + Scalar,
        Vector.z + Scalar
    };
}

inline v3_wide
operator+(__m128 Scalar, v3_wide Vector) {
    
    return (Vector + Scalar);
}

inline v3_wide
operator-(v3_wide A, v3_wide B) {
    
    return v3_wide{
        A.x - B.x,
        A.y - B.y,
        A.z - B.z
    };
}

inline v3_wide
operator-(v3_wide Vector, __m128 Scalar) {
    
    return v3_wide{
        Vector.x - Scalar,
        Vector.y - Scalar,
        Vector.z - Scalar
    };
}

inline v3_wide
operator-(__m128 Scalar, v3_wide Vector) {
    
    return v3_wide{
        Scalar - Vector.x,
        Scalar - Vector.y,
        Scalar - Vector.z
    };
}

inline v3_wide
operator-(v3_wide Vector) {
    
    return v3_wide{
        -Vector.x,
        -Vector.y,
        -Vector.z
    };
}

inline v3_wide
operator*(v3_wide Vector, __m128 Scalar) {
    
    return v3_wide{
        Vector.x * Scalar,
        Vector.y * Scalar,
        Vector.z * Scalar
    };
}

inline v3_wide
operator*(__m128 Scalar, v3_wide Vector) {
    
    return (Vector * Scalar);
}

inline v3_wide
operator/(v3_wide Vector, __m128 Scalar) {
    
    return v3_wide{
        Vector.x / Scalar,
        Vector.y / Scalar,
        Vector.z / Scalar
            
    };
}

inline v3_wide
operator/(__m128 Scalar, v3_wide Vector) {
    
    return v3_wide{
        Scalar / Vector.x,
        Scalar / Vector.y,
        Scalar / Vector.z
            
    };
}

inline __m128
Dot(v3_wide A, v3_wide B) {
    
    return (A.x * B.x + A.y * B.y + A.z * B.z);
}

inline v3_wide
Hadamard(v3_wide Vector1, v3_wide Vector2) {
    
    return v3_wide{
        Vector1.x * Vector2.x,
        Vector1.y * Vector2.y,
        Vector1.z * Vector2.z
    };
}

inline v3_wide
Cross(v3_wide A, v3_wide B) {
    
    return v3_wide{
        ((A.y * B.z) - (A.z * B.y)),
        -((A.x * B.z) - (A.z * B.x)),
        ((A.x * B.y) - (A.y * B.x))
    };
}

inline __m128
LengthSq(v3_wide Vector) {
    
    return (__m128)(Vector.x * Vector.x
                    + Vector.y * Vector.y
                    + Vector.z * Vector.z);
}

inline __m128
Length(v3_wide Vector) {
    
    return _mm_sqrt_ps(LengthSq(Vector));
}

inline v4_wide
operator/(v4_wide V, __m128 S) {
    
    return v4_wide{ V.x / S, V.y / S, V.z / S, V.w / S };
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline mat3x3_wide
Identity3Wide(void) {
    
    __m128 Zero = _mm_setzero_ps();
    __m128 One  = _mm_set1_ps(1.0f);
    
    return mat3x3_wide{
        One,  Zero, Zero,
        Zero, One,  Zero,
        Zero, Zero, One
    };
}

inline mat3x3_wide
Matrix3x3(__m128 A00, __m128 A10, __m128 A20, __m128 A01, __m128 A11, __m128 A21, __m128 A02, __m128 A12, __m128 A22) {
    
    return mat3x3_wide{
        A00, A10, A20,
        A01, A11, A21,
        A02, A12, A22
    };
}

inline mat3x3_wide
Matrix3x3Rows(v3_wide Row0, v3_wide Row1, v3_wide Row2) {
    
    return mat3x3_wide{
        Row0,
        Row1,
        Row2
    };
}

inline mat3x3_wide
Matrix3x3Columns(v3_wide Col0, v3_wide Col1, v3_wide Col2) {
    
    return mat3x3_wide{
        Col0.x, Col1.x, Col2.x,
        Col0.y, Col1.y, Col2.y,
        Col0.z, Col1.z, Col2.z
    };
}

inline v3_wide
Row(mat3x3_wide Mat, int RowIndex) {
    
    return Mat.Rows[RowIndex];
}

inline v3_wide
Column(mat3x3_wide Mat, int ColumnIndex) {
    
    return v3_wide{
        Mat.Elements[0 + ColumnIndex],
        Mat.Elements[3 + ColumnIndex],
        Mat.Elements[6 + ColumnIndex]
    };
}

inline mat3x3_wide
operator*(mat3x3_wide Mat, __m128 Scalar) {
    
    return mat3x3_wide{
        Mat.A00 * Scalar, Mat.A10 * Scalar, Mat.A20 * Scalar,
        Mat.A01 * Scalar, Mat.A11 * Scalar, Mat.A21 * Scalar,
        Mat.A02 * Scalar, Mat.A12 * Scalar, Mat.A22 * Scalar
    };
}

inline mat3x3_wide
operator*(__m128 Scalar, mat3x3_wide Mat) {
    
    return Mat * Scalar;
}

inline mat3x3_wide
operator/(mat3x3_wide Mat, __m128 Scalar) {
    
    return mat3x3_wide{
        Mat.A00 / Scalar, Mat.A10 / Scalar, Mat.A20 / Scalar,
        Mat.A01 / Scalar, Mat.A11 / Scalar, Mat.A21 / Scalar,
        Mat.A02 / Scalar, Mat.A12 / Scalar, Mat.A22 / Scalar
    };
}

inline mat3x3_wide
operator/(__m128 Scalar, mat3x3_wide Mat) {
    
    return mat3x3_wide{
        Scalar / Mat.A00, Scalar / Mat.A10, Scalar / Mat.A20,
        Scalar / Mat.A01, Scalar / Mat.A11, Scalar / Mat.A21,
        Scalar / Mat.A02, Scalar / Mat.A12, Scalar / Mat.A22
    };
}

inline mat3x3_wide
operator*(mat3x3_wide A, mat3x3_wide B) {
    
    return mat3x3_wide{
        A.A00 * B.A00 + A.A10 * B.A01 + A.A20 * B.A02,
        A.A00 * B.A10 + A.A10 * B.A11 + A.A20 * B.A12,
        A.A00 * B.A20 + A.A10 * B.A21 + A.A20 * B.A22,
        
        A.A01 * B.A00 + A.A11 * B.A01 + A.A21 * B.A02,
        A.A01 * B.A10 + A.A11 * B.A11 + A.A21 * B.A12,
        A.A01 * B.A20 + A.A11 * B.A21 + A.A21 * B.A22,
        
        A.A02 * B.A00 + A.A12 * B.A01 + A.A22 * B.A02,
        A.A02 * B.A10 + A.A12 * B.A11 + A.A22 * B.A12,
        A.A02 * B.A20 + A.A12 * B.A21 + A.A22 * B.A22,
    };
}

inline v3_wide
operator*(mat3x3_wide Mat, v3_wide Vec) {
    
    return v3_wide{
        Mat.A00 * Vec.x + Mat.A10 * Vec.y + Mat.A20 * Vec.z,
        Mat.A01 * Vec.x + Mat.A11 * Vec.y + Mat.A21 * Vec.z,
        Mat.A02 * Vec.x + Mat.A12 * Vec.y + Mat.A22 * Vec.z
    };
}

inline v3_wide
operator*(v3_wide Vec, mat3x3_wide Mat) {
    
    return v3_wide{
        Vec.x * Mat.A00 + Vec.y * Mat.A01 + Vec.z * Mat.A02,
        Vec.x * Mat.A10 + Vec.y * Mat.A11 + Vec.z * Mat.A12,
        Vec.x * Mat.A20 + Vec.y * Mat.A21 + Vec.z * Mat.A22
    };
}

inline __m128
Determinant(mat3x3_wide Mat) {
    
    return (  Mat.A00 * (Mat.A11 * Mat.A22 - Mat.A21 * Mat.A12)
            - Mat.A10 * (Mat.A01 * Mat.A22 - Mat.A21 * Mat.A02)
            + Mat.A20 * (Mat.A01 * Mat.A12 - Mat.A11 * Mat.A02)  );
}

inline mat4x4_wide
operator*(mat4x4_wide A, mat4x4_wide B) {
    
    return mat4x4_wide{
        A.A00 * B.A00 + A.A10 * B.A01 + A.A20 * B.A02 + A.A30 * B.A03,
        A.A00 * B.A10 + A.A10 * B.A11 + A.A20 * B.A12 + A.A30 * B.A13,
        A.A00 * B.A20 + A.A10 * B.A21 + A.A20 * B.A22 + A.A30 * B.A23,
        A.A00 * B.A30 + A.A10 * B.A31 + A.A20 * B.A32 + A.A30 * B.A33,
        
        A.A01 * B.A00 + A.A11 * B.A01 + A.A21 * B.A02 + A.A31 * B.A03,
        A.A01 * B.A10 + A.A11 * B.A11 + A.A21 * B.A12 + A.A31 * B.A13,
        A.A01 * B.A20 + A.A11 * B.A21 + A.A21 * B.A22 + A.A31 * B.A23,
        A.A01 * B.A30 + A.A11 * B.A31 + A.A21 * B.A32 + A.A31 * B.A33,
        
        A.A02 * B.A00 + A.A12 * B.A01 + A.A22 * B.A02 + A.A32 * B.A03,
        A.A02 * B.A10 + A.A12 * B.A11 + A.A22 * B.A12 + A.A32 * B.A13,
        A.A02 * B.A20 + A.A12 * B.A21 + A.A22 * B.A22 + A.A32 * B.A23,
        A.A02 * B.A30 + A.A12 * B.A31 + A.A22 * B.A32 + A.A32 * B.A33,
        
        A.A03 * B.A00 + A.A13 * B.A01 + A.A23 * B.A02 + A.A33 * B.A03,
        A.A03 * B.A10 + A.A13 * B.A11 + A.A23 * B.A12 + A.A33 * B.A13,
        A.A03 * B.A20 + A.A13 * B.A21 + A.A23 * B.A22 + A.A33 * B.A23,
        A.A03 * B.A30 + A.A13 * B.A31 + A.A23 * B.A32 + A.A33 * B.A33,
    };
}

inline v4_wide
operator*(mat4x4_wide M, v4_wide V) {
    
    return v4_wide{
        M.A00 * V.x + M.A10 * V.y + M.A20 * V.z + M.A30 * V.w,
        M.A01 * V.x + M.A11 * V.y + M.A21 * V.z + M.A31 * V.w,
        M.A02 * V.x + M.A12 * V.y + M.A22 * V.z + M.A32 * V.w,
        M.A03 * V.x + M.A13 * V.y + M.A23 * V.z + M.A33 * V.w,
    };
}

