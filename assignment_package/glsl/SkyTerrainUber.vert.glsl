#version 150

uniform mat4 u_Model;
uniform mat4 u_ModelInvTr;
uniform mat4 u_ViewProj;
uniform vec4 u_Color;
uniform int u_Time;
uniform bool u_QuadDraw;

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

uniform vec3 u_Eye; // Camera pos
//out vec4 fs_CamPos;
out vec3 fs_Z;

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

//float surflet3D(vec3 P, vec3 gridPoint) {
//    // Compute falloff function by converting linear distance to a polynomial
//    float distX = abs(P.x - gridPoint.x);
//    float distY = abs(P.y - gridPoint.y);
//    float distZ = abs(P.z - gridPoint.z);
//    float tX = 1 - 6 * pow(distX, 5.f) + 15 * pow(distX, 4.f) - 10 * pow(distX, 3.f);
//    float tY = 1 - 6 * pow(distY, 5.f) + 15 * pow(distY, 4.f) - 10 * pow(distY, 3.f);
//    float tZ = 1 - 6 * pow(distZ, 5.f) + 15 * pow(distZ, 4.f) - 10 * pow(distZ, 3.f);
//    // Get the random vector for the grid point (normalized)
//    vec3 gradient = normalize(2.f * random2(gridPoint) - vec3(1.f));
//    // Get the vector from the grid point to P (non-normalized)
//    vec3 diff = P - gridPoint;
//    // Get the value of our height field by dotting grid->P with our gradient
//    float height = dot(diff, gradient);
//    // Scale our height field (i.e. reduce it) by our polynomial falloff function
//    return height * tX * tY * tZ;
//}

float surflet3D(vec3 p, vec3 gridPoint) {
    // Compute the distance between p and the grid point along each axis, and warp it with a
    // quintic function so we can smooth our cells
    float distX = abs(p.x - gridPoint.x);
    float distY = abs(p.y - gridPoint.y);
    float distZ = abs(p.z - gridPoint.z);
    float tX = 1 - 6 * pow(distX, 5.f) + 15 * pow(distX, 4.f) - 10 * pow(distX, 3.f);
    float tY = 1 - 6 * pow(distY, 5.f) + 15 * pow(distY, 4.f) - 10 * pow(distY, 3.f);
    float tZ = 1 - 6 * pow(distZ, 5.f) + 15 * pow(distZ, 4.f) - 10 * pow(distZ, 3.f);
    // Get the random vector for the grid point
    vec3 gradient = normalize(random2(gridPoint) * 2.f - vec3(1.f, 1.f, 1.f));
    // Get the vector from the grid point to P
    vec3 diff = p - gridPoint;
    // Get the value of our height field by dotting grid->P with our gradient
    float height = dot(diff, gradient);
    // Scale our height field (i.e. reduce it) by our polynomial falloff function
    return height * tX * tY * tZ;
}


float perlinNoise3D(vec3 p) {
    float surfletSum = 0.f;
    // Iterate over the eight integer corners surrounding uv
    for(int dx = 0; dx <= 1; ++dx) {
        for(int dy = 0; dy <= 1; ++dy) {
            for(int dz = 0; dz <= 1; ++dz) {
                surfletSum += surflet3D(p, floor(p) + vec3(dx, dy, dz));
            }
        }
    }
    return surfletSum; // output range [-1, 1]
}

vec3 surfacePerturb( vec3 p ) {
    // define an oscillating time so that model can transition back and forth
//    float t = (cos(u_Time * 0.01) + 1)/2; // u_Timsse increments by 1 every frame. Domain [0,1]
//    vec3 temp = random2(vec3(p.x, p.y, p.z)); // range [0, 1]
//    temp = (temp - 0.5)/25; // [0, 1/scalar]

//    p.x = mix(p.x - temp.x, p.x + temp.x, t);
//    p.y = mix(p.y - temp.y, p.y + 3*temp.y, t); // move in y more
//    p.z = mix(p.z - temp.z, p.z + temp.z, t);

    // Try with perlin noise (always zero for some reason)
    float timeX = p.x + u_Time;
    float timeZ = p.z + u_Time;
    float displacement = (perlinNoise3D(p + u_Time * 0.001)/4 - 0.25); // [-0.5, 0]
//    float displacement = perlinNoise3D(u_Time * 0.0001 * p)/5; // [0, 1]
//    p.y += displacement;
//    displacement /= 10;
    p.y += (sin((timeX + timeZ)/2000)/2 + 0.5)  * displacement;
//    p.y += (cos(u_Time * 0.01)) * sin(displacement * u_Time * 0.001);

    return p;
}

