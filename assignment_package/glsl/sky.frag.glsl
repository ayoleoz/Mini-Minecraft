#version 150

uniform mat4 u_ViewProj;    // We're actually passing the inverse of the viewproj
                            // from our CPU, but it's named u_ViewProj so we don't
                            // have to bother rewriting our ShaderProgram class

uniform vec2 u_Dimensions; // Screen dimensions

uniform vec3 u_Eye; // Camera pos

uniform int u_Time;

out vec4 outColor;

const float PI = 3.14159265359;
const float TWO_PI = 6.28318530718;

const float SUNSET_THRESHOLD = 0.75;
const float DUSK_THRESHOLD = -0.1;

const float SUN_VELOCITY = 1 / 20000.f;
const float SUNSET_LEN = 0.45;

// Sunset palette
const vec3 sunset[5] = vec3[](vec3(255, 229, 119) / 255.0,
                               vec3(254, 192, 81) / 255.0,
                               vec3(255, 137, 103) / 255.0,
                               vec3(253, 96, 81) / 255.0,
                               vec3(57, 32, 51) / 255.0);

// Dusk palette
const vec3 dusk[5] = vec3[](vec3(107, 73, 132) / 255.0,
                            vec3(72, 52, 117) / 255.0,
                            vec3(43, 47, 119) / 255.0,
                            vec3(20, 24, 82) / 255.0,
                            vec3(7, 11, 52) / 255.0);

// Daytime palette
const vec3 daytime[5] = vec3[](vec3(194, 225, 247) / 255.0,
                               vec3(171, 214, 244) / 255.0,
                               vec3(149, 203, 241) / 255.0,
                               vec3(103, 181, 235) / 255.0,
                               vec3(81, 170, 232) / 255.0);

// Sun palette
const vec3 sun[3] = vec3[](vec3(255, 255, 245) / 255.0,
                            vec3(255, 255, 190) / 255.0,
                            vec3(107, 73, 132) / 255.0);

const vec3 cloudColor = sunset[3];

// treat given point as sphere coordinate and map it to UV coordinate
vec2 sphereToUV(vec3 p) {
    // compute phi
    float phi = atan(p.z, p.x);
    if(phi < 0) {
        phi += TWO_PI;
    }
    // compute theta
    float theta = acos(p.y);
    // Map to [0, 1]
    return vec2(1 - phi / TWO_PI, 1 - theta / PI);
}

// Map a 2d uv coordinate to a color in the daytime palette
// The palette is placed as bars on x direction with different colors
// The color is linearly interpolated from two adjacent colors from the lowest to the highest bar
// in other words, measures y coord for linearly arranged piecewise values
// returns color of daytime based on different value of uv.y
vec3 uvToDaytime(vec2 uv) {
    // Bar 0
    if(uv.y < 0.5) {
        return daytime[0];
    }
    // Bar 1
    else if(uv.y < 0.55) {
        return mix(daytime[0], daytime[1], (uv.y - 0.5) / 0.05);
    }
    // Bar 2
    else if(uv.y < 0.6) {
        return mix(daytime[1], daytime[2], (uv.y - 0.55) / 0.05);
    }
    // Bar 3
    else if(uv.y < 0.65) {
        return mix(daytime[2], daytime[3], (uv.y - 0.6) / 0.05);
    }
    // Bar 4
    else if(uv.y < 0.75) {
        return mix(daytime[3], daytime[4], (uv.y - 0.65) / 0.1);
    }
    // Bar 5
    return daytime[4];
}

// Map a 2d uv coordinate to a color in the sunset palette
// The palette is placed as bars on x direction with different colors
// The color is linearly interpolated from two adjacent colors from the lowest to the highest bar
// in other words, measures y coord for linearly arranged piecewise values
// returns color of daytime based on different value of uv.y
vec3 uvToSunset(vec2 uv) {
    // Bar 0
    if(uv.y < 0.5) {
        return sunset[0];
    }
    // Bar 1
    else if(uv.y < 0.55) {
        return mix(sunset[0], sunset[1], (uv.y - 0.5) / 0.05);
    }
    // Bar 2
    else if(uv.y < 0.6) {
        return mix(sunset[1], sunset[2], (uv.y - 0.55) / 0.05);
    }
    // Bar 3
    else if(uv.y < 0.65) {
        return mix(sunset[2], sunset[3], (uv.y - 0.6) / 0.05);
    }
    // Bar 4
    else if(uv.y < 0.75) {
        return mix(sunset[3], sunset[4], (uv.y - 0.65) / 0.1);
    }
    // Bar 5
    return sunset[4];
}

