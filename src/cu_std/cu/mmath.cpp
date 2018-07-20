
#define _test_matrices 1

#ifdef DEBUG

#if (_test_matrices)

#define GLM_FORCE_DEPTH_ZERO_TO_ONE 1
#define GLM_FORCE_RIGHT_HANDED 1
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"

void InternalCmpMatrix(f32* f1,f32* f2){
    
    for(u32 i = 0; i < 16; i++){
        
        auto a = f1[i];
        auto b = f2[i];
        
        if(a == 0 && b == 0){
            f1[i] = 0;
            f2[i] = 0;
        }
        
        if(f1[i] - f2[i] > 0.1f){
            printf("--------------\n");
            PrintMatrix(Transpose(*((Matrix4b4*)f1)));
            PrintMatrix(Transpose(*((Matrix4b4*)f2)));
            _kill("",1);
        }
        
    }
    
}

#endif

#endif

Matrix4b4 operator+(Matrix4b4 lhs,Matrix4b4 rhs){
    
    Matrix4b4 matrix;
    
    _storesimd4f(&matrix _ac(0,0),_addsimd4f(lhs.simd[0],rhs.simd[0]));
    
    _storesimd4f(&matrix _ac(0,1),_addsimd4f(lhs.simd[1],rhs.simd[1]));
    
    _storesimd4f(&matrix _ac(0,2),_addsimd4f(lhs.simd[2],rhs.simd[2]));
    
    _storesimd4f(&matrix _ac(0,3),_addsimd4f(lhs.simd[3],rhs.simd[3]));
    
    
    return matrix;
}



Matrix4b4 operator-(Matrix4b4 lhs,Matrix4b4 rhs){
    
    Matrix4b4 matrix;
    
    _storesimd4f(&matrix _ac(0,0),_subsimd4f(lhs.simd[0],rhs.simd[0]));
    
    _storesimd4f(&matrix _ac(0,1),_subsimd4f(lhs.simd[1],rhs.simd[1]));
    
    _storesimd4f(&matrix _ac(0,2),_subsimd4f(lhs.simd[2],rhs.simd[2]));
    
    _storesimd4f(&matrix _ac(0,3),_subsimd4f(lhs.simd[3],rhs.simd[3]));
    
    
    return matrix;
}

//NOTE:Worth unrolling? Transposing followed by a straight multiply is more cache coherrent.
//Might be faster?
Matrix4b4 operator*(Matrix4b4 lhs,Matrix4b4 rhs){
    
    //This managed to shave a few ms. ALinearBlend is doesn't hit 2ms
    Matrix4b4 matrix;
    
    rhs = Transpose(rhs);
    
    for(u32 y = 0; y < 4;y++){
        
        simd4f res1 = _mulsimd4f(lhs.simd[y],rhs.simd[0]);
        simd4f res2 = _mulsimd4f(lhs.simd[y],rhs.simd[1]);
        simd4f res3 = _mulsimd4f(lhs.simd[y],rhs.simd[2]);
        simd4f res4 = _mulsimd4f(lhs.simd[y],rhs.simd[3]);
        
#ifdef _WIN32
        
        f32* _restrict r1 = (f32*)&res1;
        f32* _restrict r2 = (f32*)&res2;
        f32* _restrict r3 = (f32*)&res3;
        f32* _restrict r4 = (f32*)&res4;
        
        matrix _ac(0,y) = r1[0] + r1[1] + r1[2] + r1[3];
        matrix _ac(1,y) = r2[0] + r2[1] + r2[2] + r2[3];
        matrix _ac(2,y) = r3[0] + r3[1] + r3[2] + r3[3];
        matrix _ac(3,y) = r4[0] + r4[1] + r4[2] + r4[3];
        
#else
        
        matrix _ac(0,y) = res1[0] + res1[1] + res1[2] + res1[3];
        matrix _ac(1,y) = res2[0] + res2[1] + res2[2] + res2[3];
        matrix _ac(2,y) = res3[0] + res3[1] + res3[2] + res3[3];
        matrix _ac(3,y) = res4[0] + res4[1] + res4[2] + res4[3];
        
#endif
        
        
    }
    
    return matrix;
}

//MARK: Complete Inverse()
Matrix4b4 operator/(Matrix4b4 lhs,Matrix4b4 rhs){
    
    return lhs * Inverse(rhs);
}


Matrix4b4 operator*(f32 lhs,Matrix4b4 rhs){
    
    Matrix4b4 matrix;
    simd4f k = _setksimd4f(lhs);
    
    simd4f res = _mulsimd4f(rhs.simd[0],k);
    
    _storesimd4f(&matrix _ac(0,0),res);
    
    res = _mulsimd4f(rhs.simd[1],k);
    
    _storesimd4f(&matrix _ac(0,1),res);
    
    res = _mulsimd4f(rhs.simd[2],k);
    
    _storesimd4f(&matrix _ac(0,2),res);
    
    res = _mulsimd4f(rhs.simd[3],k);
    
    _storesimd4f(&matrix _ac(0,3),res);
    
    return matrix;
}

Matrix4b4 operator*(Matrix4b4 lhs,f32 rhs){
    return rhs * lhs;
}

