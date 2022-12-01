#version 150

uniform int u_Time;

in vec4 vs_Pos;

out vec4 fs_LightVec;
const vec4 lightDir = normalize(vec4(0.0, 1.f, 0.0, 0));

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

void main()
{
    fs_LightVec = rotateLightVec(0.0025 * u_Time, lightDir);
    gl_Position = vs_Pos;
}
