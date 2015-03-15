#version 330
uniform vec4 fillColor = vec4(1);
uniform vec4 borderColor = vec4(1);
uniform int isBorder=0;

out vec4 overlay;
out vec4 outcolor;
out vec4 outnormal;
out vec4 outposition;

void main(){
    vec4 color = mix(fillColor, borderColor, isBorder);
    overlay = color;
    outcolor = color;
    outnormal = vec4(0);
    outposition = vec4(0);
};