Matrix4b4 Transpose(Matrix4b4 matrix){
    
    Matrix4b4 store_matrix;
    
    simd4f tmp1 = {},
    row0 = {},
    row1 = {},
    row2 = {},
    row3 = {};
    
    f32* src = matrix.container;
    
    tmp1 = 
        _loadasymsimd4f(_loadstorehsimd4f(tmp1,(simd2f*)(src)),(simd2f*)(src+4));
    
    row1 = 
        _loadasymsimd4f(_loadstorehsimd4f(row1,(simd2f*)(src+8)),(simd2f*)(src+12));
    
    row0 = _shufflesimd4f(tmp1, row1, 0x88);
    row1 = _shufflesimd4f(row1, tmp1, 0xDD);
    tmp1 = 
        _loadasymsimd4f(_loadstorehsimd4f(tmp1,(simd2f*)(src+2)),(simd2f*)(src+6));
    row3 = 
        _loadasymsimd4f(_loadstorehsimd4f(row3,(simd2f*)(src+10)),
                        (simd2f*)(src+14));
    
    row2 = _shufflesimd4f(tmp1, row3, 0x88);
    row3 = _shufflesimd4f(row3, tmp1, 0xDD);
    
    _storeusimd4f(&store_matrix _ac(0,0),row0);
    
    
    
    //this is swapped
    row1 = _shufflesimd4f(row1,row1,_MM_SHUFFLE(1,0,3,2));
    _storeusimd4f(&store_matrix _ac(0,1),row1);
    
    _storeusimd4f(&store_matrix _ac(0,2),row2);
    
    //this is swapped
    
    row3 = _shufflesimd4f(row3,row3,_MM_SHUFFLE(1,0,3,2));
    _storeusimd4f(&store_matrix _ac(0,3),row3);
    
    return store_matrix;
}

Matrix4b4 WorldMatrix(Matrix4b4 position,Matrix4b4 rotation,Matrix4b4 scale){
    return position * rotation * scale;
}

Matrix4b4 WorldMatrix(Vector3 position,Vector3 rotation,Vector3 scale){
    
    Matrix4b4 matrix;
    
    Matrix4b4 position_matrix4b4 = PositionMatrix(position);
    
    Matrix4b4 scale_matrix4b4 = ScaleMatrix(scale);
    
    Matrix4b4 rotation_matrix4b4 = RotationMatrix(rotation);
    
    
    matrix = WorldMatrix(position_matrix4b4,rotation_matrix4b4,scale_matrix4b4);
    
    return matrix;
}


//FIXME:this is broken
Matrix4b4 _ainline
ViewMatrixRHS(Vector3 position,Vector3 lookpoint,Vector3 updir){
    
    Vector3 forward = Normalize(lookpoint - position);
    
    Vector3 side = Normalize(Cross(forward,updir));
    
    Vector3 up = Cross(side,forward);
    
    f32 a = -1.0f * Dot(side,position),
    b = -1.0f * Dot(up,position),
    c = Dot(forward,position);
    
    //up z is wrong
    //forward y is wrong
    Matrix4b4 matrix = {
        {side.x,side.y,side.z,a,
            up.x,up.y,up.z,b,
            -forward.x,-forward.y,-forward.z,c,
            0,0,0,1,}
    };
    
    
#ifdef DEBUG
    
#if _test_matrices
    
    
    auto ref_matrix = Transpose(matrix);
    
    auto t_mat =
        glm::lookAt(glm::vec3(position.x,position.y,position.z),
                    glm::vec3(lookpoint.x,lookpoint.y,lookpoint.z),
                    glm::vec3(updir.x,updir.y,updir.z));
    
    auto f1 = (f32*)&ref_matrix;
    auto f2 = (f32*)&t_mat;
    
    InternalCmpMatrix(f1,f2);
    
#endif
    
#endif
    
    
    return matrix;
}

Matrix4b4 ViewMatrix(Vector3 position,Vector3 lookpoint,Vector3 updir){
    
    
#if Z_RHS
    
    return ViewMatrixRHS(position,lookpoint,updir);
    
#else
    
#endif
}

Matrix4b4 ProjectionMatrix(f32 fov,f32 aspectratio,f32 nearz,f32 farz){
    
    f32 tanhalf_fov = tanf(fov/2.0f);
    
    f32 a = (1.0f/(aspectratio * tanhalf_fov));
    f32 b = (1.0f/(tanhalf_fov));
    
#if DEPTH_ZERO_TO_ONE
    
    f32 c = (farz)/(nearz - farz);
    f32 d = -(farz * nearz)/(farz - nearz);
    
#else
    
    f32 c = -(farz + nearz)/(farz - nearz);
    f32 d = -(2.0f * farz * nearz)/(farz - nearz);
    
#endif
    
    Matrix4b4 matrix = {
        {a,0,0,0,
            0,b,0,0,
            0,0,c,d,
            0,0,-1.0f,0}
    };
    
    
#ifdef DEBUG
    
#if _test_matrices
    
    auto ref_matrix = Transpose(matrix);
    
    auto t_mat =
        glm::perspective(fov,aspectratio,nearz,farz);
    
    auto f1 = (f32*)&ref_matrix;
    auto f2 = (f32*)&t_mat;
    
    for(u32 i = 0; i < 16; i++){
        
        if(f1[i] != f2[i]){
            PrintMatrix(*((Matrix4b4*)f1));
            PrintMatrix(*((Matrix4b4*)f2));
            _kill("do not match\n",1);
            
        }
        
    }
    
#endif
    
#endif
    
    return matrix;
}

Vector4 operator+(Vector4 lhs,Vector4 rhs){
    
    Vector4 vec;
    
    simd4f res = _addsimd4f(lhs.simd,rhs.simd);
    
    _storeusimd4f(&vec.x,res);
    
    return vec;
}

Vector4 operator-(Vector4 lhs,Vector4 rhs){
    Vector4 vec;
    
    simd4f res = _subsimd4f(lhs.simd,rhs.simd);
    
    _storeusimd4f(&vec.x,res);
    
    return vec;
}

