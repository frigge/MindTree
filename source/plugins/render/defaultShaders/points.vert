#version 330
in vec3 P;
in vec3 C;
out vec3 vertex_color;
uniform float has_vertex_color = 0.0f;
uniform mat4 modelView;
uniform mat4 projection;
uniform int size = 5;
uniform vec4 pointcolor = vec4(1);

void main(){
    vertex_color = mix(pointcolor.rgb, C, has_vertex_color);

    gl_Position = projection * modelView * vec4(P, 1.);
    gl_PointSize = size;
};
