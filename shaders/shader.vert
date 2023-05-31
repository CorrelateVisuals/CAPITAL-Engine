#version 450
//#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec4 inSize;

layout (binding = 0) uniform ParameterUBO {
    int sqrtOfGrid;
    int time;                                   //TODO 'long long' : int64_t passedHours;
    mat4 model;
    mat4 view;
    mat4 projection;
    vec4 lightDirection;
    float cellSize;
} ubo;

const vec3 cubeVertices[8] = {vec3(-0.5f, -0.5f,-0.5f),   // 0
                        vec3(0.5f, -0.5f, -0.5f),   // 1
                        vec3(-0.5f, 0.5f, -0.5f),   // 2
                        vec3(0.5f,  0.5f, -0.5f),   // 3
                        vec3(-0.5f,-0.5f,  0.5f),   // 4
                        vec3(0.5f, -0.5f,  0.5f),   // 5
                        vec3(-0.5f, 0.5f,  0.5f),   // 6
                        vec3(0.5f,  0.5f,  0.5f)};  // 7
const vec3 cubeNormals[6] = { vec3( 0.0f, 0.0f,-1.0f),    // front
                        vec3( 0.0f, 0.0f, 1.0f),    // back
                        vec3(-1.0f, 0.0f, 0.0f),    // left
                        vec3( 1.0f, 0.0f, 0.0f),    // right
                        vec3( 0.0f, 1.0f, 0.0f),    // top
                        vec3( 0.0f,-1.0f, 0.0f)};   // bottom
const int cubeIndices[16] = {  0, 1, 2, 3, 6, 7, 4, 5, 2, 6, 0, 4, 1, 5, 3, 7 };

float random(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453); }
float noise(vec2 p) { 
    float total = 0.0; 
    float frequency = 0.1;
    float amplitude = 0.5;
    int octaves = 12;
    float persistence = 0.5;
    float lacunarity = 2.0;
    float scale = 0.8;

    for (int i = 0; i < octaves; i++) {
        total += random(p * scale) * amplitude;
        p *= lacunarity;
        amplitude *= persistence;
    }
    total /= scale;
    return total;
}
vec4 pos = vec4( inPosition.xy, noise( inPosition.xy ), inPosition.w );
vec4 constructCube(){ vec4 cube = pos + vec4( cubeVertices[ cubeIndices[ gl_VertexIndex ]] * inSize.x, vec2(0.0)); return cube; }
mat4 modelViewProjection(){ mat4 mvp = ubo.projection * ubo.view * ubo.model; return mvp; }
float phongLightning ( vec4 directionAndIntensity ) {
    int index = gl_VertexIndex / 4; 
    mat3 normalMatrix = mat3(ubo.model);
    vec3 normalWorldSpace = normalize(normalMatrix * cubeNormals[index]);
    float lightIntensity = directionAndIntensity.w + max(dot(normalWorldSpace, normalize(directionAndIntensity.xyz)), 0.0f);
    return lightIntensity; }

layout(location = 0) out vec3 fragColor;

void main() { 
    
    if( inSize.x == -1.0f ){ return; } else {
        gl_Position = modelViewProjection() * constructCube();  
        fragColor =  phongLightning(ubo.lightDirection.xyzw) * inColor.rgb;
    }    
}
