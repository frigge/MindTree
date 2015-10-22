#version 330

uniform sampler2D rsm_indirect_out_highres;
uniform sampler2D rsm_indirect_out_interpolated;

in vec2 st;

out vec4 rsm_indirect_out;

void main()
{
    vec4 highres = texture(rsm_indirect_out_highres, st);
    vec4 interpolated = texture(rsm_indirect_out_interpolated, st);

    rsm_indirect_out = vec4(highres + interpolated);
}