Vector4 operator*(f32 lhs,Vector4 rhs){
    Vector4 vec;
    
    simd4f k = _setksimd4f(lhs);
    
    simd4f res = _mulsimd4f(rhs.simd,k);
    
    _storeusimd4f(&vec.x,res);
    
    return vec;
}

Vector4 operator*(Vector4 lhs,f32 rhs){
    return rhs * lhs;
}


Vector4 operator/(Vector4 lhs,f32 rhs){
    
    Vector4 vec;
    simd4f k = _setksimd4f(rhs);
    
    simd4f res = _divsimd4f(lhs.simd,k);
    
    _storeusimd4f(&vec.x,res);
    
    return vec;
}




Vector3 operator+(Vector3 lhs,Vector3 rhs){
    
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    lhs.z += rhs.z;
    
    return lhs;
}

Vector3 operator-(Vector3 lhs,Vector3 rhs){
    
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;
    lhs.z -= rhs.z;
    
    return lhs;
}

Vector3 operator*(f32 lhs,Vector3 rhs){
    
    rhs.x *= lhs;
    rhs.y *= lhs;
    rhs.z *= lhs;
    
    return rhs;
}

Vector3 operator*(Vector3 lhs,f32 rhs){
    return rhs * lhs;
}


Vector3 operator/(Vector3 lhs,f32 rhs){
    
    lhs.x *= rhs;
    lhs.y *= rhs;
    lhs.z *= rhs;
    
    return lhs;
}


f32 Magnitude(Vector3 vec){
    
    //m = sqrt(x^2 * y^2 * z^2)
    
    f32 res = Dot(vec,vec);
    
    res = sqrtf(res);
    
    return res;
}


//TODO: Could be better (Use shuffle)
Vector3 Cross(Vector3 vec1,Vector3 vec2){
    /* a(x,y,z) b(x,y,z)
       cx = aybz - azby
       cy = azbx - axbz
       cz = axby - aybx
       
    */
    
    Vector3 vec;
    
    simd4f a1 = _setsimd4f(vec1.y,vec1.z,vec1.x,0.0f)
        ,a2 = _setsimd4f(vec1.z,vec1.x,vec1.y,0.0f);
    
    simd4f b1 = _setsimd4f(vec2.z,vec2.x,vec2.y,0.0f)
        ,b2 = _setsimd4f(vec2.y,vec2.z,vec2.x,0.0f);
    
    simd4f res = _subsimd4f(_mulsimd4f(a1,b1),_mulsimd4f(a2,b2));
    
    
    //FIMXE: pretty sure this is wrong
    //_storeusimd4f(&vec.x,res);
    
    vec.x = res[0];
    vec.y = res[1];
    vec.z = res[2];
    
    return vec;
}

f32 Dot(Vector3 vec1,Vector3 vec2){
    //|a| * |b| * cos(angle between a and b)
    //or for a(1,2,3....n) and b(1,2,3....n). a.b = a1b1 + a2b2+ ...anbn
    
    vec1.x *= vec2.x;
    vec1.y *= vec2.y;
    vec1.z *= vec2.z;
    
    return vec1.x + vec1.y + vec1.z;
}

f32 cosf(Vector3 vec1,Vector3 vec2){
    return Dot(vec1,vec2)/(Magnitude(vec1) * Magnitude(vec2));
}

Vector3 Normalize(Vector3 vec){
    return (vec)/(Magnitude(vec));
}

f32 Vec4::Dot(Vector4 vec1,Vector4 vec2){
    //|a| * |b| * cos(angle between a and b)
    //or for a(1,2,3....n) and b(1,2,3....n). a.b = a1b1 + a2b2+ ...anbn
    
    simd4f mul = _mulsimd4f(vec1.simd,vec2.simd);
    
#ifdef _WIN32
    
    f32* _restrict r = (f32*)&mul;
    
    f32 res = r[0] + r[1] + r[2]+ r[3];
    
#else
    f32 res = mul[0] + mul[1] + mul[2] + mul[3];
#endif
    
    
    
    return res;
}

f32 Vec4::Magnitude(Vector4 vec){
    
    //m = sqrt(x^2 + y^2 + z^2)
    
    f32 res = Dot(vec,vec);
    
    res = sqrtf(res);
    
    return res;
}

Vector4 Vec4::Normalize(Vector4 vec){
    return (vec)/(Magnitude(vec));
}

Vector4 Vec4::VectorComponentMul(Vector4 a,Vector4 b){
    
    a.simd = _mulsimd4f(a.simd,b.simd);
    
    return a;
}

Vector4 InternalComponentDiv(Vector4 a,Vector4 b){
    a.simd = _divsimd4f(a.simd,b.simd);
    return a;
}

Vector3 GetVectorRotation(Vector3 lookat){
    
    //updown,leftright,roll left roll right
    f32 x,y,z;
    
    x = AngleQuadrant(lookat.z,lookat.y);
    
    y = AngleQuadrant(lookat.x,lookat.z);
    
    z = AngleQuadrant(lookat.x,lookat.y);
    
    return Vector3{x,y,z};  
}

Vector3 Vec3(f32 x,f32 y,f32 z){
    
    return Vector3{x,y,z};
}

f32 Component(Vector3 a,Vector3 b){
    return Dot(a,Normalize(b));
}

Vector3 ProjectOnto(Vector3 a,Vector3 b){
    return Component(a,b) * Normalize(b);
}

f32 AngleQuadrant(f32 x,f32 y){
    
    if(x == 0 && y == 0){
        return 0.0f;
    }
    
    f32 teeter = atan2f(x,y);
    
    
#if 0
    
    if(teeter < 0){
        teeter += _twopi;
    }
    
#endif
    
    return teeter;
}

