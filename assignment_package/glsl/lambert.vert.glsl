#version 150
// ^ Change this to version 130 if you have compatibility issues

//This is a vertex shader. While it is called a "shader" due to outdated conventions, this file
//is used to apply matrix transformations to the arrays of vertex data passed to it.
//Since this code is run on your GPU, each vertex is transformed simultaneously.
//If it were run on your CPU, each vertex would have to be processed in a FOR loop, one at a time.
//This simultaneous transformation allows your program to run much faster, especially when rendering
//geometry with millions of vertices.

uniform mat4 u_Model;       // The matrix that defines the transformation of the
                            // object we're rendering. In this assignment,
                            // this will be the result of traversing your scene graph.

uniform mat4 u_ModelInvTr;  // The inverse transpose of the model matrix.
                            // This allows us to transform the object's normals properly
                            // if the object has been non-uniformly scaled.

uniform mat4 u_ViewProj;    // The matrix that defines the camera's transformation.
                            // We've written a static matrix for you to use for HW2,
                            // but in HW3 you'll have to generate one yourself

uniform mat4 u_depthbiasMVP;

uniform vec4 u_Color;       // When drawing the cube instance, we'll set our uniform color to represent different block types.
uniform int u_Time;
uniform vec3 u_Cam;         // The vector that defines the camera's position
uniform vec4 lightDir;

in vec4 vs_Pos;             // The array of vertex positions passed to the shader

in vec4 vs_Nor;             // The array of vertex normals passed to the shader

in vec4 vs_Col;             // The array of vertex colors passed to the shader.
in vec2 vs_UV;              // The array of vertex texture coordinates passed to the shader

out vec4 fs_Pos;
out vec4 fs_Nor;            // The array of normals that has been transformed by u_ModelInvTr. This is implicitly passed to the fragment shader.
out vec4 world_Nor;
out vec4 fs_LightVec;       // The direction in which our virtual light lies, relative to each vertex. This is implicitly passed to the fragment shader.
out vec4 fs_Col;            // The color of each vertex. This is implicitly passed to the fragment shader.
out vec2 fs_UV;              // The array of vertex texture coordinates passed to the shader

out vec4 fs_CameraPos;

out vec4 fs_shadowcoord;

#define BLK_UV  0.0625
#define IS_WATER (fs_UV.x > 12.9 * BLK_UV && fs_UV.x <= 16.1 * BLK_UV && fs_UV.y > 2.5 * BLK_UV && fs_UV.y <= 4.1 * BLK_UV)

void main()
{
    fs_Pos = u_Model * vs_Pos;
    fs_UV = vs_UV;                         // Pass the vertex colors to the fragment shader for interpolation

    mat3 invTranspose = mat3(u_ModelInvTr);
    fs_Nor = vec4(invTranspose * vec3(vs_Nor), 0);          // Pass the vertex normals to the fragment shader for interpolation.
                                                            // Transform the geometry's normals by the inverse transpose of the
                                                            // model matrix. This is necessary to ensure the normals remain
                                                            // perpendicular to the surface after the surface is transformed by
                                                            // the model matrix.
    world_Nor = vs_Nor;

    vec4 modelposition = u_Model * vs_Pos;   // Temporarily store the transformed vertex positions for use below



    // Water
    if (IS_WATER)
    {
        float mx = modelposition.x;
        float my = modelposition.y;
        float tx = mx * 0.001 + 0.01 * u_Time;
        float tz = my * 0.001 + 0.01 * u_Time;
        float hx = (sin(tx) + sin(2.2 * tx + 5.52)
                    + sin(2.9 * tx + 0.93) + sin(4.6 * tx + 8.94)) / 4.0;
        float hz = (sin(tz) + sin(2.2 * tz + 5.52)
                    + sin(2.9 * tz + 0.93) + sin(4.6 * tz + 8.94)) / 4.0;
        float h = 0.25f * (hz - 1.f + hx - 1.f);
        modelposition.y += 0.3* h;
    }


    fs_CameraPos = vec4(u_Cam[0], u_Cam[1], u_Cam[2], 1); // We want to use this uniform instead of the
                                                          // original implementation involving the inverse
                                                          // of our view matrix because invoking the inverse
                                                          // function is costly.

    fs_LightVec = (lightDir);  // Compute the direction in which the light source lies
    fs_shadowcoord = u_depthbiasMVP * modelposition;
    gl_Position = u_ViewProj * modelposition;// gl_Position is a built-in variable of OpenGL which is
                                             // used to render the final positions of the geometry's vertices

}
