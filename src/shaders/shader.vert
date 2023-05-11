#version 450
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec4 inSize;

layout (binding = 0) uniform ParameterUBO {
    int passedHours; //TODO 'long long' : int64_t passedHours;
    int gridSize;

    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;

mat4 modelViewProjection(){ mat4 mvp = ubo.projection * ubo.view * ubo.model; return mvp; }

float scaler = inSize.x;
vec3 cubeVertices[8] = { vec3(-1.0f, -1.0f, -1.0f), vec3(1.0f, -1.0f, -1.0f), vec3(-1.0f, 1.0f, -1.0f), vec3(1.0f, 1.0f, -1.0f),   
                         vec3(-1.0f, -1.0f, 1.0f),  vec3(1.0f, -1.0f, 1.0f),  vec3(-1.0f, 1.0f, 1.0f),  vec3(1.0f, 1.0f, 1.0f)};
int cubeIndices[36] = { 0, 2, 3, 0, 3, 1, 4, 5, 7, 4, 7, 6, 1, 3, 7, 1, 7, 5,
                        0, 4, 6, 0, 6, 2, 2, 6, 7, 2, 7, 3, 0, 1, 5, 0, 5, 4 };
vec3 cubeNormals[8] = { vec3(0, 0, 0), vec3(1, 0, 0), vec3(0, 1, 0), vec3(1, 1, 0),
                        vec3(0, 0, 1), vec3(1, 0, 1), vec3(0, 1, 1), vec3(1, 1, 1)};
vec4 constructCube(){
    vec4 cube = inPosition.rgba + vec4( cubeVertices[ cubeIndices[ gl_VertexIndex ]] * scaler, vec2(0.0));
    return cube;
}

vec3 lightDirection = {1.0f, 1.0f, 0.0f};
float ambient = 0.02f;
float setLightIntensity(){
    mat3 normalMatrix = transpose(inverse(mat3(ubo.model)));
    vec3 normalWorldSpace = normalize(normalMatrix * cubeNormals[ cubeIndices[ gl_VertexIndex ]]);
    float lightIntensity = ambient + max(dot(normalWorldSpace, normalize(lightDirection)), 0);
    return lightIntensity;
}

layout(location = 0) out vec3 fragColor;

void main() { if( inSize.x == -1.0f ){ return; } // Skip all dead cells.
    
    gl_Position = modelViewProjection() * constructCube();  
    fragColor =  setLightIntensity() * inColor.rgb;
}
