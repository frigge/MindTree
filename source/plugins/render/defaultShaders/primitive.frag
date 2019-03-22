#version 330
uniform vec4 fillColor = vec4(1);
uniform vec4 borderColor = vec4(1);
uniform int isBorder=0;
uniform ivec2 pointerpos;
uniform ivec2 resolution;

out vec4 overlay;
out vec4 outcolor;

void main(){
	vec2 pos = gl_FragCoord.xy;
	vec2 pointer = vec2(pointerpos);

	float dist = distance(pos, pointer);

    vec4 color = mix(fillColor, borderColor, isBorder);
    overlay = color;
    outcolor = mix(color, vec4(1,1, 0, 1), dist);
};
