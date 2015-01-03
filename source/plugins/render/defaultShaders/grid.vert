#version 330
uniform mat4 modelView;
uniform mat4 projection;
uniform vec4 alternatingColor = vec4(1);
uniform vec4 fillColor = vec4(1);
uniform vec4 borderColor = vec4(1);

uniform ivec2 gridRes;

in vec3 P;
out vec4 col;

void main(){
   gl_Position = projection * modelView * vec4(P, 1);

   int currline = gl_VertexID / 2;
   if(currline > gridRes.x) currline -= 1;

   col = mix(alternatingColor, borderColor, min(1., float(currline % 5)));

   int xaxis = int(ceil(float(gridRes.y) / 2));
   int yaxis = int(ceil(float(gridRes.x) / 2));

   if(currline == yaxis)
       col = vec4(0.3, 0.3, 1.0, .8);

   if(currline - gridRes.x == xaxis)
       col = vec4(1.0, 0.3, 0.3, .8);
};
