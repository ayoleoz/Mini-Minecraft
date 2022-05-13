#version 150
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
uniform sampler2D u_Texture; // The texture to be read from by this shader
uniform sampler2D u_Shadow;
uniform int u_Time;
uniform vec3 u_Eye;

// These are the interpolated values out of the rasterizer, so you can't know
// their specific values without knowing the vertices that contributed to them
in vec4 fs_Pos;
in vec4 fs_Nor;
in vec4 world_Nor;
in vec4 fs_LightVec;
in vec4 fs_Col;
in vec2 fs_UV;
in vec4 fs_CameraPos;
in vec4 fs_shadowcoord;

out vec4 out_Col; // This is the final output color that you will see on your
                  // screen for the pixel that is currently being processed.

#define BLK_UV  0.0625
#define IS_GRASS_SIDE (fs_UV.x > 8.f * BLK_UV && fs_UV.x <= 9.f * BLK_UV && fs_UV.y > 13.f * BLK_UV && fs_UV.y <= 14.f * BLK_UV)
#define IS_GRASS_TUFT (fs_UV.x > 7.f * BLK_UV && fs_UV.x <= 8.f * BLK_UV && fs_UV.y > 13.f * BLK_UV && fs_UV.y <= 14.f * BLK_UV)
#define IS_GRASS_TOP (fs_UV.x > 6.f * BLK_UV && fs_UV.x <= 7.f * BLK_UV && fs_UV.y > 13.f * BLK_UV && fs_UV.y <= 14.f * BLK_UV)
#define IS_WATER (fs_UV.x > 13.f * BLK_UV && fs_UV.x <= 16.f * BLK_UV && fs_UV.y > 2.f * BLK_UV && fs_UV.y <= 4.f * BLK_UV)

#define SUN_VELOCITY (1 / 20000.f)
#define SUNSET_LEN 0.4
#define SHININESS 25

// Sun palette
const vec3 sun[3] = vec3[](vec3(255, 255, 245) / 255.0,
                            vec3(255, 137, 103) / 255.0,
                            vec3(107, 73, 132) / 255.0);

const vec4 grassland_green = vec4(164, 255, 117, 255) / 255.f;
const vec4 mountain_green = vec4(35, 100, 2, 255) / 255.f;
const vec4 waterland_green = vec4(100, 171, 63, 255) / 255.f;

