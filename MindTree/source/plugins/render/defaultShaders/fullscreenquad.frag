#version 330
uniform vec4 color = vec4(1);
in vec2 st;
uniform int xres;
uniform int yres;
uniform sampler2D in_outcolor;

out vec4 outcolor;
void main(){
    vec4 col = texture(in_outcolor, st);
    outcolor = col;
};
