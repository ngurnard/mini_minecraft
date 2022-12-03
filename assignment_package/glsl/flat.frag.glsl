#version 150
// ^ Change this to version 130 if you have compatibility issues

// Refer to the lambert shader files for useful comments
uniform int u_Time;

in vec4 fs_Col;
in vec3 fs_Z;

out vec4 out_Col;

in vec4 fs_CamPos;

void main()
{
    // Copy the color; there is no shading.
    out_Col = fs_Col;

    float Z = length(fs_Z.xz) / 175.f;
    float fogfalloff = clamp(1.15 - exp(-5.5f * (Z - 1.0f)), 0.f, 1.f);
    out_Col = vec4(fs_Col.rgb, fs_Col.a * (1-fogfalloff));
}
