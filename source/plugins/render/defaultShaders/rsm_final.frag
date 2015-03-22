#version 330

uniform sampler2D rsm_indirect_out_highres;
uniform sampler2D rsm_indirect_out_interpolated;

in vec2 st;

out vec4 rsm_indirect_out;

void main()
{
    vec3 highres = texture(rsm_indirect_out_highres, st).xyz;
    vec3 interpolated = texture(rsm_indirect_out_interpolated, st).xyz;

    rsm_indirect_out = vec4(highres + interpolated, 1);
}
