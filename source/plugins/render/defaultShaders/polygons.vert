#version 330
uniform mat4 modelView;
uniform mat4 model;
uniform mat4 projection;

uniform bool defaultLighting = true;

in vec3 P;
in vec3 N;
out vec3 pos;
out vec3 sn;

void main(){
   gl_Position = projection * modelView * vec4(P, 1);
   
   if(defaultLighting) {
       pos = (modelView * vec4(P, 1)).xyz;
       sn = (modelView * vec4(N, 0)).xyz;
   }
   else {
       pos = (model * vec4(P, 1)).xyz;
       sn = (model * vec4(N, 0)).xyz;
   }
};
