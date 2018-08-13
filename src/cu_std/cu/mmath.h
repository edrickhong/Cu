#pragma once

/*

  NOTE: 
  Vec4(x,y,z,1) - point  rotating this changes the point position
  Vec4(x,y,z,0) - vector rotating this will create a new vector(eg rotating a translation)
*/

//AVX2 is slow. to get full speed of avx, it has to downclock the cpu
//maybe this will change in time (https://gist.github.com/rygorous/32bc3ea8301dba09358fd2c64e02d774)

#include "math.h"//we can replace this too I guess
#include "mode.h"
#include "ttype.h"
#include "iintrin.h"


#define _pi 3.1415926535897f
#define _twopi (3.1415926535897f * 2.0f)
#define _fourpi (3.1415926535897f * 4.0f)
#define _halfpi (3.1415926535897f * 0.5f)

#ifdef _row_major

#define MATRIX_ROW_MAJOR _row_major

#else

#define MATRIX_ROW_MAJOR 1

#endif

#define DEPTH_ZERO_TO_ONE 1
#define Z_RHS 1

#define _radians(degrees) (degrees * (_pi/180.0f))
#define _degrees(radians) (radians * (180.0f/_pi))


//direct element access
#define _ac4(x,y) [x + (y * 4)]
#define _ac3(x,y) [x + (y * 3)]
#define _ac2(x,y) [x + (y * 2)]


//access element as if it were row major
#if MATRIX_ROW_MAJOR

#define _rc4(x,y) _ac4(x,y)
#define _rc3(x,y) _ac3(x,y)
#define _rc2(x,y) _ac2(x,y)

#else

#define _rc4(y,x) [x + (y * 4)]
#define _rc3(y,x) [x + (y * 3)]
#define _rc2(y,x) [x + (y * 2)]

#endif

#define _clamp(x, upper, lower) (fmin(upper, fmax(x, lower)))

#ifdef _WIN32

struct simd4f{
    
    __m128 a;
    
    f32& operator[](ptrsize i){
        
        auto k = (f32*)&a;
        return k[i];
    }
    
};

struct simd2f{
    
    __m64 a;
    
    f32& operator[](ptrsize i){
        
        auto k = (f32*)&a;
        return k[i];
    }
};

#else

typedef __m128 simd4f;
typedef __m64 simd2f;

#endif

#define _setksimd4f _mm_set1_ps

#define _setsimd4f _mm_setr_ps

#define _setrsimd4f _mm_set_ps

#define _loadsimd4f _mm_load_ps

#define _loadusimd4f _mm_loadu_ps

#define _setrsimd4f _mm_set_ps

#define _addsimd4f _mm_add_ps

#define _subsimd4f _mm_sub_ps

#define _mulsimd4f _mm_mul_ps
#define _divsimd4f _mm_div_ps

#define _divsimd4f _mm_div_ps

#define _storesimd4f _mm_store_ps

#define _storeusimd4f _mm_storeu_ps

#define _storeksimd4f _mm_store_ps1

#define _loadasymsimd4f _mm_loadh_pi

#define _loadstorehsimd4f _mm_loadl_pi

#define _shufflesimd4f _mm_shuffle_ps

//MARK:Linear Algebra

union Quaternion{
    simd4f simd;
    
    struct{
        f32 w,x,y,z;
    };
    
}_align(16);

struct DualQuaternion{
    Quaternion q1,q2;
};

struct Vector2{
    f32 x,y;
};

union Vector4{
    
    simd4f simd;
    
    struct{
        f32 x,y,z,w;
    };
    
    Vector2 vec2[2];
    
    f32 floats[4];
    
}_align(16);

struct Vector4SOA{
    ptrsize count;
    union{
        struct{//refers to 4 floats at a time
            simd4f* simd_x;
            simd4f* simd_y;
            simd4f* simd_z;
            simd4f* simd_w;  
        };
        
        struct{//refers to 1 floats at a time
            f32* x;
            f32* y;
            f32* z;
            f32* w;  
        };
    };
    
};

union Vector3{
    
    struct{
        f32 x,y,z;
    };
    
    f32 floats[3];
    Vector2 vec2;
};

Vector4 _ainline ToVec4(Vector3 vec){
    
    return {vec.x,vec.y,vec.z,1.0f};
}

Vector3 _ainline ToVec3(Vector4 vec){
    return {vec.x,vec.y,vec.z};
}

