#version 330
uniform mat4 modelView;
uniform mat4 model;
uniform mat4 projection;

uniform bool defaultLighting = false;

in vec3 P;
in vec3 N;

out vec3 pos;
out vec3 worldPos;
out vec3 cameraPos;
out vec3 cameraNormal;
out vec3 sn;
out vec3 worldNormal;

void main(){
   gl_Position = projection * modelView * vec4(P, 1);

   worldPos = (model * vec4(P, 1)).xyz;
   worldNormal = (model * vec4(N, 0)).xyz;
   cameraPos = (modelView * vec4(P, 1)).xyz;
   cameraNormal = (modelView * vec4(N, 0)).xyz;

   if(defaultLighting) {
       pos = cameraPos;
       sn = cameraNormal;
   }
   else {
       pos = worldPos;
       sn = worldNormal;
   }
};
