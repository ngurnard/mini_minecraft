#version 150
// ^ Change this to version 130 if you have compatibility issues

//This is a vertex shader. While it is called a "shader" due to outdated conventions, this file
//is used to apply matrix transformations to the arrays of vertex data passed to it.
//Since this code is run on your GPU, each vertex is transformed simultaneously.
//If it were run on your CPU, each vertex would have to be processed in a FOR loop, one at a time.
//This simultaneous transformation allows your program to run much faster, especially when rendering
//geometry with millions of vertices.

uniform mat4 u_Model;       // The matrix that defines the transformation of the
                            // object we're rendering. In this assignment,
                            // this will be the result of traversing your scene graph.

uniform mat4 u_ModelInvTr;  // The inverse transpose of the model matrix.
                            // This allows us to transform the object's normals properly
                            // if the object has been non-uniformly scaled.

uniform mat4 u_ViewProj;    // The matrix that defines the camera's transformation.
                            // We've written a static matrix for you to use for HW2,
                            // but in HW3 you'll have to generate one yourself

uniform vec4 u_Color;       // When drawing the cube instance, we'll set our uniform color to represent different block types.
uniform int u_Time;

in vec4 vs_Pos;             // The array of vertex positions passed to the shader

in vec4 vs_Nor;             // The array of vertex normals passed to the shader

in vec4 vs_Col;             // The array of vertex colors passed to the shader.

in vec2 vs_UV;              // UV coords for texture to pass thru to fragment shader

in float vs_Anim;           // 0.f or 1.f To pass thru to fragment shader
in float vs_T2O;

out vec4 fs_Pos;
out vec4 fs_Nor;            // The array of normals that has been transformed by u_ModelInvTr. This is implicitly passed to the fragment shader.
out vec4 fs_LightVec;       // The direction in which our virtual light lies, relative to each vertex. This is implicitly passed to the fragment shader.
out vec4 fs_Col;            // The color of each vertex. This is implicitly passed to the fragment shader.
out vec2 fs_UVs;
out float fs_Anim;
out float fs_dimVal;
out float fs_T2O;

//uniform vec4 u_CamPos;
uniform vec3 u_Eye; // Camera pos
out vec4 fs_CamPos;

const vec4 lightDir = normalize(vec4(0.0, 1.f, 0.0, 0));//normalize(vec4(0.5, 1, 0.75, 0));  // The direction of our virtual light, which is used to compute the shading of
                                        // the geometry in the fragment shader.

mat4 rotationMatrix(vec3 axis, float angle) {
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;

    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

vec4 rotateLightVec(float deg, vec4 LV) {

    mat4 R = rotationMatrix(vec3(1,0,0.5), deg);
    return R * LV;
}

float random1(vec3 p) {
    return fract(sin(dot(p, vec3(127.1, 311.7, 191.999)))*43758.5453);
}

vec3 random2( vec3 p ) {
    return fract( sin( vec3(dot(p, vec3(127.1, 311.7, 58.24)),
                                           dot(p, vec3(269.5, 183.3, 657.3)),
                                           dot(p, vec3(420.69, 69.420, 469.20))) ) * 43758.5453);
}

void main()
{
//    fs_Pos = vs_Pos;
    fs_Col = vs_Col;                         // Pass the vertex colors to the fragment shader for interpolation
    fs_UVs = vs_UV;
    fs_Anim = vs_Anim;
    fs_T2O = vs_T2O;

    mat3 invTranspose = mat3(u_ModelInvTr);
    fs_Nor = vec4(invTranspose * vec3(vs_Nor), 0);          // Pass the vertex normals to the fragment shader for interpolation.
                                                            // Transform the geometry's normals by the inverse transpose of the
                                                            // model matrix. This is necessary to ensure the normals remain
                                                            // perpendicular to the surface after the surface is transformed by
                                                            // the model matrix.

    vec4 modelposition = u_Model * vs_Pos;   // Temporarily store the transformed vertex positions for use below

    if (vs_Anim != 0) { // if we want to animate this surface
        // check region in texture to decide which animatable type is drawn
        bool lava = fs_UVs.x >= 13.f/16.f && fs_UVs.y < 2.f/16.f;
        bool water = !lava && fs_UVs.x >= 13.f/16.f && fs_UVs.y <= 4.f/16.f;

        if (water) {
            // define an oscillating time so that model can transition back and forth
            float t = (cos(u_Time * 0.05) + 1)/2; // u_Time increments by 1 every frame. Domain [0,1]
            vec3 temp = random2(vec3(modelposition.x, modelposition.y, modelposition.z)); // range [0, 1]
            temp = (temp - 0.5)/25; // [0, 1/scalar]
            modelposition.x = mix(modelposition.x - temp.x, modelposition.x + temp.x, t);
            modelposition.y = mix(modelposition.y - temp.y, modelposition.y + 3*temp.y, t);
            modelposition.z = mix(modelposition.z - temp.z, modelposition.z + temp.z, t);

//            fs_Pos = normalize( cross(dFdx(vs_Pos), dFdy(vs_Pos)) );
        } else if (lava) {
            // define an oscillating time so that model can transition back and forth
            float t = (cos(u_Time * 0.01) + 1)/2; // u_Time increments by 1 every frame. Domain [0,1]
            vec3 temp = random2(vec3(modelposition.x, modelposition.y, modelposition.z)); // range [0, 1]
            temp = (temp - 0.5)/25; // [0, 1/scalar]
            modelposition.x = mix(modelposition.x - temp.x, modelposition.x + temp.x, t);
            modelposition.y = mix(modelposition.y - temp.y, modelposition.y + 3*temp.y, t);
            modelposition.z = mix(modelposition.z - temp.z, modelposition.z + temp.z, t);
        }
    }

    fs_dimVal = random1(modelposition.xyz/100.f);

//    fs_LightVec = (lightDir);  // Compute the direction in which the light source lies
    fs_LightVec = rotateLightVec(0.005 * u_Time, lightDir);  // Compute the direction in which the light source lies
//    fs_LightVec = fs_CamPos - modelposition;

    fs_CamPos = vec4(u_Eye, 1); // uniform handle for the camera position instead of the inverse
    fs_Pos = modelposition;

    gl_Position = u_ViewProj * modelposition;// gl_Position is a built-in variable of OpenGL which is
                                             // used to render the final positions of the geometry's vertices

}