Vector2 _ainline ToVec2(Vector4 vec){
    return {vec.x,vec.y};
}

Vector2 _ainline ToVec2(Vector3 vec){
    return {vec.x,vec.y};
}

union Matrix4b4{
    f32 container[16];
    simd4f simd[4];
    
    f32& operator[](u32 index){
        return container[index];
    }
}_align(16);


//NOTE: do we want to use simd for 3b3
union Matrix3b3{
    f32 container[9];
    
    struct {
        simd4f simd[2];
        f32 k;
    };
    
    
    f32& operator[](u32 index){
        return container[index];
    }
}_align(16);

union Matrix2b2{
    
    f32 container[4];
    
    struct {
        simd4f simd;
    };
    
    
    f32& operator[](u32 index){
        return container[index];
    }
}_align(16);


Matrix4b4 _ainline IdentityMatrix4b4(){
    
    Matrix4b4 matrix = {
        
        {
            1,0,0,0,
            0,1,0,0,
            0,0,1,0,
            0,0,0,1
        }
    };
    
    
    return matrix;
}


Matrix3b3 _ainline IdentityMatrix3b3(){
    
    Matrix3b3 matrix = {
        {
            1,0,0,
            0,1,0,
            0,0,1,
        }
    };
    
    
    return matrix;
}

Matrix2b2 _ainline IdentityMatrix2b2(){
    
    Matrix2b2 matrix = {
        {
            1,0,
            0,1,
        }
    };
    
    
    return matrix;
}

Matrix2b2 _ainline ToMatrix2b2(Matrix3b3 mat){
    
    Matrix2b2 m;
    
    m[0] = mat _ac3(0,0);
    m[1] = mat _ac3(1,0);
    
    m[2] = mat _ac3(0,1);
    m[3] = mat _ac3(1,1);
    
    return m;
}

Matrix2b2 _ainline ToMatrix2b2(Matrix4b4 mat){
    
    Matrix2b2 m;
    
    m[0] = mat _ac4(0,0);
    m[1] = mat _ac4(1,0);
    
    m[2] = mat _ac4(0,1);
    m[3] = mat _ac4(1,1);
    
    return m;
}

Matrix3b3 _ainline ToMatrix3b3(Matrix2b2 mat){
    
    Matrix3b3 m = IdentityMatrix3b3();
    
    m _ac3(0,0) = mat[0];
    m _ac3(1,0) = mat[1];
    
    m _ac3(0,1) = mat[2];
    m _ac3(1,1) = mat[3];
    
    
    return m;
}



Matrix3b3 _ainline ToMatrix3b3(Matrix4b4 mat){
    
    Matrix3b3 m;
    
    m[0] = mat _ac4(0,0);
    m[1] = mat _ac4(1,0);
    m[2] = mat _ac4(2,0);
    
    m[3] = mat _ac4(0,1);
    m[4] = mat _ac4(1,1);
    m[5] = mat _ac4(2,1);
    
    m[6] = mat _ac4(0,2);
    m[7] = mat _ac4(1,2);
    m[8] = mat _ac4(2,2);
    
    
    return m;
}

Matrix4b4 _ainline ToMatrix4b4(Matrix2b2 mat){
    
    Matrix4b4 m = IdentityMatrix4b4();
    
    m _ac4(0,0) = mat[0];
    m _ac4(1,0) = mat[1];
    
    m _ac4(0,1) = mat[2];
    m _ac4(1,1) = mat[3];
    
    return m;
}

Matrix4b4 _ainline ToMatrix4b4(Matrix3b3 mat){
    
    Matrix4b4 m = IdentityMatrix4b4();
    
    m _ac4(0,0) = mat[0];
    m _ac4(1,0) = mat[1];
    m _ac4(2,0) = mat[2];
    
    m _ac4(0,1) = mat[3];
    m _ac4(1,1) = mat[4];
    m _ac4(2,1) = mat[5];
    
    m _ac4(0,2)= mat[6];
    m _ac4(1,2) = mat[7];
    m _ac4(2,2) = mat[8];
    
    return m;
}


//Math operations
Matrix4b4 operator+(Matrix4b4 lhs,Matrix4b4 rhs);
Matrix4b4 operator-(Matrix4b4 lhs,Matrix4b4 rhs);
Matrix4b4 operator*(Matrix4b4 lhs,Matrix4b4 rhs);
Matrix4b4 operator*(f32 lhs,Matrix4b4 rhs);
Matrix4b4 operator*(Matrix4b4 lhs,f32 rhs);
Matrix4b4 operator/(Matrix4b4 lhs,Matrix4b4 rhs);
Matrix4b4 Transpose(Matrix4b4 matrix);
Matrix4b4 Inverse(Matrix4b4 matrix);

