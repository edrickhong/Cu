layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;


layout (location = 0) out vec4 Color;
layout (location = 1) out vec2 UV;

layout(push_constant) uniform PushConsts{
  mat4 worldviewproj;
}pushconst;

void main()
{
  Color = aColor;
  UV = aUV;
  gl_Position = pushconst.worldviewproj * vec4(aPos.xyz,1.0f);
}

