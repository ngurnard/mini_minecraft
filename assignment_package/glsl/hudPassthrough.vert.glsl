#version 150
// passthrough.vert.glsl:
// A vertex shader that simply passes along vertex data
// to the fragment shader without operating on it in any way.

uniform ivec2 u_Dimensions;

in vec4 vs_Pos;
in vec4 vs_Col;
in vec2 vs_UV;

out vec2 fs_UV;
out vec4 fs_Col;

void main()
{
    fs_Col = vs_Col;
    fs_UV = vec2(0.5 + 0.5f*vs_UV.x/float(u_Dimensions.x), 0.5 + 0.5f*vs_UV.y/float(u_Dimensions.y));
    gl_Position = vec4(vs_Pos.x/float(u_Dimensions.x), vs_Pos.y/float(u_Dimensions.y), vs_Pos.z, vs_Pos.w);
}