Vector2 RotateVector(Vector2 vec,f32 rotation){
    
    vec.x = (vec.x * cosf(rotation)) - (vec.y * sinf(rotation));
    vec.y = (vec.x * sinf(rotation)) + (vec.y * cosf(rotation));
    
    return vec;
}

Vector4 RotateVector(Vector4 vec,Vector3 rotation){
    
    /*
    
      Multiply rotation matrix by vec
      
      {rotated x,  = {cos0 ,-sin0,  * {x,
      rotated y}      sin0,cos0}       y}
    */
    
    
    //NOTE: This is not the most efficiet method. add 2x2 matrices
    
    Matrix4b4 rot_matrix = RotationMatrix(rotation);
    
    Matrix4b4 vec_matrix = {
        {vec.x,0,0,0,
            vec.y,0,0,0,
            vec.z,0,0,0,
            1,0,0,0}
    };
    
    Matrix4b4 ret_matrix = rot_matrix * vec_matrix;
    
    vec.x = ret_matrix _ac(0,0);
    vec.y = ret_matrix _ac(0,1);
    vec.z = ret_matrix _ac(0,2);
    vec.w = ret_matrix _ac(0,3);
    
    return vec/vec.w;
}


Matrix4b4 IdentityMatrix4b4(){
    
    Matrix4b4 matrix = {
        {1,0,0,0,
            0,1,0,0,
            0,0,1,0,
            0,0,0,1}
    };
    
    
    return matrix;
}

void PrintMatrix(Matrix4b4 matrix){
    
    printf("\n");
    
    for(u32 i = 0; i < 16;i++){
        
        if(i % 4 == 0)
            printf("\n");
        
        printf("%f   ",matrix[i]);
        
    }
    
    printf("\n");
    
}

void PrintMatrix(Matrix3b3 matrix){
    printf("\n");
    
    for(u32 i = 0; i < 9;i++){
        
        if(i % 3 == 0)
            printf("\n");
        
        printf("%f   ",matrix[i]);
        
    }
    
    printf("\n");
}

void PrintVector4(Vector4 vec){
    printf("%f   %f   %f   %f\n",vec.x,vec.y,vec.z,vec.w);
}

void PrintVector3(Vector3 vec){
    printf("%f   %f   %f\n",vec.x,vec.y,vec.z);
}

void PrintVector2(Vector2 vec){
    printf("%f   %f\n",vec.x,vec.y);
}

void PrintQuaternion(Quaternion vec){
    printf("%f   %f   %f   %f\n",vec.w,vec.x,vec.y,vec.z);
}

Matrix4b4 CompMul(Matrix4b4 a,Matrix4b4 b){
    
    Matrix4b4 matrix;
    
    for(u32 i = 0; i < 4; i++){
        matrix.simd[i] = _mulsimd4f(a.simd[i],b.simd[i]);
    }
    
    
    return matrix;
}

void _ainline GetMinorMatrix(f32* in_matrix,u32 n,u32 k_x,u32 k_y,f32* out_matrix){
    
    u32 index = 0;
    
    for(u32 y = 0; y < n; y++){
        
        for(u32 x = 0; x < n; x++){
            
            if(y != k_y && x != k_x){
                out_matrix[index] = in_matrix[(y * n) + x];
                index++;
            }
            
        }
        
    }
    
}


f32 inline GenericGetDeterminant(f32* in_matrix,u32 n){
    
    _kill("we do not support this case\n",n > 4);
    
    if(n == 2){
        
        f32 a = in_matrix[0];
        f32 b = in_matrix[1];
        f32 c = in_matrix[2];
        f32 d = in_matrix[3];
        
        return (a * d) - (b * c);
    }
    
    f32 res = 0;
    
    for(u32 i = 0; i < n; i++){
        
        auto entry = in_matrix[i];
        
        if(i & 1){
            entry *= -1.0f;
        }
        
        f32 minor_mat[16] = {};
        
        GetMinorMatrix(in_matrix,n,i,0,&minor_mat[0]);
        
        auto det = GenericGetDeterminant(&minor_mat[0],n - 1);
        
        res += det * entry;
        
    }
    
    return res;
}


Matrix4b4 Inverse(Matrix4b4 matrix){
    
    /*
      Computes the inverse matrix using the adjugate formula
      inverse A = 1/(det(A)) x adj(A)
      
      where det is the determinant of the matrix and adj the adjoint matrix of A.
      where 1 <= i <= n and 1 <= j <= n of an n by n matrix and (i,j) corresponds to a value of matrix A
      adjoint_matrix of elements = -(-1)^(i * j) * det(MinorMatrix(matrix,i,j))
      adj(A) = Transpose(adjoint_matrix)
    */
    
    
    
    //we'll use these to get the det. Calling generic det will compute the first row minor matrix det twice
    f32 first_row[] = {matrix _ac(0,0),matrix _ac(1,0),matrix _ac(2,0),matrix _ac(3,0)};
    
    //get the adjoint matrix, create matrices of cofactors
    
    Matrix4b4 adj_matrix = {};
    
    for(u32 y = 0; y < 4; y++){
        
        for(u32 x = 0; x < 4; x++){
            
            Matrix3b3 minormatrix = {};
            
            GetMinorMatrix((f32*)&matrix[0],4,x,y,(f32*)&minormatrix);
            
            adj_matrix _ac(x,y) = GenericGetDeterminant((f32*)&minormatrix,3);
        }
        
    }
    
    Matrix4b4 checkerboard_matrix = {
        1,-1,1,-1,
        -1,1,-1,1,
        1,-1,1,-1,
        -1,1,-1,1,
    };
    
    adj_matrix = CompMul(adj_matrix,checkerboard_matrix);
    
    f32 det = 0;
    
    for(u32 i = 0; i < 4; i++){
        det += first_row[i] * adj_matrix _ac(i,0);
    }
    
    adj_matrix = (1.0f/det) * Transpose(adj_matrix);
    
    return adj_matrix;
}



