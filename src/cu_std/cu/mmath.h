#pragma once

/*

  NOTE: 
  Vec4(x,y,z,1) - point  rotating this changes the point position
  Vec4(x,y,z,0) - vector rotating this will create a new vector(eg rotating a translation)
*/

//we will update this to use avx in 2 years

#include "math.h"//we can replace this too I guess
#include "mode.h"
#include "ttype.h"
#include "iintrin.h"


#define _pi 3.1415926535897f
#define _twopi (3.1415926535897f * 2.0f)
#define _fourpi (3.1415926535897f * 4.0f)
#define _halfpi (3.1415926535897f * 0.5f)

#define DEPTH_ZERO_TO_ONE 1
#define Z_RHS 1

#define _radians(degrees) (degrees * (_pi/180.0f))
#define _degrees(radians) (radians * (180.0f/_pi))

#define _ac(x,y) [x + (y * 4)]

#if 1

#define _clamp(x, upper, lower) (fmin(upper, fmax(x, lower)))

#else

#define _clamp(x, upper, lower) x

#endif

typedef __m128 simd4f;
typedef __m64 simd2f;

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

typedef Vector4 Vector3;

typedef Vector3 EuelerAngle;

union Matrix4b4{
    f32 container[16];
    simd4f simd[4];
    
    f32& operator[](u32 index){
        return container[index];
    }
}_align(16);

struct Matrix3b3{
    f32 container[9];
    
    f32& operator[](u32 index){
        return container[index];
    }
};

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

namespace Vec3{
    
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
}

namespace Vec4{
    Vector4 Normalize(Vector4 vec);
    f32 Magnitude(Vector4 vec);
    f32 Dot(Vector4 vec1,Vector4 vec2);
    Vector4 VectorComponentMul(Vector4 a,Vector4 b);
}

Matrix4b4 operator+(Matrix4b4 lhs,Matrix4b4 rhs);
Matrix4b4 operator-(Matrix4b4 lhs,Matrix4b4 rhs);
Matrix4b4 operator*(Matrix4b4 lhs,Matrix4b4 rhs);
Matrix4b4 operator*(f32 lhs,Matrix4b4 rhs);
Matrix4b4 operator*(Matrix4b4 lhs,f32 rhs);


Vector4 operator+(Vector4 lhs,Vector4 rhs);
Vector4 operator-(Vector4 lhs,Vector4 rhs);
Vector4 operator*(f32 lhs,Vector4 rhs);
Vector4 operator*(Vector4 lhs,f32 rhs);
Vector4 operator/(Vector4 lhs,f32 rhs);

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


//FIXME:this one has issues
Vector2 RotateVector(Vector2 vec,f32 rotation);

Vector4 RotateVector(Vector4 vec,Vector4 rotation);

Matrix4b4 Transpose(Matrix4b4 matrix);

Matrix4b4 ViewMatrix(Vector4 position,Vector4 lookpoint,Vector4 updir);
Matrix4b4 ProjectionMatrix(f32 fov,f32 aspectration,f32 nearz,f32 farz);


Matrix4b4 _ainline PositionMatrix(Vector4 position){
    
    Matrix4b4 matrix = {
        {1,0,0,position.x,
            0,1,0,position.y,
            0,0,1,position.z,
            0,0,0,1}
    };
    
    return matrix;
}

Matrix4b4 _ainline RotationMatrix(EuelerAngle rotation){
    
    f32 cosv = cosf(rotation.x);
    f32 sinv = sinf(rotation.x);
    
    Matrix4b4 rotationx_matrix4b4 = {
        {1,0,0,0,
            0,cosv,-sinv,0,
            0,sinv,cosv,0,
            0,0,0,1}
    };
    
    cosv = cosf(rotation.y);
    sinv = sinf(rotation.y);
    
    Matrix4b4 rotationy_matrix4b4 = {
        {cosv,0,sinv,0,
            0,1,0,0,
            -sinv,0,cosv,0,
            0,0,0,1}
    };
    
    cosv = cosf(rotation.z);
    sinv = sinf(rotation.z);
    
    Matrix4b4 rotationz_matrix4b4 = {
        {cosv,-sinv,0,0,
            sinv,cosv,0,0,
            0,0,1,0,
            0,0,0,1}
    };
    
    return rotationz_matrix4b4 * rotationy_matrix4b4 * rotationx_matrix4b4;
}