Matrix3b3 operator+(Matrix3b3 lhs,Matrix3b3 rhs);
Matrix3b3 operator-(Matrix3b3 lhs,Matrix3b3 rhs);
Matrix3b3 operator*(Matrix3b3 lhs,Matrix3b3 rhs);
Matrix3b3 operator*(f32 lhs,Matrix3b3 rhs);
Matrix3b3 operator*(Matrix3b3 lhs,f32 rhs);
Matrix3b3 operator/(Matrix3b3 lhs,Matrix3b3 rhs);
Matrix3b3 Transpose(Matrix3b3 matrix);
Matrix3b3 Inverse(Matrix3b3 matrix);

Matrix2b2 operator+(Matrix2b2 lhs,Matrix2b2 rhs);
Matrix2b2 operator-(Matrix2b2 lhs,Matrix2b2 rhs);
Matrix2b2 operator*(Matrix2b2 lhs,Matrix2b2 rhs);
Matrix2b2 operator*(f32 lhs,Matrix2b2 rhs);
Matrix2b2 operator*(Matrix2b2 lhs,f32 rhs);
Matrix2b2 operator/(Matrix2b2 lhs,Matrix2b2 rhs);
Matrix2b2 Transpose(Matrix2b2 matrix);
Matrix2b2 Inverse(Matrix2b2 matrix);


//conversions
Matrix4b4 QuaternionToMatrix(Quaternion quaternion);
Quaternion MatrixToQuaternion(Matrix4b4 matrix);
DualQuaternion ConstructDualQuaternion(Matrix4b4 transform);
Matrix4b4 DualQuaternionToMatrix(DualQuaternion d);

Vector3 _ainline MatrixToTranslationVector(Matrix4b4 matrix){
    
    return Vector3{matrix _rc4(3,0),matrix _rc4(3,1),matrix _rc4(3,2)};
}

//print
void PrintMatrix(Matrix4b4 matrix);


Matrix4b4 ViewMatrix(Vector3 position,Vector3 lookpoint,Vector3 updir);
Matrix4b4 ProjectionMatrix(f32 fov,f32 aspectration,f32 nearz,f32 farz);

Matrix4b4 _ainline PositionMatrix(Vector3 position){
    
    Matrix4b4 matrix = IdentityMatrix4b4();
    
    matrix _rc4(3,0) = position.x;
    matrix _rc4(3,1) = position.y;
    matrix _rc4(3,2) = position.z;
    
    return matrix;
    
    
}

Matrix4b4 _ainline RotationMatrix(Vector3 rotation){
    
    f32 cosv = cosf(rotation.x);
    f32 sinv = sinf(rotation.x);
    
    Matrix4b4 rotationx_matrix4b4 = IdentityMatrix4b4();
    
    rotationx_matrix4b4 _rc4(1,1) = cosv;
    rotationx_matrix4b4 _rc4(2,1) = -sinv;
    rotationx_matrix4b4 _rc4(1,2) = sinv;
    rotationx_matrix4b4 _rc4(2,2) = cosv;
    
    cosv = cosf(rotation.y);
    sinv = sinf(rotation.y);
    
    Matrix4b4 rotationy_matrix4b4 = IdentityMatrix4b4();
    
    rotationy_matrix4b4 _rc4(0,0) = cosv;
    rotationy_matrix4b4 _rc4(2,0) = sinv;
    rotationy_matrix4b4 _rc4(0,2) = -sinv;
    rotationy_matrix4b4 _rc4(2,2) = cosv;
    
    cosv = cosf(rotation.z);
    sinv = sinf(rotation.z);
    
    Matrix4b4 rotationz_matrix4b4 = IdentityMatrix4b4();
    
    rotationz_matrix4b4 _rc4(0,0) = cosv;
    rotationz_matrix4b4 _rc4(1,0) = -sinv;
    rotationz_matrix4b4 _rc4(0,1) = sinv;
    rotationz_matrix4b4 _rc4(1,1) = cosv;
    
    return rotationz_matrix4b4 * rotationy_matrix4b4 * rotationx_matrix4b4;
}



