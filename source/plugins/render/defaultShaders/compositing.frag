#version 330

uniform sampler2D layer;
in vec2 st;
uniform int last_type = -1;
uniform float mixValue;

out vec4 final_out;

void main() {
    vec4 color = texture(layer, st);
    color.a  = clamp(color.a, 0, 1);
    if(last_type == 3) {
        color *= 1 - mixValue;
    }
    else {
        color *= mixValue;
    }
    final_out = color;
};
