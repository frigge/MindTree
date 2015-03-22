#version 330
uniform sampler2D rsm_indirect_out_lowres;
uniform sampler2D outnormal;
uniform sampler2D outposition;

in vec2 st;
uniform ivec2 resolution;

uniform float cosAngleTolerance = 0.3;
uniform float distanceTolerance = 0.1;

out vec4 rsm_indirect_out_interpolated;

int checkDistance(vec3 ref, vec3 other)
{
    return length(ref - other) < distanceTolerance ? 1 : 0;
}

int checkAngle(vec3 ref, vec3 other)
{
    return abs(dot(ref, other)) > cosAngleTolerance ? 1 : 0;
}

void main()
{
    ivec2 pixel = ivec2(st * resolution);

    ivec2 px1 = pixel + ivec2(-1, 0);
    ivec2 px2 = pixel + ivec2(1, 0);
    ivec2 py1 = pixel + ivec2(0, -1);
    ivec2 py2 = pixel + ivec2(0, 1);

    int distance_similar = 0;
    vec3 pos = texelFetch(outposition, pixel, 0).xyz;
    vec3 pos_x1 = texelFetch(outposition, px1, 0).xyz;
    vec3 pos_x2 = texelFetch(outposition, px2, 0).xyz;
    vec3 pos_y1 = texelFetch(outposition, py1, 0).xyz;
    vec3 pos_y2 = texelFetch(outposition, py2, 0).xyz;

    distance_similar += checkDistance(pos, pos_x1);
    distance_similar += checkDistance(pos, pos_x2);
    distance_similar += checkDistance(pos, pos_y1);
    distance_similar += checkDistance(pos, pos_y2);

    int cosAngle_similar = 0;
    vec3 Nn = normalize(texelFetch(outnormal, pixel, 0).xyz);
    vec3 Nn_x1 = normalize(texelFetch(outnormal, px1, 0).xyz);
    vec3 Nn_x2 = normalize(texelFetch(outnormal, px2, 0).xyz);
    vec3 Nn_y1 = normalize(texelFetch(outnormal, py1, 0).xyz);
    vec3 Nn_y2 = normalize(texelFetch(outnormal, py2, 0).xyz);

    cosAngle_similar += checkAngle(Nn, Nn_x1);
    cosAngle_similar += checkAngle(Nn, Nn_x2);
    cosAngle_similar += checkAngle(Nn, Nn_y1);
    cosAngle_similar += checkAngle(Nn, Nn_y2);

    if (distance_similar < 3 || cosAngle_similar < 3) {
        discard;
    }

    ivec2 res = textureSize(rsm_indirect_out_lowres, 0);
    vec2 lp = res * st;
    vec2 samplepos = mod(lp, ivec2(1));
    vec3 output = texture(rsm_indirect_out_lowres, st).xyz;
    rsm_indirect_out_interpolated = vec4(output, 1);
}
