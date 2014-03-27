#version 330
in vec3 P;
uniform mat4 modelView;
uniform mat4 projection;
uniform int size;

void main(){
   gl_Position = projection * modelView * vec4(P, 1.);
   gl_PointSize = size;
};
