#version 430

uniform sampler2D outnormal;
uniform sampler2D depth;
in vec2 st;

out vec4 reflection;

void main()
{
    vec4 n = texture(outnormal, st);
    if(n.a < 0.5) discard;
    reflection = vec4(1, 0, 0, .1);
}
