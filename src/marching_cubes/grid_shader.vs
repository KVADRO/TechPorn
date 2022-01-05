#version 410 

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 aColor;

uniform mat4 uMMtx;
uniform mat4 uVMtx;
uniform mat4 uPMtx;

out vec4 VtxColor;

void main()
{
	gl_Position = uPMtx * uVMtx * uMMtx * vec4(aPos, 1.0);
	VtxColor = aColor;
}