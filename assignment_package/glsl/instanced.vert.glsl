#version 400

//This is a vertex shader. While it is called a "shader" due to outdated conventions, this file
//is used to apply matrix transformations to the arrays of vertex data passed to it.
//Since this code is run on your GPU, each vertex is transformed simultaneously.
//If it were run on your CPU, each vertex would have to be processed in a FOR loop, one at a time.
//This simultaneous transformation allows your program to run much faster, especially when rendering
//geometry with millions of vertices.

uniform mat4 u_Model;       // The matrix that defines the transformation of the
                            // object we're rendering. In this assignment,
                            // this will be the result of traversing your scene graph.

uniform mat4 u_ViewProj;    // The matrix that defines the camera's transformation.
                            // We've written a static matrix for you to use for HW2,
                            // but in HW3 you'll have to generate one yourself

in vec4 vs_Pos;             // The array of vertex positions passed to the shader
in vec4 vs_Nor;             // The array of vertex normals passed to the shader

in vec2 vs_UV;              // UV coords for texture to pass thru to fragment shader
in float vs_Anim;           // 0.f or 1.f To pass thru to fragment shader
in float vs_T2O;

in vec3 vs_ColInstanced;    // The array of vertex colors passed to the shader.
in vec3 vs_OffsetInstanced; // Used to position each instance of the cube

out vec4 fs_Pos;
out vec4 fs_Nor;            // The array of normals that has been transformed by u_ModelInvTr. This is implicitly passed to the fragment shader.
out vec4 fs_LightVec;       // The direction in which our virtual light lies, relative to each vertex. This is implicitly passed to the fragment shader.
out vec4 fs_Col;            // The color of each vertex. This is implicitly passed to the fragment shader.

out vec2 fs_UVs;
out float fs_Anim;
out float fs_dimVal;
out float fs_T2O;

const vec4 lightDir = normalize(vec4(0.5, 1, 0.75, 0));  // The direction of our virtual light, which is used to compute the shading of
                                        // the geometry in the fragment shader.

uniform vec4 u_CamPos;
out vec4 fs_CamPos;

float random1(vec3 p) {
    return fract(sin(dot(p, vec3(127.1, 311.7, 191.999)))
                 *43758.5453);
}

void main()
{

    fs_UVs = vs_UV;
    fs_Anim = vs_Anim;
    fs_T2O = vs_T2O;

    vec4 dim = u_Model * vs_Pos;
    fs_dimVal = random1(dim.xyz/100.f);


    vec4 offsetPos = vs_Pos + vec4(vs_OffsetInstanced, 0.);
    fs_Pos = offsetPos;
    fs_Col = vec4(vs_ColInstanced, 1.);                         // Pass the vertex colors to the fragment shader for interpolation

    fs_Nor = vs_Nor;

    fs_LightVec = (lightDir);  // Compute the direction in which the light source lies

    fs_CamPos = u_CamPos; // uniform handle for the camera position instead of the inverse

    gl_Position = u_ViewProj * offsetPos;// gl_Position is a built-in variable of OpenGL which is
                                             // used to render the final positions of the geometry's vertices
}
