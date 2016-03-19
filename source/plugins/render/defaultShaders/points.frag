#version 330
in vec3 vertex_color;
out vec4 outcolor;
out vec4 outnormal;
out vec4 outposition;

void main(){
    outcolor = vec4(vertex_color, 1);
    outnormal = vec4(0);
    outposition = vec4(0);
};
