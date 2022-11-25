#version 150
// ^ Change this to version 130 if you have compatibility issues

// Refer to the lambert shader files for useful comments
uniform int u_Time;

in vec4 fs_Col;

out vec4 out_Col;

in vec4 fs_CamPos;

void main()
{
    // Copy the color; there is no shading.
    out_Col = fs_Col;
}
