#version 150

in  vec3 inPosition;
in  vec3 inNormal;
in vec2 inTexCoord;
out vec3 exNormal; // Phong
out vec2 texCoord;

uniform mat4 modelviewMatrix;
//uniform mat4 camMatrix;
uniform mat4 projectionMatrix;

void main(void)
{
	exNormal = mat3(modelviewMatrix) * inNormal; // Phong

//	gl_Position = vec4(inPosition, 1.0);
	
	texCoord = inTexCoord;

	gl_Position = projectionMatrix * modelviewMatrix * vec4(inPosition, 1.0);
}