//Geometry

void MinkowskiAddition(Point3* a,ptrsize a_count,Point3* b,ptrsize b_count,Point3** ret){
    
    u32 watch = 0;
    
    for(u32 i = 0; i < a_count;i++){
        
        for(u32 j = 0; j < b_count;j++){
            
            watch = j + (i * (b_count));
            
            (*ret)[watch ] = a[i] + b[j];
            
        }
        
    }
    
}

void MinkowskiDifference(Point3* a,ptrsize a_count,Point3* b,ptrsize b_count,Point3** ret){
    
    u32 watch = 0;
    
    for(u32 i = 0; i < a_count;i++){
        
        for(u32 j = 0; j < b_count;j++){
            
            watch = j + (i * (b_count));
            
            (*ret)[watch ] = a[i] - b[j];
            
        }
        
    }
    
}


Quaternion ConstructQuaternion(Vector3 vector,f32 angle){
    
    vector = Normalize(vector);
    
    Quaternion quaternion;
    
    f32 k = sinf(angle/2.0f);
    
    vector.x *= k;
    vector.y *= k;
    vector.z *= k;
    
    quaternion.w = cosf(angle/2.0f);
    quaternion.x = vector.x;
    quaternion.y = vector.y;
    quaternion.z = vector.z;
    
    return quaternion;
}


Quaternion ConjugateQuaternion(Quaternion quaternion){
    
    simd4f k = _setksimd4f(-1.0f);
    
    quaternion.simd = _mulsimd4f(quaternion.simd,k);
    
    quaternion.w *= -1.0f;
    
    return quaternion;
}


Quaternion operator*(Quaternion lhs,Quaternion rhs){
    
    /*    
   x = w1x2 + x1w2 + y1z2 - z1y2
   y = w1y2 + y1w2 + z1x2 - x1z2
   z = w1z2 + z1w2 + x1y2 - y1x2
   w=  w1w2 - x1x2 - y1y2 - z1z2
    */
    
    Quaternion quaternion;
    
    quaternion.x = (lhs.w * rhs.x) + (lhs.x * rhs.w) + (lhs.y * rhs.z) - (lhs.z * rhs.y);
    quaternion.y = (lhs.w * rhs.y) + (lhs.y * rhs.w) + (lhs.z * rhs.x) - (lhs.x * rhs.z);
    quaternion.z = (lhs.w * rhs.z) + (lhs.z * rhs.w) + (lhs.x * rhs.y) - (lhs.y * rhs.x);
    quaternion.w = (lhs.w * rhs.w) - (lhs.x * rhs.x) - (lhs.y * rhs.y) - (lhs.z * rhs.z);
    
    return quaternion;
}

void DeconstructQuaternion(Quaternion quaternion,Vector3* vector,f32* angle){
    
    f32 anglew = acos(quaternion.w) * 2.0f;
    
    f32 scale = sinf(anglew);
    
    //we should handle the case scale == 0
    if(scale == 0){
        *vector = Vector3{1,0,0};
        *angle = 0;
        return;
    }
    vector->x = quaternion.w / scale;
    vector->y = quaternion.x / scale;
    vector->z = quaternion.y / scale;
    
    *angle = anglew;
}


Matrix4b4 QuaternionToMatrix(Quaternion quaternion){
    
    Quaternion squared;
    
    squared.simd = _mulsimd4f(quaternion.simd,quaternion.simd);
    
    f32 a = 1 - (2 * (squared.y + squared.z));
    f32 b = ((quaternion.x * quaternion.y) - (quaternion.w * quaternion.z)) * 2;
    f32 c = ((quaternion.x * quaternion.z) + (quaternion.w * quaternion.y)) * 2;
    
    
    
    f32 d = ((quaternion.x * quaternion.y) + (quaternion.w * quaternion.z)) * 2;
    f32 e = 1 - (2 * (squared.x + squared.z));
    f32 f = ((quaternion.y * quaternion.z) - (quaternion.w * quaternion.x)) * 2;
    
    
    f32 g = ((quaternion.x * quaternion.z) - (quaternion.w * quaternion.y)) * 2;
    f32 h = ((quaternion.y * quaternion.z) + (quaternion.w * quaternion.x)) * 2;
    f32 i = 1 - (2 * (squared.x + squared.y));
    
    Matrix4b4 matrix =
    {
        a,b,c,0,
        d,e,f,0,
        g,h,i,0,
        0,0,0,1,
    };
    
    return matrix;
}

