#version 330 core
out vec4 color;
in vec3 vertex_pos;
in vec2 vertex_tex;
in vec3 vertex_normal;

uniform sampler2D tex;
uniform sampler2D tex2;
uniform vec3 lightpos;
uniform vec3 campos;

uniform float texoff;

void main()
{

	//vec2 texcoords = vertex_tex;
	//float t = 1./100;
	//texcoords -= vec2(camoff.x, camoff.z) * t;

	vec3 normalcolor = texture(tex, vertex_tex * 8 + vec2(texoff, texoff * 0.4)).rgb;
	vec3 texnormal = normalize(normalcolor - vec3(0.5,0.5,0.5));
	texnormal = vec3(texnormal.x, texnormal.z, texnormal.y);

	vec3 normalcolor2 = texture(tex2, vertex_tex * 8 + vec2(-texoff * 0.2,- texoff * 0.9)).rgb;
	vec3 texnormal2 = normalize(normalcolor2 - vec3(0.5,0.5,0.5));
	texnormal2 = vec3(texnormal2.x, texnormal2.z, texnormal2.y);

	//texturecolor.b = 0.1 + texturecolor.b*0.9;
	//color.rgb = texturecolor;

	//normal lighting
	vec3 ld = normalize(lightpos-vertex_pos);
	vec3 n = normalize(texnormal + texnormal2);
	//float light = dot(texnormal, ld);
	//light = clamp(light, 0.1, 1);

	//ambient
	//color.rgb = texturecolor * light;

	//specular
	vec3 camdir = normalize(-campos - vertex_pos);
	vec3 halfVec = normalize(camdir + ld);
	float spec = dot(n, halfVec);
	spec = pow(spec, 10);
	spec = clamp(spec, 0.1, 1);

	color.rbg = vec3(spec);
	color.b += 0.07;

	//faded background
	float len = length(vertex_pos.xz + campos.xz);
	len -= 45;
	len /= 4.;
	len = clamp(len, 0, 1);
	color.a = 1 - len;

}
