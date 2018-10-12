#pragma once

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef  char s8;
typedef  short s16;
typedef  int s32;
typedef  long long s64;

typedef char logic; //MARK:should we make this 4 bytes long?

typedef float f32;
typedef double f64;

#if __x86_64__ || _M_X64

typedef unsigned long long ptrsize;

#else

typedef unsigned int ptrsize;

#endif


union m32{
    
    u32 u;
    s32 i;
    f32 f;
    
    operator u32(){
        
        return u;
    }
    
    operator s32(){
        
        return i;
    }
    
    operator f32(){
        
        return f;
    }
    
    void operator=(u32 o){
        u = o;
    }
    
    void operator=(f32 o){
        f= o;
    }
};

union m64{
    
    u64 u;
    s64 i;
    f64 f;
    
    
    union{
        
        void* ptr;
        
        struct{
            u32 pad1;
            u16 pad2;
            u16 top16_bits;
        };
    };
    
    
    m32 array[2];
    
    m32& operator [](ptrsize index){
        
        return array[index];
    }
    
    operator u64(){
        
        return u;
    }
    
    operator s64(){
        
        return i;
    }
    
    operator f64(){
        
        return f;
    }
    
    void operator=(u64 o){
        u= o;
    }
    
    void operator=(f64 o){
        f= o;
    }
};

#define _writetop16(m64t,value) (m64t.u | (((u64)value) << 48))
#define _gettop16(m64t) (m64t.u >> 48)
#define _masktop16(m64t)  (m64t.u ^ 0xFFFF000000000000)
#define _zerotop16(m64t) (m64t.u ^= 0xFFFF000000000000)

void _ainline ClearPtrTop16Bits(void** ptr){
    
    m64 v ={};
    v.ptr = *ptr;
    
    v.u ^= 0xFFFF000000000000;
    
    *ptr = v.ptr;
    
}

void _ainline WritePtrTop16Bits(void** ptr,u16 value){
    
    ClearPtrTop16Bits(ptr);
    
    m64 v ={};
    v.ptr = *ptr;
    
    v.u |= ((u64)value) << 48;
    
    *ptr = v.ptr;
};

u16 _ainline GetPtrTop16Bits(void* ptr){
    
    m64 v ={};
    v.ptr = ptr;
    
    return (v.u >> 48);
}

_ainline void* MaskPtrTop16Bits(void* ptr){
    
    ClearPtrTop16Bits(&ptr);
    
    return ptr;
}