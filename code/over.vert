#version 150

in vec3 inPosition;
in vec2 inTexCoord;

uniform mat4 proj;

out vec2 texCoord;

void main()
{
    texCoord = inTexCoord;
    gl_Position = proj * vec4(inPosition, 1.0);
}
