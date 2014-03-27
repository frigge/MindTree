#version 330
in vec3 pos;
in vec3 sn;
in vec3 eye;
vec3 Nn;
uniform vec4 color;
uniform int flatShading;
out vec4 outcolor;

struct Light {
    vec3 pos;
    vec3 color;
    float intensity;
};
Light lights[3];
vec3 lambert(){
    vec3 outcol = vec3(0.0);
    for(int i=0; i<3; i++){
        Light l = lights[i];
        vec3 lvec = l.pos - pos ;
        float cosine = dot(Nn, normalize(lvec));
        cosine = clamp(cosine, 0.0, 1.0);
        vec3 col = l.color * l.intensity * cosine;
        outcol += col * 1./dot(lvec, lvec);
    }
    return outcol;
}

vec3 phong(){
    vec3 outcol = vec3(0.);
    for(int i=0; i<3; i++){
        Light l = lights[i];
        vec3 lvec = l.pos-pos;
        vec3 ln = normalize(lvec);
        vec3 Half = normalize(eye + ln);
        float cosine = clamp(dot(Nn, Half), 0., 1.);
        float roughness =  .01;
        cosine = pow(cosine, 1./roughness);
        vec3 col = vec3(1.0) * l.intensity * (cosine);
        outcol += col/dot(lvec, lvec);
    }
    return outcol;
}

void main(){
    Nn = mix(normalize(sn), normalize(cross(dFdx(pos), dFdy(pos))), flatShading);
    lights[0] = Light(vec3(50, 50, -20), vec3(1, .8, .7), 5000.);
    lights[1] = Light(vec3(-50, 0, 0), vec3(.6, .6, 1), 700.);
    lights[2] = Light(vec3(0, 0, -50), vec3(.6, .6, 1), 100.);
    vec3 ambient = vec3(.8, .8, 1.0);
    float ambientIntensity = .15;
    float diffint = .8;
    float specint = .7;
    outcolor = color.a * vec4(color.rgb * lambert()*diffint + ambient*ambientIntensity + phong()*specint, 1);
}
