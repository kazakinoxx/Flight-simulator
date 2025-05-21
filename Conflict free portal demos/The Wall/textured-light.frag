#version 150

out vec4 outColor;

in vec2 texCoordG;
in vec3 exNormalG;
uniform sampler2D tex;
uniform mat4 modelviewMatrix;
uniform vec3 lightDirection;

void main(void)
{
	vec4 t = texture(tex, texCoordG); //  * vec2(modelviewMatrix[0][0], modelviewMatrix[1][1]));
	vec3 n = normalize(exNormalG);
	vec3 light = lightDirection; // vec3(0.7, 0.7, 0.7);
//	vec3 light = vec3(0.7, 0.7, 0.7);
	float shade = dot(n, normalize(mat3(modelviewMatrix) * light))*0.4 + 0.7;
	if (t.a < 0.01) discard;
	else
		outColor = t * shade;
}
