#version 330

in vec3 cameraPos;

out float shadow;
void main()
{
    shadow = gl_FragCoord.z;
}