void main()
{
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

    fs_dimVal = random1(modelposition.xyz/100.f);
    fs_LightVec = rotateLightVec(0.0025 * u_Time, lightDir);  // Compute the direction in which the light source lies

    if (vs_Anim != 0) { // if we want to animate this surface
        // check region in texture to decide which animatable type is drawn
        bool lava = fs_UVs.x >= 13.f/16.f && fs_UVs.y < 2.f/16.f;
        bool water = !lava && fs_UVs.x >= 13.f/16.f && fs_UVs.y <= 4.f/16.f;

        // don't distort bottom face verts
        if (fs_Nor.y != -1) {
            if (water) {
                // If top block or top two verts on sides, perform distortion
                if (fs_Nor.y == 1 || vs_UV.y*16.f > 3.5) {
                    // Perturb the surface
                    modelposition.xyz = surfacePerturb(modelposition.xyz);

                    // Recompute the normals
                    // vec3 tangent = normalize(cross(vec3(fs_Nor), normalize(vec3(1.f, 0.f, 1.f)))); // take cross product with oblique so cube normals are never zero
                    float eps = 0.008;
                    //            vec3 tangent = normalize(cross(vec3(fs_Nor), normalize(vec3(fs_Nor.x + eps, fs_Nor.y + eps, fs_Nor.z + eps)))); // make it so normals are never zero
                    vec3 tangent = normalize(cross(vec3(fs_Nor), normalize(vec3(1.f, 0.f, 0.f)))); // make it so normals are never zero
                    // vec3 tangent = normalize(cross(vec3(fs_Nor), normalize(vec3(fs_LightVec.x, fs_LightVec.y, fs_LightVec.z)))); // take cross product with oblique so cube normals are never zero
                    vec3 bitangent = normalize(cross(vec3(fs_Nor), tangent));

                    vec3 p1 = surfacePerturb(modelposition.xyz + (eps * tangent));
                    vec3 p2 = surfacePerturb(modelposition.xyz - (eps * tangent));
                    vec3 p3 = surfacePerturb(modelposition.xyz + (eps * bitangent));
                    vec3 p4 = surfacePerturb(modelposition.xyz - (eps * bitangent));

                    //            vec3 p1 = surfacePerturb(modelposition.xyz + (eps * vec3(1.f, 0.f, 0.f)));
                    //            vec3 p2 = surfacePerturb(modelposition.xyz - (eps * vec3(1.f, 0.f, 0.f)));
                    //            vec3 p3 = surfacePerturb(modelposition.xyz + (eps * vec3(0.f, 0.f, 1.f)));
                    //            vec3 p4 = surfacePerturb(modelposition.xyz - (eps * vec3(0.f, 0.f, 1.f)));

                    // only make the top surface shiny
                    //            if (vs_Nor.y == 1) {
                    //                fs_Nor = vec4(normalize(cross(normalize(p1 - p2), normalize(p3 - p4))), 0);
                    //            }
                    fs_Nor = vec4(normalize(cross(normalize(p1 - p2), normalize(p3 - p4))), 0);
                }

            } else if (lava) {
                // If top block or top two verts on sides, perform distortion
                if (fs_Nor.y == 1 || vs_UV.y*16.f > 1.f) {
                    // Perturb the surface
                    modelposition.xyz = surfacePerturb(modelposition.xyz);
                    // Do not recompute the normals
                }
            }
        }
    }

    // set final position AFTER all these deformations
    fs_Pos = modelposition;
    fs_Z = modelposition.xyz - u_Eye;

    // position for screen-spanning quad (sky) will be
    // different from actual terriain blockfaces
    if (u_QuadDraw) {
        gl_Position = vs_Pos;
    } else {
        gl_Position = u_ViewProj * modelposition;
    }
}
