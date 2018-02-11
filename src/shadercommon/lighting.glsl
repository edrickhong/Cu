
float LambertianDiffuse(vec3 normal,vec3 lightdir){

  lightdir = lightdir * -1.0f;

  return dot(lightdir,normal);
}

float PhongSpecular(vec3 lightdir,vec3 normal,vec3 vertextoeye,float power,
		     float intensity){

  vec3 reflectvec = reflect(lightdir,normal).xyz;
  float specularfactor = dot(reflectvec,vertextoeye);

  return max(pow(specularfactor,power),0.0f) * intensity;
}
