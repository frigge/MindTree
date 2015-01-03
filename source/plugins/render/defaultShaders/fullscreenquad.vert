#version 330
in vec3 P;
in vec2 _st;
out vec3 pos;
out vec2 st;

uniform mat4 modelView;
uniform mat4 projection;

void main(){
   gl_Position = vec4(P, 1);
   pos = P;
   st = _st;
   gl_PointSize = 5;
};
