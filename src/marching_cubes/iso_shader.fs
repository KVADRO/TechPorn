#version 410

in vec4 VtxColor;
in vec3 VtxNormal;

out vec4 oColor;

void main()
{
	vec3 light = normalize(vec3(1.0, 1.0, 1.0));
	oColor = (VtxColor * dot(light, normalize(VtxNormal))) + (VtxColor * 0.1);
}