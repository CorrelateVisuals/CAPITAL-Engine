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

vec3 cubeVertices[24] = {{1, 1, 1},    {-1, 1, 1}, {-1, -1, 1},  {1, -1, 1},  // v0,v1,v2,v3 (front)
                         {1, 1, 1},    {1, -1, 1}, {1, -1, -1},  {1, 1, -1},  // v0,v3,v4,v5 (right)
                         {1, 1, 1},    {1, 1, -1}, {-1, 1, -1},  {-1, 1, 1},  // v0,v5,v6,v1 (top)
                         {-1, 1, 1},   {-1, 1, -1}, {-1, -1, -1}, {-1, -1, 1},  // v1,v6,v7,v2 (left)
                         {-1, -1, -1}, {1, -1, -1}, {1, -1, 1},   {-1, -1, 1},  // v7,v4,v3,v2 (bottom)
                         {1, -1, -1},  {-1, -1, -1}, {-1, 1, -1},  {1, 1, -1}};  // v4,v7,v6,v5 (back)
vec3 cubeNormals[24] = {
    {0, 0, 1},  {0, 0, 1},  {0, 0, 1},  {0, 0, 1},    // v0,v1,v2,v3 (front)
    {1, 0, 0},  {1, 0, 0},  {1, 0, 0},  {1, 0, 0},    // v0,v3,v4,v5 (right)
    {0, 1, 0},  {0, 1, 0},  {0, 1, 0},  {0, 1, 0},    // v0,v5,v6,v1 (top)
    {-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0},   // v1,v6,v7,v2 (left)
    {0, -1, 0}, {0, -1, 0}, {0, -1, 0}, {0, -1, 0},   // v7,v4,v3,v2 (bottom)
    {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}};  // v4,v7,v6,v5 (back)
vec3 cubeColors[24] = {
    {1, 1, 1}, {1, 1, 0}, {1, 0, 0}, {1, 0, 1},   // v0,v1,v2,v3 (front)
    {1, 1, 1}, {1, 0, 1}, {0, 0, 1}, {0, 1, 1},   // v0,v3,v4,v5 (right)
    {1, 1, 1}, {0, 1, 1}, {0, 1, 0}, {1, 1, 0},   // v0,v5,v6,v1 (top)
    {1, 1, 0}, {0, 1, 0}, {0, 0, 0}, {1, 0, 0},   // v1,v6,v7,v2 (left)
    {0, 0, 0}, {0, 0, 1}, {1, 0, 1}, {1, 0, 0},   // v7,v4,v3,v2 (bottom)
    {0, 0, 1}, {0, 0, 0}, {0, 1, 0}, {0, 1, 1}};  // v4,v7,v6,v5 (back)
int cubeIndices[36] = {0,  1,  2,  2,  3,  0,    // front 
                       4,  5,  6,  6,  7,  4,    // right
                       8,  9,  10, 10, 11, 8,    // top
                       12, 13, 14, 14, 15, 12,   // left
                       16, 17, 18, 18, 19, 16,   // bottom
                       20, 21, 22, 22, 23, 20};  // back

vec4 constructCube(){ return position + vec4( cubeVertices[ cubeIndices[ gl_VertexIndex ]] * inSize.x, vec2(0.0)); }

float quadIllumination() { 
    switch (gl_VertexIndex / 6){
        case 0: return 1.0f;
        case 1: return 0.8f;
        case 2: return 0.7f; 
        case 3: return 0.4f;
        case 4: return 0.3f;
        case 5: return 0.1f;
        default: return 0.2f;
    }
}

layout(location = 0) out vec4 fragColor;

void main() {
    if (inStates.x == -1) {
        gl_Position = modelViewProjection() * constructCube();
        fragColor   = inColor * quadIllumination();
        return;
    } else {
        gl_Position = modelViewProjection() * constructCube();
        fragColor   = inColor * quadIllumination();
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