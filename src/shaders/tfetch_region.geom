in layout(triangles);
out layout(triangle_strip,max_vertices = 3);

/*
  https://developer.nvidia.com/gpugems/GPUGems2/gpugems2_chapter42.html
*/

//TODO:
void SWConservativeRaster(vec4 coord){
}

//TODO: Test this
vec4 SWConservativeRaster2(vec4 cur_coord,vec4 next_coord,vec4 prev_coord,
			   vec2 half_pixcell){

  half_pixcell = half_pixcell/(8.0f * 128.0f);

  //get the plane of the bounding triangle edges

  vec3 plane[2];

  plane[0] = cross(cur_coord.xyw - prev_coord.xyw,prev_coord.xyw);
  
  plane[1] = cross(next_coord.xyw - cur_coord.xyw,cur_coord.xyw);

  //move plane by pixel cell semi diagonal

  plane[0].z -= dot(half_pixcell.xy,abs(plane[0].xy));
  plane[1].z -= dot(half_pixcell.xy,abs(plane[1].xy));

  //intersection of the 2 planes = final point

  vec4 point;

  point.xyw = cross(plane[0],plane[1]);

  point.xyw /= point.w;

  point.z = cur_coord.z;

  return point;
}



void main(){

#if 0
  
  gl_Position = gl_in[0].gl_Position;
  EmitVertex();

  gl_Position = gl_in[1].gl_Position;
  EmitVertex();

  gl_Position = gl_in[2].gl_Position;
  EmitVertex();
  
  EndPrimitive();

#else
  
  gl_Position = SWConservativeRaster2(gl_in[0].gl_Position,gl_in[1].gl_Position,
				      gl_in[2].gl_Position,vec2(64,64));
  EmitVertex();

  gl_Position = SWConservativeRaster2(gl_in[1].gl_Position,gl_in[0].gl_Position,
				      gl_in[2].gl_Position,vec2(64,64));
  EmitVertex();

  gl_Position = SWConservativeRaster2(gl_in[2].gl_Position,gl_in[0].gl_Position,
				      gl_in[1].gl_Position,vec2(64,64));
  EmitVertex();
  
  EndPrimitive();

#endif
}
