#version 150

out vec4 outColor;
in vec3 exNormal; // Phong
in vec2 texCoord;
uniform sampler2D tex;

void main(void)
{
//	out_Color = vec4(1.0);
	const vec3 light = vec3(0.58, 0.58, 0.58);
	float shade;
	
	shade = dot(normalize(exNormal), light);
//	outColor = vec4(shade, shade, shade, 1.0); // inColor;

	
//	float a = sin(texCoordG.s * 50.0)/2.0 + 0.5;
//	float b = sin(texCoordG.t * 50.0)/2.0 + 0.5;
//	outColor = vec4(a, b, 1.0, 1.0); // inColor;

	outColor = texture(tex, texCoord);
}