Quaternion MatrixToQuaternion(Matrix4b4 matrix){
    
#if 0
    
    Quaternion quaternion;
    
    f64 w = sqrt(1.0f + matrix _ac(0,0) + matrix _ac(1,1) + matrix _ac(2,2))/2.0f;
    
    quaternion.w = (f32)w;
    
    w *= 4.0;
    
    quaternion.x = (matrix _ac(1,2) - matrix _ac(2,1))/w;
    quaternion.y = (matrix _ac(2,0) - matrix _ac(0,2))/w;
    quaternion.z = (matrix _ac(0,1) - matrix _ac(1,0))/w;
    
    return quaternion;
    
#else
    
    Quaternion q;
    
    f32 trs = matrix _ac(0,0) + matrix _ac(1,1) + matrix _ac(2,2);
    
    if(trs > 0.0f){
        f32 s = sqrt(trs + 1.0f) * 2.0f;
        q.w = 0.25f * s;
        q.x = (matrix _ac(1,2) - matrix _ac(2,1))/s;
        q.y = (matrix _ac(2,0) - matrix _ac(0,2))/s;
        q.z = (matrix _ac(0,1) - matrix _ac(1,0))/s;    
    }
    
    else if((matrix _ac(0,0) > matrix _ac(1,1)) && (matrix _ac(0,0) > matrix _ac(2,2))){
        f32 s = sqrt(1.0f + matrix _ac(0,0) - matrix _ac(1,1) - matrix _ac(2,2)) * 2.0f;
        q.w = (matrix _ac(1,2) -  matrix _ac(2,1)) /s;
        q.x = 0.25f * s;
        q.y = (matrix _ac(1,0) + matrix _ac(1,0))/s;
        q.z = (matrix _ac(2,0) + matrix _ac(0,2))/s;        
    }
    
    else if(matrix _ac(1,1) > matrix _ac(2,2)){
        f32 s = sqrt(1.0f + matrix _ac(1,1) - matrix _ac(0,0) - matrix _ac(2,2)) * 2.0f;
        q.w = (matrix _ac(2,0) -  matrix _ac(0,2)) /s;
        q.x = (matrix _ac(1,0) + matrix _ac(1,0))/s;
        q.y = 0.25f * s;
        q.z = (matrix _ac(2,1) + matrix _ac(1,2))/s;            
    }
    
    else{
        f32 s = sqrt(1.0f + matrix _ac(2,2) - matrix _ac(0,0) - matrix _ac(1,1)) * 2.0f;
        q.w = (matrix _ac(0,1) -  matrix _ac(1,0)) /s;
        q.x = (matrix _ac(2,0) + matrix _ac(0,2))/s;
        q.y = (matrix _ac(2,1) + matrix _ac(1,2))/s;
        q.z = 0.25f * s;
    }
    
    return Normalize(q);
    
#endif
}


Matrix4b4 WorldMatrix(Vector3 position,Quaternion rotation,Vector3 scale){
    
    Matrix4b4 matrix;
    
    Matrix4b4 position_matrix4b4 = PositionMatrix(position);
    
    Matrix4b4 scale_matrix4b4 = ScaleMatrix(scale);
    
    Matrix4b4 rotation_matrix4b4 = QuaternionToMatrix(rotation);
    
    
    matrix = WorldMatrix(position_matrix4b4,rotation_matrix4b4,scale_matrix4b4);
    
    return matrix;
}

Quaternion _ainline Neighbourhood(Quaternion a,Quaternion b){
    
    f32 dot = Dot(a,b);
    
    //neighbourhood operator.
    if(dot < 0.0f){
        b = b * -1.0f;
    }
    
    return b;
}

Quaternion NLerp(Quaternion a,Quaternion b,f32 step){
    
    b = Neighbourhood(a,b);
    
    Quaternion q = InterpolateQuaternion(a,b,step);
    
    q = Normalize(q);
    
    return q; 
}

Quaternion SLerp(Quaternion a,Quaternion b,f32 step){
    
    f32 dot = _clamp(Dot(a,b),-1.0f, 1.0f);
    
    f32 angle = acos(dot) * step;
    
    Quaternion n =  Normalize(b - (a * dot));
    
    return (n * sinf(angle)) + (a * cosf(angle));
}

DualQuaternion ConstructDualQuaternion(Quaternion rotation,Vector3 translation){
    
    /*
      real = rotation
      dual = 0.5 * translation * rotation
    */
    
    DualQuaternion d;
    d.q1 = rotation;
    d.q2 = Quaternion{0,translation.x,translation.y,translation.z} * d.q1 * 0.5f;
    return d;
}

DualQuaternion ConstructDualQuaternion(Matrix4b4 transform){
    DualQuaternion d;
    d.q1 = MatrixToQuaternion(transform);
    Vector3 translation = MatrixToTranslationVector(transform);
    d.q2 = Quaternion{0,translation.x,translation.y,translation.z} * d.q1 * 0.5f;
    return d;
}

DualQuaternion operator+(DualQuaternion lhs,DualQuaternion rhs){
    lhs.q1 = lhs.q1 + rhs.q1;
    lhs.q2 = lhs.q2 + rhs.q2;
    return lhs;
}

DualQuaternion operator-(DualQuaternion lhs,DualQuaternion rhs){
    lhs.q1 = lhs.q1 - rhs.q1;
    lhs.q2 = lhs.q2 - rhs.q2;
    return lhs;
}

DualQuaternion operator*(DualQuaternion lhs,DualQuaternion rhs){
    DualQuaternion d;
    d.q1 = lhs.q1 * rhs.q1;
    d.q2 = (lhs.q2 * rhs.q1) + (lhs.q1 * rhs.q2);
    return d;
}

DualQuaternion operator*(f32 lhs,DualQuaternion rhs){
    rhs.q1 = rhs.q1 * lhs;
    rhs.q2 = rhs.q2 * lhs;
    return rhs;
}

DualQuaternion operator*(DualQuaternion lhs,f32 rhs){
    return rhs * lhs;
}


DualQuaternion Normalize(DualQuaternion d){
    
    f32 magnitude = Dot(d.q1,d.q1);
    
    _kill("dq normalize error\n",magnitude <= 0.000001f);
    
    d.q1 = d.q1/magnitude;
    d.q2 = d.q2/magnitude;
    
    return d;
}

