#version 330 core
out vec4 color;
in vec3 vertex_pos;
in vec3 vertex_normal;
in vec2 vertex_tex;

uniform vec3 campos;
uniform vec3 ldn;

uniform sampler2D tex;
uniform sampler2D tex2;

void main()
{
	vec3 lp = vec3(5,100,100);
	vec3 dn = normalize(lp-vertex_pos);
	vec3 n = normalize(vertex_normal);
	float light = dot(dn, n);


	vec2 texcoords = vertex_tex;
	vec3 texturecolor = texture(tex,texcoords).rgb;
	if (vertex_pos.y < 2.46)
	{
		color.rgb = vec3(.45,.30,.14);
	}
	else
		color.rgb = texturecolor;
	color.rgb *= light;
	color.a = 1;

	//faded background
	float len = length(vertex_pos.xz + campos.xz);
	len -= 45;
	len /= 4.;
	len = clamp(len, 0, 1);
	color.a = 1 - len;
}
