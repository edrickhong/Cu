

vec4 TextureSpaceToScreenSpace(vec2 texcoord){

  vec2 tpos = (texcoord * 2.0f) - vec2(1.0f,1.0f);

  return vec4(tpos.x,tpos.y,1.0f,1.0f);  
}

float TextureCull(vec4 vert_worldpos,vec4 vert_screenpos,vec3 eye_pos,vec3 normal){

  /*
    Vk Primitive clipping:
    -w <= (x || y) <= w
    0 <= z <= w
  */

  vec3 posval = vert_screenpos.xyz/vert_screenpos.w;
  posval.z = floor(posval.z);
  posval.xyz = abs(posval.xyz);

  //back face culling
  vec3 vertextoeye = normalize(eye_pos - vec3(vert_worldpos.xyz));
  float backfactor =  1 - dot(vertextoeye,normal);

  //All valid ranges of z becomes 0 once floored. sign -1 * < 0 is always positive
  return max(max(max(posval.x,posval.y),backfactor),posval.z * 5);
}
