#version 330

in vec3 cameraPos;

out vec4 shadow;
void main()
{
    shadow = vec4(vec3(cameraPos.z), 1);
}
