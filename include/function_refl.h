#pragma once

u32 REFL TestFunction(u32 a,u32 b){
    
    return a + b;
}

void REFL FOO(u32 a,f32 b,u32 c,f32 d){
	TestFunction(a,c);
}