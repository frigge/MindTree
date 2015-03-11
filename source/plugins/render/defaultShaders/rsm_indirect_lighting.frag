uniform sampler2D shadow_position;
uniform sampler2D shadow_normal;
uniform sampler2D shadow_flux;
uniform sampler2D samplingPattern;

struct Light {
    vec4 color;
    float intensity;
    float coneangle;
    vec4 pos;
    vec3 dir;
    mat4 shadowmvp;
    int shadow;
};

vec4 shadowP;

uniform Light light;
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

    shadowP = (light.shadowmvp * vec4(pos, 1));
    shadowP /= shadowP.w;
    shadowP += 1;
    shadowP *= 0.5;

    rsm_indirect_out = vec4(1, 0, 0, 1);
}
