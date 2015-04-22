#version 330
uniform sampler2D rsm_indirect_out_lowres;
uniform sampler2D outnormal;
uniform sampler2D outposition;

in vec2 st;
uniform ivec2 resolution;

uniform float cosAngleTolerance = 2;
uniform float distanceTolerance = 1.0;

out vec4 rsm_indirect_out_interpolated;
out vec4 rsm_indirect_out_mask;

int checkDistance(vec3 ref, vec4 other)
{
    if(other.a < 0.5) return 0;
    return distance(ref, other.xyz) <= distanceTolerance ? 1 : 0;
}

int checkAngle(vec3 ref, vec4 other)
{
    if(other.a < 0.5) return 0;
    return acos(abs(dot(ref, other.xyz))) < radians(cosAngleTolerance) ? 1 : 0;
}

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
    ivec2 lowres = textureSize(rsm_indirect_out_lowres, 0);
    vec2 fp = vec2(st * lowres);
    fp -= vec2(0.5);
    //fp = clamp(vec2(0), lowres, fp);

    vec2 remain = mod(fp, vec2(1));

    vec2 resFac = resolution / lowres;
    float lx = floor(fp.x);
    float ly = floor(fp.y);
    float hx = ceil(fp.x);
    float hy = ceil(fp.y);

    ivec2 px1 = ivec2(lx, ly);
    ivec2 px2 = ivec2(hx, ly);
    ivec2 px3 = ivec2(lx, hy);
    ivec2 px4 = ivec2(hx, hy);

    //rsm_indirect_out_mask = vec4(vec3(rand(vec2(hx, hy))), 1);
    //return;

    ivec2 px1h = ivec2(round(px1 * resFac));
    ivec2 px2h = ivec2(round(px3 * resFac));
    ivec2 px3h = ivec2(round(px3 * resFac));
    ivec2 px4h = ivec2(round(px4 * resFac));

    vec3 N = normalize(texture(outnormal, st)).xyz;
    vec3 P = texture(outposition, st).xyz;

    int simCnt = 0;
    ivec4 isSimilar = ivec4(0);

    vec4 p1 = texelFetch(outposition, px1h, 0);
    vec4 p2 = texelFetch(outposition, px2h, 0);
    vec4 p3 = texelFetch(outposition, px3h, 0);
    vec4 p4 = texelFetch(outposition, px4h, 0);

    vec4 n1 = normalize(texelFetch(outnormal, px1h, 0));
    vec4 n2 = normalize(texelFetch(outnormal, px2h, 0));
    vec4 n3 = normalize(texelFetch(outnormal, px3h, 0));
    vec4 n4 = normalize(texelFetch(outnormal, px4h, 0));

    isSimilar[0] += checkAngle(N, n1);
    isSimilar[1] += checkAngle(N, n2);
    isSimilar[2] += checkAngle(N, n3);
    isSimilar[3] += checkAngle(N, n4);

    isSimilar[0] *= checkDistance(P, p1);
    isSimilar[1] *= checkDistance(P, p2);
    isSimilar[2] *= checkDistance(P, p3);
    isSimilar[3] *= checkDistance(P, p4);

    simCnt += isSimilar[0];
    simCnt += isSimilar[1];
    simCnt += isSimilar[2];
    simCnt += isSimilar[3];

    if (simCnt < 4) {
        rsm_indirect_out_interpolated = vec4(0);
        rsm_indirect_out_mask = vec4(1, 0, 0, 1);
        return;
    }

    float wx1 = (1 - remain.x) * (1 - remain.y) * isSimilar[0];
    float wx2 = remain.x * (1 - remain.y) * isSimilar[1];
    float wy1 = (1 - remain.x) * remain.y * isSimilar[2];
    float wy2 =  remain.x * remain.y * isSimilar[3];

    float weightSum = wx1 + wx2 + wy1 + wy2;

    vec3 gi1 = texelFetch(rsm_indirect_out_lowres, px1, 0).xyz * wx1;
    vec3 gi2 = texelFetch(rsm_indirect_out_lowres, px2, 0).xyz * wx2;
    vec3 gi3 = texelFetch(rsm_indirect_out_lowres, px3, 0).xyz * wy1;
    vec3 gi4 = texelFetch(rsm_indirect_out_lowres, px4, 0).xyz * wy2;

    vec3 gi = gi1 + gi2 + gi3 + gi4;
    gi /= weightSum;

    rsm_indirect_out_interpolated = vec4(gi, 1);
    rsm_indirect_out_mask = vec4(texture(rsm_indirect_out_lowres, st).rgb, 1);
    //rsm_indirect_out_interpolated = vec4(0);
}
