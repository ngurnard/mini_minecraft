#version 330
//#version 150

in vec4 fs_Col;
in vec2 fs_UV;

out vec4 out_Col; // ensure is a vec4

uniform sampler2D u_RenderedTexture;

vec3 argmax(vec3 v)
{
    float maximum = max(max(v.r, v.g),v.b);

    if (maximum == v.r) {
        return vec3(1, 0, 0);
    } else if (maximum == v.g) {
        return vec3(0, 1, 0);
    } else {
        return vec3(0, 0, 1);
    }
}

void main()
{
    // Make crosshair color the inverse of the rendered environment behind
    vec4 texCol = texture(u_RenderedTexture, fs_UV);

//    vec3 extreme = argmax(texCol.rgb);
//    out_Col = vec4(vec3(1.f) - extreme, 1.f);

    vec3 invert = vec3(1.f) - texCol.rgb;
    out_Col = vec4(invert, 1.f);
}
