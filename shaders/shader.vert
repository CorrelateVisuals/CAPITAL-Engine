#version 450

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec4 inSize;
layout(location = 3) in ivec4 inStates;

layout(push_constant, std430) uniform pc { vec4 data; };

layout (binding = 0) uniform ParameterUBO {
    vec4 light;
    ivec2 gridDimensions;
    float gridHeight;
    float cellSize;
    mat4 model;
    mat4 view;
    mat4 projection;
    int passedHours;
} ubo;

float random(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453); }
float total = -ubo.gridHeight / 2;  float frequency = 10.1;      float amplitude = ubo.gridHeight;   
int octaves = 2;                    float persistence = 0.2;    float lacunarity = 2.0;     float scale = 1.5;
float noise(vec2 p) {   for (int i = 0; i < octaves; i++) 
                        { total += random(p * scale) * amplitude; 
                        p *= lacunarity; amplitude *= persistence; }
                        total /= scale; 
                        return total; }
vec4 position = vec4( inPosition.xy, noise( inPosition.xy ), inPosition.w );

const vec3 cubeVertices[24] = {
    {1, 1, 1},   {-1, 1, 1},  {-1, -1, 1},  {1, -1, 1},  {1, 1, 1},   {1, -1, 1},  
    {1, -1, -1}, {1, 1, -1},  {1, 1, 1},    {1, 1, -1},  {-1, 1, -1}, {-1, 1, 1},   
    {-1, 1, 1},  {-1, 1, -1}, {-1, -1, -1}, {-1, -1, 1}, {-1, -1, -1},{1, -1, -1}, 
    {1, -1, 1},  {-1, -1, 1}, {1, -1, -1},  {-1, -1, -1},{-1, 1, -1}, {1, 1, -1}};
vec3 cubeNormals[24] = {
    {0, 0, 1},  {0, 0, 1},  {0, 0, 1},  {0, 0, 1},  {1, 0, 0},  {1, 0, 0},
    {1, 0, 0},  {1, 0, 0},  {0, 1, 0},  {0, 1, 0},  {0, 1, 0},  {0, 1, 0},
    {-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}, {0, -1, 0}, {0, -1, 0},
    {0, -1, 0}, {0, -1, 0}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}};
const int cubeIndices[36] = {0,  1,  2,  2,  3,  0,  4,  5,  6,  6,  7,  4,
                             8,  9,  10, 10, 11, 8,  12, 13, 14, 14, 15, 12,
                             16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20};
                             
vec4 constructCube(){ return position + vec4( cubeVertices[ cubeIndices[ gl_VertexIndex ]] * inSize.x, vec2(0.0)); }

vec4 worldPosition = ubo.model * constructCube() ;
vec4 viewPosition = ubo.view * worldPosition;
vec3 worldNormal = mat3(ubo.model) * cubeNormals[cubeIndices[gl_VertexIndex]];

float gouraudShading() {vec3 lightDirection = normalize(ubo.light.rgb - worldPosition.xyz);
                        float diffuseIntensity = max(dot(worldNormal, lightDirection), ubo.light.a);
                        return diffuseIntensity; }

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = inColor * gouraudShading();
    gl_Position = ubo.projection * viewPosition;
}










/*vec3 cubeNormals[24] = {
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
    {0, 0, 1}, {0, 0, 0}, {0, 1, 0}, {0, 1, 1}};  // v4,v7,v6,v5 (back)*/

/*    
vec3 lightPosition = vec3(1.0, 2.5, 5.0);

vec4 worldPosition = ubo.model * constructCube();
    vec4 viewPosition = ubo.view * worldPosition;
    
    vec3 worldNormal = mat3(transpose(inverse(ubo.model))) * cubeNormals[cubeIndices[ gl_VertexIndex ]];
    vec3 lightDirection = normalize(lightPosition - worldPosition.xyz);
    
    float diffuseIntensity = max(dot(worldNormal, lightDirection), 0.0);
    
    fragColor = inColor * diffuseIntensity;
    
    gl_Position = ubo.projection * viewPosition;*/







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