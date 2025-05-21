#version 150


in  vec3 inPosition;
in  vec3 inNormal;
out vec3 outNormal;
in vec2 inTexCoord;
out vec2 texCoord;

in vec3 inHeight;
out float outHeight;

uniform mat4 projMatrix;
uniform mat4 mdlMatrix;
uniform mat4 trans;



void main(void){
	
	texCoord = inTexCoord;
	outNormal = inNormal;
	outHeight = inHeight.y;
	gl_Position = projMatrix * mdlMatrix * trans * vec4(inPosition , 1.0);
}
