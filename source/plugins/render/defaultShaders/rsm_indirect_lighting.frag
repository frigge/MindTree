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
uniform float searchradius;
uniform float intensity;

out vec4 rsm_indirect_out;

const float PI = 3.14159265359;

void main()
{
    ivec2 p = ivec2(st.x * resolution.x, st.y * resolution.y);
    vec4 _pos = texelFetch(outposition, p, 0);

    if (_pos.a < 0.5)
        discard;

    pos = _pos.xyz;
    Nn = normalize(texelFetch(outnormal, p, 0).xyz);

    shadowP = (light.shadowmvp * vec4(pos, 1));
    shadowP /= shadowP.w;
    shadowP += 1;
    shadowP *= 0.5;

    vec3 indirect = vec3(0);
    for(int i = 1; i <= 20; ++i)
        for(int j = 1; j <= 20; j++) {
            //vec2 samplePosPolar = texelFetch(samplingPattern, ivec2(i, j), 0).rg;
            vec2 samplePosPolar = vec2(float(i) / 20, float(j) / 20);
            float radius = samplePosPolar.y;

            float radius_squared = radius * radius;
            float polar = 2 * PI * samplePosPolar.x;
            vec2 sampleOffset = vec2(sin(polar), cos(polar)) * radius;
            sampleOffset *= searchradius;

            vec2 samplePosition = shadowP.xy + sampleOffset;

            vec3 flux = texture(shadow_flux, samplePosition).rgb;
            vec3 n = texture(shadow_normal, samplePosition).xyz;
            n = normalize(n);
            vec3 p = texture(shadow_position, samplePosition).xyz;
            p -= n * 0.05;

            vec3 lvec = normalize(pos - p);
            float lightAngleCos = clamp(dot(lvec, normalize(n)), 0, 1);

            float lightLambert = clamp(dot(Nn, -lvec), 0, 1);

            indirect += flux * lightLambert * lightAngleCos * radius_squared;
        }

    indirect /= 400;

    rsm_indirect_out = vec4(indirect, 1) * intensity;

    vec2 myst = ((st * resolution) / 40);
    myst.x = mod(myst.x, 1);
    myst.y = mod(myst.y, 1);
    rsm_indirect_out = vec4(texture(samplingPattern, myst));
    //rsm_indirect_out = vec4(myst, 0, 1);
}
