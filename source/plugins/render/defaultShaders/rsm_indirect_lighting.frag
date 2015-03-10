uniform sampler2D shadow_position;
uniform sampler2D shadow_normal;
uniform sampler2D shadow_flux;
uniform sampler2D samplingPattern;

uniform int searchradius;

out vec4 rsm_indirect_out;

void main()
{
    for(int i = 0; i < 20; ++i)
        for(int j = 0; j < 20; j++) {
            vec2 samplePosPolar = texelFetch(samplingPattern, vec2(i, j)).rg;

            float polar = 2 * PI * samplePosPolar.y;
            vec2 samplePos = vec2(sin(polar) * samplePosPolar.x, cos(polar) * samplePosPolar.x);
        }

    rsm_indirect_out = vec4(1, 0, 0, 1);
}
