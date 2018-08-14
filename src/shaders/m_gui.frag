
layout (location = 0) in vec4 Color;
layout (location = 1) in vec2 UV;

layout(set=0, binding=0) uniform sampler2D sTexture;

layout (location = 0) out vec4 outFragColor;

void main(){
    
#ifdef  USE_TEXTURE
    
    vec4 tcolor = texture(sTexture, UV.st);
    
    outFragColor = Color * tcolor;
    
#else
    outFragColor = vec4(Color.r,Color.g,Color.b,1.0f);
#endif
}
