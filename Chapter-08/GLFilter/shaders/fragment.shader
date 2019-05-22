#version 420

in vec2 texCoord;

out vec4 frag_color;

uniform sampler2D theTexture;

void main()
{
    vec4 color = vec4(texCoord.x, texCoord.y, 0.5, 1.0);
    frag_color = texture(theTexture, texCoord);
}