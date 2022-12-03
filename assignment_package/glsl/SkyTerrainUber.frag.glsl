#version 330
//#version 150

// ---------------------------------
// SETUP UNIFORM/IN/OUT VARS

uniform sampler2D textureSampler;
uniform ivec2 u_Dimensions; // Screen dimensions
uniform int u_Time;
//uniform mat4 u_ViewProj;
uniform mat4 u_ViewProjInv;
uniform mat4 u_Model;
uniform vec3 u_Eye;
uniform bool u_QuadDraw; // bool that tells whether we're drawing sky or terrain

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
//in vec4 fs_CamPos;

// This is the final output color
out vec4 out_Col;

// ---------------------------------
// USEFUL CONSTANTS

const float PI = 3.14159265359;
const float TWO_PI = 6.28318530718;

const float sunSize = 5;
const float coronaSize = 50;
const vec3 sunColor = vec3(255, 255, 200) / 255.0;

const mat4x3 vibrantsunsetPalette = mat4x3(
            vec3(0.580, 0.580, 0.090),
            vec3(0.650, 0.710, 0.140),
            vec3(0.530, 0.440, 0.420),
            vec3(3.420, 3.333, 0.897));

const mat4x3 sunsetPalette = mat4x3(
            vec3(0.580, 0.580, 0.040),
            vec3(0.650, 0.940, 0.140),
            vec3(0.480, 0.410, 0.640),
            vec3(3.430, 3.363, 1.027));

const mat4x3 dayPalette = mat4x3(
            vec3(0.540, 0.770, 1.000),
            vec3(0.220, 0.100, 0.050),
            vec3(0.340, 0.410, 0.500),
            vec3(0.020, 0.023, 0.487));

const mat4x3 nightPalette = mat4x3(
            vec3(0.000, 0.000, 0.078),
            vec3(0.020, 0.058, 0.050),
            vec3(0.340, 0.410, 0.500),
            vec3(0.117, 0.920, 1.383));

// ---------------------------------
// HELPER FUNCS

vec3 cosinePalette(mat4x3 P, float t) {
    // takes the 12 parameters and outputs proper cosine gradient color
    return P[0] + P[1] * cos(TWO_PI * (t * P[2] + P[3]));
}

vec3 avgofPalette(mat4x3 P, float t1, float t2) {
    // Returns the average RBG color of the cosine palette
    // in range [t1, t2] (avg val definite integral of cosinePalette formula)
    return P[0] + P[1] / (TWO_PI * P[2] * (t2 - t1)) *
           (sin(TWO_PI * (P[2]*t2 + P[3])) - sin(TWO_PI * (P[2]*t1 + P[3])));
}

float brightness(vec3 col) {
    return  0.21 * col.r + 0.72 * col.g + 0.07 * col.b;
}

vec2 sphereToUV(vec3 p) {
    float phi = atan(p.z, p.x);
    if(phi < 0) {
        phi += TWO_PI;
    }
    float theta = acos(p.y);
    return vec2(1 - phi / TWO_PI, 1 - theta / PI);
}

float random1(vec3 p) {
    return fract(sin(dot(p,vec3(127.1, 311.7, 191.999)))
                 *43758.5453);
}

vec2 random2( vec2 p ) {
    return fract(sin(vec2(dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3))))*43758.5453);
}

vec3 random3( vec3 p ) {
    return fract(sin(vec3(dot(p,vec3(127.1, 311.7, 191.999)),
                          dot(p,vec3(269.5, 183.3, 765.54)),
                          dot(p, vec3(420.69, 631.2,109.21))))
                 *43758.5453);
}

