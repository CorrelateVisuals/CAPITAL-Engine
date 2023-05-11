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
vec3 cubeVertices[8] = {vec3(-0.5f, -0.5f,-0.5f),   // 0
                        vec3(0.5f, -0.5f, -0.5f),   // 1
                        vec3(-0.5f, 0.5f, -0.5f),   // 2
                        vec3(0.5f,  0.5f, -0.5f),   // 3
                        vec3(-0.5f,-0.5f,  0.5f),   // 4
                        vec3(0.5f, -0.5f,  0.5f),   // 5
                        vec3(-0.5f, 0.5f,  0.5f),   // 6
                        vec3(0.5f,  0.5f,  0.5f)};  // 7
vec3 cubeNormals[6] = { vec3( 0.0f, 0.0f,-1.0f),    // front
                        vec3( 0.0f, 0.0f, 1.0f),    // back
                        vec3(-1.0f, 0.0f, 0.0f),    // left
                        vec3( 1.0f, 0.0f, 0.0f),    // right
                        vec3( 0.0f, 1.0f, 0.0f),    // top
                        vec3( 0.0f,-1.0f, 0.0f)};   // bottom
int cubeIndices[] = {   0, 1, 2, 3, 6, 7, 4, 5,     // front and back faces
                        2, 6, 0, 4, 1, 5, 3, 7,     // connecting strips
                        2, 3, 6, 7, 4, 5, 0, 1,     // top and bottom faces
                        2 };                        // degenerate triangle to start new strip
vec4 constructCube(){ vec4 cube = inPosition.rgba + vec4( cubeVertices[ cubeIndices[ gl_VertexIndex ]] * scaler, vec2(0.0));
                      return cube; }


vec3 lightDirection = vec3(0.0f, 1.0f, 1.0f);
float ambient = 0.2f;

float setLightIntensity() {
    int index = gl_VertexIndex / 4; 
    vec3 normalWorldSpace = cubeNormals[index];
    mat3 normalMatrix = transpose(inverse(mat3(ubo.model)));
    normalWorldSpace = normalize(normalMatrix * normalWorldSpace);
    float lightIntensity = ambient + max(dot(normalWorldSpace, normalize(lightDirection)), 0.0f);
    return lightIntensity;
}


layout(location = 0) out vec3 fragColor;

void main() { if( inSize.x == -1.0f ){ return; } // Skip all dead cells.
    
    gl_Position = modelViewProjection() * constructCube();  
    fragColor =  setLightIntensity() * inColor.rgb;
}
