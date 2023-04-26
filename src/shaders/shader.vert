#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 velocityNOTUSED;
layout(location = 3) in vec3 normalsNOTUSED; 
layout(location = 4) in vec3 uvNOTUSED;

vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

layout(location = 0) out vec3 fragColor;

void main() {

    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragColor = colors[gl_VertexIndex];

//    gl_PointSize = 14.0;
//    gl_Position = vec4(inPosition.xy, 1.0, 1.0);
//    fragColor = inColor.rgb;
}
