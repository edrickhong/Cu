#pragma once
#include "ccontainer.h"
#include "mmath.h"

#define _using_TAlloc 1 

#if _using_TAlloc


#include "aallocator.h"

#define DBGPTR(A) DEBUGPTR(A)

#else

#define DBGPTR(A) A*

#endif

//TODO: We will use the skeleton to store the final result

enum AAnimationBehaviour{
  AANIMATION_DEFAULT = 0,
  AANIMATION_CONSTANT = 1,
  AANIMATION_LINEAR = 2,
  AANIMATION_REPEAT = 3,
  AANIMATION_FORCE32BIT = 0x8fffffff,
};

struct AAnimationKey{
  f32 time;
  Vector4 value;
};

struct AAnimationData{

  //MARK: could have precision issues
  u16 scalekey_count;
  u16 positionkey_count;
  u32 rotationkey_count;
  
  AAnimationKey* positionkey_array;
  AAnimationKey* rotationkey_array;
  AAnimationKey* scalekey_array;
};


struct AAnimationSet{
  u32 animationdata_count;
  f32 duration;
  f32 tps;
};

//MARK:Cache can still be better
struct ALinearBone{
  Matrix4b4 offset;
  AAnimationData* animationdata_array;
  ALinearBone** children_array;
  u32 children_count;
};

struct ADQBone{
  DualQuaternion offset;
  DualQuaternion final;//we will be using this afterall
  //u32 bone_hash;//we only use this to map to corresponding animation data.might as well replace w data
  //AAnimationData* animationdata
  u32 children_count;
  u32 childrenindex_array[10];
};


void ALinearBlend(f32 time_ms,u32 animation_index,AAnimationSet* animation_array,
		  ALinearBone* root,DBGPTR(Matrix4b4) result);

void ADualQuaternionBlend(f32 time_ms,ADQBone* root,AAnimationSet animation);

//NOTE: We can interpolate between both. sounds slow though?
//void LinearDQ(f32 time_ms,LinearSkeleton skeleton,AAnimation animation);
