#version 450

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec4 inSize;
layout(location = 3) in ivec4 inStates;
layout(location = 4) in vec4 inTileSidesHeight;
layout(location = 5) in vec4 inTileCornersHeight;

layout (binding = 0) uniform ParameterUBO {
    vec4 light;
    ivec2 gridDimensions;
    float gridHeight;
    float cellSize;
    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;

vec4 ts = inTileSidesHeight;
vec4 tc = inTileCornersHeight;
vec3 tileVertices[20] = {
    {1, 1, 1},    // 0 right front top
    {-1, 1, 1},   // 1 left front top
    {-1, -1, 1},  // 2 left back top
    {1, -1, 1},   // 3 right back top
    {1, 1, -1},   // 4 right front bottom
    {-1, 1, -1},  // 5 left front bottom
    {-1, -1, -1}, // 6 left back bottom
    {1, -1, -1},  // 7 right back bottom
    {3, -1, ts.x},  // 8 right back bottom extension center right
    {3, 1, ts.x},   // 9 right front bottom extension center right
    {3, -3, tc.x},  // 10 right front bottom extension up right         CORNER UP RIGHT
    {1, -3, ts.w},  // 11 right back bottom extension up right
    {3, 3, tc.y},   // 12 right front bottom extension down right     CORNER DOWN RIGHT
    {1, 3, ts.y},   // 13 left front bottom extension down right
    {-1, 3, ts.y},  // 14 left front bottom extension down center
    {-3, 1, ts.z},  // 15 left front bottom extension down left
    {-3, 3, tc.z},  // 16 left front bottom extension down left     CORNDER DOWN LEFT
    {-3, -1, ts.z}, // 17 left back bottom extension center left
    {-3, -3, tc.w}, // 18 left back bottom extension up left          CORNER UP LEFT
    {-1, -3, ts.w}  // 19 left back bottom extension up right
};

const int tileIndices[90] = {
    // Top face
    0, 1, 2, 0, 2, 3,
    // Right face
    0, 3, 7, 0, 7, 4,
    // Front face
    0, 4, 5, 0, 5, 1,
    // Left face
    1, 5, 6, 1, 6, 2,
    // Back face
    2, 6, 7, 2, 7, 3,
    // Bottom face
    4, 7, 6, 4, 6, 5,
    // Right rectangle center
    4, 7, 8, 4, 8, 9,
    // Right rectangle up
    10, 8, 7, 10, 7, 11,
    // Right rectangle down
    4, 9, 12, 12, 13, 4,
    // Center rectangle down
    4, 13, 14, 14, 5, 4,
    // Left rectangle down
    5, 14, 16, 15, 5, 16,
    // Left rectangle center
    17, 5, 15, 17, 6, 5,
    // Left rectangle up
    18, 6, 17, 18, 19, 6,
    // Center rectangle up
    19, 11, 6, 11, 7, 6,
    // Bottom face
    4, 5, 6, 4, 6, 7,
};

vec3 vertex = tileVertices[tileIndices[gl_VertexIndex]];

vec4 constructTile() {
    float cubeIndices = float(gl_VertexIndex < 36);
    return inPosition + vec4(vertex * (cubeIndices * inSize.x + (1.0 - cubeIndices) * 0.1), vec2(0.0));
}
    
vec3 getNormal(){   int vertexPerFace = 3;      int faceIndex = gl_VertexIndex / vertexPerFace;
                    vec3 v0 = tileVertices[tileIndices[faceIndex * vertexPerFace]];
                    vec3 v1 = tileVertices[tileIndices[faceIndex * vertexPerFace + 1]];
                    vec3 v2 = tileVertices[tileIndices[faceIndex * vertexPerFace + 2]];
                    vec3 normal = normalize(cross(v1 - v0, v2 - v0));
                    return normal; }

vec4 worldPosition = ubo.model * constructTile();
vec4 viewPosition = ubo.view * worldPosition;
vec3 worldNormal = mat3(ubo.model) * getNormal();

float gouraudShading(float brightness, float contrast) {
    vec3 lightDirection = normalize(ubo.light.rgb - worldPosition.xyz);
    float diffuseIntensity = max(dot(worldNormal, lightDirection), ubo.light.a);
    diffuseIntensity = (diffuseIntensity - 0.5) * contrast + 0.5;
    return diffuseIntensity * brightness;
}

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = inColor * gouraudShading(1.0f, 2.0f);
    gl_Position = ubo.projection * viewPosition;
}