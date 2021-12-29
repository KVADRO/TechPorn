#version 410

layout(location = 0) out vec3 oPosition;
layout(location = 1) out vec3 oNormal;
layout(location = 2) out vec4 oAlbedo;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

//#TODO: Texture samplers

void main()
{
	oPosition = FragPos;
	oNormal = Normal;
	oAlbedo = vec4(1.0, 0.0, 0.0, 1.0); //#TOOD: Take albedo from samplers
}