#version 430
in vec2 st;
uniform ivec2 resolution;
uniform sampler2D outputtx;

out vec4 color;

void main(){
    ivec2 px = ivec2(st * resolution);
    vec2 tiles = vec2((px % 32)) / 32.;
    float x = smoothstep(.4, .5, tiles.x);
    float y = smoothstep(.4, .5, tiles.y);
    float both = 1 - x * y;
    float pattern = max(x, y) * both;

    vec4 col1 = vec4(.3, .3, .3, 1);
    vec4 col2 = vec4(.8, .8, .8, 1);

    vec4 checker = mix(col1, col2, pattern);

    vec4 col = texture(outputtx, st);
    col.a = clamp(col.a, 0, 1);
    color = col;
    //color = mix(checker, col, col.a);
};
