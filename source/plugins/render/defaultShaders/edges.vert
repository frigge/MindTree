#version 330
uniform mat4 modelView;
uniform mat4 projection;
in vec3 P;
void main(){
   gl_Position = projection * modelView * vec4(P, 1);
};
