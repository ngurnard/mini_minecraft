#version 330
//#version 150

in vec2 fs_UV;
out vec4 out_Col; // This is the final output color that you will see on your
                  // screen for the pixel that is currently being processed.
uniform int u_Time;
uniform sampler2D u_RenderedTexture; // literally the 2D screen

// Inspriation for this lava shader came from:
// https://www.shadertoy.com/view/WsSGDw
// Ironically this was created by amallay!!
// I used this to understand it:
// https://inspirnathan.com/posts/62-shadertoy-tutorial-part-15/

// Image
vec3 a = vec3(0.4, 0.5, 0.5);
vec3 b = vec3(0.43, 0.5, 0.5);
vec3 c = vec3(0.4, 0.57, 0.46);
vec3 d = vec3(-1.573, 0.4233, 0.63);

vec3 cosinePalette(float t) {
    return clamp(a + b * cos(2.0 * 3.14159 * (c * t + d)), 0.0, 1.0);
}

vec2 random2( vec2 p ) {
    return fract(sin(vec2(dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3))))*43758.5453);
}

vec2 mySmoothStep(vec2 a, vec2 b, float t) {
    t = smoothstep(0.0, 1.0, t);
    return mix(a, b, t);
}

vec2 interpNoise2D(vec2 uv) {
    vec2 uvFract = fract(uv);
    vec2 ll = random2(floor(uv));
    vec2 lr = random2(floor(uv) + vec2(1,0));
    vec2 ul = random2(floor(uv) + vec2(0,1));
    vec2 ur = random2(floor(uv) + vec2(1,1));

    vec2 lerpXL = mySmoothStep(ll, lr, uvFract.x);
    vec2 lerpXU = mySmoothStep(ul, ur, uvFract.x);

    return mySmoothStep(lerpXL, lerpXU, uvFract.y);
}

vec2 fbm(vec2 uv) {
    float amp = 0.5;
    float freq = 1.0;
    vec2 sum = vec2(0.0);
    float maxSum = 0.0;
    for(int i = 0; i < 6; i++) {
        sum += interpNoise2D(uv * freq) * amp;
        maxSum += amp;
        amp *= 0.5;
        freq *= 2.0;
    }
    return sum / maxSum;
}

float WorleyNoise(vec2 uv)
{
    // Tile the space
    vec2 uvInt = floor(uv);
    vec2 uvFract = fract(uv);

    float minDist = 1.0; // Minimum distance initialized to max.

    // Search all neighboring cells and this cell for their point
    for(int y = -1; y <= 1; y++)
    {
        for(int x = -1; x <= 1; x++)
        {
            vec2 neighbor = vec2(float(x), float(y));

            // Random point inside current neighboring cell
            vec2 point = random2(uvInt + neighbor);

            // Animate the point
            point = 0.5 + 0.5 * sin(u_Time * 0.03 + 6.2831 * point); // 0 to 1 range

            // Compute the distance b/t the point and the fragment
            // Store the min dist thus far
            vec2 diff = neighbor + point - uvFract;
            float dist = length(diff);
            minDist = min(minDist, dist);
        }
    }
    return minDist;
}

void main()
{

    vec4 base_color = texture(u_RenderedTexture, fs_UV);
    base_color.r -= 0.15f;
    base_color.g -= 0.1;
    base_color.b += 0.2;

    vec2 uv = fs_UV;

    // Gridify and animate with rightward motion
    uv = uv * 8.0;
    vec2 offset = fbm(uv); // [0, 1]
    offset = (2.0 * offset) - 1.0; // [-1, 1]
    uv = uv + offset * 0.5;
    // Worley cells
    float h = WorleyNoise(uv);

     h = min(1.0, pow(h, 2.0)); // make it darker

//    out_Col = vec4(cosinePalette(h), 1.0);
    vec3 temp = mix(cosinePalette(h), base_color.rgb, 0.8);
    out_Col = vec4(temp, 1);
//    out_Col = cosinePalette(h);
}
