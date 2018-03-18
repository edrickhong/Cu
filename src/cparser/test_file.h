#define REFLCOMPONENT
#define REFLSTRUCT(TAG)
#define REFLFUNC(TAG)

/*
  hello world
*/

//THIS IS A TEST COMMENT

struct REFLCOMPONENT Character{
    s8* name;
    u32 health;
    u32 atk;
};

struct REFLCOMPONENT BossCharacter{
    Character character;//this is a char name
    u32 special_ability;/*asdasd*/
};

struct REFLSTRUCT(GENERIC) Foo{
    u32 a;
    u32 b;
    u32 c;
};

s32 REFLFUNC(GENERIC) AddFunction(u32 a,u32 b){
    
    return a + b;
}
