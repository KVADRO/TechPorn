#version 410 

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 aColor;
layout(location = 2) in vec3 aNormal;

uniform mat4 uMMtx;
uniform mat4 uVMtx;
uniform mat4 uPMtx;

out vec4 VtxColor;
out vec3 VtxNormal;

void main()
{
	gl_Position = uPMtx * uVMtx * uMMtx * vec4(aPos, 1.0);

	VtxColor = aColor;
	VtxNormal = transpose(inverse(mat3(uMMtx))) * aNormal;
}