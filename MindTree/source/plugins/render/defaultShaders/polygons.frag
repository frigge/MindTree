#version 330
in vec3 pos;
in vec3 sn;
in vec3 eye;
vec3 Nn;
uniform vec4 polygoncolor = vec4(1);
uniform int flatShading = 0;
uniform float ambientIntensity = 0.1;
uniform vec4 ambient; // = vec4(vec3(.1), 1.);
uniform float diffint = 0.9;
uniform float specint = 0.8;
uniform float specint2 = 0.7;
uniform float specrough1 = 0.15;
uniform float specrough2 = 0.01;

out vec4 outcolor;
out vec4 outnormal;
out vec4 outposition;

const float GAMMA=2.2;

struct Light {
    vec3 pos;
    vec4 color;
    float intensity;
};

uniform Light light0;
uniform Light light1;
uniform Light light2;
uniform Light light3;
uniform Light light4;

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

vec3 _lambert(Light l) {
    vec3 lvec = l.pos - pos ;
    float cosine = dot(Nn, normalize(lvec));
    cosine = clamp(cosine, 0.0, 1.0);
    vec3 col = gamma(l.color.rgb, GAMMA) * l.intensity * cosine;
    return col;
}

vec3 lambert(){
    vec3 outcol = vec3(0.0);
    outcol += _lambert(light0);
    outcol += _lambert(light1);
    outcol += _lambert(light2);
    outcol += _lambert(light3);
    outcol += _lambert(light4);
    return outcol;
}

vec3 _phong(Light l, float rough) {
    vec3 lvec = l.pos-pos;
    vec3 ln = normalize(lvec);
    vec3 Half = normalize(eye + ln);
    float cosine = clamp(dot(Nn, Half), 0., 1.);
    cosine = pow(cosine, 1./rough);
    vec3 col = gamma(l.color.rgb, GAMMA) * l.intensity * (cosine);
    return col;
}

vec3 phong(float rough){
    vec3 outcol = vec3(0.);
    outcol += _phong(light0, rough);
    outcol += _phong(light1, rough);
    outcol += _phong(light2, rough);
    outcol += _phong(light3, rough);
    outcol += _phong(light4, rough);
    return outcol;
}

float value(vec3 col) {
    return (col.r + col.g + col.b) / 3;
}

void main(){
    Nn = mix(normalize(sn), normalize(cross(dFdx(pos), dFdy(pos))), flatShading);

    vec3 spec1 = phong(specrough1) * specint;
    vec3 spec2 = phong(specrough2) * specint2;

    //vec3 spectotal = mix(spec1, spec2, 0.5);
    //vec3 spectotal = vec3(value(spec1));
    float specratio = 0.5 * value(spec1) / clamp(0.0001, 1., value(spec2));
    vec3 spectotal = mix(spec1, spec2, specratio);

    vec3 diff = gamma(polygoncolor.rgb, GAMMA) * lambert()*diffint;
    float diffspecratio = 0.5 * value(diff) / clamp(0.0001, 1., value(spectotal));
    vec3 diffspec = mix(diff, spectotal, diffspecratio);
    outcolor = vec4(gamma(
                    diffspec + 
                    gamma(ambient.rgb, GAMMA) * ambientIntensity + 
                    spectotal
                    , 1./GAMMA), 1
                   );

    outcolor.a = polygoncolor.a;
    outposition = vec4(pos, 1);
    outnormal = vec4(Nn, 1);
}
