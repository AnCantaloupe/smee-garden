// TODO(canta): Format this up all nice like

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <math.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// NOTE: General purpose

#define Sq(A) ((A) * (A))

inline f32
Sign(f32 Value) {
    
    return Value >= 0.0f ? 1.0f : -1.0f;
}

inline f32
Abs(f32 Value) {
    
    return (Value > 0) * Value + (Value <= 0) * (-Value);
}

inline int
Clamp(int Min, int Value, int Max) {
    
    return (Value < Min ? Min : (Value > Max ? Max : Value));
}

inline f32
Clamp(f32 Min, f32 Value, f32 Max) {
    
    return (Value < Min ? Min : (Value > Max ? Max : Value));
}

inline f32
DegToRad(f32 Angle) {
    
    return (Angle / 180.0f * Pi32);
}

inline f32
RadToDeg(f32 Angle) {
    
    return (Angle / Pi32 * 180.0f);
}

inline f32
Lerp(f32 Initial, f32 T, f32 Final) {
    
    return (1.0f - T) * Initial + T * Final;
}

inline v2
Lerp(v2 Initial, f32 T, v2 Final) {
    
    return v2{
        Lerp(Initial.x, T, Final.x),
        Lerp(Initial.y, T, Final.y)
    };
}

inline v3
Lerp(v3 Initial, f32 T, v3 Final) {
    
    return v3{
        Lerp(Initial.x, T, Final.x),
        Lerp(Initial.y, T, Final.y),
        Lerp(Initial.z, T, Final.z)
    };
}

inline v4
Lerp(v4 Initial, f32 T, v4 Final) {
    
    return v4{
        Lerp(Initial.x, T, Final.x),
        Lerp(Initial.y, T, Final.y),
        Lerp(Initial.z, T, Final.z),
        Lerp(Initial.w, T, Final.w)
    };
}

inline int
PosCeil(f32 Value) {
    
#if 1
    return ((int)Value + 1 - (Value == (int)Value));
#else
    return (int)ceilf(Value);
#endif
}

inline f32
SafeRatioN(f32 Num, f32  Denom, f32 Default)
{
    f32 Result = Default;
    
    if(Denom != 0.0f)
    {
        Result = Num / Denom;
    }
    
    return Result;
}

inline f32
SafeRatio0(f32 Num, f32 Denom)
{
    f32 Result = SafeRatioN(Num, Denom, 0.0f);
    
    return Result;
}

inline f32
SafeRatio1(f32 Num, f32 Denom)
{
    f32 Result = SafeRatioN(Num, Denom, 1.0f);
    
    return Result;
}

// NOTE(canta): From Niklaus Wirth's 'Data Structures and Algorithms'
internal f32
Power(f32 Base, s64 Exponent) {
    
    f32 Result = 1.0f;
    
    if (Exponent < 0) Base = 1 / Base;
    
    while (Exponent) {
        if (Exponent % 2) Result = Result * Base;
        Base = Base * Base;
        Exponent/= 2;
    }
    
    return Result;
}