// Map a 2d uv coordinate to a color in the dusk palette
// The palette is placed as bars on x direction with different colors
// The color is linearly interpolated from two adjacent colors from the lowest to the highest bar
// in other words, measures y coord for linearly arranged piecewise values
// returns color of daytime based on different value of uv.y
vec3 uvToDusk(vec2 uv) {
    // Bar 0
    if(uv.y < 0.5) {
        return dusk[0];
    }
    // Bar 1
    else if(uv.y < 0.55) {
        return mix(dusk[0], dusk[1], (uv.y - 0.5) / 0.05);
    }
    // Bar 2
    else if(uv.y < 0.6) {
        return mix(dusk[1], dusk[2], (uv.y - 0.55) / 0.05);
    }
    // Bar 3
    else if(uv.y < 0.65) {
        return mix(dusk[2], dusk[3], (uv.y - 0.6) / 0.05);
    }
    // Bar 4
    else if(uv.y < 0.75) {
        return mix(dusk[3], dusk[4], (uv.y - 0.65) / 0.1);
    }
    // Bar 5
    return dusk[4];
}

// a noise function that returns a random 2D point based on given seed
vec2 random2( vec2 p ) {
    return fract(sin(vec2(dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3))))*43758.5453);
}

// a noise function that returns a random 3D point based on given seed
vec3 random3( vec3 p ) {
    return fract(sin(vec3(dot(p,vec3(127.1, 311.7, 191.999)),
                          dot(p,vec3(269.5, 183.3, 765.54)),
                          dot(p, vec3(420.69, 631.2,109.21))))
                 *43758.5453);
}

// generate worley noise based on given vector
float WorleyNoise3D(vec3 p)
{
    // Tile the space
    vec3 pointInt = floor(p);
    vec3 pointFract = fract(p);

    float minDist = 1.0; // Minimum distance initialized to max.

    // Search all neighboring cells and this cell for their point
    for(int z = -1; z <= 1; z++)
    {
        for(int y = -1; y <= 1; y++)
        {
            for(int x = -1; x <= 1; x++)
            {
                vec3 neighbor = vec3(float(x), float(y), float(z));

                // Random point inside current neighboring cell
                vec3 point = random3(pointInt + neighbor);

                // Animate the point
                point = 0.5 + 0.5 * sin(u_Time * 0.01 + 6.2831 * point); // 0 to 1 range

                // Compute the distance b/t the point and the fragment
                // Store the min dist thus far
                vec3 diff = neighbor + point - pointFract;
                float dist = length(diff);
                minDist = min(minDist, dist);
            }
        }
    }
    return minDist;
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
            point = 0.5 + 0.5 * sin(u_Time * 0.01 + 6.2831 * point); // 0 to 1 range

            // Compute the distance b/t the point and the fragment
            // Store the min dist thus far
            vec2 diff = neighbor + point - uvFract;
            float dist = length(diff);
            minDist = min(minDist, dist);
        }
    }
    return minDist;
}

// sum 3D worley noise of different frequencies and amplitudes by FBM algorithm
float worleyFBM(vec3 uv) {
    float sum = 0;
    float freq = 4;
    float amp = 0.5;

    // changing frequency and amplitudes and sum the worley noise on each iteration
    for(int i = 0; i < 8; i++) {
        sum += WorleyNoise3D(uv * freq) * amp;
        freq *= 2;
        amp *= 0.5;
    }
    return sum;
}



