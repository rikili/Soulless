#version 330 core
out vec4 color;

in vec3 vertexPos;

uniform vec3 progress_color;
uniform vec3 non_progress_color;
uniform bool is_vertical;
uniform float progress;

void main()
{
	if (is_vertical)
	{
		if (vertexPos.y < (1 - 2 * progress))
		{
			color = vec4(non_progress_color, 1);
		}
		else
		{
			color = vec4(progress_color, 1);
		}
	}
	else
	{
		if (vertexPos.x < (1 - 2 * progress))
		{
			color = vec4(0, 0, 0, 1);
		}
		else
		{
			color = vec4(progress_color, 1);
		}
	}
}