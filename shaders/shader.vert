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

vec4 side = inTileSidesHeight;
vec4 corner = inTileCornersHeight;
vec3 tileVertices[20] = {
    // Cube
    {1, 1, 1},    // 0 right front top
    {-1, 1, 1},   // 1 left front top
    {-1, -1, 1},  // 2 left back top
    {1, -1, 1},   // 3 right back top
    {1, 1, -1},   // 4 right front bottom
    {-1, 1, -1},  // 5 left front bottom
    {-1, -1, -1}, // 6 left back bottom
    {1, -1, -1},  // 7 right back bottom

    // Grid
    {3, -1, side.x},    // 8 right back bottom extension center right
    {3, 1, side.x},     // 9 right front bottom extension center right
    {3, -3, corner.x},  // 10 right front bottom extension up right   . . . CORNER UP RIGHT
    {1, -3, side.w},    // 11 right back bottom extension up right
    {3, 3, corner.y},   // 12 right front bottom extension down right . . . CORNER DOWN RIGHT
    {1, 3, side.y},     // 13 left front bottom extension down right
    {-1, 3, side.y},    // 14 left front bottom extension down center
    {-3, 1, side.z},    // 15 left front bottom extension down left
    {-3, 3, corner.z},  // 16 left front bottom extension down left   . . . CORNDER DOWN LEFT
    {-3, -1, side.z},   // 17 left back bottom extension center left
    {-3, -3, corner.w}, // 18 left back bottom extension up left    . . . . CORNER UP LEFT
    {-1, -3, side.w}    // 19 left back bottom extension up right
};

const int tileIndices[90] = {
    0, 1, 2, 0, 2, 3,       // Top face
    0, 3, 7, 0, 7, 4,       // Right face
    0, 4, 5, 0, 5, 1,       // Front face
    1, 5, 6, 1, 6, 2,       // Left face
    2, 6, 7, 2, 7, 3,       // Back face
    4, 7, 6, 4, 6, 5,       // Bottom face

    4, 7, 8, 4, 8, 9,       // Right rectangle center
    10, 8, 7, 10, 7, 11,    // Right rectangle up
    4, 9, 12, 12, 13, 4,    // Right rectangle down
    4, 13, 14, 14, 5, 4,    // Center rectangle down
    5, 14, 16, 15, 5, 16,   // Left rectangle down
    17, 5, 15, 17, 6, 5,    // Left rectangle center
    18, 6, 17, 18, 19, 6,   // Left rectangle up
    19, 11, 6, 11, 7, 6,    // Center rectangle up
    4, 5, 6, 4, 6, 7,       // Bottom face
};
vec3 vertex = tileVertices[tileIndices[gl_VertexIndex]];

vec4 constructTile() {
    float cubeIndices = float(gl_VertexIndex < 36);
    float adjustSize = cubeIndices * inSize.x + (1.0 - cubeIndices) * 0.1;
    float floorOffset = adjustSize - inSize.x; // Calculate the floor offset
    
    vec4 position = inPosition;
    position.xyz += vertex.xyz * adjustSize; // Adjust the x, y, and z coordinates
    
    return position;
}


vec3 getNormal(){   
    int vertexPerFace = 3;      
    int faceIndex = gl_VertexIndex / vertexPerFace;
    vec3 v0 = tileVertices[tileIndices[faceIndex * vertexPerFace]];
    vec3 v1 = tileVertices[tileIndices[faceIndex * vertexPerFace + 1]];
    vec3 v2 = tileVertices[tileIndices[faceIndex * vertexPerFace + 2]];
    vec3 normal = normalize(cross(v1 - v0, v2 - v0));
    return normal; 
}

vec4 worldPosition = ubo.model * constructTile();
vec4 viewPosition =  ubo.view * worldPosition;
vec3 worldNormal =   mat3(ubo.model) * getNormal();

float gouraudShading(float brightness, float contrast, float emit, float gamma) {
    vec3 lightDirection = normalize(ubo.light.rgb - worldPosition.xyz);
    float diffuseIntensity = max(dot(worldNormal, lightDirection), emit);

    diffuseIntensity = (diffuseIntensity - 0.5) * contrast + 0.5;
    diffuseIntensity = pow(diffuseIntensity, 1.0 / gamma);

    return diffuseIntensity * brightness;
}


layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = inColor * gouraudShading(1.0f, 1.75f, 0.3f, 1.2f);
    gl_Position = ubo.projection * viewPosition;
}