#version 150

in vec2 fs_UV;
uniform int u_Time;

out vec4 out_Col;

uniform sampler2D u_RenderedTexture;

const vec3 a = vec3(0.4, 0.5, 0.5);
const vec3 b = vec3(0.43, 0.5, 0.5);
const vec3 c = vec3(0.4, 0.57, 0.46);
const vec3 d = vec3(-1.573, 0.4233, 0.63);
vec3 cosinePalette(float t) {
    return clamp(a + b * cos(2.0 * 3.14159 * (c * t + d)), 0.0, 1.0);
}
float timeSpeed = 0.2;

float randomVal (float inVal)
{
    return fract(sin(dot(vec2(inVal, 2523.2361) ,vec2(12.9898,78.233))) * 43758.5453)-0.5;
}

vec2 randomVec2 (float inVal)
{
    return normalize(vec2(randomVal(inVal), randomVal(inVal+151.523)));
}

float makeWaves(vec2 uv, float theTime, float offset)
{
    float result = 0.0;
    float direction = 0.0;
    float sineWave = 0.0;
    vec2 randVec = vec2(1.0,0.0);
    float i;
    for(int n = 0; n < 16; n++)
    {
        i = float(n)+offset;
        randVec = randomVec2(float(i));
        direction = (uv.x*randVec.x+uv.y*randVec.y);
        sineWave = sin(direction*randomVal(i+1.6516)+theTime*timeSpeed);
        sineWave = smoothstep(0.0,1.0,sineWave);
        result += randomVal(i+123.0)*sineWave;
    }
    return result;
}

void main()
{
    vec2 uv = fs_UV;
    vec2 uv2 = uv * 150.0; // scale


    float result = 0.0;
    float result2 = 0.0;

    result = makeWaves( uv2+vec2(u_Time*timeSpeed,0.0), u_Time, 0.1);
    result2 = makeWaves( uv2-vec2(u_Time*0.8*timeSpeed,0.0), u_Time*0.8+0.06, 0.26);

    //result *= 2.6;

    result = smoothstep(0.4,1.1,1.0-abs(result));
    result2 = smoothstep(0.4,1.1,1.0-abs(result2));

    result = 2.0*smoothstep(0.35,1.8,(result+result2)*0.5);

        //fragColor = vec4(result)*0.7+texture( iChannel0 , uv );

    // thank for this code below Shane!
    vec2 p = vec2(result, result2)*.015 + sin(uv*16. - cos(uv.yx*16. + u_Time*timeSpeed))*.015; // Etc.
    vec4 color = vec4(result)*0.7+texture( u_RenderedTexture , uv + p );

    float h = color.b;

    vec4 fragColor = vec4(h, h, h, 1.0);

    h = min(1.0, pow(h, 2.0));

    // Output to screen
    out_Col = vec4(cosinePalette(h), 1.0);
}
