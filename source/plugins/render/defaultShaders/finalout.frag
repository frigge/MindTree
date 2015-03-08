#version 330
in vec2 st;
uniform sampler2D final_out;

out vec4 color;

void main(){
    vec4 col = texture(final_out, st);
    color = col;
};