float random1(vec3 p) {
    return fract(sin(dot(p,vec3(127.1, 311.7, 191.999)))
                 *43758.5453);
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

vec2 random2( vec2 p )
{
    return fract(sin(vec2(dot(p, vec2(127.1, 311.7)),
                dot(p, vec2(269.5,183.3))))
                * 43758.5453);
}

float surflet(vec2 P, vec2 gridPoint)
{
    // Compute falloff function by converting linear distance to a polynomial
    float distX = abs(P.x - gridPoint.x);
    float distY = abs(P.y - gridPoint.y);
    float tX = 1 - 6 * pow(distX, 5.f) + 15 * pow(distX, 4.f) - 10 * pow(distX, 3.f);
    float tY = 1 - 6 * pow(distY, 5.f) + 15 * pow(distY, 4.f) - 10 * pow(distY, 3.f);

    // Get the random vector for the grid point
    vec2 gradient = 2.f * random2(gridPoint) - vec2(1.f);

    // Get the vector from the grid point to P
    vec2 diff = P - gridPoint;

    // Get the value of our height field by dotting grid->P with our gradient
    float height = dot(diff, gradient);

    // Scale our height field (i.e. reduce it) by our polynomial falloff function
    return height * tX * tY;
}

float perlinNoise(vec2 uv)
{
    float surfletSum = 0.f;

    // Iterate over the four integer corners surrounding uv
    for(int dx = 0; dx <= 1; ++dx)
    {
        for(int dy = 0; dy <= 1; ++dy)
        {
            surfletSum += surflet(uv, floor(uv) + vec2(dx, dy));
        }
    }

    return surfletSum;
}

vec2 biomeValue(vec2 xz)
{
    float noise = perlinNoise(xz);
    return vec2(noise, noise);
}

vec4 biomeGrassGreen()
{
    vec2 biome = 0.5f * (biomeValue(fs_Pos.xz / 1750.f) + vec2(1.f));
    biome = smoothstep(0.4f, 0.6f, smoothstep(0.25f, 0.75f, biome));
    vec4 grasslandWaterGreen = mix(grassland_green, waterland_green, biome.x);
    return mix(grasslandWaterGreen, mountain_green, biome.y);
}
vec2 poissonDisk[16] = vec2[](
        vec2( -0.94201624, -0.39906216 ),
           vec2( 0.94558609, -0.76890725 ),
           vec2( -0.094184101, -0.92938870 ),
           vec2( 0.34495938, 0.29387760 ),
           vec2( -0.91588581, 0.45771432 ),
           vec2( -0.81544232, -0.87912464 ),
           vec2( -0.38277543, 0.27676845 ),
           vec2( 0.97484398, 0.75648379 ),
           vec2( 0.44323325, -0.97511554 ),
           vec2( 0.53742981, -0.47373420 ),
           vec2( -0.26496911, -0.41893023 ),
           vec2( 0.79197514, 0.19090188 ),
           vec2( -0.24188840, 0.99706507 ),
           vec2( -0.81409955, 0.91437590 ),
           vec2( 0.19984126, 0.78641367 ),
           vec2( 0.14383161, -0.14100790 )
);

void main()
{
    // Material base color (before shading)
        vec4 diffuseColor;
        float time_var = u_Time;
        if((fs_UV.r > 0.8125) && (fs_UV.g < 0.25))
        {
            vec2 fs_UVin = fs_UV;
            fs_UVin.r = fs_UV.r + floor(fract(time_var / 128)*16)/256;
            diffuseColor = texture(u_Texture,fs_UVin);
        }
        else{
            diffuseColor = texture(u_Texture, fs_UV);
        }



        if(diffuseColor.a == 0.f)
        {
            discard;
        }

        vec3 comp = diffuseColor.rgb;
        if (IS_GRASS_TOP || IS_GRASS_TUFT)
        {
            vec4 green = biomeGrassGreen();
            diffuseColor = diffuseColor * green;
        }
        //else if (IS_GRASS_SIDE && comp.r > 0.678f && comp.g > 0.678f && comp.b > 0.678f)
        else if (IS_GRASS_SIDE)
        {
            vec4 green = biomeGrassGreen();
            diffuseColor = diffuseColor * green;
        }



        // Calculate the diffuse term for Lambert shading
        float diffuseTerm = dot(normalize(fs_Nor), normalize(fs_LightVec));
        // Avoid negative lighting values
        diffuseTerm = clamp(diffuseTerm, 0, 1);
        float bias = 0.0005*tan(acos(diffuseTerm)); // cosTheta is dot( n,l ), clamped between 0 and 1
        bias = clamp(bias, 0,0.005);
        float visibility = 1.0;
        for (int i=0;i<16;i++){
          int index = int(16.0*random1(fs_Pos.xyy))%16;
          if ( texture(u_Shadow,fs_shadowcoord.xy + poissonDisk[i]/2048.0 ).r  <  fs_shadowcoord.z-bias ){
            visibility-=0.05;
          }
        }
        if((visibility > 0.9)){
            visibility = 1.0;
        }
        float ambientTerm = 0.25;
        float lightIntensity = 0.f;

        if (IS_WATER)
        {
            float exp = 200.f;
            vec4 view = fs_CameraPos - fs_Pos;
            vec4 H = (view + fs_LightVec) / 2;
            vec4 normalized_H = normalize(H);
            vec4 normalized_N = normalize(fs_Nor);
            float specularIntensity = max(pow(dot(normalized_H , normalized_N), exp), 0);
            lightIntensity = diffuseTerm + specularIntensity; // add specular highlight intensity
                                                                        // to a basic Lambertian shading calculation
        }
        else
        {
            lightIntensity = diffuseTerm;   //Add a small float value to the color multiplier
                                                                //to simulate ambient lighting. This ensures that faces that are not
                                                                //lit by our point light are not completely black.
        }

        if((fs_LightVec.y <= 0.1) && (fs_LightVec.y >= 0.01f)){
            lightIntensity = mix(0.2f,lightIntensity,fs_LightVec.y * 10);
            visibility = mix(0.5f,visibility,fs_LightVec.y * 10);
        }
        if(fs_LightVec.y <= 0.01f){
            lightIntensity = 0.2f;
            visibility = 0.5f;
        }
        if(fs_LightVec.y >= 0.999){
            visibility = mix(1.f,visibility,(1-fs_LightVec.y) * 1000);
        }

        //Compute sun
        float timeIndicator = sin(u_Time * SUN_VELOCITY);

                // Color of sun light
                vec3 sun_color;
                if (timeIndicator > SUNSET_LEN) {
                    sun_color = sun[0];
                }
                else if (timeIndicator < -SUNSET_LEN) {
                    sun_color = sun[2];
                }
                else {
                    if (timeIndicator > 0) {
                        float smooth_t = smoothstep(0.f, 1.f, abs(timeIndicator) / SUNSET_LEN);
                        sun_color = mix(sun[1], sun[0], smooth_t);
                    }
                    else {
                        float smooth_t = smoothstep(0.f, 1.f, abs(timeIndicator) / (SUNSET_LEN));
                        sun_color = mix(sun[1], sun[2], smooth_t);
                    }
                }

                // Calculate the diffuse term for Lambert shading
                vec3 sunDir = vec3(fs_LightVec);
                float diffuseTermSun = dot(normalize(vec3(fs_Nor)), normalize(sunDir));
                // Avoid negative lighting values
                diffuseTermSun = clamp(diffuseTermSun, 0, 1);

                // Calculate the specular term
                vec3 view_dir = u_Eye - vec3(fs_Pos);
                vec3 H = (normalize(view_dir) + normalize(sunDir)) / 2;
                float specularSun = pow(dot(normalize(vec3(fs_Nor)), normalize(H)), SHININESS);
                // Avoid negative lighting values
                specularSun = clamp(specularSun, 0, 1);

                if (timeIndicator < -SUNSET_LEN) {
                    diffuseTermSun = 0.f;
                    specularSun = 0.f;
                }
                else if (timeIndicator >= -SUNSET_LEN && timeIndicator < SUNSET_LEN) {
                    diffuseTermSun = mix(0.f, diffuseTermSun, (timeIndicator + SUNSET_LEN) / (2.f * SUNSET_LEN));
                    specularSun = mix(0.f, specularSun, (timeIndicator + SUNSET_LEN) / (2.f * SUNSET_LEN));
                }

        lightIntensity = (0.5 * diffuseTermSun + specularSun)/ 2 + lightIntensity / 2 + ambientTerm;
        if((fs_Pos.y < 128) && (fs_Pos.y >= 90)){
            sun_color = vec3(1,1,1);
            if(world_Nor.x != 0)
            {
                visibility = 1.0;
            }
            if(world_Nor.y != 0)
            {
                visibility = 0.8;
            }
            if(world_Nor.z != 0)
            {
                visibility = 0.6;
            }
            lightIntensity = mix(1.f,0.2,(fs_Pos.y - 90.f) / 38.f);
        }
        if((fs_Pos.y < 90)){
            sun_color = vec3(1,1,1);
            if(world_Nor.x != 0)
            {
                visibility = 1.0;
            }
            if(world_Nor.y != 0)
            {
                visibility = 0.8;
            }
            if(world_Nor.z != 0)
            {
                visibility = 0.6;
            }
            lightIntensity = 1.f;
        }
        float dist = length(fs_Pos.xz - u_Eye.xz) * 0.01;
        // Compute final shaded color
        out_Col = vec4(diffuseColor.xyz * visibility * lightIntensity * sun_color,diffuseColor.a);
}
