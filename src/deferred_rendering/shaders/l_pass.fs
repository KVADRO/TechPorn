#version 410

in vec2 TexCoord;

uniform sampler2D uPosition;
uniform sampler2D uNormal;
uniform sampler2D uAlbedo;

out vec4 FragColor;

void main()
{
	//#TODO: Calculate lightning
	FragColor = texture(uAlbedo, TexCoord);
}