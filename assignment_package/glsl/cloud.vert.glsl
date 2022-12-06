#version 150
// ^ Change this to version 130 if you have compatibility issues

// Refer to the lambert shader files for useful comments

uniform mat4 u_Model;
uniform mat4 u_ViewProj;

in vec4 vs_Pos;
in vec4 vs_Col;

out vec4 fs_Col;
out vec4 fs_Pos;

//uniform vec4 u_CamPos;
uniform vec3 u_Eye;
out vec4 fs_CamPos;

out vec3 fs_Z;

void main()
{
    fs_Col = vs_Col;
    vec4 modelposition = u_Model * vs_Pos;
    fs_Pos = modelposition;

    fs_Z = modelposition.xyz - u_Eye;

    //built-in things to pass down the pipeline
    gl_Position = u_ViewProj * modelposition;

    fs_CamPos = vec4(u_Eye, 1); // uniform handle for the camera position instead of the inverse

}
