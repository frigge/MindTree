#version 330
uniform vec4 fillColor = vec4(1);
uniform vec4 borderColor = vec4(1);
uniform int isBorder=0;

out vec4 overlay;
out vec4 outcolor;

void main(){
    vec4 color = mix(fillColor, borderColor, isBorder);
    overlay = color;
    outcolor = color;
};
