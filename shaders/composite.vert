#version 450

layout(location = 0) out vec2 fragUV;

void main() {
    // Fullscreen triangle using gl_VertexIndex (0, 1, 2)
    const vec2 pos[3] = vec2[](
        vec2(-1.0, -1.0), // bottom-left
        vec2( 3.0, -1.0), // bottom-right far (for covering entire screen)
        vec2(-1.0,  3.0)  // top-left far (for covering entire screen)
    );
    const vec2 uv[3] = vec2[](
        vec2(0.0, 0.0),
        vec2(2.0, 0.0),
        vec2(0.0, 2.0)
    );

    fragUV = uv[gl_VertexIndex];
    gl_Position = vec4(pos[gl_VertexIndex], 0.0, 1.0);
}