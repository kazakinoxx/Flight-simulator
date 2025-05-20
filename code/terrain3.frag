#version 150

out vec4 outColor;
in vec2 texCoord;
uniform sampler2D tex;
uniform sampler2D gtex;
uniform sampler2D snowTex;
uniform sampler2D sky;   
uniform int texUnit;     
uniform int layer;
//in  vec3 inPosition;
in vec3 outNormal;
in float outHeight;



void main()
{
	
	if (texUnit == 3) // Skybox
	{
		outColor = texture(sky, texCoord); // Sample skybox texture directly
	}
	else if(texUnit <3)
	{
		vec3 lightDirection = vec3(0.58f, 0.58f, 0.58f);
		float shade = max(dot(outNormal, lightDirection), 0.25);

		float slope = dot(normalize(outNormal), vec3(0.0, 1.0, 0.0)); 
		float blendFactor = clamp((slope - 0.3) * 3.0, 0.0, 1.0);

		float grassFactor = clamp(outHeight, -1.0, 1.0);

		vec4 snowColor = texture(snowTex, texCoord / 50.0); 
		vec4 rockColor = texture(tex, texCoord / 5.0);  
		vec4 grass = texture(gtex, texCoord / 10.0); 

		outColor = shade * mix(grass, mix(rockColor, snowColor, blendFactor), grassFactor);
	}
}
