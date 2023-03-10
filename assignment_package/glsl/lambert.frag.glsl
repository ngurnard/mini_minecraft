#version 330
//#version 150
// ^ Change this to version 130 if you have compatibility issues

// This is a fragment shader. If you've opened this file first, please
// open and read lambert.vert.glsl before reading on.
// Unlike the vertex shader, the fragment shader actually does compute
// the shading of geometry. For every pixel in your program's output
// screen, the fragment shader is run for every bit of geometry that
// particular pixel overlaps. By implicitly interpolating the position
// data passed into the fragment shader by the vertex shader, the fragment shader
// can compute what color to apply to its pixel based on things like vertex
// position, light position, and vertex color.

uniform vec4 u_Color; // The color with which to render this instance of geometry.
uniform sampler2D textureSampler;
uniform int u_Time;
uniform mat4 u_ViewProj;
uniform mat4 u_Model;

// These are the interpolated values out of the rasterizer, so you can't know
// their specific values without knowing the vertices that contributed to them
in vec4 fs_Pos;
in vec4 fs_Nor;
in vec4 fs_LightVec;
in vec4 fs_Col;
in vec2 fs_UVs;
in float fs_Anim;
in float fs_T2O;

in vec3 fs_Z;

in float fs_dimVal;

out vec4 out_Col; // This is the final output color that you will see on your
                  // screen for the pixel that is currently being processed.

in vec4 fs_CamPos;

float random1(vec3 p) {
    return fract(sin(dot(p,vec3(127.1, 311.7, 191.999)))
                 *43758.5453);
}

float random1b(vec3 p) {
    return fract(sin(dot(p,vec3(169.1, 355.7, 195.999)))
                 *95751.5453);
}

float mySmoothStep(float a, float b, float t) {
    t = smoothstep(0, 1, t);
    return mix(a, b, t);
}

float cubicTriMix(vec3 p) {
    vec3 pFract = fract(p);
    float llb = random1(floor(p) + vec3(0,0,0));
    float lrb = random1(floor(p) + vec3(1,0,0));
    float ulb = random1(floor(p) + vec3(0,1,0));
    float urb = random1(floor(p) + vec3(1,1,0));

    float llf = random1(floor(p) + vec3(0,0,1));
    float lrf = random1(floor(p) + vec3(1,0,1));
    float ulf = random1(floor(p) + vec3(0,1,1));
    float urf = random1(floor(p) + vec3(1,1,1));

    float mixLoBack = mySmoothStep(llb, lrb, pFract.x);
    float mixHiBack = mySmoothStep(ulb, urb, pFract.x);
    float mixLoFront = mySmoothStep(llf, lrf, pFract.x);
    float mixHiFront = mySmoothStep(ulf, urf, pFract.x);

    float mixLo = mySmoothStep(mixLoBack, mixLoFront, pFract.z);
    float mixHi = mySmoothStep(mixHiBack, mixHiFront, pFract.z);

    return mySmoothStep(mixLo, mixHi, pFract.y);
}

float fbm(vec3 p) {
    float amp = 0.5;
    float freq = 4.0;
    float sum = 0.0;
    for(int i = 0; i < 8; i++) {
        sum += cubicTriMix(p * freq) * amp;
        amp *= 0.5;
        freq *= 2.0;
    }
    return sum;
}

