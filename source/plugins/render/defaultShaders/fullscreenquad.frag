#version 330
uniform vec4 color = vec4(1);
uniform vec4 bgcolor;

in vec2 st;
uniform sampler2D overlay;
uniform sampler2D shading_out;
uniform sampler2D outcolor;

out vec4 final_color;

float _gamma(float val, float g) {
    return pow(val, g);
}

vec3 gamma(vec3 col, float g) {
    vec3 outcol;
    outcol.r = _gamma(col.r, g);
    outcol.g = _gamma(col.g, g);
    outcol.b = _gamma(col.b, g);
    return outcol;
}

void main(){
    vec4 over = texture(overlay, st);
    vec4 shading = texture(shading_out, st);
    vec4 col = texture(outcolor, st);

    vec4 _bg = mix(bgcolor * .85, bgcolor, smoothstep(0., .5, length(vec2(.5) - st)));

    col = col + shading;
    col = vec4(gamma(col.rgb, 1/2.2), col.a);
    final_color = mix(bgcolor, col, col.a);
    final_color = mix(final_color, over, over.a);
};
