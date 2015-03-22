#version 330

in vec3 sn;
in vec3 pos;
in vec3 cameraPos;
uniform vec4 diffuse_color = vec4(1);
uniform mat4 modelView;
uniform float coneangle;
uniform float intensity;
uniform mat4 view;

//layout (location = 0) out float shadow;
out vec4 shadow_position;
out vec4 shadow_normal;
out vec4 shadow_flux;

uniform int flatShading = 0;

vec3 Nn;

void main()
{
    //shadow = gl_FragCoord.z;
    Nn = mix(normalize(sn), normalize(cross(dFdx(pos), dFdy(pos))), flatShading);
    shadow_position = vec4(pos, 1);
    shadow_normal = vec4(Nn, 1);

    vec3 output = diffuse_color.rgb;
    vec3 dir = -normalize(cameraPos);
    float flux_atten = dot(dir, vec3(0, 0, 1));
    float lightangle = acos(flux_atten);
    float angleMask = smoothstep(coneangle, coneangle - 0.1, lightangle);
    output *= flux_atten * angleMask * intensity;
    shadow_flux = vec4(output, 1);
}
