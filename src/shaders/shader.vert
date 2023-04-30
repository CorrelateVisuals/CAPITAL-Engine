#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 newVariableToShader;

layout(location = 0) out vec3 fragColor;

void main() {

    gl_PointSize = 20.0;
    gl_Position = vec4(inPosition.xy, 1.0, 1.0);
   // fragColor = inColor.rgb;
   fragColor = vec3(newVariableToShader.rg, 1.0);
}
