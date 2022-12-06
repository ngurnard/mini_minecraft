#version 150
// ^ Change this to version 130 if you have compatibility issues

// Refer to the lambert shader files for useful comments
uniform int u_Time;

in vec4 fs_Col;
in vec3 fs_Z;
in vec4 fs_Pos;

out vec4 out_Col;

float noise2D(vec2 p)
{
    return fract(sin(dot(p, vec2(139.2, 269.1))) * 45858.5453);
}

float interpNoise2D(float x, float y) {
    int intX = int(floor(x));
    float fractX = fract(x);
    fractX = fractX * fractX * (3 - 2 * fractX); //cubic interp var
    int intY = int(floor(y));
    float fractY = fract(y);
    fractY = fractY * fractY * (3 - 2 * fractY); //cubic interp var

    float v1 = noise2D(vec2(intX, intY));
    float v2 = noise2D(vec2(intX + 1, intY));
    float v3 = noise2D(vec2(intX, intY + 1));
    float v4 = noise2D(vec2(intX + 1, intY + 1));

    float i1 = mix(v1, v2, fractX);
    float i2 = mix(v3, v4, fractX);
    return mix(i1, i2, fractY);
}


float fbmLargeH(float x, float y) {
    float noiseSum = 0;

    int octaves = 4;
    float freq = 0.05f;

    float amp = 0.5f;
    float persistence = 0.5f;

    for(int i = 1; i <= octaves; i++) {
        noiseSum += interpNoise2D(x * freq, y * freq) * amp;
        freq *= 2.f;
        amp *= persistence;
    }
    return noiseSum;
}

float fbmSmallH(float x, float y) {
    float noiseSum = 0;

    int octaves = 8;
    float freq = 0.03f;

    float amp = 0.5f;
    float persistence = 0.5f;

    for(int i = 1; i <= octaves; i++) {
        noiseSum += interpNoise2D(x * freq, y * freq) * amp;
        freq *= 2.f;
        amp *= persistence;
    }
    return noiseSum;
}

float fbmMask(float x, float y) {
    float noiseSum = 0;

    int octaves = 8;
    float freq = 0.009f;

    float amp = 0.5f;
    float persistence = 0.6f;

    for(int i = 1; i <= octaves; i++) {
        noiseSum += interpNoise2D(x * freq, y * freq) * amp;
        freq *= 2.f;
        amp *= persistence;
    }
    return noiseSum;
}

float LargeHeightPostProcess(float val)
{
    // Changes peak distribution to Gaussian
    return 0.95 * exp(-pow(val, 2.f) / 0.65f);
}

float SmallHeightPostProcess(float val)
{
    // Changes peak distribution to Gaussian
    return 0.03 + 0.135 * (pow(val+.2, 2) - 0.5 * pow(val, 3));
}

float MaskPostProcess(float val) {
    // smoothstep to increase contrast
    val = pow(val, 2.25);
    return smoothstep(0.15f, 0.95f, val); // original
}

void main()
{
    // Copy the color; there is no shading.
    out_Col = fs_Col;
    float a = 1.f;
    float threshold = 0.15f + 0.15f * sin(a * 0.00025 * u_Time);
    ivec2 offset_coords = ivec2(floor(fs_Pos.x + a * 0.025 * u_Time) + 1000, floor(fs_Pos.z + a * 0.025 * u_Time) + 1000);

    float largeH = fbmLargeH(offset_coords.x, offset_coords.y);
    largeH = LargeHeightPostProcess(largeH);

    float smallH = fbmSmallH(offset_coords.x, offset_coords.y);
    smallH = SmallHeightPostProcess(smallH);

    float mask = fbmMask(offset_coords.x, offset_coords.y);
    mask = MaskPostProcess(mask);

    float H = mix(smallH, largeH, mask);

    if (H > threshold) {
        out_Col.a = (H - threshold)/(0.75f - threshold);
//        out_Col = vec4(1, 0, 0, 1);
    } else {
        out_Col.a = 0.f;
    }

    float Z = length(fs_Z.xz) / 150.f;
    float fogfalloff = clamp(1.15 - exp(-5.5f * (Z - 1.0f)), 0.f, 1.f);
    out_Col = vec4(out_Col.rgb, out_Col.a * (1-fogfalloff));
}
