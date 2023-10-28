#version 460 core

flat in int fragAxis;

out vec4 color;

void main()
{
    switch(fragAxis)
    {
        case 1:
            color = vec4(1, 0, 0, 1);
            return;
        case 2:
            color = vec4(0, 1, 0, 1);
            return;
        case 3:
            color = vec4(0, 0, 1, 1);
            return;
    }
    color = vec4(1, 1, 1, 1);
}