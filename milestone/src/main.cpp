/*
CPE/CSC 471 Lab base code Wood/Dunn/Eckhardt
*/

#include <iostream>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"

#include "WindowManager.h"
#include "Shape.h"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <stdio.h>      /* printf, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>

using namespace std;
using namespace glm;
shared_ptr<Shape> shape, frog, frogL, frogR, mushroom, tree;

typedef struct position {
	vec3 pos;
	bool loc = false;
} Position;

bool start = false;
float win = 0;


double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime =glfwGetTime();
	double difference = actualtime- lasttime;
	lasttime = actualtime;
	return difference;
}
class camera
{
public:
	glm::vec3 pos, rot;
	int w, a, s, d;
	camera()
	{
		w = a = s = d = 0;
		pos = glm::vec3(0, -1.5 , 0);  // 0, -1.5, -3 // y goes opp. direction
		rot = glm::vec3(0, 0, 0);
	}
	glm::mat4 process(double ftime)
	{
		float speed = 0;
		if(start == true)
		{
			speed = 1.0 * ftime;
		}
		if (w == 1)
		{
			speed = 4 * ftime + 0.01;
		}
		else if (s == 1)
		{
			speed = -4 * ftime + 0.01;
		}
		float yangle=0; // yaw
		if (a == 1)
			yangle = -0.5*ftime;
		else if(d==1)
			yangle = 0.5*ftime;
		rot.y += yangle;
		glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		glm::vec4 dir = glm::vec4(0, 0, speed, 1);
		dir = dir*R;
		pos += glm::vec3(dir.x, dir.y, dir.z);
		glm::mat4 T = glm::translate(glm::mat4(1), pos);
		return R*T;
	}
};

camera mycam;
float on = 0.;
float splash = 0.;
float splashtime = 0.;
float GRAVITY = 0.015;
float JUMP_VELOCITY = .25;
float FALL_VELOCITY = -GRAVITY;

float key_a, key_d, key_w, key_s = 0;
float key_m = 0;
float frog_x = 0;
float frog_y = 0.9;
float frog_z = -4;
bool left = false;
bool right = false;
bool jump = false;
float leftleg, rightleg = 0;
bool swingingUpParentLL = true;
bool swingingUpParentRL = true;

int MUSH_NUM = 100;
vec3 mushpos[100];
vec3 treepos[12];


bool compareMush(vec3& i, vec3& j)
{
	vec3 diff_i = i - (-mycam.pos);
	vec3 diff_j = j - (-mycam.pos);
	return (glm::length(diff_i) > glm::length(diff_j)); // > or < not sure
}

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog, pmush, psky, pwater, heightShader, psplash, ptree, pgrass;
	std::shared_ptr<Program> prog_framebuffer;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	//ADDED FBO for post-processing
	GLuint FBOtex, FrameBufferObj, depth_rb, FBOtexpos;
	GLuint VertexArrayIDRect, VertexBufferIDRect, VertexBufferTexRect;

	// Contains vertex information for OpenGL
	GLuint VertexArrayIDBill;
	// Data necessary to give our box to OpenGL
	GLuint VertexBufferIDBill, VertexNormDBoxBill, VertexTexBoxBill, IndexBufferIDBoxBill;
	//GLuint InstanceBuffer;

	// Data necessary to give our box to OpenGL
	GLuint VertexBufferID, VertexNormDBox, VertexTexBox, IndexBufferIDBox;
	GLuint MeshPosID, MeshTexID;

	//texture data
	GLuint Texture, Texture2, TextureMush, treeTexture, grassTexture;
	GLuint waterTexture, waterTexture2, waterSplash;
	GLuint HeightTex;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		
		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			if (jump == true)
			{
				key_w = 1;
			}
			//mycam.w = 1;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			key_w = 0;
			//mycam.w = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			if (jump == true)
			{
				key_s = 1;
			}
			//mycam.s = 1;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			key_s = 0;
			//mycam.s = 0;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			if (jump == true)
			{
				key_a = 1;
			}
			//mycam.a = 1;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			key_a = 0;
			//mycam.a = 0;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			if (jump == true)
			{
				key_d = 1;
			}
			//mycam.d = 1;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			key_d = 0;
			//mycam.d = 0;
		}
		if (key == GLFW_KEY_M && action == GLFW_PRESS)
		{
			key_m = 1;
		}
		if (key == GLFW_KEY_M && action == GLFW_RELEASE)
		{
			key_m = 0;
		}
		/**********************JUMP**********************************/
		if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		{
			if(start == true)
				jump = true;
		}
		/*******START GAME**********/
		if (key == GLFW_KEY_P && action == GLFW_PRESS)
		{
			if(win == 0)
				start = true;
		}
		if (key == GLFW_KEY_B && action == GLFW_PRESS)
		{
			if (on == 0.)
				on = 1.;
			else
				on = 0.;
		}
	}

	// callback for the mouse when clicked move the triangle when helper functions
	// written
	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;
		float newPt[2];
		//if (action == GLFW_PRESS)
		//{
		//	glfwGetCursorPos(window, &posX, &posY);
		//	std::cout << "Pos X " << posX <<  " Pos Y " << posY << std::endl;

		//	//change this to be the points converted to WORLD
		//	//THIS IS BROKEN< YOU GET TO FIX IT - yay!
		//	newPt[0] = 0;
		//	newPt[1] = 0;

		//	std::cout << "converted:" << newPt[0] << " " << newPt[1] << std::endl;
		//	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
		//	//update the vertex array with the updated points
		//	glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*6, sizeof(float)*2, newPt);
		//	glBindBuffer(GL_ARRAY_BUFFER, 0);
		//}
	}

	//if the window is resized, capture the new size and reset the viewport
	void resizeCallback(GLFWwindow *window, int in_width, int in_height)
	{
		//get the window size - may be different then pixels for retina
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}

