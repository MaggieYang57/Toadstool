#version 330 core
out vec4 color;
in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;
uniform vec3 campos;

uniform sampler2D tex;
uniform sampler2D tex2;

void main()
{
	vec3 n = normalize(vertex_normal);
	vec3 lp = vec3(20, 20, 100);//20 20 100
	vec3 ld = normalize(lp-vertex_pos);
	float light = dot(ld, n);

	//color.rgb = texture(tex, vertex_tex).rgb;
	color.rgb = vec3(0,0.5,0.2);
	color.rgb *= light;

	vec3 cd = normalize(campos - vertex_pos);
	vec3 h = normalize(cd + ld);
	float spec = dot(n,h);
	spec = clamp(spec,0,1);
	spec = pow(spec, 10);
	color.rgb += vec3(1,1,1) * spec ; 
	color.a = 1;
//vec4 tcol = texture(tex, vertex_tex*10.);
//color = tcol;
}
