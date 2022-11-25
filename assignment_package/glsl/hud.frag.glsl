#version 330
//#version 150

in vec4 fs_Col;
in vec2 fs_UV;

out vec4 out_Col; // ensure is a vec4

uniform sampler2D u_RenderedTexture;

void main()
{
    // Make crosshair color the inverse of the rendered environment behind
    vec3 invert = 1.f - texture(u_RenderedTexture, fs_UV).rgb;
    out_Col = vec4(invert, 1.f);
}
