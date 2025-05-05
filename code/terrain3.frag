#version 150

out vec4 outColor;
in vec2 texCoord;
uniform sampler2D tex;
uniform sampler2D snowTex;
in  vec3 inPosition;
in vec3 outNormal;

void main(void)
{
	vec3 lightDirection = vec3(0.58f, 0.58f, 0.58f);
	float shade = max(dot(outNormal, lightDirection), 0.25);
	
	if(inPosition.y>0.05){
		outColor = (shade + 0.25)*(texture(tex, texCoord));	
	}
	else{
		outColor = (shade + 0.25)*(texture(snowTex, texCoord));
	}
}
