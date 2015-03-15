#version 330
uniform vec4 color = vec4(1);
uniform vec4 bgcolor;

in vec2 st;
uniform sampler2D overlay;
uniform sampler2D shading_out;
uniform sampler2D rsm_indirect_out;
uniform sampler2D outcolor;
uniform sampler2D outnormal;

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
    vec4 rsm_indirect = texture(rsm_indirect_out, st);
    vec4 col = texture(outcolor, st);
    vec4 n = texture(outnormal, st);

    vec4 _bg = mix(bgcolor * .85, bgcolor, smoothstep(0., .5, length(vec2(.5) - st)));

    shading += rsm_indirect;
    shading = vec4(gamma(shading.rgb, 1/2.2), n.a);
    final_color = mix(bgcolor, shading, shading.a);
    final_color = mix(shading, col, col.a);
    final_color = mix(final_color, over, over.a);

    final_color = rsm_indirect;
};
