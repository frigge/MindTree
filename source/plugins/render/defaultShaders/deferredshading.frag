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

uniform vec4 polygoncolor = vec4(1);
uniform int flatShading = 0;
uniform float ambientIntensity = 0.1;
uniform vec4 ambient; // = vec4(vec3(.1), 1.);
uniform float diffint = 0.9;
uniform float specint = 0.8;
uniform float specint2 = 0.7;
uniform float specrough1 = 0.15;
uniform float specrough2 = 0.01;

out vec4 shading_out;

const float GAMMA=2.2;

struct Light {
    vec4 pos;
    vec4 color;
    float intensity;
    float coneangle;
    bool directional;
};

uniform Light light;

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
    vec3 lvec;
    float atten = 1;
    if(light.directional)
        lvec = -light.pos.xyz;
    else {
        lvec = light.pos.xyz - pos;
        atten = 1.0 / dot(lvec, lvec);
    }


    float cosine = dot(Nn, normalize(lvec));
    cosine = clamp(cosine, 0.0, 1.0);
    vec3 col = gamma(light.color.rgb, GAMMA) * cosine * light.intensity * atten;
    return col;
}

vec3 phong(float rough) {
    vec3 lvec;
    float atten = 1;
    if(light.directional)
        lvec = -light.pos.xyz;
    else {
        lvec = light.pos.xyz - pos;
        atten = 1.0 / dot(lvec, lvec);
    }

    vec3 ln = normalize(lvec);
    vec3 Half = normalize(eye + ln);
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
    pos = _pos.xyz;
    Nn = normalize(texelFetch(outnormal, p, 0).xyz);

    vec3 spec1 = phong(specrough1) * specint;
    vec3 spec2 = phong(specrough2) * specint2;

    float specratio = 0.5 * value(spec1) / clamp(0.0001, 1., value(spec2));
    vec3 spectotal = mix(spec1, spec2, specratio);

    vec3 diff = gamma(polygoncolor.rgb, GAMMA) * lambert()*diffint;
    float diffspecratio = 0.5 * value(diff) / clamp(0.0001, 1., value(spectotal));
    vec3 diffspec = mix(diff, spectotal, diffspecratio);
    shading_out = vec4( diffspec + 
                    gamma(ambient.rgb, GAMMA) * ambientIntensity + 
                    spectotal
                    , _pos.a
                   );
}
