#version 330
uniform sampler2D outnormal;
uniform sampler2D outposition;
uniform sampler2D shadow_position;
uniform sampler2D shadow_normal;
uniform sampler2D shadow_flux;
uniform sampler2D rsm_indirect_out_interpolated;
uniform sampler2D outdiffusecolor;
uniform sampler2D outdiffuseintensity;

uniform sampler1D samplingPattern;

uniform bool highres = false;

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

vec3 pos;
vec3 Nn;

uniform Light light;
uniform float searchradius;
uniform float intensity;

uniform int numSamples = 400;

out vec4 rsm_indirect_out;

const float PI = 3.14159265359;

void main()
{
    if(highres) {
        vec4 interpolated = texture(rsm_indirect_out_interpolated, st);
        if(interpolated.a > 0.5) {
            discard;
        }
    }
    vec4 _pos = texture(outposition, st, 0);

    if (_pos.a < 0.5)
        discard;

    pos = _pos.xyz;
    Nn = normalize(texture(outnormal, st, 0).xyz);

    shadowP = (light.shadowmvp * vec4(pos, 1));
    shadowP /= shadowP.w;
    shadowP += 1;
    shadowP *= 0.5;

    vec3 indirect = vec3(0);
    for(int i = 0; i < numSamples; ++i) {
        vec2 samplePosPolar = texelFetch(samplingPattern, i, 0).rg;
        float radius = samplePosPolar.y;

        float radius_squared = radius * radius;
        float polar = 2 * PI * samplePosPolar.x;
        vec2 sampleOffset = vec2(sin(polar), cos(polar)) * radius;
        sampleOffset *= searchradius * 0.5;

        vec2 samplePosition = shadowP.xy + sampleOffset;

        vec3 flux = texture(shadow_flux, samplePosition).rgb;
        vec3 n = texture(shadow_normal, samplePosition).xyz;
        n = normalize(n);
        vec3 p = texture(shadow_position, samplePosition).xyz;
        p -= n * 0.01;

        vec3 lvec = pos - p;
        vec3 nlvec = normalize(lvec);
        float lightAngleCos = max(dot(nlvec, n), 0);

        float lightLambert = max(dot(Nn, -nlvec), 0);
        float atten = max(1 / dot(lvec, lvec), 0);

        indirect += flux * lightLambert * lightAngleCos * radius_squared * atten;
        //indirect += flux * lightLambert * lightAngleCos * radius_squared;
    }

    indirect /= numSamples;

    indirect *= texture(outdiffusecolor, st).rgb;
    rsm_indirect_out = vec4(indirect * intensity, 1);
}