// NOTE(canta): From Chris Hecker's GDM column
inline void
FloorDivMod(int Numerator, int Denominator, int *Floor, int *Mod) {
    
    // We assume the denominator is positive as the rasteriser will step from top to bottom.
    Assert(Denominator > 0);
    
    if (Numerator >= 0) {
        *Floor = Numerator / Denominator;
        *Mod   = Numerator % Denominator;
    }
    else {
        // We want floor to be the next lowest value, not the next lowest absolute value, hence this.
        *Floor = -((-Numerator) / Denominator);
        *Mod = (-Numerator) % Denominator;
        if (*Mod) {
            *Floor--;
            *Mod = Denominator - *Mod;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// NOTE: Linear algebra

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//// NOTE: 2D vector operations

// NOTE: Int
inline v2i
operator+(v2i A, v2i B) {
    
    return v2i{
        A.x + B.x,
        A.y + B.y
    };
}

inline v2i
operator-(v2i Vector) {
    
    return v2i{
        -Vector.x,
        -Vector.y
    };
}

// NOTE: Float
inline v2
V2(s32 X, s32 Y) {
    
    return v2{ (f32)X, (f32)X };
}

inline b32
operator==(v2 A, v2 B) {
    
    return (A.x == B.x && A.y == B.y);
}


inline b32
operator!=(v2 A, v2 B) {
    
    return (A.x != B.x || A.y != B.y);
}


inline v2
operator+(v2 A, v2 B) {
    
    return v2{
        A.x + B.x,
        A.y + B.y
    };
}

inline void
operator+=(v2 &A, v2 B) {
    
    A.x += B.x;
    A.y += B.y;
}

inline v2
operator-(v2 A, v2 B) {
    
    return v2{
        A.x - B.x,
        A.y - B.y
    };
}

inline void
operator-=(v2 &A, v2 B) {
    
    A.x -= B.x;
    A.y -= B.y;
}

inline v2
operator-(v2 Vector) {
    
    return v2{
        -Vector.x,
        -Vector.y
    };
}

inline v2
operator*(v2 Vector, f32 Scalar) {
    
    return v2{
        Vector.x * Scalar,
        Vector.y * Scalar
    };
}

inline v2
operator*(f32 Scalar, v2 Vector) {
    
    return (Vector * Scalar);
}

inline v2
operator/(v2 Vector, f32 Scalar) {
    
    return v2{
        Vector.x / Scalar,
        Vector.y / Scalar
    };
}

inline v2
operator/(f32 Scalar, v2 Vector) {
    
    return v2{
        Scalar / Vector.x,
        Scalar / Vector.y,
    };
}

#define SAME_DIRECTION(VecA, VecB) (Dot(VecA, VecB) > 0)
inline f32
Dot(v2 A, v2 B) {
    
    return (A.x * B.x + A.y * B.y);
}

inline v2
Hadamard(v2 A, v2 B) {
    
    return v2{
        A.x * B.x,
        A.y * B.y
    };
}

inline v2
Perp(v2 Vector) {
    
    return v2{ -Vector.y, Vector.x };
}

inline f32
LengthSq(v2 Vector) {
    
    return (Vector.x * Vector.x + Vector.y * Vector.y);
}

inline f32
Length(v2 Vector) {
    
    return sqrtf(LengthSq(Vector));
}

inline v2
Normalise(v2 Vector) {
    
    return (Vector / Length(Vector));
}

//// NOTE: 3D vector operations

// NOTE: Int
inline v3i
operator+(v3i A, v3i B) {
    
    return v3i{
        A.x + B.x,
        A.y + B.y,
        A.z + B.z
    };
}

inline v3i
operator+(v3i Vector, int Scalar) {
    
    return v3i{
        Vector.x + Scalar,
        Vector.y + Scalar,
        Vector.z + Scalar
    };
}

inline void
operator+=(v3i &A, v3i B) {
    
    A.x += B.x;
    A.y += B.y;
    A.z += B.z;
}

inline v3i
operator-(v3i Vector) {
    
    return v3i{
        -Vector.x,
        -Vector.y,
        -Vector.z
    };
}

inline v3i
operator*(v3i Vector, int Scalar) {
    
    return v3i{
        Vector.x * Scalar,
        Vector.y * Scalar,
        Vector.z * Scalar
    };
}

inline v3
operator*(v3i Vector, f32 Scalar) {
    
    return v3{
        Vector.x * Scalar,
        Vector.y * Scalar,
        Vector.z * Scalar
    };
}

inline b32
operator==(v3i A, v3i B) {
    
    return (A.x == B.x) && (A.y == B.y) && (A.z == B.z);
}

inline int
Volume(v3i Dimensions) {
    
    return (Dimensions.x * Dimensions.y * Dimensions.z);
}

// NOTE: Float
inline v3
V3(int X, int Y, int Z) {
    
    return v3{ (f32)X, (f32)Y, (f32)Z };
}

inline b32
operator==(v3 A, v3 B) {
    
    return (   A.x == B.x
            && A.y == B.y
            && A.z == B.z   );
}

inline b32
operator!=(v3 A, v3 B) {
    
    return (   A.x != B.x
            || A.y != B.y
            || A.z != B.z   );
}

inline v3
operator+(v3 A, v3 B) {
    
    return v3{
        A.x + B.x,
        A.y + B.y,
        A.z + B.z
    };
}

inline v3
operator+(v3 Vector, f32 Scalar) {
    
    return v3{
        Vector.x + Scalar,
        Vector.y + Scalar,
        Vector.z + Scalar
    };
}

inline v3
operator+(f32 Scalar, v3 Vector) {
    
    return (Vector + Scalar);
}

inline void
operator+=(v3 &A, v3 B) {
    
    A.x += B.x;
    A.y += B.y;
    A.z += B.z;
}

inline void
operator+=(v3 &Vector, f32 Scalar) {
    
    Vector.x += Scalar;
    Vector.y += Scalar;
    Vector.z += Scalar;
}

inline void
operator+=(f32 Scalar, v3 &Vector) {
    
    Vector += Scalar;
}

inline v3
operator-(v3 A, v3 B) {
    
    return v3{
        A.x - B.x,
        A.y - B.y,
        A.z - B.z
    };
}

inline v3
operator-(v3 Vector, f32 Scalar) {
    
    return v3{
        Vector.x - Scalar,
        Vector.y - Scalar,
        Vector.z - Scalar
    };
}

inline v3
operator-(f32 Scalar, v3 Vector) {
    
    return v3{
        Scalar - Vector.x,
        Scalar - Vector.y,
        Scalar - Vector.z
    };
}

inline void
operator-=(v3 &VecA, v3 VecB) {
    
    VecA.x -= VecB.x;
    VecA.y -= VecB.y;
    VecA.z -= VecB.z;
}

inline void
operator-=(v3 &Vector, f32 Scalar) {
    
    Vector.x -= Scalar;
    Vector.y -= Scalar;
    Vector.z -= Scalar;
}

inline void
operator-=(f32 Scalar, v3 &Vector) {
    
    Vector -= Scalar;
}

inline v3
operator-(v3 Vector) {
    
    return v3{
        -Vector.x,
        -Vector.y,
        -Vector.z
    };
}

inline v3
operator*(v3 Vector, f32 Scalar) {
    
    return v3{
        Vector.x * Scalar,
        Vector.y * Scalar,
        Vector.z * Scalar
    };
}

inline v3
operator*(f32 Scalar, v3 Vector) {
    
    return (Vector * Scalar);
}

inline v3
operator/(v3 Vector, f32 Scalar) {
    
    return v3{
        Vector.x / Scalar,
        Vector.y / Scalar,
        Vector.z / Scalar
            
    };
}

inline v3
operator/(f32 Scalar, v3 Vector) {
    
    return v3{
        Scalar / Vector.x,
        Scalar / Vector.y,
        Scalar / Vector.z
            
    };
}

inline f32
Dot(v3 A, v3 B) {
    
    return (  A.x * B.x
            + A.y * B.y
            + A.z * B.z);
}

inline v3
Hadamard(v3 A, v3 B) {
    
    return v3{
        A.x * B.x,
        A.y * B.y,
        A.z * B.z
    };
}

inline v3
Cross(v3 A, v3 B) {
    
    return v3{
        ((A.y * B.z) - (A.z * B.y)),
        ((A.z * B.x) - (A.x * B.z)),
        ((A.x * B.y) - (A.y * B.x))
    };
}

inline f32
LengthSq(v3 Vector) {
    
    return (f32)(Vector.x * Vector.x
                 + Vector.y * Vector.y
                 + Vector.z * Vector.z);
}

inline f32
Length(v3 Vector) {
    
    return sqrtf(LengthSq(Vector));
}

inline v3
Normalise(v3 Vector) {
    
    return (Vector / Length(Vector));
}

// NOTE: 4D vector operations

inline v4
operator+(v4 A, v4 B) {
    
    return v4{ A.x + B.x, A.y + B.y, A.z + B.z, A.w + B.w };
}

inline v4
operator/(v4 V, f32 S) {
    
    return v4{ V.x / S, V.y / S, V.z / S, V.w / S };
}

inline void
operator/=(v4 &V, f32 S) {
    
    V.x /= S;
    V.y /= S;
    V.z /= S;
    V.w /= S;
}

inline f32
Dot(v4 A, v4 B) {
    
    return A.x * B.x + A.y * B.y + A.z * B.z + A.w * B.w;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//// NOTE: Matrix operations
// NOTE: 2x2
inline mat2x2
Identity2() {
    
    return mat2x2{
        1, 0,
        0, 1
    };
}

inline mat2x2
Matrix2x2(f32 A00, f32 A10, f32 A01, f32 A11) {
    
    return mat2x2{
        A00, A10,
        A01, A11
    };
}

inline mat2x2
Matrix2x2Rows(v2 Row0, v2 Row1) {
    
    return mat2x2{
        Row0,
        Row1
    };
}

inline mat2x2
Matrix2x2Columns(v2 Col0, v2 Col1) {
    
    Assert(0);
}

inline v2
Row(mat2x2 Mat, int RowIndex) {
    
    return Mat.Rows[RowIndex];
}

inline v2
Column(mat2x2 Mat, int ColumnIndex) {
    
    return v2{
        Mat.Elements[0 + ColumnIndex],
        Mat.Elements[2 + ColumnIndex]
    };
}

inline mat2x2
operator*(mat2x2 Mat, f32 Scalar) {
    
    return mat2x2{
        Mat.A00 * Scalar, Mat.A10 * Scalar,
        Mat.A01 * Scalar, Mat.A11 * Scalar
    };
}

inline mat2x2
operator*(f32 Scalar, mat2x2 Mat) {
    
    return Mat * Scalar;
}

inline mat2x2
operator/(mat2x2 Mat, f32 Scalar) {
    
    return mat2x2{
        Mat.A00 / Scalar, Mat.A10 / Scalar,
        Mat.A01 / Scalar, Mat.A11 / Scalar
    };
}

inline mat2x2
operator/(f32 Scalar, mat2x2 Mat) {
    
    return mat2x2{
        Scalar / Mat.A00, Scalar / Mat.A10,
        Scalar / Mat.A01, Scalar / Mat.A11
    };
}

inline mat2x2
operator*(mat2x2 A, mat2x2 B) {
    
    return {
        A.A00 * B.A00 + A.A10 * B.A01, A.A00 * B.A10 + A.A10 * B.A11, 
        A.A01 * B.A00 + A.A11 * B.A01, A.A01 * B.A10 + A.A11 * B.A11, 
    };
}

inline v2
operator*(mat2x2 Mat, v2 Vec) {
    
    return v2{
        Mat.A00 * Vec.x + Mat.A10 * Vec.y,
        Mat.A01 * Vec.x + Mat.A11 * Vec.y,
    };
}

inline v2
operator*(v2 Vec, mat2x2 Mat) {
    
    return v2{
        Vec.x * Mat.A00 + Vec.y * Mat.A01,
        Vec.x * Mat.A10 + Vec.y * Mat.A11,
    };
}

inline f32
Determinant(mat2x2 Mat) {
    
    return Mat.A00 * Mat.A11 - Mat.A10 * Mat.A01;
}

inline mat2x2
Inverse(mat2x2 Mat) {
    
    return mat2x2{
        Mat.A11, -Mat.A10,
        -Mat.A01,   Mat.A00
    } / Determinant(Mat);
}

// NOTE: 3x3
inline mat3x3
Identity3(void) {
    
    return mat3x3{
        1, 0, 0,
        0, 1, 0,
        0, 0, 1
    };
}

inline mat3x3
Matrix3x3(f32 A00, f32 A10, f32 A20, f32 A01, f32 A11, f32 A21, f32 A02, f32 A12, f32 A22) {
    
    return mat3x3{
        A00, A10, A20,
        A01, A11, A21,
        A02, A12, A22
    };
}

inline mat3x3
Matrix3x3Rows(v3 Row0, v3 Row1, v3 Row2) {
    
    return mat3x3{
        Row0,
        Row1,
        Row2
    };
}

inline mat3x3
Matrix3x3Columns(v3 Col0, v3 Col1, v3 Col2) {
    
    return mat3x3{
        Col0.x, Col1.x, Col2.x,
        Col0.y, Col1.y, Col2.y,
        Col0.z, Col1.z, Col2.z
    };
}

inline v3
Row(mat3x3 Mat, int RowIndex) {
    
    return Mat.Rows[RowIndex];
}

inline v3
Column(mat3x3 Mat, int ColumnIndex) {
    
    return v3{
        Mat.Elements[0 + ColumnIndex],
        Mat.Elements[3 + ColumnIndex],
        Mat.Elements[6 + ColumnIndex]
    };
}

inline mat3x3
operator*(mat3x3 Mat, f32 Scalar) {
    
    return mat3x3{
        Mat.A00 * Scalar, Mat.A10 * Scalar, Mat.A20 * Scalar,
        Mat.A01 * Scalar, Mat.A11 * Scalar, Mat.A21 * Scalar,
        Mat.A02 * Scalar, Mat.A12 * Scalar, Mat.A22 * Scalar
    };
}

inline mat3x3
operator*(f32 Scalar, mat3x3 Mat) {
    
    return Mat * Scalar;
}

inline mat3x3
operator/(mat3x3 Mat, f32 Scalar) {
    
    return mat3x3{
        Mat.A00 / Scalar, Mat.A10 / Scalar, Mat.A20 / Scalar,
        Mat.A01 / Scalar, Mat.A11 / Scalar, Mat.A21 / Scalar,
        Mat.A02 / Scalar, Mat.A12 / Scalar, Mat.A22 / Scalar
    };
}

inline mat3x3
operator/(f32 Scalar, mat3x3 Mat) {
    
    return mat3x3{
        Scalar / Mat.A00, Scalar / Mat.A10, Scalar / Mat.A20,
        Scalar / Mat.A01, Scalar / Mat.A11, Scalar / Mat.A21,
        Scalar / Mat.A02, Scalar / Mat.A12, Scalar / Mat.A22
    };
}

inline mat3x3
operator*(mat3x3 A, mat3x3 B) {
    
    return mat3x3{
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

inline v3
operator*(mat3x3 Mat, v3 Vec) {
    
    return v3{
        Mat.A00 * Vec.x + Mat.A10 * Vec.y + Mat.A20 * Vec.z,
        Mat.A01 * Vec.x + Mat.A11 * Vec.y + Mat.A21 * Vec.z,
        Mat.A02 * Vec.x + Mat.A12 * Vec.y + Mat.A22 * Vec.z
    };
}

inline v3
operator*(v3 Vec, mat3x3 Mat) {
    
    return v3{
        Vec.x * Mat.A00 + Vec.y * Mat.A01 + Vec.z * Mat.A02,
        Vec.x * Mat.A10 + Vec.y * Mat.A11 + Vec.z * Mat.A12,
        Vec.x * Mat.A20 + Vec.y * Mat.A21 + Vec.z * Mat.A22
    };
}

inline f32
Determinant(mat3x3 Mat) {
    
    return (  Mat.A00 * (Mat.A11 * Mat.A22 - Mat.A21 * Mat.A12)
            - Mat.A10 * (Mat.A01 * Mat.A22 - Mat.A21 * Mat.A02)
            + Mat.A20 * (Mat.A01 * Mat.A12 - Mat.A11 * Mat.A02)  );
}

inline mat3x3
Transpose(mat3x3 M) {
    
    return mat3x3{
        M.A00, M.A01, M.A02,
        M.A10, M.A11, M.A12,
        M.A20, M.A21, M.A22,
    };
}

// TODO(canta): Test this.
inline mat3x3
Inverse(mat3x3 M) {
    
    f32 efhi = M.e * M.i - M.f * M.h;
    f32 dfgi = M.d * M.i - M.f * M.g;
    f32 degh = M.d * M.h - M.e * M.g;
    f32 Det = (  M.a * efhi
               - M.b * dfgi
               + M.c * degh);
    
    return mat3x3{
        efhi,
        M.c * M.h - M.b * M.i,
        M.b * M.f - M.c * M.e,
        
        -dfgi,
        M.a * M.i - M.c * M.g,
        M.c * M.d - M.a * M.f,
        
        degh,
        M.b * M.g - M.a * M.h,
        M.a * M.e - M.b * M.d
    } / Det;
}

# if 0 // NOTE(canta): Reference implementation
inline mat3x3
Inverse(mat3x3 M) {
    
    mat3x3 Result;
    
    // Matrix of minors
    Result = {
        Determinant(mat2x2{ M.A11, M.A21, M.A12, M.A22 }),
        Determinant(mat2x2{ M.A01, M.A21, M.A02, M.A22 }),
        Determinant(mat2x2{ M.A01, M.A11, M.A02, M.A12 }),
        
        Determinant(mat2x2{ M.A10, M.A20, M.A12, M.A22 }),
        Determinant(mat2x2{ M.A00, M.A20, M.A02, M.A22 }),
        Determinant(mat2x2{ M.A00, M.A10, M.A02, M.A12 }),
        
        Determinant(mat2x2{ M.A10, M.A20, M.A11, M.A21 }),
        Determinant(mat2x2{ M.A00, M.A20, M.A01, M.A21 }),
        Determinant(mat2x2{ M.A00, M.A10, M.A01, M.A11 }),
    };
    
    // Matrix of cofactors
    Result.A10 = -Result.A10;
    Result.A01 = -Result.A01;
    Result.A21 = -Result.A21;
    Result.A12 = -Result.A12;
    
    // Adjunct matrix
    f32 Temp;
    
    Temp = Result.A10;
    Result.A10 = Result.A01;
    Result.A01 = Temp;
    
    Temp = Result.A20;
    Result.A20 = Result.A02;
    Result.A02 = Temp;
    
    Temp = Result.A12;
    Result.A12 = Result.A21;
    Result.A21 = Temp;
    
    Result = Result / Determinant(M);
    
    return Result;
}
#endif

// NOTE: 4x4
inline mat4x4
operator*(mat4x4 A, mat4x4 B) {
    
    return mat4x4{
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
        A.A03 * B.A30 + A.A13 * B.A31 + A.A23 * B.A32 + A.A33 * B.A33
    };
}

inline v4
operator*(mat4x4 M, v4 V) {
    
    return v4{
        M.A00 * V.x + M.A10 * V.y + M.A20 * V.z + M.A30 * V.w,
        M.A01 * V.x + M.A11 * V.y + M.A21 * V.z + M.A31 * V.w,
        M.A02 * V.x + M.A12 * V.y + M.A22 * V.z + M.A32 * V.w,
        M.A03 * V.x + M.A13 * V.y + M.A23 * V.z + M.A33 * V.w,
    };
}

inline mat4x4
operator*(mat4x4 M, f32 S) {
    
    return mat4x4{
        M.A00 * S, M.A10 * S, M.A20 * S, M.A30 * S,
        M.A01 * S, M.A11 * S, M.A21 * S, M.A31 * S,
        M.A02 * S, M.A12 * S, M.A22 * S, M.A32 * S,
        M.A03 * S, M.A13 * S, M.A23 * S, M.A33 * S,
    };
}

inline mat4x4
operator*(f32 S, mat4x4 M) {
    
    return M * S;
}

inline mat4x4
operator/(mat4x4 M, f32 S) {
    
    return mat4x4{
        M.A00 / S, M.A10 / S, M.A20 / S, M.A30 / S,
        M.A01 / S, M.A11 / S, M.A21 / S, M.A31 / S,
        M.A02 / S, M.A12 / S, M.A22 / S, M.A32 / S,
        M.A03 / S, M.A13 / S, M.A23 / S, M.A33 / S,
    };
}

inline mat4x4
Transpose(mat4x4 M) {
    
    return {
        M.A00, M.A01, M.A02, M.A03,
        M.A10, M.A11, M.A12, M.A13,
        M.A20, M.A21, M.A22, M.A23,
        M.A30, M.A31, M.A32, M.A33
    };
}


// TODO(canta): This is slow and loses precision very fast!
inline f32
Determinant(mat4x4 M) {
    
    return (  M.A00 * Determinant(mat3x3{ M.A11, M.A21, M.A31, M.A12, M.A22, M.A32, M.A13, M.A23, M.A33 })
            - M.A10 * Determinant(mat3x3{ M.A01, M.A21, M.A31, M.A02, M.A22, M.A32, M.A03, M.A23, M.A33 })
            + M.A20 * Determinant(mat3x3{ M.A01, M.A11, M.A31, M.A02, M.A12, M.A32, M.A03, M.A13, M.A33 })
            - M.A30 * Determinant(mat3x3{ M.A01, M.A11, M.A21, M.A02, M.A12, M.A22, M.A03, M.A13, M.A23 })
            );
}

// TODO(canta): This is slow and loses precision very fast!
inline mat4x4
Inverse(mat4x4 M) {
    
    mat4x4 Result;
    
    // Matrix of minors
    Result = {
        Determinant(mat3x3{ M.A11, M.A21, M.A31, M.A12, M.A22, M.A32, M.A13, M.A23, M.A33 }),
        Determinant(mat3x3{ M.A01, M.A21, M.A31, M.A02, M.A22, M.A32, M.A03, M.A23, M.A33 }),
        Determinant(mat3x3{ M.A01, M.A11, M.A31, M.A02, M.A12, M.A32, M.A03, M.A13, M.A33 }),
        Determinant(mat3x3{ M.A01, M.A11, M.A21, M.A02, M.A12, M.A22, M.A03, M.A13, M.A23 }),
        
        Determinant(mat3x3{ M.A10, M.A20, M.A30, M.A12, M.A22, M.A32, M.A13, M.A23, M.A33 }),
        Determinant(mat3x3{ M.A00, M.A20, M.A30, M.A02, M.A22, M.A32, M.A03, M.A23, M.A33 }),
        Determinant(mat3x3{ M.A00, M.A10, M.A30, M.A02, M.A12, M.A32, M.A03, M.A13, M.A33 }),
        Determinant(mat3x3{ M.A00, M.A10, M.A20, M.A02, M.A12, M.A22, M.A03, M.A13, M.A23 }),
        
        Determinant(mat3x3{ M.A10, M.A20, M.A30, M.A11, M.A21, M.A31, M.A13, M.A23, M.A33 }),
        Determinant(mat3x3{ M.A00, M.A20, M.A30, M.A01, M.A21, M.A31, M.A03, M.A23, M.A33 }),
        Determinant(mat3x3{ M.A00, M.A10, M.A30, M.A01, M.A11, M.A31, M.A03, M.A13, M.A33 }),
        Determinant(mat3x3{ M.A00, M.A10, M.A20, M.A01, M.A11, M.A21, M.A03, M.A13, M.A23 }),
        
        Determinant(mat3x3{ M.A10, M.A20, M.A30, M.A11, M.A21, M.A31, M.A12, M.A22, M.A32 }),
        Determinant(mat3x3{ M.A00, M.A20, M.A30, M.A01, M.A21, M.A31, M.A02, M.A22, M.A32 }),
        Determinant(mat3x3{ M.A00, M.A10, M.A30, M.A01, M.A11, M.A31, M.A02, M.A12, M.A32 }),
        Determinant(mat3x3{ M.A00, M.A10, M.A20, M.A01, M.A11, M.A21, M.A02, M.A12, M.A22 }),
    };
    
    // Matrix of cofactors
    Result.A10 = -Result.A10;
    Result.A30 = -Result.A30;
    Result.A01 = -Result.A01;
    Result.A21 = -Result.A21;
    Result.A12 = -Result.A12;
    Result.A32 = -Result.A32;
    Result.A02 = -Result.A03;
    Result.A22 = -Result.A23;
    
    // Adjunct matrix
    Result = Transpose(Result);
    
    Result = Result / Determinant(M);
    
    return Result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// NOTE: Geometric Algebra

// https://marctenbosch.com/quaternions/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// NOTE: Bivectors
inline biv3
Wedge(v3 VecS, v3 VecD) {
    
    return biv3{
        (VecS.x * VecD.y - VecS.y * VecD.x),
        (VecS.x * VecD.z - VecS.z * VecD.x),
        (VecS.y * VecD.z - VecS.z * VecD.y)
    };
}

inline f32
LengthSq(biv3 Bivector) {
    
    return (Sq(Bivector.xy) + Sq(Bivector.xz) + Sq(Bivector.yz));
}

inline f32
Length(biv3 Bivector) {
    
    return sqrtf(LengthSq(Bivector));
}

// NOTE: Rotors

inline rot3
operator*(rot3 RotA, rot3 RotB) {
    
    // (canta): For a walkthrough of this, see       with-seeds-of-love/misc/Derivations/GeoAlg/RotRotMul
    return rot3 {
        (RotA.a * RotB.a   - RotA.bxy * RotB.bxy - RotA.bxz * RotB.bxz - RotA.byz * RotB.byz),
        (RotA.a * RotB.bxy + RotA.bxy * RotB.a   - RotA.bxz * RotB.byz + RotA.byz * RotB.bxz),
        (RotA.a * RotB.bxz + RotA.bxz * RotB.a   + RotA.bxy * RotB.byz - RotA.byz * RotB.bxy),
        (RotA.a * RotB.byz + RotA.byz * RotB.a   - RotA.bxz * RotB.bxz + RotA.bxz * RotB.bxy)
    };
}

inline rot3
Conjugate(rot3 Rotor) {
    
    return rot3{
        Rotor.a, -Rotor.bxy, -Rotor.bxz, -Rotor.byz,
    };
}

inline rot3
Geometric(v3 VecA, v3 VecB) {
    
    return rot3{ Dot(VecA, VecB), Wedge(VecA, VecB) };
}

inline f32
LengthSq(rot3 Rotor) {
    
    return (Sq(Rotor.a) + LengthSq(Rotor.b));
}

inline f32
Length(rot3 Rotor) {
    
    return sqrtf(LengthSq(Rotor));
}

inline rot3 
Normalise(rot3 Rotor){
    
    f32 RotorLength = Length(Rotor);
    return {
        Rotor.a   / RotorLength,
        Rotor.bxy / RotorLength,
        Rotor.bxz / RotorLength,
        Rotor.byz / RotorLength
    };
}







// STUDY(canta): UNDERSTANDING QUARANTINE
inline rot3
ConstructRotor(v3 VecS, v3 VecD) {
    
    rot3 Result;
    
    Result.a = 1 + Dot(VecD, VecS); // Somehow when the rotor is normalised this turns into cos(theta/2)?? Haven't been able to figure it out yet
    Result.b = Wedge(VecD, VecS);
    
    return Normalise(Result);
}

inline rot3
ConstructRotor(f32 Angle, biv3 Plane) {
    
    rot3 Result;
    
    Result.a = cosf(Angle * 0.5f);
    
    f32 SinAngle = sinf(Angle * 0.5f);
    Result.bxy = -SinAngle * Plane.xy;
    Result.bxz = -SinAngle * Plane.xz;
    Result.byz = -SinAngle * Plane.yz;
    
    return Result;
}

inline v3
Rotate(v3 V, rot3 R) {
    
    v3 Result;
    
    v3 Q;
    Q.x = V.x * R.a + V.y * R.bxy + V.z * R.bxz;
    Q.y = V.y * R.a - V.x * R.bxy + V.z * R.byz;
    Q.z = V.z * R.a - V.x * R.bxz - V.y * R.byz;
    
    f32 Q012 = -V.x * R.byz + V.y * R.bxz - V.z * R.bxy;
    
    // https://www.eleceng.adelaide.edu.au/personal/dabbott/wiki/images/8/8f/GA_Worksheet.pdf
    Result.x = Q.x * R.a + Q.y  * R.bxy + Q.z  * R.bxz + Q012 * R.byz;
    Result.y = Q.y * R.a + Q.x  * R.bxy + Q012 * R.bxz + Q.z  * R.byz;
    Result.z = Q.z * R.a + Q012 * R.bxy + Q.x  * R.bxz + Q.y  * R.byz;
    
    return Result;
}

inline rot3
Reverse(rot3 Rotor) {
    
    return rot3{ Rotor.a, Rotor.b };
}

inline rot3
Rotate(rot3 A, rot3 B) {
    
    return B * A * Reverse(B);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

