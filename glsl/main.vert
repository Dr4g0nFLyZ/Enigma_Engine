#version 460 core
out vec3 fragColor;

// Update arrays to hold 6 elements for 2 triangles
vec2 positions[6] = vec2[](
    vec2(-0.5, -0.5), // T1 - bottom left (0)
    vec2( 0.5, -0.5), // T1 - bottom right (1)
    vec2( 0.5,  0.5), // T1 - top right (2)
    
    vec2(-0.5, -0.5), // T2 - bottom left (3) - same as 0
    vec2( 0.5,  0.5), // T2 - top right (4) - same as 2
    vec2(-0.5,  0.5)  // T2 - top left (5)
);

vec3 colors[6] = vec3[](
    vec3(1.0, 0.0, 0.0), // Red
    vec3(0.0, 1.0, 0.0), // Green
    vec3(0.0, 0.0, 1.0), // Blue
    
    vec3(1.0, 0.0, 0.0), // Red
    vec3(0.0, 0.0, 1.0), // Blue
    vec3(1.0, 1.0, 1.0)  // White
);

void main() {
    gl_Position = vec4(positions[gl_VertexID], 0.0, 1.0);
    fragColor = colors[gl_VertexID];
}