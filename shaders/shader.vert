#version 450

layout(location = 0) in vec4 inpositionition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec4 inSize;
layout(location = 3) in ivec4 inStates;

layout (binding = 0) uniform ParameterUBO {
    ivec2 gridDimensions;
    int time;
    mat4 model;
    mat4 view;
    mat4 projection;
    float cellSize;
    float gridHeight;
} ubo;
mat4 modelViewProjection(){ return ubo.projection * ubo.view * ubo.model; }
float random(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453); }
float total = -ubo.gridHeight / 2;  float frequency = 10.1;      float amplitude = ubo.gridHeight;   
int octaves = 2;                    float persistence = 0.2;    float lacunarity = 2.0;     float scale = 1.5;
float noise(vec2 p) {   for (int i = 0; i < octaves; i++) 
                        { total += random(p * scale) * amplitude; 
                        p *= lacunarity; amplitude *= persistence; }
                        total /= scale; 
                        return total; }
vec4 position = vec4( inpositionition.xy, noise( inpositionition.xy ), inpositionition.w );
const vec3 cubeVertices[8] ={ { -0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, -0.5f},
                              { -0.5f, -0.5f, 0.5f},  {0.5f, -0.5f, 0.5f},  {-0.5f, 0.5f, 0.5f},  {0.5f, 0.5f, 0.5f} };
const int cubeIndices[36] = {
    0, 1, 2, 2, 1, 3,     // front face
    1, 5, 3, 3, 5, 7,     // right face
    5, 4, 7, 7, 4, 6,     // back face
    4, 0, 6, 6, 0, 2,     // left face
    4, 5, 0, 0, 5, 1,     // bottom face
    2, 3, 6, 6, 3, 7      // top face
};

vec4 constructCube(){ return position + vec4( cubeVertices[ cubeIndices[ gl_VertexIndex ]] * inSize.x, vec2(0.0)); }

float quadIllumination() { 
    switch (gl_VertexIndex / 6){
        case 0: return 0.1f;     // bottom
        case 1: return 1.0f;     // top
        case 2: return 0.4f;     // right
        case 3: return 0.8f;     // left
        case 4: return 0.9f;     // front
        case 5: return 0.2f;     // back
        default: return 0.2f;    // fallback value
    }
}

layout(location = 0) out vec3 fragColor;

void main() {
    if (inStates.x == -1) {
        gl_Position = modelViewProjection() * constructCube();
        fragColor   = inColor.rgb * quadIllumination();
        return;
    } else {
        gl_Position = modelViewProjection() * constructCube();
        fragColor   = inColor.rgb * quadIllumination();
        return;
    }
}











// const vec3 cubeVertices[8] = {vec3(-0.5f, -0.5f,-0.5f),   // 0
//                         vec3(0.5f, -0.5f, -0.5f),   // 1
//                         vec3(-0.5f, 0.5f, -0.5f),   // 2
//                         vec3(0.5f,  0.5f, -0.5f),   // 3
//                         vec3(-0.5f,-0.5f,  0.5f),   // 4
//                         vec3(0.5f, -0.5f,  0.5f),   // 5
//                         vec3(-0.5f, 0.5f,  0.5f),   // 6
//                         vec3(0.5f,  0.5f,  0.5f)};  // 7
// const int cubeIndices[16] = {  0, 1, 2, 3, 6, 7, 4, 5, 2, 6, 0, 4, 1, 5, 3, 7 };
/*const vec3 cubeNormals[6] = { { 0.0f, 0.0f,-1.0f},  {0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f},
                              { 1.0f, 0.0f, 0.0f},  {0.0f, 1.0f, 0.0f}, {0.0f,-1.0f, 0.0f} };*/