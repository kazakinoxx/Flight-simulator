#version 150

out vec4 outColor;

in vec2 texCoord;

void main(void)
{
	float shade = sqrt(sin(texCoord.s)*sin(texCoord.t));
	outColor = vec4(1.0, shade, shade, 1-shade);
}
