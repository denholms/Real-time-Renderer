/*	
	Denholm Scrimshaw 
	OpenGL - 3D Model 
	Utilizes GLEW for interfacing with OpenGL
	Includes SDL for window context, SOIL for textures, and GLM for 3D math
	June 13th, 2016
*/

#define GLEW_STATIC

#include <GL/glew.h>
#include <SOIL.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>		//Calculating view and projection matrices
#include <glm/gtc/type_ptr.hpp>				//Converting matrix object into float array for OpenGL
#include <stdio.h>
#include <string>
#include <fstream>
#include <vector>
#include <chrono>
#include <algorithm>

std::string StringFromFile(const char* filename)
{
	std::ifstream fs(filename);
	if (!fs)
	{
		return "";
	}

	std::string s(
		std::istreambuf_iterator<char>{fs},
		std::istreambuf_iterator<char>{});

	return s;
}

int main(int argc, char *argv[])
{
	auto t_start = std::chrono::high_resolution_clock::now();

	//Set up window and context
	SDL_Init(SDL_INIT_VIDEO);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	SDL_Window* window = SDL_CreateWindow("OpenGL", 100, 100, 800, 600, SDL_WINDOW_OPENGL);
	if (!window)
	{
		fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
		return 1;
	}

	SDL_GLContext context = SDL_GL_CreateContext(window);
	if (!context)
	{
		fprintf(stderr, "SDL_GL_CreateContext: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Event windowEvent;

	//Load GLEW to enable OpenGL functionality
	glewExperimental = GL_TRUE;
	glewInit();

	//Create and bind Vertex Array Object (while store all links between atrtibutes and VBOs with raw vertex data)
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Compile vertex shader
	const char* version = "#version 150\n";
	GLint status;
	std::string vs_source = StringFromFile("scene.vert");
	const char* vs_strings[] = { version, vs_source.c_str() };
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 2, vs_strings, NULL);
	glCompileShader(vs);
	glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
	if (!status)
	{
		GLint logLength;
		glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &logLength);
		std::vector<char> log(logLength + 1);
		glGetShaderInfoLog(vs, logLength, NULL, log.data());
		fprintf(stderr, "Error compiling vertex shader: %s\n", log.data());
		return 1;
	}

	// Compile fragment shader
	std::string fs_source = StringFromFile("scene.frag");
	const char* fs_strings[] = { version, fs_source.c_str() };
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 2, fs_strings, NULL);
	glCompileShader(fs);
	glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
	if (!status)
	{
		GLint logLength;
		glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &logLength);
		std::vector<char> log(logLength + 1);
		glGetShaderInfoLog(fs, logLength, NULL, log.data());
		fprintf(stderr, "Error compiling fragment shader: %s\n", log.data());
		return 1;
	}

	// Link program (vertex + fragment shader)
	GLuint program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glBindFragDataLocation(program, 0, "outColor");
	//Use glDrawBuffers when rendering to multiple buffers, because only the first output will be enabled by default
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (!status)
	{
		GLint logLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
		std::vector<char> log(logLength + 1);
		glGetProgramInfoLog(program, logLength, NULL, log.data());
		fprintf(stderr, "Error linking program: %s\n", log.data());
		return 1;
	}

	// Can now bind the program to the graphics pipeline, to render with it.
	glUseProgram(program);

	//Enable zBuffering
	glEnable(GL_DEPTH_TEST);

	//Create vertex data and copy to Vertex Buffer Object
	GLuint vbo;
	glGenBuffers(1, &vbo);					// Generate 1 buffer?

	GLfloat vertices[] = {
		//X		 Y		Z	  R		G	  B		U	  V		Nx	  Ny	 Nz

		//Cube bottom
		-0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		 0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,

		//Cube top
		-0.5f, -0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		 0.5f, -0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		 0.5f,  0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		 0.5f,  0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		-0.5f,  0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		-0.5f, -0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,

		//Cube left
		-0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,

		//Cube right
		0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
		0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
		0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
		0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,

		//Cube back
		-0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f,
		 0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
		 0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
		 0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
		-0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f,

		//Cube front
		-0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		 0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		 0.5f, 0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		 0.5f, 0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		-0.5f, 0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		-0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,

		//Mirrored cube
		-1.5f, -1.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		 1.5f, -1.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		 1.5f,  1.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		 1.5f,  1.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		-1.5f,  1.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		-1.5f, -1.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		
		// -- Line grid --

		//Bottom
		 2.4f, -2.4f, -0.4f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		-2.4f, -2.4f, -0.4f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

		//Top
		-2.4f,  2.4f, -0.4f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	     2.4f,  2.4f, -0.4f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

		 //Left
		 2.4f, -2.4f, -0.4f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		 2.4f,  2.4f, -0.4f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	
		//Right
		-2.4f, -2.4f, -0.4f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		-2.4f,  2.4f, -0.4f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

		 //Center x
		 0.0f, -4.8f, -0.4f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		 0.0f,  4.8f, -0.4f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

		 //Center y
		-4.8f,  0.0f, -0.4f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		 4.8f,  0.0f, -0.4f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

		 //Mid right
		 1.2f, -2.4f, -0.4f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		 1.2f,  2.4f, -0.4f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

		 //Mid up
		-2.4f,  1.2f, -0.4f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		 2.4f,  1.2f, -0.4f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

		 //Mid Left
		-1.2f, -2.4f, -0.4f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	    -1.2f,  2.4f, -0.4f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

		//Mid bottom
		-2.4f, -1.2f, -0.4f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		 2.4f, -1.2f, -0.4f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
	};
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo);		//Make it the active object
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Copy over vertex data

	// -- Setup drawing attributes --
	//Input, # values (# components of vec), type of component, bool for -1 -> 1 normalized,
	//stride (bytes between each position attribute), offset (how many bytes from start the attribute occurs)
	//Will also store the VBO bound to GL_ARRAY_BUFFER - can use a different VBO for each attribute

	//Position attribute
	GLint posAttrib = glGetAttribLocation(program, "position");
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(posAttrib);

	//Color attribute
	GLint colAttrib = glGetAttribLocation(program, "color");
	glEnableVertexAttribArray(colAttrib);
	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

	//Texture coordinate attribute
	GLint texAttrib = glGetAttribLocation(program, "texcoord");
	glEnableVertexAttribArray(texAttrib);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

	//Normal attribute
	GLint normAttrib = glGetAttribLocation(program, "normal");
	glEnableVertexAttribArray(normAttrib);
	glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void *)(8 * sizeof(GLfloat)));

	//Texture
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	int width, height;
	unsigned char* image = SOIL_load_image("img.jpg", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	
	//Texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	// Texture X
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);	// Texture Y
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);		// Scaled down
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);		// Scale up

	//Model matrix
	GLint uniModel = glGetUniformLocation(program, "model");

	//View matrix
	glm::mat4 view = glm::lookAt(
		glm::vec3(2.5f, 2.5f, 2.5f),		//Position of camera
		glm::vec3(0.0f, 0.0f, 0.0f),		//Point centered on screen
		glm::vec3(0.0f, 0.0f, 1.0f)			//Up axis (x,y is the ground)
	);
	GLint uniView = glGetUniformLocation(program, "view");
	glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

	//Projection matrix: vertical FOV, aspect ratio, near plane, far plane (for clipping)
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 1.0f, 10.0f);
	GLint uniProj = glGetUniformLocation(program, "proj");
	glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

	//Uniform to decrease brightness of reflection
	GLint uniColor = glGetUniformLocation(program, "overrideColor");

	//Main event loop
	float direction;
	float xMovement = 0.0f;
	float yMovement = 0.8f;		//Start at appropriate zoom
	float zMovement = 0.0f;
	while (true) {
		
		//Movement events
		if (SDL_PollEvent(&windowEvent)) {
			if (windowEvent.type == SDL_QUIT) break;
			
			//Keyboard presses
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_LEFT) {
				xMovement = fmodf(xMovement - 0.1f, -2.0f);
			}
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_RIGHT){
				xMovement = fmodf(xMovement + 0.1f, 2.0f);
			}

			//Mouse - trolley
			if (windowEvent.type == SDL_MOUSEMOTION && windowEvent.button.button == SDL_BUTTON(SDL_BUTTON_MIDDLE)){
				direction = windowEvent.motion.yrel;
				if (direction > 0) {
					yMovement = std::max(yMovement + 0.05f, 0.0f);
				}
				else if (direction < 0) {
					yMovement = std::max(yMovement - 0.05f, 0.0f);
				}
			}

			//Mouse - rotation
			if (windowEvent.type == SDL_MOUSEMOTION && windowEvent.button.button == SDL_BUTTON(SDL_BUTTON_LEFT)){
				direction = windowEvent.motion.xrel;
				if (direction > 0) {
					xMovement = fmodf(xMovement + 0.01f, 2.0f);
				}
				else if (direction < 0){
					xMovement = fmodf(xMovement - 0.01f, -2.0f);
				}
			}

			//Mouse - viewing angle
			if (windowEvent.type == SDL_MOUSEMOTION && windowEvent.button.button == SDL_BUTTON(SDL_BUTTON_RIGHT)){
				direction = windowEvent.motion.yrel;
				if (direction > 0) {
					zMovement = fmodf(zMovement - 0.01f, -2.0f);
				}
				else if (direction < 0) {
					zMovement = fmodf(zMovement + 0.01f, 2.0f);
				}
			}
		}

		//Clear screen to black
		glClearColor(0.1f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Rotation Transformation
		glm::mat4 model;
		model = glm::rotate(model, xMovement * glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::rotate(model, zMovement * glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

		//Trolly Transformation
		glm::mat4 proj = glm::perspective(yMovement * glm::radians(45.0f), 800.0f / 600.0f, 1.0f, 10.0f);
		GLint uniProj = glGetUniformLocation(program, "proj");
		glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

		//Draw regular cube
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//Drawing mirror (w/ stencil buffer)
		glEnable(GL_STENCIL_TEST);
			
			glStencilFunc(GL_ALWAYS, 1, 0xFF);
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
			glStencilMask(0xFF);
			//glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			glClear(GL_STENCIL_BUFFER_BIT);
			glDepthMask(GL_FALSE);
			glDrawArrays(GL_TRIANGLES, 36, 6);
			

			//Draw inverted cube
			glStencilFunc(GL_EQUAL, 1, 0xFF);
			glStencilMask(0x00);
			glDepthMask(GL_TRUE);

			model = glm::scale(
				glm::translate(model, glm::vec3(0, 0, -1)),
				glm::vec3(1, 1, -1)
				);
			glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
			glUniform3f(uniColor, 0.1f, 0.1f, 0.1f);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glUniform3f(uniColor, 1.0f, 1.0f, 1.0f);

		glDisable(GL_STENCIL_TEST);

		//Floor grid
		glDepthMask(GL_FALSE);
		glDrawArrays(GL_LINES, 42, 20);
		glDepthMask(GL_TRUE);

		//Swap buffers
		SDL_GL_SwapWindow(window);
	}


	SDL_Delay(100);

	//Unbind from graphics pipeline
	glDeleteTextures(1, &tex);
	glDeleteProgram(program);
	glDeleteShader(fs);
	glDeleteShader(vs); 
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);

	SDL_GL_DeleteContext(context);
	SDL_Quit();
	return 0;
}