Matrix4b4 DualQuaternionToMatrix(DualQuaternion d){
    
    d = Normalize(d);
    
    Matrix4b4 matrix = QuaternionToMatrix(d.q1);
    Quaternion t = d.q2 * 2.0f;
    t = t * ConjugateQuaternion(d.q1);
    
    matrix _ac(3,0) = t.x;
    matrix _ac(3,1) = t.y;
    matrix _ac(3,2) = t.z;
    matrix _ac(3,3) = 1.0f;
    
    return matrix;
}

Vector2 operator+(Vector2 lhs,Vector2 rhs){
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    return lhs;
}

Vector2 operator-(Vector2 lhs,Vector2 rhs){
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;
    return lhs;
}


Vector2 operator*(f32 lhs,Vector2 rhs){
    rhs.x *= lhs;
    rhs.y *= lhs;
    return rhs;
}

Vector2 operator*(Vector2 lhs,f32 rhs){
    return rhs * lhs;
}

Vector2 operator/(Vector2 lhs,f32 rhs){
    lhs.x /= rhs;
    lhs.y /= rhs;
    return lhs;
}

f32 Magnitude(Vector2 a){
    
    a.x = a.x * a.x;
    a.y = a.y * a.y;
    
    return sqrt(a.x + a.y); 
}

Vector2 CompMul(Vector2 a,Vector2 b){
    a.x *= b.x;
    a.y *= b.y;
    return a;
}

f32 Dot(Vector2 a,Vector2 b){
    a = CompMul(a,b);
    return a.x + a.y;
}


Vector2 Normalize(Vector2 a){
    return a/Magnitude(a);
}


Vector4 WorldSpaceToClipSpace(Vector4 pos,Matrix4b4 viewproj){
    
    Matrix4b4 vertmat = {
        {pos.x,0,0,0,
            pos.y,0,0,0,
            pos.z,0,0,0,
            1,0,0,0}
    };
    
    auto mat = viewproj * vertmat;
    
    pos.x = mat _ac(0,0);
    pos.y = mat _ac(0,1);
    pos.z = mat _ac(0,2);
    pos.w = mat _ac(0,3);
    
    pos = pos/pos.w;
    
    return pos;
}

Vector4 ClipSpaceToWorldSpace(Vector4 pos,Matrix4b4 viewproj){
    
    auto inv_viewproj = Inverse(viewproj);
    
    Matrix4b4 vertmat = {
        {pos.x,0,0,0,
            pos.y,0,0,0,
            pos.z,0,0,0,
            pos.w,0,0,0}
    };
    
    auto mat = inv_viewproj * vertmat;
    
    pos.x = mat _ac(0,0);
    pos.y = mat _ac(0,1);
    pos.z = mat _ac(0,2);
    pos.w = mat _ac(0,3);
    
    return pos/pos.w;
}


Vector3 ProjectVectorOntoPlane(Vector3 vec,Plane plane){
    auto w = ProjectOnto(vec,plane.norm);
    return vec - w;
}


#define _f32_error_offset  0.0001f

logic Intersect(Line3 a,Line3 b){
    
    /*
      the form of a vector as a line is as follows:
      r = p + sd
      r is any point on the line (var)
      p is a specific point of the line (const)
      d is the dir of the line relative to point p (const)
      s is the scale factor of dir. (var)
      
      s functionally moves r along the line by scaling dir and adding it to point p. 
      
      if 2 lines intersect, a == b
      
      a.pos + (t * a.dir) = b.pos + (k * b.dir)
      (t * a.dir) = (b.pos - a.pos) + (k * b.dir)
      
      crossing both sides by b.dir gets rid of the (k * b.dir) (cross(x,x) = 0), leaving
      
      t * cross(a.dir,b.dir) = cross((b.pos - a.pos),b.dir)
      
      if they are the same then norm(cross(a.dir,b.dir)) = norm(cross((b.pos - a.pos),b.dir)) are the same or
      norm(cross(a.dir,b.dir)) = norm(cross((b.pos - a.pos),b.dir))
      
    */
    
    auto cross_ab = Cross(a.dir,b.dir);
    auto cross_diff = Cross(b.pos - a.pos,b.dir);
    
    auto dot = Dot(Normalize(cross_ab),Normalize(cross_diff));
    
    return (u32)(fabsf(dot) + _f32_error_offset);
}

logic Intersect(Line3 a,Line3 b,Point3* out_point){
    
    auto cross_ab = Cross(a.dir,b.dir);
    auto cross_diff = Cross(b.pos - a.pos,b.dir);
    
    auto dot = Dot(Normalize(cross_ab),Normalize(cross_diff));
    
    if(!(u32)(fabsf(dot) + _f32_error_offset)){
        return false;
    }
    
    f32 t = 0.0f;
    
    for(u32 i = 0; i < 3; i++){
        
        m32 fi1;
        
        fi1.f = cross_ab.floats[i];
        
        if(fi1.i){
            t = cross_diff.floats[i]/cross_ab.floats[i];
            break;
        }
        
    }
    
    *out_point = (a.dir * t) + a.pos;
    
    return true;
}

logic TypedIntersect(Line3 a,Line3 b){
    
    auto dir = Normalize(b.pos - a.pos);//checks if the lines are the same
    
    logic res =
        (u32)(fabsf(Dot(Normalize(a.dir),dir)) + _f32_error_offset) +
        Intersect(a,b);
    
    if(res){
        return res;
    }
    
    return false;
}

