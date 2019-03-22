#version 330
in vec3 P;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
uniform mat4 mvp;
uniform mat4 staticTransformation;
uniform bool fixed_screensize = true;
uniform bool screen_oriented = false;
uniform int point_size = 5;

uniform ivec2 resolution;
out vec3 pos;

void main(){
    mat4 finalTransform = mat4(1);
    vec4 translation = model[3];

    vec4 staticTransformedP = staticTransformation * vec4(P, 1);
    if (screen_oriented) {
	    finalTransform[3] = -translate;
    }
    else
        finalTransform = model;

    if (fixed_screensize)
	{
		float scale = abs(mvp[3].z) * .1;
		finalTransform[0] *= scale;
		finalTransform[1] *= scale;
		finalTransform[2] *= scale;
	}

    gl_Position = projection * view * finalTransform * staticTransformedP;
    gl_PointSize = point_size;

    pos = (finalTransform * staticTransformedP).xyz;
};
