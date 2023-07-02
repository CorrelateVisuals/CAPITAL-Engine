#version 450

layout(location = 0) in vec4 inColor;
layout(location = 0) out vec4 outColor;

const float contrast = 1.2; // Adjust contrast value as desired
const float gamma = 1.2; // Adjust gamma value as desired

void main() {
    // Increase contrast
    vec4 color = (inColor - 0.5) * contrast + 0.5;
    
    // Apply gamma correction
    color.rgb = pow(color.rgb, vec3(1.0 / gamma));
    
    outColor = color;
}
