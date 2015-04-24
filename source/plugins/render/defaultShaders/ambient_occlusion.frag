#version 330

in vec2 st;

uniform ivec2 resolution;

uniform float searchradius = 0.1;
uniform int numSamples = 400;

uniform sampler2D outnormal;
uniform sampler2D outposition;
uniform sampler2D outcolor;
uniform sampler2D outdiffusecolor;

uniform sampler1D samplingPattern;
uniform vec4 ambient_color = vec4(0.5, 0.5, 0.5, 1.);


out vec4 ambient_occlusion;

const float PI = 3.14159265359;

void main()
{
    vec3 Nn = normalize(texture(outnormal, st).xyz);
    vec3 P = texture(outposition, st).xyz;
    vec3 col = texture(outdiffusecolor, st).rgb;

    float occlusion = 0;

    for(int i = 0; i < numSamples; ++i) {
        vec2 sam = texelFetch(samplingPattern, i, 0).xy;

        float posPolar = sam.x * 2 * PI;
        vec2 sampleOffset = vec2(sin(posPolar), cos(posPolar)) * sam.y;

        sampleOffset *= searchradius * 0.5;
        
        vec2 samplePosition = st + sampleOffset;

        vec3 sN = texture(outnormal, samplePosition).xyz;
        vec3 sP = texture(outposition, samplePosition).xyz;

        occlusion += max(0, dot(sN, Nn));
    }

    occlusion /= numSamples;

    ambient_occlusion = vec4(((1 - occlusion) * ambient_color).rgb * col, 1);
}
