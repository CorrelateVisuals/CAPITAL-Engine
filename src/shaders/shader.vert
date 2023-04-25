#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 velocityNOTUSED;
layout(location = 3) in vec3 normalsNOTUSED; 
layout(location = 4) in vec3 uvNOTUSED;

layout(location = 0) out vec3 fragColor;

void main() {

    gl_PointSize = 14.0;
    gl_Position = vec4(inPosition.xy, 1.0, 1.0);
    fragColor = inColor.rgb;
}