#define MESHSIZE 100

	void init_mesh()
	{

		/************ADDED POST PROCESSING**************/
		glGenVertexArrays(1, &VertexArrayIDRect);
		glBindVertexArray(VertexArrayIDRect);

		glGenBuffers(1, &VertexBufferIDRect);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferIDRect);
		GLfloat* ver = new GLfloat[18];
		int verc = 0;
		ver[verc++] = -1.0, ver[verc++] = -1.0, ver[verc++] = 0.0;
		ver[verc++] = 1.0, ver[verc++] = -1.0, ver[verc++] = 0.0;
		ver[verc++] = -1.0, ver[verc++] = 1.0, ver[verc++] = 0.0;
		ver[verc++] = 1.0, ver[verc++] = -1.0, ver[verc++] = 0.0;
		ver[verc++] = 1.0, ver[verc++] = 1.0, ver[verc++] = 0.0;
		ver[verc++] = -1.0, ver[verc++] = 1.0, ver[verc++] = 0.0;
		glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float), ver, GL_DYNAMIC_DRAW);

		//we need to set up the vertex array
		glEnableVertexAttribArray(0);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//repeat for texture coords
		glGenBuffers(1, &VertexBufferTexRect);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferTexRect);

		float t = 1. / 100;
		GLfloat* cube_tex = new GLfloat[12];
		int texc = 0;
		cube_tex[texc++] = 0, cube_tex[texc++] = 0;
		cube_tex[texc++] = 1, cube_tex[texc++] = 0;
		cube_tex[texc++] = 0, cube_tex[texc++] = 1;
		cube_tex[texc++] = 1, cube_tex[texc++] = 0;
		cube_tex[texc++] = 1, cube_tex[texc++] = 1;
		cube_tex[texc++] = 0, cube_tex[texc++] = 1;

		glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), cube_tex, GL_DYNAMIC_DRAW);

		//we need to set up the vertex array
		glEnableVertexAttribArray(1);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		/*******************billboard************************/
		//generate the VAO
		glGenVertexArrays(1, &VertexArrayIDBill);
		glBindVertexArray(VertexArrayIDBill);

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VertexBufferIDBill);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferIDBill);

		GLfloat rect_vertices[] = {
			// front
			-1.0, -1.0,  1.0,//LD
			1.0, -1.0,  1.0,//RD
			1.0,  1.0,  1.0,//RU
			-1.0,  1.0,  1.0,//LU
		};
		//make it a bit smaller
		for (int i = 0; i < 12; i++)
			rect_vertices[i] *= 0.5;
		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, sizeof(rect_vertices), rect_vertices, GL_DYNAMIC_DRAW);

		//we need to set up the vertex array
		glEnableVertexAttribArray(0);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//color
		GLfloat cube_norm[] = {
			// front colors
			0.0, 0.0, 1.0,
			0.0, 0.0, 1.0,
			0.0, 0.0, 1.0,
			0.0, 0.0, 1.0,

		};
		glGenBuffers(1, &VertexNormDBoxBill);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexNormDBoxBill);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_norm), cube_norm, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//color
		glm::vec2 cube_tex1[] = {
			// front colors
			glm::vec2(0.0, 1.0),
			glm::vec2(1.0, 1.0),
			glm::vec2(1.0, 0.0),
			glm::vec2(0.0, 0.0),

		};
		glGenBuffers(1, &VertexTexBoxBill);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexTexBoxBill);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_tex1), cube_tex1, GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glGenBuffers(1, &IndexBufferIDBoxBill);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBoxBill);
		GLushort cube_elements[] = {
			// front
			0, 1, 2,
			2, 3, 0,
		};
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW);

		//generate vertex buffer to hand off to OGL ###########################
		/*glGenBuffers(1, &InstanceBuffer);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, InstanceBuffer);
		glm::vec4* positions = new glm::vec4[20];
		for (int i = 0; i < 20; i++)
			positions[i] = glm::vec4(-250 + i, 0, -10, 0);
		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, 20 * sizeof(glm::vec4), positions, GL_STATIC_DRAW);
		int position_loc = glGetAttribLocation(pgrass->pid, "InstancePos");
		for (int i = 0; i < 20; i++)
		{
			// Set up the vertex attribute
			glVertexAttribPointer(position_loc + i,              // Location
				4, GL_FLOAT, GL_FALSE,       // vec4
				sizeof(vec4),                // Stride
				(void*)(sizeof(vec4) * i)); // Start offset
											 // Enable it
			glEnableVertexAttribArray(position_loc + i);
			// Make it instanced
			glVertexAttribDivisor(position_loc + i, 1);
		}
		//glBindVertexArray(0); */

		/******************mesh************************/

		//generate the VAO
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &MeshPosID);
		glBindBuffer(GL_ARRAY_BUFFER, MeshPosID);
		vec3 vertices[MESHSIZE * MESHSIZE * 4]; //4 vertices in rectangle
		for (int x = 0; x < MESHSIZE; x++)
		{
			for (int z = 0; z < MESHSIZE; z++)
			{
				vertices[x * 4 + z * MESHSIZE * 4 + 0] = vec3(0.0, 0.0, 0.0) + vec3(x, 0, z);
				vertices[x * 4 + z * MESHSIZE * 4 + 1] = vec3(1.0, 0.0, 0.0) + vec3(x, 0, z);
				vertices[x * 4 + z * MESHSIZE * 4 + 2] = vec3(1.0, 0.0, 1.0) + vec3(x, 0, z);
				vertices[x * 4 + z * MESHSIZE * 4 + 3] = vec3(0.0, 0.0, 1.0) + vec3(x, 0, z);
			}
		}
		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * MESHSIZE * MESHSIZE * 4, vertices, GL_DYNAMIC_DRAW);
		//we need to set up the vertex array
		glEnableVertexAttribArray(0);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//tex coords
		t = 1. / 100;
		vec2 tex[MESHSIZE * MESHSIZE * 4];
		for (int x = 0; x < MESHSIZE; x++)
		{
			for (int y = 0; y < MESHSIZE; y++)
			{
				tex[x * 4 + y * MESHSIZE * 4 + 0] = vec2(0.0, 0.0) + vec2(x,y) * t;
				tex[x * 4 + y * MESHSIZE * 4 + 1] = vec2(t, 0.0) + vec2(x, y) * t;
				tex[x * 4 + y * MESHSIZE * 4 + 2] = vec2(t, t) + vec2(x, y) * t;
				tex[x * 4 + y * MESHSIZE * 4 + 3] = vec2(0.0, t) + vec2(x, y) * t;
			}
		}
		glGenBuffers(1, &MeshTexID);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, MeshTexID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * MESHSIZE * MESHSIZE * 4, tex, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0); //it was this

		glGenBuffers(1, &IndexBufferIDBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
		GLushort elements[MESHSIZE * MESHSIZE * 6]; //each rectangle (4 vertices) has two triangles
		int ind = 0;
		for (int i = 0; i < MESHSIZE * MESHSIZE * 6; i += 6, ind += 4)
		{
			elements[i + 0] = ind + 0;
			elements[i + 1] = ind + 1;
			elements[i + 2] = ind + 2;
			elements[i + 3] = ind + 0;
			elements[i + 4] = ind + 2;
			elements[i + 5] = ind + 3;
		}
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * MESHSIZE * MESHSIZE * 6, elements, GL_STATIC_DRAW);
		glBindVertexArray(0);
	}

	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom()
	{

		initMushPos();
		initTreePos(); 

		//initialize net mesh
		init_mesh();

		string resourceDirectory = "../resources" ;
		// Initialize mesh.
		shape = make_shared<Shape>();
		//shape->loadMesh(resourceDirectory + "/t800.obj");
		shape->loadMesh(resourceDirectory + "/sphere.obj");
		shape->resize();
		shape->init();

		/******************add frog**********************/
		frog = make_shared<Shape>();
		frog->loadMesh(resourceDirectory + "/frog-body.obj");
		frog->resize();
		frog->init();
		frogL = make_shared<Shape>();
		frogL->loadMesh(resourceDirectory + "/frog-leftleg.obj");
		frogL->resize();
		frogL->init();
		frogR = make_shared<Shape>();
		frogR->loadMesh(resourceDirectory + "/frog-rightleg.obj");
		frogR->resize();
		frogR->init();

		/*******************add mushroom******************/
		mushroom = make_shared<Shape>();
		mushroom->loadMesh(resourceDirectory + "/mushroom.obj");
		mushroom->resize();
		mushroom->init();

		/*******************add tree******************/
		tree = make_shared<Shape>();
		tree->loadMesh(resourceDirectory + "/lowpolytree.obj");
		tree->resize();
		tree->init();

		int width, height, channels;
		char filepath[1000];


		//texture 1
		string str = resourceDirectory + "/grass.jpg";
		strcpy(filepath, str.c_str());
		unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		/*str = resourceDirectory + "/grassbill.png";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &grassTexture);
		//glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, grassTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D); */

		//texture 2
		str = resourceDirectory + "/sky.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture2);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		/**********************WATER*****************************/

		str = resourceDirectory + "/waternormal.png";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &waterTexture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, waterTexture);
		// affects x and y of texture - should repeat, stretch, or not
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// filter parameters- affects texture depending on camera distance, do not interpolate (GL_NEAREST) or interpolate betwn surrounding pixels --> more blurry
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		str = resourceDirectory + "/waternormal2.png";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &waterTexture2);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, waterTexture2);
		// affects x and y of texture - should repeat, stretch, or not
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// filter parameters- affects texture depending on camera distance, do not interpolate (GL_NEAREST) or interpolate betwn surrounding pixels --> more blurry
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		str = resourceDirectory + "/watersplash.png";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &waterSplash);
		//glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, waterSplash);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		/********************HEIGHT TEXTURE***************************/
		str = resourceDirectory + "/height2.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &HeightTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, HeightTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		/**********************MUSHROOM*********************/
		str = resourceDirectory + "/mushroom.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &TextureMush);
		//glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureMush);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		/**********************TREE*********************/
		str = resourceDirectory + "/tree.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &treeTexture);
		//glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, treeTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//[TWOTEXTURES]
		//set the 2 textures to the correct samplers in the fragment shader:
		GLuint Tex1Location = glGetUniformLocation(prog->pid, "tex");//tex, tex2... sampler in the fragment shader
		GLuint Tex2Location = glGetUniformLocation(prog->pid, "tex2");
		// Then bind the uniform samplers to texture units:
		glUseProgram(prog->pid);
		glUniform1i(Tex1Location, 0);
		glUniform1i(Tex2Location, 1);

		Tex1Location = glGetUniformLocation(heightShader->pid, "tex");//tex, tex2... sampler in the fragment shader
		Tex2Location = glGetUniformLocation(heightShader->pid, "tex2");
		// Then bind the uniform samplers to texture units:
		glUseProgram(heightShader->pid);
		glUniform1i(Tex1Location, 0);
		glUniform1i(Tex2Location, 1);

		Tex1Location = glGetUniformLocation(pwater->pid, "tex");//tex, tex2... sampler in the fragment shader
		Tex2Location = glGetUniformLocation(pwater->pid, "tex2");
		// Then bind the uniform samplers to texture units:
		glUseProgram(pwater->pid);
		glUniform1i(Tex1Location, 0);
		glUniform1i(Tex2Location, 1);

		Tex1Location = glGetUniformLocation(psplash->pid, "tex");//tex, tex2... sampler in the fragment shader
		Tex2Location = glGetUniformLocation(psplash->pid, "tex2");
		// Then bind the uniform samplers to texture units:
		glUseProgram(psplash->pid);
		glUniform1i(Tex1Location, 0);
		glUniform1i(Tex2Location, 1);;

		Tex1Location = glGetUniformLocation(ptree->pid, "tex");//tex, tex2... sampler in the fragment shader
		Tex2Location = glGetUniformLocation(ptree->pid, "tex2");
		// Then bind the uniform samplers to texture units:
		glUseProgram(ptree->pid);
		glUniform1i(Tex1Location, 0);
		glUniform1i(Tex2Location, 1);

		Tex1Location = glGetUniformLocation(prog_framebuffer->pid, "tex");//tex, tex2... sampler in the fragment shader
		Tex2Location = glGetUniformLocation(prog_framebuffer->pid, "tex2");
		// Then bind the uniform samplers to texture units:
		glUseProgram(prog_framebuffer->pid);
		glUniform1i(Tex1Location, 0);
		glUniform1i(Tex2Location, 1);

		/*Tex1Location = glGetUniformLocation(pgrass->pid, "tex");//tex, tex2... sampler in the fragment shader
		Tex2Location = glGetUniformLocation(pgrass->pid, "tex2");
		// Then bind the uniform samplers to texture units:
		glUseProgram(pgrass->pid);
		glUniform1i(Tex1Location, 0);
		glUniform1i(Tex2Location, 1);*/


		//diffuse into background
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		/********************************************************************/
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glGenTextures(1, &FBOtex);
		glBindTexture(GL_TEXTURE_2D, FBOtex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
		glGenerateMipmap(GL_TEXTURE_2D);

		// hold another texture for positions
		glGenTextures(1, &FBOtexpos);
		glBindTexture(GL_TEXTURE_2D, FBOtexpos);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_BGRA, GL_FLOAT, NULL);
		glGenerateMipmap(GL_TEXTURE_2D);

		//----------generate frame buffer-------------
		glGenFramebuffers(1, &FrameBufferObj);
		glBindFramebuffer(GL_FRAMEBUFFER, FrameBufferObj);
		//attach 2D texture to FBO
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FBOtex, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, FBOtexpos, 0);
		//depth buffer
		glGenRenderbuffers(1, &depth_rb);
		glBindRenderbuffer(GL_RENDERBUFFER, depth_rb);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
		//attach depth buffer to FBO
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rb);

		//Tex1Location = glGetUniformLocation(prog_framebuffer->pid, "tex");
		// Then bind the uniform samplers to texture units:
		//glUseProgram(prog_framebuffer->pid);
		//glUniform1i(Tex1Location, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}

	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);
		//glDisable(GL_DEPTH_TEST);
		// Initialize the GLSL program.
		prog = std::make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/shader_fragment.glsl");
		if (!prog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("campos");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		prog->addAttribute("vertTex");

		pmush = std::make_shared<Program>();
		pmush->setVerbose(true);
		pmush->setShaderNames(resourceDirectory + "/mush_vertex.glsl", resourceDirectory + "/mush_fragment.glsl");
		if (!pmush->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		pmush->addUniform("P");
		pmush->addUniform("V");
		pmush->addUniform("M");
		pmush->addUniform("campos");
		pmush->addAttribute("vertPos");
		pmush->addAttribute("vertNor");
		pmush->addAttribute("vertTex");


		heightShader = std::make_shared<Program>();
		heightShader->setVerbose(true);
		heightShader->setShaderNames(resourceDirectory + "/height_vertex.glsl", resourceDirectory + "/height_frag.glsl");
		if (!heightShader->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		heightShader->addUniform("P");
		heightShader->addUniform("V");
		heightShader->addUniform("M");
		heightShader->addUniform("campos");
		heightShader->addUniform("camoff");
		heightShader->addAttribute("vertPos");
		heightShader->addAttribute("vertTex");


		psky = std::make_shared<Program>();
		psky->setVerbose(true);
		psky->setShaderNames(resourceDirectory + "/skyvertex.glsl", resourceDirectory + "/skyfrag.glsl");
		if (!psky->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		psky->addUniform("dn");
		psky->addUniform("P");
		psky->addUniform("V");
		psky->addUniform("M");
		psky->addUniform("campos");
		psky->addAttribute("vertPos");
		psky->addAttribute("vertNor");
		psky->addAttribute("vertTex");

		pwater = std::make_shared<Program>();
		pwater->setVerbose(true);
		pwater->setShaderNames(resourceDirectory + "/water_vertex.glsl", resourceDirectory + "/water_frag.glsl");
		if (!pwater->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		pwater->addUniform("P");
		pwater->addUniform("V");
		pwater->addUniform("M");
		pwater->addUniform("campos");
		pwater->addUniform("texoff");
		pwater->addUniform("lightpos");
		pwater->addAttribute("vertPos");
		pwater->addAttribute("vertTex");

		psplash = std::make_shared<Program>();
		psplash->setVerbose(true);
		psplash->setShaderNames(resourceDirectory + "/splash_vertex.glsl", resourceDirectory + "/splash_frag.glsl");
		if (!psplash->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		psplash->addUniform("P");
		psplash->addUniform("V");
		psplash->addUniform("M");
		psplash->addUniform("campos");
		psplash->addUniform("texoff");
		psplash->addUniform("splash");
		psplash->addAttribute("vertPos");
		psplash->addAttribute("vertNor");
		psplash->addAttribute("vertTex");

		ptree = std::make_shared<Program>();
		ptree->setVerbose(true);
		ptree->setShaderNames(resourceDirectory + "/tree_vertex.glsl", resourceDirectory + "/tree_fragment.glsl");
		if (!ptree->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		ptree->addUniform("P");
		ptree->addUniform("V");
		ptree->addUniform("M");
		ptree->addUniform("campos");
		ptree->addAttribute("vertPos");
		ptree->addAttribute("vertNor");
		ptree->addAttribute("vertTex");

		prog_framebuffer = std::make_shared<Program>();
		prog_framebuffer->setVerbose(true);
		prog_framebuffer->setShaderNames(resourceDirectory + "/vertFB.glsl", resourceDirectory + "/fragFB.glsl");
		if (!prog_framebuffer->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog_framebuffer->init();
		prog_framebuffer->addUniform("P");
		prog_framebuffer->addUniform("V");
		prog_framebuffer->addUniform("M");
		prog_framebuffer->addUniform("on");
		prog_framebuffer->addAttribute("vertPos");
		prog_framebuffer->addAttribute("vertTex");

		/*pgrass = std::make_shared<Program>();
		pgrass->setVerbose(true);
		pgrass->setShaderNames(resourceDirectory + "/grass_vertex.glsl", resourceDirectory + "/grass_fragment.glsl");
		if (!pgrass->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		pgrass->addUniform("P");
		pgrass->addUniform("V");
		pgrass->addUniform("M");
		pgrass->addUniform("campos");
		pgrass->addAttribute("vertPos");
		pgrass->addAttribute("vertNor");
		pgrass->addAttribute("vertTex");
		pgrass->addAttribute("InstancePos");*/
	}

	void render()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width / (float)height;
		glViewport(0, 0, width, height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		prog_framebuffer->bind();
		glUniform1f(prog_framebuffer->getUniform("on"), on);
		glActiveTexture(GL_TEXTURE0);

		glBindTexture(GL_TEXTURE_2D, FBOtex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, FBOtexpos);
		glBindVertexArray(VertexArrayIDRect);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
		prog_framebuffer->unbind();
	}

	/****DRAW
	This is the most important function in your program - this is where you
	will actually issue the commands to draw any geometry you have set up to
	draw
	********/
	void render_to_framebuffer()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, FrameBufferObj);
		GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, buffers);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		double frametime = get_last_elapsed_time();

		static double totaltime = 0;
		totaltime += frametime;

		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width/(float)height;
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClearColor(0.8, 0.8, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create the matrix stacks - please leave these alone for now
		
		glm::mat4 V, M, P, ML,Temp; //View, Model and Perspective matrix
		V = mycam.process(frametime);
		M = glm::mat4(1);
		ML = glm::mat4(1);
		Temp = glm::mat4(1);
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width/ (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones
		

		float sangle = 3.1415926 / 2.;
		glm::mat4 RotateXSky = glm::rotate(glm::mat4(1.0f), sangle, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 RotateYSky = glm::rotate(glm::mat4(1.0f), (float)3.1415926, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec3 camp = -mycam.pos;
		glm::mat4 Trans = glm::translate(glm::mat4(1.0f), camp);
		//glm::mat4 TransSky = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
		glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(0.8f, 0.8f, 0.8f));

		M = Trans * RotateYSky * RotateXSky * S;


		/*
		---------------------------------------------------------
		-----------------------SKYSPHERE-------------------------
		---------------------------------------------------------
		*/

		psky->bind();

		glUniformMatrix4fv(psky->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(psky->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(psky->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(psky->getUniform("campos"), 1, &mycam.pos[0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		glActiveTexture(GL_TEXTURE1);
		glDisable(GL_DEPTH_TEST);
		shape->draw(psky, FALSE);
		glEnable(GL_DEPTH_TEST);
		psky->unbind();


		/*
		---------------------------------------------------------
		------------------------FROG MODEL---------------------
		---------------------------------------------------------
		*/
		//glm::mat4 RotateX = glm::rotate(glm::mat4(1.0f), -sangle, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 RotateY = glm::rotate(glm::mat4(1.0f), (float)3.1415926, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 Rotate = glm::rotate(glm::mat4(1.0f), -frog_x, glm::vec3(0.0f, 1.0f, 0.0f));
		//camp = -mycam.pos;
		//glm::mat4 TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -2) );
		Trans = glm::translate(glm::mat4(1.0f), glm::vec3(frog_x, frog_y, frog_z) );
		S = glm::scale(glm::mat4(1.0f), glm::vec3(0.4f, 0.4f, 0.4f));


		// Draw the box using GLSL.
		prog->bind();

		V = mycam.process(frametime);
		/*mat4 iV = inverse(V);
		V = TransZ * V; */

		M = Trans * Rotate * RotateY;
		Temp = M * S;

		//send the matrices to the shaders
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &Temp[0][0]);
		glUniform3fv(prog->getUniform("campos"), 1, &mycam.pos[0]);	
		/*glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);*/
		frog->draw(prog, FALSE);
		//tree->draw(prog, FALSE);


		leftleg += waveParentRF(leftleg, &swingingUpParentLL);
		if (jump == 0)
			leftleg = 0;
		mat4 Tleg = glm::translate(glm::mat4(1.0f), glm::vec3(0.18, -0.09, -0.3));
		mat4 TOleg = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0, 0.1));
		mat4 RYleg = glm::rotate(glm::mat4(1.0f), leftleg, glm::vec3(1.0f, 0.0f, 0.0f));
		S = glm::scale(glm::mat4(1.0f), glm::vec3(0.22f, 0.22f, 0.22f));
		ML =  M * Tleg * RYleg * TOleg;
		Temp = ML * S;
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &Temp[0][0]);
		frogL->draw(prog, FALSE);


		rightleg += waveParentRF(rightleg, &swingingUpParentRL);
		if (jump == 0)
			rightleg = 0;
		Tleg = glm::translate(glm::mat4(1.0f), glm::vec3(-0.2, -0.09, -0.3));
		TOleg = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0, 0.1));
		RYleg = glm::rotate(glm::mat4(1.0f), rightleg, glm::vec3(1.0f, 0.0f, 0.0f));
		S = glm::scale(glm::mat4(1.0f), glm::vec3(0.22f, 0.22f, 0.22f));
		ML = M * Tleg * RYleg * TOleg;
		Temp = ML * S;
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &Temp[0][0]);
		frogR->draw(prog, FALSE);

		prog->unbind();

		/*
		---------------------------------------------------------
		-----------------------TREE-----------------------------
		---------------------------------------------------------
		*/
		ptree->bind();

		V = mycam.process(frametime);
		glUniformMatrix4fv(ptree->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(ptree->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniform3fv(ptree->getUniform("campos"), 1, &mycam.pos[0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, treeTexture);
		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_DEPTH_TEST);

		S = glm::scale(glm::mat4(1.0f), glm::vec3(8.f, 8.f, 8.f));

		//on left side
		for (int i = 0; i < 12; i++)
		{
			Trans = glm::translate(glm::mat4(1.0f), treepos[i]);
			M = S * Trans;
			glUniformMatrix4fv(ptree->getUniform("M"), 1, GL_FALSE, &M[0][0]);
			tree->draw(ptree, FALSE);
		}
		ptree->unbind();

		/*
		---------------------------------------------------------
		--------------------MUSHROOM MODEL-----------------------
		---------------------------------------------------------
		*/

		pmush->bind();

		glm::mat4 RotateX = glm::rotate(glm::mat4(1.0f), -sangle, glm::vec3(1.0f, 0.0f, 0.0f));
		S = glm::scale(glm::mat4(1.0f), glm::vec3(0.8f, 0.8f, 0.8f));
		for (int i = 0; i < MUSH_NUM; i++) {
			Trans = glm::translate(glm::mat4(1.0f), mushpos[i]);
			M = Trans * RotateX * S;
			glUniformMatrix4fv(pmush->getUniform("P"), 1, GL_FALSE, &P[0][0]);
			glUniformMatrix4fv(pmush->getUniform("M"), 1, GL_FALSE, &M[0][0]);
			glUniformMatrix4fv(pmush->getUniform("V"), 1, GL_FALSE, &V[0][0]);
			glUniform3fv(pmush->getUniform("campos"), 1, &mycam.pos[0]);
			glBindVertexArray(VertexArrayID);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, TextureMush);
			mushroom->draw(pmush, FALSE);
		}

		pmush->unbind();

		

		/*
		---------------------------------------------------------
		---------------------HEIGHT MAPPING----------------------
		---------------------------------------------------------
		*/

		heightShader->bind();
		V = mycam.process(frametime);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		Trans = glm::translate(glm::mat4(1.0f), glm::vec3(-50,-1.0f, -50));
		M = Trans;
		glUniformMatrix4fv(heightShader->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(heightShader->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(heightShader->getUniform("M"), 1, GL_FALSE, &M[0][0]);

		/**for infinite mapping**/
		vec3 offset = mycam.pos;
		offset.y = 0;
		offset.x = (int)offset.x;
		offset.x = (int)offset.z;
		/*************************/
		glUniform3fv(heightShader->getUniform("campos"), 1, &mycam.pos[0]);
		glUniform3fv(heightShader->getUniform("camoff"), 1, &mycam.pos[0]);
		glBindVertexArray(VertexArrayID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, HeightTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glDrawElements(GL_TRIANGLES, MESHSIZE*MESHSIZE*6, GL_UNSIGNED_SHORT, (void*)0);		
		heightShader->unbind();

		/*
		---------------------------------------------------------
		-----------------------WATER-----------------------------
		---------------------------------------------------------
		*/

		pwater->bind();
		Trans = glm::translate(glm::mat4(1.0f), glm::vec3(-50.f, -0.8f, -200.));//-50.f, -0.8f, -200
		M = Trans;

		vec3 lightpos = vec4(0, 200., -300, 0);
		static float texoff = 0;
		texoff += frametime * 0.02;

		glUniformMatrix4fv(pwater->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(pwater->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(pwater->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(pwater->getUniform("lightpos"), 1, &lightpos[0]);
		glUniform3fv(pwater->getUniform("campos"), 1, &mycam.pos[0]);
		glUniform1f(pwater->getUniform("texoff"), texoff);
		glBindVertexArray(VertexArrayID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, waterTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, waterTexture2);
		glDrawElements(GL_TRIANGLES, MESHSIZE * MESHSIZE * 6, GL_UNSIGNED_SHORT, (void*)0);

		pwater->unbind();

		/******************************SPLASH********************************/
		psplash->bind();
		glUniformMatrix4fv(psplash->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(psplash->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniform3fv(psplash->getUniform("campos"), 1, &mycam.pos[0]);
		glBindVertexArray(VertexArrayIDBill);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBoxBill);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, waterSplash);

		//for tile animation 
		float trigger = totaltime * 10.f;
		int trigger_i = (int)trigger;
		int tx = trigger_i % 4;
		int ty = trigger_i / 2;
		vec2 toff = vec2(tx, ty);

		//for billboards
		mat4 iVR = V;
		//getting out translation part - always on last row / column - depend on how view matrix
		iVR[3][0] = 0;
		iVR[3][1] = 0;
		iVR[3][2] = 0;
		iVR = inverse(iVR);

		glEnable(GL_DEPTH_TEST);

		S = glm::scale(glm::mat4(1.0f), glm::vec3(1., 1., 1.));
		Trans = glm::translate(glm::mat4(1.0f), vec3(frog_x, -0.3, frog_z)); //frog_x, -0.1, frog_z //0, 2, -9
		RotateX = glm::rotate(glm::mat4(1.0f), -sangle, glm::vec3(1.0f, 0.0f, 0.0f));
		M = Trans * iVR * S;
		glUniform2fv(psplash->getUniform("texoff"), 1, &toff.x);
		glUniformMatrix4fv(psplash->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform1f(psplash->getUniform("splash"), splash);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0);

		psplash->unbind();

		/*
		---------------------------------------------------------
		-----------------------GRASS BILLBOARDS-----------------------------
		---------------------------------------------------------
		
		pgrass->bind();

		glUniformMatrix4fv(pgrass->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(pgrass->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(pgrass->getUniform("M"), 1, GL_FALSE, &M[0][0]);


		glBindVertexArray(VertexArrayIDBill);
		//actually draw from vertex 0, 3 vertices
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBoxBill);
		//glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);
		mat4 Vi = glm::transpose(V);
		Vi[0][3] = 0;
		Vi[1][3] = 0;
		Vi[2][3] = 0;
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, grassTexture);

		Trans = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3));
		S = glm::scale(glm::mat4(1.0f), glm::vec3(1., 1., 1.));
		M = Trans * S * Vi;
		glUniformMatrix4fv(pgrass->getUniform("M"), 1, GL_FALSE, &M[0][0]);

		glDisable(GL_DEPTH_TEST);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0, 20);
		glEnable(GL_DEPTH_TEST);
		/*for (int z = 0; z < 5; z++)
		{
			glm::mat4 TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f+z, 0.0f, -3 - z));
			M = TransZ * S* Vi;
			glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0);
		}
		glBindVertexArray(0);
		pgrass->unbind();
		*/

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, FBOtex);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, FBOtexpos);
		glGenerateMipmap(GL_TEXTURE_2D);

		
		/*
		---------------------------------------------------------
		---------------------MOVEMENT----------------------------
		---------------------------------------------------------
		*/

		if (key_w == 1) {
			frog_z -= 0.2;
		}
		if (key_s == 1) {
			frog_z += 0.2;
		}
		if (key_a == 1) {
			frog_x -= 0.1;
		}
		if (key_d == 1) {
			frog_x += 0.1;
		}


		/*
		---------------------------------------------------------
		-----------------COLLISION TESTING-----------------------
		---------------------------------------------------------
		*/

		for (int i = 0; i < MUSH_NUM; i++)
		{
			vec2 mushroom1 = vec2(mushpos[i].x, mushpos[i].z);
			vec2 frog = vec2(frog_x, frog_z);
			if (distance(frog, mushroom1) < 0.7)
			{
				if (frog_y < mushpos[i].y + 0.8 && frog_y > 0.3)
				{
					frog_x = mushpos[i].x;
					frog_z = mushpos[i].z;
					frog_y = mushpos[i].y + 0.9;
					JUMP_VELOCITY = 0.25;
					jump = false;
				}
			}
		}

		if (jump == true)
		{
			frog_y += JUMP_VELOCITY;
			JUMP_VELOCITY -= GRAVITY;
			JUMP_VELOCITY = terminalVelocity(JUMP_VELOCITY);
			if (frog_y < -1.5) //if falls to floor - restarts to beginning
			{
				frog_x = 0;
				frog_y = 0.9;
				frog_z = -4;
				mycam.pos = glm::vec3(0, -1.5, 0);
				mycam.rot = glm::vec3(0, 0, 0);
				JUMP_VELOCITY = 0.25;
				jump = false;
				start = false;
			}
			if (frog_z < -100 && frog_y < 0.0)//if reach water
			{
				frog_y = -0.8;
				start = false;
				jump = false;
				win = 1;
			}
		}
		if (jump == false && frog_z < -100 && frog_y < 0.0)
		{
			splashtime += 0.03;
			if (splashtime > 0.8)
				splash = 0;
			else
				splash = 1;
		}

		if (start == true && -mycam.pos.z < frog_z) //frog off camera
		{
			start = false;
			mycam.pos = glm::vec3(0, -1.5, 0);
			mycam.rot = glm::vec3(0, 0, 0);

			frog_x = 0;
			frog_y = 0.9;
			frog_z = -4;
			JUMP_VELOCITY = 0.25;
			jump = false;
		}

		
	}

	float waveParentRF(float wave, bool* swingingUpParent)
	{
		if (jump == 1)
		{
			if (wave < 0.0)
				*swingingUpParent = true;
			else if (wave > 1)
				//*swingingUpParent = false;
				return 0;
			// update based on direction
			if (*swingingUpParent == false)
				return -0.05;
			else
				return 0.05;
		}
		else
			return 0;
	}

	float terminalVelocity(float speed) {
		if (speed < 0) {
			if (speed < GRAVITY * -5) {
				return GRAVITY * -5;
			}
		}
		return speed;
	}

	float x, y, z;
	float initMushPos()
	{
		mushpos[0] = vec3(0.0f, 0.0f, -4.0f); // starting mushroom pos
		mushpos[1] = vec3(1.0f, 0.0f, -5.0f);

		for (int i = 2; i < MUSH_NUM; i++)
		{
			 // (rand) * (max-min) + min
				x = ( (float)rand() / (float)RAND_MAX ) * (8) - 4 ;
				//x = -20 + (float)(rand()) / (float)((RAND_MAX / (25))); //-3 - 3
				z = ((float)rand() / (float)RAND_MAX) * (96) + (-100);
				//z = -30 + (float)(rand()) / (float)((RAND_MAX / (24))); // -6 - -100
			
			mushpos[i] = vec3(x, 0.0, z);
		}
		return 0;
	}
	float initTreePos()
	{
		treepos[0] = vec3(-2, 1., -3.4);
		treepos[1] = vec3(-2.5, 1, -7);
		treepos[2] = vec3(-3, 1, -10);
		treepos[3] = vec3(-2.9, 1, -13);
		treepos[4] = vec3(-2.4, 1, -15);
		treepos[5] = vec3(-2.3, 0.98, -16);
		treepos[6] = vec3(3, 1, -4);
		treepos[7] = vec3(2.5, 1, -7);
		treepos[8] = vec3(3.2, 1, -9);
		treepos[9] = vec3(3, 1, -12);
		treepos[10] = vec3(2.5, 1, -14);
		treepos[11] = vec3(2.9, 1, -16);

		return 0;
	}

};
//******************************************************************************************
int main(int argc, char **argv)
{
	std::string resourceDir = "../resources"; // Where the resources are loaded from
	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	/* your main will always include a similar set up to establish your window
		and GL context, etc. */
	WindowManager * windowManager = new WindowManager();
	windowManager->init(1920, 1080);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
	// Initialize scene.
	application->init(resourceDir);
	application->initGeom();
	srand(time(NULL));

	// Loop until the user closes the window.
	while(! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render_to_framebuffer();
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
