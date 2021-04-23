#version 330 core
out vec4 color;
in vec3 vertex_pos;
in vec3 vertex_normal;
in vec2 vertex_tex;
uniform vec3 campos;

uniform sampler2D tex;
uniform sampler2D tex2;

void main()
{
	vec3 n = normalize(vertex_normal);
	vec3 lp = vec3(5,100,100);
	vec3 ld = normalize(lp-vertex_pos);
	float light = dot(ld, n) * 0.9;

	
	vec2 texcoords = vertex_tex;
	vec3 texturecolor = texture(tex, texcoords).rgb;
	if (vertex_pos.y < 0.4)
	{
		color.rgb = vec3(1,.9,.7);
	}
	else
		color.rgb = texturecolor;

	//color.rgb = texture(tex, vertex_tex).rgb;
	color.rgb *= light;

	vec3 cd = normalize(campos - vertex_pos);
	vec3 h = normalize(cd + ld);
	float spec = dot(n,h);
	spec = clamp(spec,0,1);
	spec = pow(spec, 10);
	color.rgb +=spec ; 
	color.a = 1;
	
	//faded background
	float len = length(vertex_pos.xz + campos.xz);
	len -= 45;
	len /= 4.;
	len = clamp(len, 0, 1);
	color.a = 1 - len;
	len = length(vertex_pos.xz + campos.xz);

	}
