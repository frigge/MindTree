#version 430

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform samplerBuffer polygon_colors;

out vec3 poly_color;

void main() {
    vec4 color = texelFetch(polygon_colors, gl_PrimitiveIDIn);
    gl_Position = gl_in[0].gl_Position;
    //poly_colors[0] = color;
    poly_color = vec3(mod(gl_PrimitiveIDIn, 2),
                      mod(gl_PrimitiveIDIn + 1, 2),
                      mod(gl_PrimitiveIDIn + 2, 2));
    EmitVertex();
    gl_Position = gl_in[1].gl_Position;
    //poly_colors[1] = color;
    poly_color = vec3(mod(gl_PrimitiveIDIn, 2),
                      mod(gl_PrimitiveIDIn + 1, 2),
                      mod(gl_PrimitiveIDIn + 2, 2));
    EmitVertex();
    gl_Position = gl_in[2].gl_Position;
    //poly_colors[2] = color;
    poly_color = vec3(mod(gl_PrimitiveIDIn, 2),
                      mod(gl_PrimitiveIDIn + 1, 2),
                      mod(gl_PrimitiveIDIn + 2, 2));
    EmitVertex();
    EndPrimitive();
}