void main()
{
    vec4 diffuseColor = texture(textureSampler, fs_UVs);

    bool apply_lambert = true;

    float specularIntensity = 0;

    vec4 fs_Nor_new = fs_Nor;

    if (fs_Anim != 0) {
        // check region in texture to decide which animatable type is drawn
        bool lava = fs_UVs.x >= 13.f/16.f && fs_UVs.y < 2.f/16.f;
        bool water = !lava && fs_UVs.x >= 13.f/16.f && fs_UVs.y < 4.f/16.f;

        if (lava) {
            // slowly gyrate texture and lighten and darken with random dimVal from vert shader
            vec2 movingUVs = vec2(fs_UVs.x + fs_Anim * 0.065/16 * sin(0.01*u_Time),
                                  fs_UVs.y - fs_Anim * 0.065/16 * sin(0.01*u_Time + 3.14159/2));
            diffuseColor = texture(textureSampler, movingUVs);
            vec4 warmerColor = diffuseColor + vec4(0.3, 0.3, 0, 0);
            vec4 coolerColor = diffuseColor - vec4(0.1, 0.1, 0, 0);
            diffuseColor = mix(warmerColor, coolerColor, 0.5 + fs_dimVal * 0.65*sin(0.02*u_Time));

            apply_lambert = false;

        } else if (water) {
            // blend between 3 different points in texture to create a wavy subtle change over time
            vec2 offsetUVs = vec2(fs_UVs.x - 0.5f/16.f, fs_UVs.y - 0.5f/16.f);
            diffuseColor = texture(textureSampler, fs_UVs);
            vec4 altColor = texture(textureSampler, offsetUVs);

//            altColor.x += fs_dimVal * pow(altColor.x+.15, 5);
//            altColor.y += fs_dimVal * pow(altColor.y+.15, 5);
//            altColor.z += 0.5 * fs_dimVal * pow(altColor.z+.15, 5);

            diffuseColor = mix(diffuseColor, altColor, 0.5 + 0.35*sin(0.05*u_Time));
            offsetUVs -= 0.25f/16.f;
            vec4 newColor = texture(textureSampler, offsetUVs);
            diffuseColor = mix(diffuseColor, newColor, 0.5 + 0.5*sin(0.025*u_Time));// + fs_dimVal * vec4(0.025);
            diffuseColor.a = 0.75;

            // ----------------------------------------------------
            // Blinn-Phong Shading
            // ----------------------------------------------------
            vec4 lightDir = normalize(fs_LightVec - fs_Pos); // vector from the fragment's world-space position to the light source (assuming a point light source)
            vec4 viewDir = normalize(fs_CamPos - fs_Pos); // vector from the fragment's world-space position to the camera
            vec4 halfVec = normalize(lightDir + viewDir); // vector halfway between light source and view direction
            // float shininess = 10.f;
            float shininess = 400.f;
            specularIntensity = max(pow(dot(halfVec, normalize(fs_Nor)), shininess), 0);
        }
    }

    // Calculate the diffuse term for Lambert shading
    float diffuseTerm = dot(normalize(fs_Nor_new), normalize(fs_LightVec));

    // Avoid negative lighting values
    diffuseTerm = clamp(diffuseTerm, 0, 1);

    float ambientTerm = 0.3;

    float lightIntensity = diffuseTerm + ambientTerm;   //Add a small float value to the color multiplier
                                                        //to simulate ambient lighting. This ensures that faces that are not
                                                        //lit by our point light are not completely black.

    vec3 col = diffuseColor.rgb;

    // Compute final shaded color
    if (apply_lambert) {
//        col *= lightIntensity;
        col = col * lightIntensity + col * specularIntensity;
    }

    // & Check the rare, special case where we draw face between two diff transparent blocks as opaque
    if (fs_T2O != 0) {
        out_Col = vec4(col, 1.f);
    } else {
        out_Col = vec4(col, diffuseColor.a);
    }

    // New distance fog w/ improved Z coord from vert shader and new alpha fadeout
//    vec4 fogColor = vec4(0.57f, 0.71f, 1.0f, 1.0f);
//    float Z = length(fs_Z) / 135.f;
//    float fogfalloff = clamp(1.15 - exp(-5.5f * (Z - 1.0f)), 0.f, 1.f);
//    out_Col = vec4(mix(out_Col.rgb, fogColor.rgb, fogfalloff), clamp(diffuseColor.a - fogfalloff, 0.f, 1.f));
}
