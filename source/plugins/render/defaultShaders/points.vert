#version 330
in vec3 P;
in vec3 C;
out vec3 vertex_color;
uniform bool has_vertex_color = false;
uniform mat4 modelView;
uniform mat4 projection;
uniform int size = 5;
uniform vec4 pointcolor = vec4(1);

void main(){
    if(has_vertex_color)
        vertex_color = C;
    else
        vertex_color = pointcolor.rgb;

    gl_Position = projection * modelView * vec4(P, 1.);
    gl_PointSize = size;
};
