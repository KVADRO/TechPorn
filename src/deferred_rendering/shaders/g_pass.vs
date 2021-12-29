#version 410

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 uPMtx;
uniform mat4 uVMtx;
uniform mat4 uMMtx;

void main()
{
	vec4 transformedPos = uMMtx * vec4(aPos, 1.0);

	FragPos = transformedPos.xyz;
	TexCoord = aTexCoord;
	Normal = transpose(inverse(mat3(uMMtx))) * Normal;

	gl_Position = uPMtx * uVMtx * uMMtx * vec4(aPos, 1.0);
}

