#define REFLCOMPONENT

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
  Character character;
  u32 special_ability;
};
