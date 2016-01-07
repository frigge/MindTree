#version 430

uniform sampler2D outnormal;
uniform sampler2D outworldposition;
uniform sampler2D depth;
uniform sampler2D shading_out;
in vec2 st;

vec3 camPos;
uniform mat4 view;
uniform mat4 projection;

const float MAXDIST = 5;
const int MAXSTEPS = 50;
const float thickness = 0.2;

out vec4 reflection;
out vec4 reflection_dir;

bool traceWorld(vec3 start, vec3 dir, out vec2 coord)
{
    vec3 d = dir * (MAXDIST / MAXSTEPS);

    vec3 s = start;
    int i = 0;
    for(; i < MAXSTEPS; ++i, s += d) {
        vec4 ps = projection * view * vec4(s, 1);
        ps /= ps.w;
        ps.xy += vec2(1);
        ps.xy *= 0.5;
        coord = ps.st;
        vec3 txpos = texture(outworldposition, ps.st).xyz;
        if (distance(s, camPos) > distance(txpos, camPos)
            && distance(s, txpos) < thickness) {
            return true;
        }
    }
}

bool traceScreen(vec3 start, vec3 dir, out vec2 coord)
{
    return false;
}

void main()
{
    camPos = (inverse(view) * vec4(0, 0, 0, 1)).xyz;
    vec4 n = texture(outnormal, st);
    if(n.a < 0.5) discard;

    vec3 pos = texture(outworldposition, st).xyz;
    vec3 ref = normalize(reflect(normalize(pos - camPos), n.xyz));
    reflection_dir = vec4(ref, 1);
    vec2 coord;
    if(traceWorld(pos, ref, coord))
        reflection = vec4(texture(shading_out, coord).rgb, 1);
}
