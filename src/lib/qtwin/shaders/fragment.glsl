#line 2

#if _FRAGMENT_

in vec2 texPos;
out vec4 fragColor;

uniform sampler2D sampler;

void main()
{
    fragColor = texture(sampler, texPos);
}

#endif

