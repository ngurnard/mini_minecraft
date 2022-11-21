//#version 150
#version 330
// noOp.vert.glsl:
// A fragment shader used for post-processing that simply reads the
// image produced in the first render pass by the surface shader
// and outputs it to the frame buffer


in vec2 fs_UV;

out vec4 out_Col; // ensure is a vec4

uniform sampler2D u_RenderedTexture;

void main()
{
    out_Col = vec4(texture(u_RenderedTexture, fs_UV).rgb, 1);
}
