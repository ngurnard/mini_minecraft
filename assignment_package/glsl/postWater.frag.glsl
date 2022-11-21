//#version 150
#version 330

in vec2 fs_UV;

out vec3 color;

uniform sampler2D u_RenderedTexture; // literally the 2D screen

void main()
{
    // TODO Homework 5
//    // Material base color
//    // vec4 base_color = texture2D(u_RenderedTexture, fs_UV);
//    vec4 base_color = texture(u_RenderedTexture, fs_UV);

//    // The weighted average of each pixel given the fragment that is colored to the screen
//    float grey = 0.21 * base_color[0] + 0.72 * base_color[1] + 0.07 * base_color[2];

//    // The output color is simply the weighted avergage for each channel if no vignette
//    color = vec3(grey, grey, grey);

//    // Get the center of the rendered textur
//    vec2 center = vec2(0.5, 0.5);

//    // Get the distance from the u_RenderedTexture to the center of image
//    float dist = distance(fs_UV, center); // distance function built into GLSL

//    color = color * (1 - dist); // apply the vignette
    out_Col = vec4(0.0, 1.0, 0, 1.0);
}
