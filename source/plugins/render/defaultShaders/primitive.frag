#version 330
uniform vec4 fillColor = vec4(1);
uniform vec4 borderColor = vec4(1);
uniform int isBorder=0;

out vec4 overlay;
void main(){
    overlay = mix(fillColor, borderColor, isBorder);
};
