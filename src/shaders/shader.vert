#version 450

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec4 inSize;
layout(location = 3) in vec4 gridSize;
layout(location = 4) in vec4 printGLSL;

layout(location = 0) out vec3 fragColor;

float size = 0.025f;
vec3 cubeVertices[8] = {
    vec3(-size, -size, -size), vec3(size, -size, -size), vec3(-size, size, -size),
    vec3(size, size, -size),   vec3(-size, -size, size), vec3(size, -size, size),
    vec3(-size, size, size),   vec3(size, size, size)};
int cubeIndices[36] = {
    0, 2, 3, 0, 3, 1, 4, 5, 7, 4, 7, 6, 1, 3, 7, 1, 7, 5,
    0, 4, 6, 0, 6, 2, 2, 6, 7, 2, 7, 3, 0, 1, 5, 0, 5, 4 };

void main() {

    if( inSize.x == 0.0f ){
        return;
    }

    gl_Position = inPosition.rgba + vec4( cubeVertices[ cubeIndices[ gl_VertexIndex ]], vec2(0.0));
    fragColor = vec3(inColor.rgb);
}
