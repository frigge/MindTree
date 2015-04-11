#version 330
in vec2 st;
uniform sampler2D output;

out vec4 color;

void main(){
    vec4 col = texture(output, st);
    color = col;
};
