layout(location = 0) in vec4 aPos;


layout(push_constant) uniform PushConsts{
	mat4 viewproj;
	vec4 camerapos;
}pushconst;

out gl_PerVertex {

	vec4 gl_Position;

};

void main()
{
	gl_Position = pushconst.viewproj * aPos;
}


