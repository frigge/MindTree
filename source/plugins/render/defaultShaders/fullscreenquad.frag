#version 330
uniform vec4 color = vec4(1);
uniform vec4 bgcolor;

in vec2 st;
uniform sampler2D overlay;
uniform sampler2D outcolor;

out vec4 final_color;
void main(){
    vec4 col = texture(outcolor, st);
    vec4 over = texture(overlay, st);

    vec4 _bg = mix(bgcolor * .85, bgcolor, smoothstep(0., .5, length(vec2(.5) - st)));

    final_color = mix(bgcolor, col, col.a);
    final_color = mix(final_color, over, over.a);
};