Matrix4b4 _ainline ScaleMatrix(Vector3 scale){
    
    Matrix4b4 matrix = {
        {
            scale.x,0,0,0,
            0,scale.y,0,0,
            0,0,scale.z,0,
            0,0,0,1
        }
    };
    
    return matrix;
}

Matrix4b4 WorldMatrix(Matrix4b4 position,Matrix4b4 rotation,Matrix4b4 scale);

Matrix4b4 WorldMatrix(Vector3 position,Vector3 rotation,Vector3 scale);

Matrix4b4 WorldMatrix(Vector3 position,Quaternion rotation,Vector3 scale);

Vector4 WorldSpaceToClipSpace(Vector4 pos,Matrix4b4 viewproj);
Vector4 ClipSpaceToWorldSpace(Vector4 pos,Matrix4b4 viewproj);

Vector3 _ainline WorldSpaceToClipSpace(Vector3 pos,Matrix4b4 viewproj){
    
    return ToVec3(WorldSpaceToClipSpace(ToVec4(pos),viewproj));
}

Vector3 _ainline ClipSpaceToWorldSpace(Vector3 pos,Matrix4b4 viewproj){
    
    return ToVec3(ClipSpaceToWorldSpace(ToVec4(pos),viewproj));
}


typedef Vector4 Point4;
typedef Vector3 Point3;
typedef Vector2 Point2;

struct Line3{
    Point3 pos;
    Vector3 dir;
};

struct Plane{
    Point3 pos;
    Vector3 norm;
};

struct Line2{
    Point2 pos;
    Vector2 dir;
};

enum IntersectType : u8{
    INTERSECT_FALSE = 0,
    INTERSECT_FINITE = 1,
    INTERSECT_INFINITE = 2,
};

logic Intersect(Line2 a,Line2 b);

logic Intersect(Line2 a,Line2 b,Point2* out_point);

logic TypedIntersect(Line2 a,Line2 b);

f32 Magnitude(Vector3 vec);
Vector3 Cross(Vector3 vec1,Vector3 vec2);
f32 Dot(Vector3 vec1,Vector3 vec2);
f32 cosf(Vector3 vec1,Vector3 vec2);
Vector3 Normalize(Vector3 vec);
Vector3 GetVectorRotation(Vector3 lookat);

Vector3 Vec3(f32 x,f32 y,f32 z);

f32 Component(Vector3 a,Vector3 b);

Vector3 ProjectOnto(Vector3 a,Vector3 b);

Vector3 ProjectVectorOntoPlane(Vector3 vec,Plane plane);

//line intersections

logic Intersect(Line3 a,Line3 b);

logic Intersect(Line3 a,Line3 b,Point3* out_point);

logic TypedIntersect(Line3 a,Line3 b);


//plane intersections

logic Intersect(Line3 a,Plane b);

logic TypedIntersect(Line3 a,Plane b);

logic Intersect(Line3 a,Plane b,Point3* out_point);

Vector4 Normalize(Vector4 vec);
f32 Magnitude(Vector4 vec);
f32 Dot(Vector4 vec1,Vector4 vec2);
Vector4 VectorComponentMul(Vector4 a,Vector4 b);


Vector4 operator+(Vector4 lhs,Vector4 rhs);
Vector4 operator-(Vector4 lhs,Vector4 rhs);
Vector4 operator*(f32 lhs,Vector4 rhs);
Vector4 operator*(Vector4 lhs,f32 rhs);
Vector4 operator/(Vector4 lhs,f32 rhs);

Vector3 operator+(Vector3 lhs,Vector3 rhs);
Vector3 operator-(Vector3 lhs,Vector3 rhs);
Vector3 operator*(f32 lhs,Vector3 rhs);
Vector3 operator*(Vector3 lhs,f32 rhs);
Vector3 operator/(Vector3 lhs,f32 rhs);

Vector2 operator+(Vector2 lhs,Vector2 rhs);
Vector2 operator-(Vector2 lhs,Vector2 rhs);
Vector2 operator*(f32 lhs,Vector2 rhs);
Vector2 operator*(Vector2 lhs,f32 rhs);
Vector2 operator/(Vector2 lhs,f32 rhs);


Vector2 Normalize(Vector2 a);

f32 Dot(Vector2 a,Vector2 b);

Vector2 CompMul(Vector2 a,Vector2 b);

f32 AngleQuadrant(f32 x,f32 y);

f32 Magnitude(Vector2 vec);


Vector2 RotateVector(Vector2 vec,f32 rotation);

Vector3 RotateVector(Vector3 vec,Vector3 rotation);







