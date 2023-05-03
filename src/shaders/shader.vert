#version 450

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec4 inSize;
layout(location = 3) in vec4 gridSize;

layout(location = 0) out vec3 fragColor;

void main() {

    gl_PointSize = inSize.x;
    gl_Position = inPosition.rgba;
    fragColor = vec3(inColor.rgb);
}