Matrix4b4 _ainline ScaleMatrix(Vector4 scale){
    
    Matrix4b4 matrix = {
        {scale.x,0,0,0,
            0,scale.y,0,0,
            0,0,scale.z,0,
            0,0,0,1}
    };
    
    return matrix;
}


Matrix4b4 IdentityMatrix4b4();

void PrintMatrix(Matrix4b4 matrix);
void PrintVector4(Vector4 vec);
void PrintVector3(Vector3 vec);
void PrintVector2(Vector2 vec);

Matrix4b4 WorldMatrix(Matrix4b4 position,Matrix4b4 rotation,Matrix4b4 scale);
Matrix4b4 WorldMatrix(Vector4 position,EuelerAngle rotation,Vector4 scale);

//NOTE: Do not use these for now
Matrix4b4 Inverse(Matrix4b4 matrix);
Matrix4b4 operator/(Matrix4b4 lhs,Matrix4b4 rhs);

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

union Quaternion{
    simd4f simd;
    
    struct{
        f32 w,x,y,z;
    };
    
}_align(16);

namespace Quat{
    Quaternion Normalize(Quaternion a);
    f32 Magnitude(Quaternion a);
    f32 Dot(Quaternion a,Quaternion b);
}


Quaternion operator+(Quaternion lhs,Quaternion rhs);
Quaternion operator-(Quaternion lhs,Quaternion rhs);
Quaternion operator*(f32 lhs,Quaternion rhs);
Quaternion operator*(Quaternion lhs,f32 rhs);
Quaternion operator/(Quaternion lhs,f32 rhs);
Quaternion operator*(Quaternion lhs,Quaternion rhs);

Quaternion Inverse(Quaternion q);

//TODO: implement this
Vector3 RotateVector3(Vector3 v,Quaternion q);

Quaternion ConstructQuaternion(Vector3 vector,f32 angle);

Quaternion ConjugateQuaternion(Quaternion quaternion);

void DeconstructQuaternion(Quaternion quaternion,Vector3* vector,f32* angle);


Matrix4b4 QuaternionToMatrix(Quaternion quaternion);

Quaternion MatrixToQuaternion(Matrix4b4 matrix);

Matrix4b4 WorldMatrix(Vector4 position,Quaternion rotation,Vector4 scale);

Quaternion _ainline MQuaternionIdentity(){
    return {1.0f,0.0f,0.0f,0.0f};
}

Quaternion _ainline AQuaternionIdentity(){
    return {0.0f,0.0f,0.0f,0.0f};
}

void PrintQuaternion(Quaternion quat);


Quaternion _ainline CastVectorToQuaternion(Vector4 vector){
    
    Quaternion q;
    
    q.w = vector.x;
    q.x = vector.y;
    q.y = vector.z;
    q.z = vector.w;
    
    return q;
}

Vector4 _ainline CastQuaternionToVector(Quaternion quaternion){
    
    Vector4 v;
    
    v.x = quaternion.w;
    v.y = quaternion.x;
    v.z = quaternion.y;
    v.w = quaternion.z;
    
    return v;
}


Vector3 _ainline MatrixToTranslationVector(Matrix4b4 matrix){
    
    return {matrix _ac(3,0),matrix _ac(3,1),matrix _ac(3,2),1.0f};
}

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

struct DualQuaternion{
    Quaternion q1,q2;
};

DualQuaternion ConstructDualQuaternion(Quaternion rotation,Vector3 translation);

DualQuaternion ConstructDualQuaternion(Matrix4b4 transform);

DualQuaternion operator+(DualQuaternion lhs,DualQuaternion rhs);
DualQuaternion operator-(DualQuaternion lhs,DualQuaternion rhs);
DualQuaternion operator*(DualQuaternion lhs,DualQuaternion rhs);
DualQuaternion operator*(f32 lhs,DualQuaternion rhs);
DualQuaternion operator*(DualQuaternion lhs,f32 rhs);

DualQuaternion Normalize(DualQuaternion d);
Matrix4b4 DualQuaternionToMatrix(DualQuaternion d);


Vector4 WorldSpaceToClipSpace(Vector4 pos,Matrix4b4 viewproj);
Vector4 ClipSpaceToWorldSpace(Vector4 pos,Matrix4b4 viewproj);
