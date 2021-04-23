#version 330 core
out vec4 color;
in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;
uniform vec3 campos;

uniform float splash;

uniform vec2 texoff;
uniform sampler2D tex;
uniform sampler2D tex2;

void main()
{
	
	vec2 tc = vertex_tex * (0.2) + texoff * 0.25 ;
	vec4 tcol = texture(tex, tc);
	//tcol.b += pow(tcol.b, 3);
	float a = (tcol.x + tcol.y + tcol.z)/3;
	color = tcol;
	if(splash == 1)
	{
		if (color.rgb == vec3(0,0,0))
			discard;
		//color.a = 1;
	}
	else
	color.a =0;

	//faded background
	float len = length(vertex_pos.xz + campos.xz);
	len -= 8;
	len /= 5.;
	len = clamp(len, 0, 0.7);
	//color.a = 1;
}
