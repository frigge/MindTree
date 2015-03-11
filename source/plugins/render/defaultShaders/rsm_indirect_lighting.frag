#version 330
uniform sampler2D outnormal;
uniform sampler2D outposition;
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

in vec2 st;
uniform ivec2 resolution;

vec3 pos;
vec3 Nn;

uniform Light light;
const int searchradius = 10;

out vec4 rsm_indirect_out;

const float PI = 3.14159265359;

void main()
{
    ivec2 p = ivec2(st.x * resolution.x, st.y * resolution.y);
    vec4 _pos = texelFetch(outposition, p, 0);
    pos = _pos.xyz;
    Nn = normalize(texelFetch(outnormal, p, 0).xyz);

    shadowP = (light.shadowmvp * vec4(pos, 1));
    shadowP /= shadowP.w;
    shadowP += 1;
    shadowP *= 0.5;

    vec3 indirect = vec3(0);
    for(int i = 1; i <= 20; ++i)
        for(int j = 1; j < 20; j++) {
            vec2 samplePosPolar = texelFetch(samplingPattern, ivec2(i, j), 0).rg;

            float polar = 2 * PI * samplePosPolar.y;
            vec2 sampleOffset = vec2(sin(polar) * samplePosPolar.x, cos(polar) * samplePosPolar.x);
            sampleOffset *= searchradius;
            sampleOffset /= resolution;

            vec2 samplePosition = shadowP.xy + sampleOffset;

            vec3 flux = texture(shadow_flux, samplePosition).rgb;
            vec3 n = texture(shadow_normal, samplePosition).xyz;
            n = normalize(n);
            vec3 p = texture(shadow_position, samplePosition).xyz;

            vec3 lvec = pos - p;
            float lightAngleCos = clamp(dot(lvec, normalize(n)), 0, 1);

            float lightLambert = clamp(dot(Nn, normalize(-lvec)), 0, 1);

            indirect += flux * lightLambert * lightAngleCos;
        }

    indirect /= 400;

    rsm_indirect_out = vec4(indirect, 1);
}
