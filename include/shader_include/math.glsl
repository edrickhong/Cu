
/* float InterpolateValue(float a,float b,float step){ */
/*   return (a + (step * (b-a))); */
/* } */

#define InterpolateValue(a,b,step) mix(a,b,step)

mat4 TranslationMatrix(vec4 translation){

  float x = translation.x;
  float y = translation.y;
  float z = translation.z;

  mat4 matrix = mat4(
  		     1,0,0,0,
  		     0,1,0,0,
  		     0,0,1,0,
  		     x,y,z,1
  		     );

  return matrix;
}

mat4 QuaternionToMatrix(vec4 quaternion){

  vec4 squared;

  squared = quaternion * quaternion;

  float a = 1 - (2 * (squared.z + squared.w));
  float b = ((quaternion.y * quaternion.z) - (quaternion.x * quaternion.w)) * 2;
  float c = ((quaternion.y * quaternion.w) + (quaternion.x * quaternion.z)) * 2;
  float d = ((quaternion.y * quaternion.z) + (quaternion.x * quaternion.w)) * 2;
  float e = 1 - (2 * (squared.y + squared.w));
  float f = ((quaternion.z * quaternion.w) - (quaternion.x * quaternion.y)) * 2;
  float g = ((quaternion.y * quaternion.w) - (quaternion.x * quaternion.z)) * 2;
  float h = ((quaternion.z * quaternion.w) + (quaternion.x * quaternion.y)) * 2;
  float i = 1 - (2 * (squared.y + squared.z));

  mat4 matrix =
    mat4(
	 a,d,g,0,
	 b,e,h,0,
	 c,f,i,0,
	 0,0,0,1
	 );

  return matrix;
}


mat4 ConstructBoneMatrix(vec4 translation,vec4 rotation){

  mat4 rot = QuaternionToMatrix(rotation);

  mat4 trans = TranslationMatrix(translation);

  return trans * rot;
}
