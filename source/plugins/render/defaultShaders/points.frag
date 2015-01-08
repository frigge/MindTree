#version 330
uniform vec4 pointcolor = vec4(1);
out vec4 outcolor;
out vec4 outnormal;
out vec4 outposition;

void main(){
   outcolor = pointcolor;
    outnormal = vec4(0);
    outposition = vec4(0);
};