void main()
{
    // From pixel space to world space
    vec2 ndc = (gl_FragCoord.xy / vec2(u_Dimensions)) * 2.0 - 1.0; // -1 to 1 NDC

    vec4 p = vec4(ndc.xy, 1, 1); // Pixel at the far clip plane
    p *= 1000.0; // Times far clip plane value
    p = /*Inverse of*/ u_ViewProj * p; // Convert from unhomogenized screen to world

    // Direction from camera to this point
    vec3 rayDir = normalize(p.xyz - u_Eye);

    // get UV coordinate based on ray direction
    vec2 uv = sphereToUV(rayDir);

    // Compute offset to get cloud effect based on worley noise
    vec2 offset = vec2(0.0);
    // Get a noise value in the range [-1, 1]
    // by using Worley noise as the noise basis of FBM
    offset = vec2(worleyFBM(rayDir));
    offset *= 2.0;
    offset -= vec2(1.0);

    // Compute a gradient from the bottom of the sky-sphere to the top
    vec3 daytimeColor = uvToDaytime(uv + offset * 0.1);
    vec3 sunsetColor = uvToSunset(uv + offset * 0.1);
    vec3 duskColor = uvToDusk(uv);

    // Add a glowing sun in the sky
    // and make it rotate based on u_Time
    vec3 sunDir = normalize(vec3(cos(u_Time * SUN_VELOCITY), sin(u_Time * SUN_VELOCITY), 0.f));

    // Indicator for day:1 or night:-1
    float timeIndicator = sin(u_Time * SUN_VELOCITY);


    vec3 sunColor;
    vec3 skyColor;
    vec3 out_color;
    vec3 outSunsetColor;

    // Before sunset / After sunrise (DAYTIME)
    if (timeIndicator > SUNSET_LEN) {
        skyColor = daytimeColor;
        sunColor = sun[0];
    }
    // Before sunrise / After sunset (NIGHT)
    else if (timeIndicator < -SUNSET_LEN) {
        skyColor = duskColor;
        sunColor = sun[2];
    }

    // During sunset or sunrise
    else {
        skyColor = mix(duskColor, daytimeColor, (timeIndicator + SUNSET_LEN) / (SUNSET_LEN * 2.f));

        // sun is above the ground
        if (timeIndicator > 0) {
            // smooth function
            float smooth_t = smoothstep(0.f, 1.f, abs(timeIndicator) / SUNSET_LEN);
            sunColor = mix(sun[1], sun[0], smooth_t);
        }
        // sun is below the ground
        else {
            // smooth function
            float smooth_t = smoothstep(0.f, 1.f, abs(timeIndicator) / (SUNSET_LEN));
            sunColor = mix(sun[1], sun[2], smooth_t);
        }
    }

    float sunSize = 30;
    float angle = acos(dot(rayDir, sunDir)) * 180.0 / PI;
    // If the angle between our ray dir and vector to center of sun
    // is less than the threshold, then we're looking at the sun

    // If the angle between our ray dir and vector to center of sun
    // is less than the threshold, then we're looking at the sun
    if(angle < sunSize) {
        // Full center of sun
        if(angle < 7.5) {
            out_color = sunColor;
        }
        // Corona of sun, mix with sky color
        else {
            out_color = mix(sunColor, skyColor, (angle - 7.5) / 22.5);

            // Sunrise or sunset
            if (timeIndicator <= SUNSET_LEN && timeIndicator >= -SUNSET_LEN) {
                outSunsetColor = mix(sunColor, sunsetColor, (angle - 7.5) / 22.5);

                float smooth_t = smoothstep(0.f, 1.f, 1.f - abs(timeIndicator) / SUNSET_LEN);
                out_color = mix(out_color, outSunsetColor, smooth_t);
            }
        }
    }
    // Otherwise our ray is looking into just the sky
    else {
        out_color = skyColor;

        // Sunrise or sunset
        if (timeIndicator <= SUNSET_LEN && timeIndicator >= -SUNSET_LEN) {
            float raySunDot = dot(rayDir, sunDir);

            if(raySunDot > SUNSET_THRESHOLD) {
                // Do nothing, sky is already correct color
                outSunsetColor = sunsetColor;
            }
            // Any dot product between 0.75 and -0.1 is a LERP b/t sunset and sky color
            else if(raySunDot > DUSK_THRESHOLD) {
                float t = (raySunDot - SUNSET_THRESHOLD) / (DUSK_THRESHOLD - SUNSET_THRESHOLD);
                outSunsetColor = mix(sunsetColor, skyColor, t);
            }
            // Any dot product <= -0.1 are pure sky color
            else {
                outSunsetColor = skyColor;
            }
            // smooth step
            float smooth_t = smoothstep(0.f, 1.f, 1.f - abs(timeIndicator) / SUNSET_LEN);
            out_color = mix(out_color, outSunsetColor, smooth_t);
        }
    }
    outColor = vec4(out_color, 1);
}

