#version 330

in vec3 sn;
in vec3 pos;
uniform vec3 color;
uniform mat4 modelView;

//layout (location = 0) out float shadow;
out vec4 shadow_position;
out vec4 shadow_normal;
out vec4 flux;

void main()
{
    //shadow = gl_FragCoord.z;
    shadow_position = vec4(pos, 1);
    shadow_normal = vec4(sn, 1);

    vec3 output = color;
    output *= abs(dot(normalize(sn), modelView[2].xyz));
    flux = vec4(output, 1);
}
