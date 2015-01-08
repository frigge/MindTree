#version 330
uniform vec4 fillColor = vec4(1);
uniform vec4 borderColor = vec4(1);
uniform int isBorder=0;

out vec4 outnormal;
out vec4 outposition;
out vec4 outcolor;
void main(){
    outcolor = mix(fillColor, borderColor, isBorder);
    outnormal = vec4(0);
    outposition = vec4(0);
};
