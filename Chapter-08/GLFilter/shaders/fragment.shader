#version 420

in vec2 texCoord;

out vec4 frag_color;

uniform sampler2D theTexture;

uniform vec2 pixelScale;

void main()
{
    int kernel_size = 7;
    vec4 color = vec4(0.0, 0.0, 0.0, 0.0);
    // if(texCoord.x > 0.5) {
    for(int i = -(kernel_size / 2); i <= kernel_size / 2; i++) {
        for(int j = -(kernel_size / 2); j <= kernel_size / 2; j++) {
            if(i == 0 && j == 0) continue;
            vec2 coord = vec2(texCoord.x + i * pixelScale.x, texCoord.y + i * pixelScale.y);
            color = color + texture(theTexture, coord);
        }
    }
    frag_color = color / (kernel_size * kernel_size - 1);
    // } else {
    // frag_color = texture(theTexture, texCoord);
    // }
}
