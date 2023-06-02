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
    vec4 lightDirection;
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
const vec3 cubeNormals[6] = { { 0.0f, 0.0f,-1.0f},  {0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f},
                              { 1.0f, 0.0f, 0.0f},  {0.0f, 1.0f, 0.0f}, {0.0f,-1.0f, 0.0f} };
const int cubeIndices[36] = { 0, 2, 3, 0, 3, 1, 4, 5, 7, 4, 7, 6, 1, 3, 7, 1, 7, 5,
                              0, 4, 6, 0, 6, 2, 2, 6, 7, 2, 7, 3, 0, 1, 5, 0, 5, 4 };
vec4 constructCube(){ return position + vec4( cubeVertices[ cubeIndices[ gl_VertexIndex ]] * inSize.y, vec2(0.0)); }
float light() { return max(dot(cubeNormals[gl_VertexIndex / 6], ubo.lightDirection.xyz), 0.0) + ubo.lightDirection.w; }

layout(location = 0) out vec3 fragColor;

void main() {
    if (inStates.x == -1) {
        gl_Position = modelViewProjection() * constructCube();
        fragColor   = inColor.rgb * light();
        return;
    } else {
        gl_Position = modelViewProjection() * constructCube();
        fragColor   = inColor.rgb * light();
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