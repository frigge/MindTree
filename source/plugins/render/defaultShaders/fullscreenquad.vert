#version 330
in vec3 P;
in vec2 _st;
out vec2 st;

uniform mat4 modelView;
uniform mat4 projection;

void main(){
    vec3 p = vec3(P.xy, 1);
   gl_Position = vec4(p, 1);
   st = _st;
};
