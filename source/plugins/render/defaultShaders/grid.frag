#version 330

in vec4 col;
out vec4 outcolor;
out vec4 outnormal;
out vec4 outposition;

void main(){
    outcolor = col;
    outnormal = vec4(0);
    outposition = vec4(0);
};
