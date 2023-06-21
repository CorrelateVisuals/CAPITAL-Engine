#version 450

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec4 inSize;
layout(location = 3) in ivec4 inStates;

layout (binding = 0) uniform ParameterUBO {
    vec4 light;
    ivec2 gridDimensions;
    float gridHeight;
    float cellSize;
    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;

float random(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453); }
const lowp float frequency = 10.1;      mediump float amplitude = ubo.gridHeight;    const int octaves = 2;
const lowp float persistence = 0.2;     const lowp float lacunarity = 2.0;           const lowp float scale = 1.5;
float noise(vec2 p) { float total = -amplitude / 2.0;
                        for (int i = 0; i < octaves; i++) {
                            total += random(p * scale) * amplitude;
                            p *= lacunarity;
                            amplitude *= persistence;
                        }
                        total /= scale;
                        return total; }
vec4 position = vec4( inPosition.xy, noise( inPosition.xy ), inPosition.w );

const vec3 cubeVertices[17] = { {1, 1,  1},     // right front top
                                {-1, 1, 1},     // left front top
                                {-1, -1,  1},   // left back top
                                {1, -1,  1},    // right back top
                                {1, 1, -1},     // right front bottom
                                {-1, 1, -1},    // left front bottom
                                {-1, -1, -1},   // left back bottom
                                {1, -1, -1},    // right back bottom

                                {3,-1,-2},      // right back bottom extension center right
                                {3,1,-2},       // right front bottom extension center right

                                {3,-3,-2},      // right front bottom extension up right
                                {1,-3,-2},       // right back bottom extension up right

                                {3,3,-2},        // right front bottom extension down right
                                {1,3,-2},        // left front bottom extension down right

                                {-1,3,-2},        // left front bottom extension down center

                                {-3,1,-2},         // left back bottom extension down left
                                {-3,3,-2}          // left front bottom extension down left
                                };

const int cubeIndices[66] = {   0, 1, 2, 0, 2, 3, // top face
                                0, 3, 7, 0, 7, 4, // right face
                                0, 4, 5, 0, 5, 1, // front face
                                1, 5, 6, 1, 6, 2, // left face
                                2, 6, 7, 2, 7, 3, // back face
                                4, 7, 6, 4, 6, 5, // bottom face 

                                4,7,8,  4,8,9,    // right extra rectangle center
                                7,10,8, 10,7,11,  // right extra rectangle up
                                4,9,12, 12,13,4,  // right extra rectangle down   

                                4,13,14, 14,5,4,   // center extra rectangle down

                                5,14,15, 15,14,16  // left extra rectangle down

                                };

vec3 vertex = cubeVertices[cubeIndices[gl_VertexIndex]];

vec4 constructCube() { return position + vec4(vertex * inSize.x, vec2(0.0)); }

vec3 getNormal(){   int vertexPerFace = 3;      int faceIndex = gl_VertexIndex / vertexPerFace;
                    vec3 v0 = cubeVertices[cubeIndices[faceIndex * vertexPerFace]];
                    vec3 v1 = cubeVertices[cubeIndices[faceIndex * vertexPerFace + 1]];
                    vec3 v2 = cubeVertices[cubeIndices[faceIndex * vertexPerFace + 2]];
                    vec3 normal = normalize(cross(v1 - v0, v2 - v0));
                    return normal; }

vec4 worldPosition = ubo.model * constructCube();
vec4 viewPosition = ubo.view * worldPosition;
vec3 worldNormal = mat3(ubo.model) * getNormal();






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