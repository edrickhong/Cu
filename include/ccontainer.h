#pragma  once

#include "mode.h"
#include "ttype.h"

#include "aallocator.h"

#define _defaultcount 20
#define _growcount 10

//WARNING:salloc has a very limited scope.
//TODO: just the functions local to the struct


#define _declare_list(name,type) struct name{				\
    type* container = 0;							\
    ptrsize count = 0;							\
    ptrsize reserve_count = 0;						\
    type& operator [](ptrsize index){					\
        _kill("",index >= count)						\
        return container[index];					\
    }									\
    void Init(ptrsize rcount = _defaultcount){				\
        _kill("",rcount == 0);						\
        container = (type*)alloc(rcount * sizeof(type));			\
        reserve_count = rcount;						\
        count = 0;							\
    }									\
    void PushBack(type element){					\
        count ++;								\
        if(count > reserve_count){					\
            reserve_count += _growcount;					\
            container = (type*)realloc(container,reserve_count * sizeof(type)); \
        }									\
        container[count -1] = element;					\
    }									\
    type PopBack(){						\
        _kill("",count <= 0);						\
        count --;								\
        return container[count];						\
    }									\
    void Remove(ptrsize index){						\
        for(ptrsize i = index; i < count-1; i++){				\
            container[i] = container[i + 1];				\
        }									\
        count --;								\
    }									\
    void Remove(type* element){						\
        type* end_ptr = container + (count -1);				\
        for(; element != end_ptr; element++){				\
            *element = *(element + 1);					\
        }									\
        count --;								\
    }									\
    void UnorderedRemove(ptrsize index){				\
        container[index] = container[count -1];				\
        count --;								\
    }									\
    void UnorderedRemove(type* element){				\
        *element = container[count -1];					\
        count --;								\
    }									\
    void Insert(type element, ptrsize index){				\
        type* end_ptr = container + (count -1);				\
        type* index_ptr = container + index;				\
        for(;end_ptr != index_ptr; end_ptr --){				\
            *end_ptr = *(end_ptr -1 );					\
        }									\
        *index_ptr = element;						\
        count ++;								\
    }									\
    type PopFront(){						\
        _kill("",count <= 0);						\
        type return_type = container[0];					\
        for(ptrsize i = 0; i < count; i++){				\
            container[i] = container[i + 1];				\
        }									\
        count --;								\
        return return_type;						\
    }									\
    void PushFront(type element){					\
        count++;								\
        if(count > reserve_count){					\
            reserve_count += _growcount;					\
            container = (type*)realloc(container,reserve_count * sizeof(type)); \
        }									\
        for(ptrsize i = count; i != 0; i--){				\
            container[i] = container[i -1];					\
        }									\
        container[0] = element;						\
    }									\
    void Destroy(){						\
        unalloc(container);							\
    }									\
    void Fit(){						\
        reserve_count = ((count/10) * 10) + _growcount;			\
        container = (type*)realloc(container,reserve_count * sizeof(type)); \
    }									\
}									\


#define _declare_btree(name,type)

#define _declare_octree(name,type)

#define _declare_colony(name,type)

#define _declare_array(name,type) struct name{				\
    u64 count;		\
    type* container;		\
    type& operator [](ptrsize index){	\
        _kill("",index >= count); \
        return container[index];	\
    } \
};			\


#define _initarray(type,array,cnt) _kill("",cnt == 0);array.container = _salloc(type, cnt * sizeof(type)); array.count = cnt;

struct BufferRegion{
    s8* container;
    u32 count;
    u32 reserve_count;
    
    void Init(ptrsize rcount = _defaultcount){
        _kill("",rcount == 0);
        container = (s8*)alloc(sizeof(s8) * rcount);
        reserve_count = rcount;
        count = 0;
    }
    
    void Write(void* data,u32 size){
        
        u32 offset = count;
        count += size;
        
        if(count > reserve_count){
            reserve_count += (count - reserve_count) + _growcount;
            container = (s8*)realloc(container,reserve_count);
        }
        
        memcpy(container + offset,data,size);
        
        
    }
    
    void Release(u32 size){
        count -=size;
        _kill("",(count) <= 0);
    }
    
    void Fit(){
        reserve_count = ((count/10) * 10) + _growcount;
        container = (s8*)realloc(container,reserve_count); 
    }
    
    void Destroy(){
        unalloc(container);
    }
    
};


//structures that dictate the storage,retrieval and layout of data

/*TODO:
  static array with length
  bit array
  SOA and conversion to SOA
  Sectioned structs
  Dictionary
  Map
  Linked list
  Binary trees
*/

//TODO: push_front pop_front

//NOTE: On Linux, realloc calls into sbrk

//TODO: Replace the c stdblib 