vec4 worley3D(vec3 p) {
    vec3 pInt = floor(p);
    vec3 pFract = fract(p);
    vec3 min_point = p;
    float minDist = 100000.0; // Minimum distance initialized to max.
    for(int y = -1; y <= 1; ++y) {
        for(int x = -1; x <= 1; ++x) {
            for(int z = -1; z <= 1; ++z) {
                vec3 neighbor = vec3(float(x), float(y), float(z)); // Direction in which neighbor cell lies
                vec3 point = random3(pInt + neighbor); // Get the Voronoi centerpoint for the neighboring cell

                // moves point location so the voronoi cells change over time
                vec3 diff = neighbor - pFract + point;

                //vec2 diff = neighbor + point - pFract; // Distance between fragment coord and neighbor’s Voronoi point
                float dist = length(diff);
                if (dist < minDist)
                {
                    minDist = dist;
                    min_point = p + diff;
                }
            }
        }
    }
    // return a vec3 packet containing minimun distance,
    // as well as the x and y coordinates of the closest vornoi point
    return vec4(minDist, min_point.x, min_point.y, min_point.z);
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

// ---------------------------------
// MAIN PROGRAM

void main()
{
    // ---------------------------------
    // COMMON SKY/TERRAIN STUFF
    // ---------------------------------
    vec4 skyCol = vec4(0);

    // Convert back to world coords
    vec2 ndc = (gl_FragCoord.xy / vec2(u_Dimensions)) * 2.0 - 1.0; // -1 to 1 NDC
    vec4 p = vec4(ndc.xy, 1, 1); // Pixel at the far clip plane
    p *= 1000.0; // Times far clip plane value
    p = /*Inverse of*/ u_ViewProjInv * p; // Convert from unhomogenized screen to world
    vec3 rayDir = normalize(p.xyz - u_Eye);
    vec3 sunDir = normalize(fs_LightVec.xyz);

    // New Sunset Color that follows lightDir
    float t = 0.5 + 0.5 * dot(rayDir, sunDir);
    float tstatic = 0.5 + 0.5 * dot(rayDir, vec3(0,1,0));
    float sunDotUp = dot(vec3(0,1,0), sunDir);

    vec3 sunsetSky = cosinePalette(sunsetPalette, t);
    vec3 daySky    = cosinePalette(dayPalette, tstatic);
    vec3 nightSky  = cosinePalette(nightPalette, tstatic);

    // Nonlinearly interpolate between day, night, and sunset palettes
    // lower bias = more day/night time, less sunset/sunrise, harsher transition
    float sunsetBias = 0.70f;
    if (sunDotUp > 0) // Sun above horizon
        {skyCol = vec4(mix(sunsetSky, daySky, pow(sunDotUp, sunsetBias)), 1.f);}
    else              // Sun below horizon
        {skyCol = vec4(mix(sunsetSky, nightSky, pow(-sunDotUp, sunsetBias)), 1.f);}

    if (u_QuadDraw) // drawing sky
    {
        // ---------------------------------
        // SKY ONLY STUFF
        // ---------------------------------
        out_Col = skyCol;
        // Create stars from Worley noise
        vec4 nearStar = worley3D(rayDir * 75.f);
        if (nearStar.x <= 0.07f) {
            // stars fade in proportionally as sky darkens
            out_Col = vec4(mix(vec3(1,1,1), out_Col.xyz, clamp(length(out_Col.xyz), 0.f, 1.f)),1.f);
        }

        // Add a glowing sun in the sky
        float angle = acos(dot(rayDir, sunDir)) * 360.0 / PI;
        // If the angle between our ray dir and vector to center of sun
        // is less than the threshold, then we're looking at the sun
        if(angle < coronaSize) {
            // Full center of sun
            if(angle < sunSize) {
                out_Col = vec4(sunColor, 1);
            }
            // Corona of sun, mix with sky color (physically-inspired exponential falloff)
            else {
                float t = (angle - sunSize) / (coronaSize - sunSize);
                if (t < 1) {
                    out_Col = vec4(mix(sunColor, out_Col.xyz, 1 + exp(-2*t)*(t - 1)), 1);
                }
            }
        }
    }
    else
    {
        // ---------------------------------
        // TERRAIN ONLY STUFF
        // ---------------------------------

        vec4 diffuseColor = texture(textureSampler, fs_UVs);
        vec3 norDir = normalize(fs_Nor.xyz);

        if (false)
        {
            // Naive backface culling
            // seems to fix noisy white pixels at edges of blocks & should reduce shader load
            discard;
        }
        else
        {
            bool apply_lambert = true;
            float specularIntensity = 0;

            // water and lava animation logic
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

                    diffuseColor = mix(diffuseColor, altColor, 0.5 + 0.35*sin(0.05*u_Time));
                    offsetUVs -= 0.25f/16.f;
                    vec4 newColor = texture(textureSampler, offsetUVs);
                    diffuseColor = mix(diffuseColor, newColor, 0.5 + 0.5*sin(0.025*u_Time));
                    diffuseColor.a = 0.75;

                    // ----------------------------------------------------
                    // Blinn-Phong Shading
                    // ----------------------------------------------------
                    vec4 lightDir = normalize(fs_LightVec - fs_Pos); // vector from the fragment's world-space position to the light source (assuming a point light source)
                    vec4 viewDir = normalize(vec4(u_Eye,1) - fs_Pos); // vector from the fragment's world-space position to the camera
                    vec4 halfVec = normalize(lightDir + viewDir); // vector halfway between light source and view direction
                    float shininess = 400.f;
                    float specularIntensity = max(pow(dot(halfVec, normalize(fs_Nor)), shininess), 0);
                }
            }

            // Calculate the diffuse term for Lambert shading
            float diffuseTerm = dot(norDir, sunDir);
            diffuseTerm = clamp(diffuseTerm, 0, 1); // Avoid negative lighting values (backface lighting)

            //Add ambient light term to get non-black shadows
            float ambientTerm = 0.2;
            vec3 ambientCol = vec3(0.2, 0.2, 0.3);
            vec3 diffuseCol = diffuseTerm * vec3(255, 255, 200) / 255.0;
//            float lightIntensity = diffuseTerm + ambientTerm;
            vec3 lightIntensity = diffuseCol + ambientCol;

            vec3 diffuseRGB = diffuseColor.rgb;
            if (apply_lambert) {
                // apply shading given light intensity
                diffuseRGB = diffuseRGB * lightIntensity + diffuseRGB * specularIntensity;
            }

            // grayscale nighttime???
            if (sunDotUp < 0) {
                diffuseRGB = mix(diffuseRGB, vec3(brightness(diffuseRGB)), -0.75*sunDotUp);
            }

            // & Check the rare, special case where we draw face between two diff transparent blocks as opaque
            if (fs_T2O != 0) {out_Col = vec4(diffuseRGB, 1.f);}
            else {out_Col = vec4(diffuseRGB, diffuseColor.a);}

            // New distance fog w/ improved Z coord from vert shader and new alpha fadeout
            float bloomfactor = 0.8f + 0.1f * length(skyCol.rgb);
            vec4 fogColor = vec4(bloomfactor * skyCol.rgb, 1);//vec4(0.57f, 0.71f, 1.0f, 1.0f);
            float Z = length(fs_Z) / 135.f;
            float fogfalloff = clamp(1.15 - exp(-5.5f * (Z - 1.0f)), 0.f, 1.f);
            out_Col = vec4(mix(out_Col.rgb, fogColor.rgb, fogfalloff), diffuseColor.a);

//            out_Col = vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, 1.);
        }
    }
}
