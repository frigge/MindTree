#version 330
vec3 pos;
vec3 Nn;

vec3 eye;
uniform mat4 view;
uniform bool defaultLighting = true;

in vec2 st;
uniform ivec2 resolution;

uniform sampler2D outnormal;
uniform sampler2D outposition;
uniform sampler2D outdiffusecolor;
uniform sampler2D outdiffuseintensity;
uniform sampler2D outspecintensity;
uniform sampler2DShadow shadow;
float inLight = 1;

uniform int flatShading = 0;

out vec4 shading_out;

const float GAMMA=2.2;

struct Light {
    vec4 color;
    float intensity;
    float coneangle;
    vec4 pos;
    vec3 dir;
    mat4 shadowmvp;
    int shadow;
};

uniform Light light;

vec3 lvec;
float atten = 1;
float angleMask = 1; // for spots only

float _gamma(float val, float g) {
    return pow(val, g);
}

vec3 gamma(vec3 col, float g) {
    vec3 outcol;
    outcol.r = _gamma(col.r, g);
    outcol.g = _gamma(col.g, g);
    outcol.b = _gamma(col.b, g);
    return outcol;
}

vec3 lambert() {
    float cosine = dot(Nn, lvec);
    cosine = clamp(cosine, 0.0, 1.0);
    vec3 col = gamma(light.color.rgb, GAMMA) * cosine * light.intensity * atten;
    return col;
}

vec3 phong(float rough) {
    vec3 Half = normalize(eye + lvec);
    float cosine = clamp(dot(Nn, Half), 0., 1.);
    cosine = pow(cosine, 1./rough);
    vec3 col = gamma(light.color.rgb, GAMMA) * light.intensity * cosine * atten;
    return col;
}

float value(vec3 col) {
    return (col.r + col.g + col.b) / 3;
}

void main(){
    if (defaultLighting)
        eye = vec3(0, 0, 1);
    else
        eye = normalize((view * vec4(0, 0, 1, 0)).xyz);

    ivec2 p = ivec2(st.x * resolution.x, st.y * resolution.y);
    vec4 _pos = texelFetch(outposition, p, 0);
    if (_pos.a < 0.5)
        discard;

    pos = _pos.xyz;
    Nn = normalize(texelFetch(outnormal, p, 0).xyz);

    vec4 diffuse_color = texture(outdiffusecolor, st);
    float diffint = texture(outdiffusecolor, st).r;
    float specint = texture(outdiffusecolor, st).r;

    float lightDirLength = length(light.dir);

    if(light.pos.w > 0.1) {// is point
        lvec = light.pos.xyz - pos;
        atten = 1.0 / dot(lvec, lvec);
    } else {
        lvec = -light.dir;
    }
    lvec = normalize(lvec);
    vec4 shadowP = vec4(1);
    if(lightDirLength > 0.1
       && light.pos.w > 0.1) { // is spot
        float lightAngleCos = abs(dot(lvec, normalize(light.dir)));
        float lightangle = acos(lightAngleCos);
        angleMask = smoothstep(light.coneangle, light.coneangle - 0.1, lightangle);
        angleMask *= lightAngleCos;
        //float bias = tan(acos(clamp(0, 1, dot(Nn, light.dir))));

        shadowP = (light.shadowmvp * vec4(pos, 1));
        shadowP /= shadowP.w;
        shadowP += 1;
        shadowP *= 0.5;

        float bias = .005;
        shadowP.z -= bias;
        inLight = texture(shadow, shadowP.xyz);

        inLight += clamp(1 - light.shadow, 0, 1);
    }

    float specrough = .3;
    vec3 spec = clamp(phong(specrough) * specint, vec3(0), vec3(1));

    vec3 diff = clamp(lambert()*diffint * diffuse_color.xyz, vec3(0), vec3(1));
    float diffspecratio = 0.5 * value(diff) / clamp(0.0001, 1., value(spec));
    vec3 diffspec = mix(diff, spec, diffspecratio);
    shading_out = vec4(diffspec, 1);
    shading_out *= angleMask;
    shading_out *= inLight;
}
