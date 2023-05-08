#version 450

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec4 inSize;
layout(location = 3) in vec4 gridSize;
layout(location = 4) in vec4 printGLSL;

layout(location = 0) out vec3 fragColor;

float triangleSize = 0.03f;

vec2 triangle[3] = {
    vec2(0.0f, -triangleSize),
    vec2(triangleSize, triangleSize),
    vec2(-triangleSize, triangleSize)
};

void main() {

   // gl_PointSize = printGLSL.x / 25 + 25;
    gl_PointSize = 15;
    gl_Position = inPosition.rgba + vec4(triangle[gl_VertexIndex], vec2(0.0));

    fragColor = vec3(inColor.rgb);
}