//TODO: this is very inefficient. we can solve this in 2d space easier
logic Intersect(Line2 a_2,Line2 b_2){
    
    Line3 a = {Vector3{a_2.pos.x,a_2.pos.y,1.0f},Vector3{a_2.dir.x,a_2.dir.y,1.0f}};
    Line3 b = {Vector3{b_2.pos.x,b_2.pos.y,1.0f},Vector3{b_2.dir.x,b_2.dir.y,1.0f}};
    
    auto cross_ab = Cross(a.dir,b.dir);
    auto cross_diff = Cross(b.pos - a.pos,b.dir);
    
    auto dot = Dot(Normalize(cross_ab),Normalize(cross_diff));
    
    return (u32)(fabsf(dot) + _f32_error_offset);
}

//TODO: this is very inefficient. we can solve this in 2d space easier
logic Intersect(Line2 a_2,Line2 b_2,Point2* out_point){
    
    Line3 a = {Vector3{a_2.pos.x,a_2.pos.y,1.0f},Vector3{a_2.dir.x,a_2.dir.y,1.0f}};
    Line3 b = {Vector3{b_2.pos.x,b_2.pos.y,1.0f},Vector3{b_2.dir.x,b_2.dir.y,1.0f}};
    
    auto cross_ab = Cross(a.dir,b.dir);
    auto cross_diff = Cross(b.pos - a.pos,b.dir);
    
    auto dot = Dot(Normalize(cross_ab),Normalize(cross_diff));
    
    if(!(u32)(fabsf(dot) + _f32_error_offset)){
        return false;
    }
    
    f32 t = 0.0f;
    
    for(u32 i = 0; i < 3; i++){
        
        m32 fi1;
        
        fi1.f = cross_ab.floats[i];
        
        if(fi1.i){
            t = cross_diff.floats[i]/cross_ab.floats[i];
            break;
        }
        
    }
    
    *out_point = (a_2.dir * t) + a_2.pos;
    
    return true;
}

logic TypedIntersect(Line2 a_2,Line2 b_2){
    
    Line3 a = {Vector3{a_2.pos.x,a_2.pos.y,1.0f},Vector3{a_2.dir.x,a_2.dir.y,1.0f}};
    Line3 b = {Vector3{b_2.pos.x,b_2.pos.y,1.0f},Vector3{b_2.dir.x,b_2.dir.y,1.0f}};
    
    auto dir = Normalize(b.pos - a.pos);//checks if the lines are the same
    
    logic res =
        (u32)(fabsf(Dot(Normalize(a.dir),dir)) + _f32_error_offset) +
        Intersect(a,b);
    
    if(res){
        return res;
    }
    
    return false;
}

logic Intersect(Line3 a,Plane b){
    
    m32 fi1;
    
    //if they are perpendicular, f is 0 and they do not intersect
    fi1.f = Dot(Normalize(a.dir),Normalize(b.norm));
    
    return fi1.i != 0;
}

logic TypedIntersect(Line3 a,Plane b){
    
    /*
      if the dir of a line is perpendicular to a plane normal, it will never intersect unless the line is on the
      plane
    */
    
    auto is_intersect = Intersect(a,b);
    
    m32 fi;
    
    auto dir = Normalize(b.pos - a.pos);
    
    fi.f = Dot(dir,Normalize(b.norm));//check if on the plane
    
    auto is_perpendicular = !fi.i;
    
    if(!is_intersect && is_perpendicular){
        return INTERSECT_INFINITE;
    }
    
    else if(!is_intersect){
        return INTERSECT_FALSE;
    }
    
    return INTERSECT_FINITE;
}

logic Intersect(Line3 a,Plane b,Point3* out_point){
    
    /*
      we first imagine a line (A) from our plane normal position (B) to any position on our line (L).
      the angle between the normal (N) and the line (A) will be known as (d). As the line (A) moves along
      the line (L), approaching the point of intersection between line (L) and the plane, the angle (d)
      will approach 90 degrees. So it is given that the dot product of line (A) and the normal line (N)
      will be 0 at the point of intersection. Given the vector from of a line:
      r = P + t * dir
      where r is any point on the line, p is a known point on the line, t is the scale factor to the direction 
      vector and dir is the direction vector, we can say:
      
      dot((P + (t * dir) - B),N) = 0
      which can be rewritten to:
      t = -(dot((P - B),N))/(dot(dir,N))
      or
      t = (dot((B - P),N))/(dot(dir,N))
    */
    
    
    if(!Intersect(a,b)){
        return false;
    }
    
    auto n = Normalize(b.norm);
    auto dir = Normalize(a.dir);
    
    auto t = (Dot((b.pos - a.pos),n))/(Dot(dir,n));
    
    *out_point = a.pos + (t * dir);
    
    return true;
}

Vector3 RotateVector3(Vector3 v,Quaternion q){
    
    /*
      the rotated vector v' is given by
      v' = q * v * inverse(q)
      we are gonna optimize this case so we're not gonna do this verbatim
    */
    
#if 1
    
    //better version? we need to condense the math
    Vector3 q_v = {q.x,q.y,q.z};
    f32 q_w =q.w;
    
    auto r = (2.0f * Dot(q_v,v) * q_v) + (((q_w * q_w) - Dot(q_v,q_v)) * v) + (2.0f * q_w * Cross(q_v,v));
    
    return r;
    
#else
    
    //non-optimal (verbatim formula above)
    Quaternion t = {0.0f,v.x,v.y,v.z};
    auto q_inv = Inverse(q);
    
    auto k = q * t * q_inv;
    
    return Vector3{k.x,k.y,k.z,1.0f};
    
#endif
}


Quaternion Inverse(Quaternion q){
    
    q.w *= -1.0f;
    
    return q * -1.0f;
}