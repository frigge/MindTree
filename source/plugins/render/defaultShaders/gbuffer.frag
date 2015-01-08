#version 330

in vec3 pos;
in vec3 sn;
in vec3 eye;

out vec4 outnormal;
out vec4 outposition;
out vec4 outcolor;

uniform int flatShading = 0;

vec3 Nn;

void main() {
    Nn = mix(normalize(sn), normalize(cross(dFdx(pos), dFdy(pos))), flatShading);
    outnormal = vec4(Nn, 1);
    outposition = vec4(pos, 1);
    outcolor = vec4(0);
}
