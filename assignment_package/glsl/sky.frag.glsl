//#version 330
#version 150

uniform mat4 u_ViewProj;    // We're actually passing the inverse of the viewproj
                            // from our CPU, but it's named u_ViewProj so we don't
                            // have to bother rewriting our ShaderProgram class

uniform ivec2 u_Dimensions; // Screen dimensions

uniform vec3 u_Eye; // Camera pos

uniform float u_Time;

in vec4 fs_LightVec;

out vec4 out_Col;

const float PI = 3.14159265359;
const float TWO_PI = 6.28318530718;

const vec3 sunColor = vec3(255, 255, 200) / 255.0;

vec2 sphereToUV(vec3 p) {
    float phi = atan(p.z, p.x);
    if(phi < 0) {
        phi += TWO_PI;
    }
    float theta = acos(p.y);
    return vec2(1 - phi / TWO_PI, 1 - theta / PI);
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

const mat4x3 vibrantsunsetPalette = mat4x3(
            vec3(0.850, 0.580, 0.190),
            vec3(0.540, 0.600, 0.290),
            vec3(0.420, 0.550, 0.420),
            vec3(3.430, 3.253, 1.027));

const mat4x3 sunsetPalette = mat4x3(
            vec3(0.580, 0.580, 0.090),
            vec3(0.650, 0.710, 0.140),
            vec3(0.530, 0.440, 0.420),
            vec3(3.420, 3.333, 0.897));

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

vec3 cosinePalette(mat4x3 P, float t) {
    // takes the 12 parameters and outputs proper cosine gradient color
    return P[0] + P[1] * cos(TWO_PI * (t * P[2] + P[3]));
}

void main()
{
    // Convert back to world
    vec2 ndc = (gl_FragCoord.xy / vec2(u_Dimensions)) * 2.0 - 1.0; // -1 to 1 NDC
    vec4 p = vec4(ndc.xy, 1, 1); // Pixel at the far clip plane
    p *= 1000.0; // Times far clip plane value
    p = /*Inverse of*/ u_ViewProj * p; // Convert from unhomogenized screen to world
    vec3 rayDir = normalize(p.xyz - u_Eye);
    // Ray as color:
    //out_Col = vec4(0.5 * (rayDir + vec3(1,1,1)), 1.f);


    // New Sunset Color that follows lightDir
    vec3 sunDir = normalize(fs_LightVec.xyz);
    float t = 0.5 + 0.5 * dot(rayDir, sunDir);
    float sunDotUp = dot(vec3(0,1,0), sunDir);

    vec3 sunsetSky = cosinePalette(sunsetPalette, t);
    vec3 daySky    = cosinePalette(dayPalette, t);
    vec3 nightSky  = cosinePalette(nightPalette, t);

    // range [0,1], with lower values favoring day/night over sunset colors
    float sunsetBias = 1.f / 2.f;
    // interp between day/sunset/night
    if (sunDotUp > 0) {
        out_Col = vec4(mix(sunsetSky, daySky, pow(sunDotUp, sunsetBias)), 1.f);
    } else {
        out_Col = vec4(mix(sunsetSky, nightSky, pow(-sunDotUp, sunsetBias)), 1.f);
    }

    // Create stars from Worley noise
    vec4 nearStar = worley3D(rayDir * 55.f);
    if (nearStar.x <= 0.07f) {
        // stars fade in proportionally as sky darkens
        out_Col = vec4(mix(vec3(1,1,1), out_Col.xyz, clamp(length(out_Col.xyz), 0.f, 1.f)),1.f);
    }

    // Add a glowing sun in the sky
    float sunSize = 5;
    float coronaSize = 50;
    float angle = acos(dot(rayDir, sunDir)) * 360.0 / PI;
    // If the angle between our ray dir and vector to center of sun
    // is less than the threshold, then we're looking at the sun
    if(angle < coronaSize) {
        // Full center of sun
        if(angle < sunSize) {
            out_Col = vec4(sunColor, 1);
        }
        // Corona of sun, mix with sky color
        else {
            float t = (angle - sunSize) / (coronaSize - sunSize);
            if (t < 1) {
                out_Col = vec4(mix(sunColor, out_Col.xyz, 1 + exp(-2*t)*(t - 1)), 1);
            } else {
                out_Col = vec4(sunColor,1);
            }
        }
    }
}