void PrintVector4(Vector4 vec);
void PrintVector3(Vector3 vec);
void PrintVector2(Vector2 vec);



struct Triangle{
    Point3 a;
    Point3 b;
    Point3 c;
};

//?
struct Teathedron{
    
};

struct Polygon{
    Point3* point_array;
    ptrsize count;
    
    Point3& operator[](u32 index){
        return point_array[index];
    }
};

void MinkowskiAddition(Point3* a,ptrsize a_count,Point3* b,ptrsize b_count,Point3** ret);

void MinkowskiDifference(Point3* a,ptrsize a_count,Point3* b,ptrsize b_count,Point3** ret);


typedef Vector4SOA PolygonSOA;


Vector4 _ainline ToVec4(Quaternion q){
    
    Vector4 v;
    
    v.x = q.w;
    v.y = q.x;
    v.z = q.y;
    v.w = q.z;
    
    return v;
}

Quaternion _ainline ToQuat(Vector4 v){
    
    Quaternion q;
    
    q.w = v.x;
    q.x = v.y;
    q.y = v.z;
    q.z = v.w;
    
    return q;
    
}


Quaternion _ainline Normalize(Quaternion a){
    return ToQuat(Normalize(ToVec4(a)));
}

f32 _ainline Magnitude(Quaternion a){
    return Magnitude(ToVec4(a));
}

f32 _ainline Dot(Quaternion a,Quaternion b){
    return Dot(ToVec4(a),ToVec4(b));
}


Quaternion _ainline operator+(Quaternion lhs,Quaternion rhs){
    
    Vector4 l = ToVec4(lhs);
    Vector4 r = ToVec4(rhs);
    
    return ToQuat(l + r);
}

Quaternion _ainline operator-(Quaternion lhs,Quaternion rhs){
    
    Vector4 l = ToVec4(lhs);
    Vector4 r = ToVec4(rhs);
    
    return ToQuat(l - r);
}

Quaternion _ainline operator*(f32 lhs,Quaternion rhs){
    
    Vector4 r = ToVec4(rhs);
    
    return ToQuat(lhs * r);
}

Quaternion _ainline operator*(Quaternion lhs,f32 rhs){
    
    Vector4 l = ToVec4(lhs);
    
    return ToQuat(l * rhs);
}

Quaternion _ainline operator/(Quaternion lhs,f32 rhs){
    
    Vector4 l = ToVec4(lhs);
    
    return ToQuat(l / rhs);
}


Quaternion operator*(Quaternion lhs,Quaternion rhs);

Quaternion Inverse(Quaternion q);

Vector3 RotateVector(Vector3 v,Quaternion q);

Quaternion ConstructQuaternion(Vector3 vector,f32 angle);

Quaternion ConjugateQuaternion(Quaternion quaternion);

void DeconstructQuaternion(Quaternion quaternion,Vector3* vector,f32* angle);

Quaternion _ainline MQuaternionIdentity(){
    return Quaternion{1.0f,0.0f,0.0f,0.0f};
}

Quaternion _ainline AQuaternionIdentity(){
    return Quaternion{0.0f,0.0f,0.0f,0.0f};
}

void PrintQuaternion(Quaternion quat);

Vector4 _ainline InterpolateVector(Vector4 a,Vector4 b,f32 step){
    
    Vector4 vec = b - a;
    vec = vec * step;
    vec = a + vec;
    
    return vec;
}


Quaternion _ainline InterpolateQuaternion(Quaternion a,Quaternion b,f32 step){
    
    Quaternion q = b - a;
    q = q * step;
    q = a + q;
    
    return q;
}

f32 _ainline Interpolate(f32 a,f32 b,f32 step){
    
    return  (a + (step * (b - a)));
}

Quaternion NLerp(Quaternion a,Quaternion b,f32 step);

Quaternion SLerp(Quaternion a,Quaternion b,f32 step);

DualQuaternion ConstructDualQuaternion(Quaternion rotation,Vector3 translation);



DualQuaternion operator+(DualQuaternion lhs,DualQuaternion rhs);
DualQuaternion operator-(DualQuaternion lhs,DualQuaternion rhs);
DualQuaternion operator*(DualQuaternion lhs,DualQuaternion rhs);
DualQuaternion operator*(f32 lhs,DualQuaternion rhs);
DualQuaternion operator*(DualQuaternion lhs,f32 rhs);

DualQuaternion Normalize(DualQuaternion d);