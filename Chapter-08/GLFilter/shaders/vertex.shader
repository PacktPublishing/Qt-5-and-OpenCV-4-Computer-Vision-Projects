#version 420

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 inTexCoord;

out vec2 texCoord;

void main()
{
	gl_Position = vec4(vertex, 1.0);
    texCoord = inTexCoord;
}