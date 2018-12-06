using namespace std;
#include <string.h>
#include <iostream>
#include "glad/glad.h"  //Include order can matter here
#if defined(__APPLE__) || defined(__linux__)
 #include <SDL2/SDL.h>
 #include <SDL2/SDL_opengl.h>
#else
 #include <SDL.h>
 #include <SDL_opengl.h>
#endif
#include <fstream>

using std::ifstream;


void printShitMatey(float ex[],int length) {
  for (int i=0;i<length;i+=3) {
    cout<<" vals at ("<<i<<"): "<<ex[i]<<endl;
  }
}

bool fullscreen = false;
int screen_width = 800;
int screen_height = 600;
void loadShader(GLuint shaderID, const GLchar* shaderSource){
  glShaderSource(shaderID, 1, &shaderSource, NULL);
  glCompileShader(shaderID);

  //Let's double check the shader compiled
  GLint status;
  glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status); //Check for errors
  if (!status){
    char buffer[512]; glGetShaderInfoLog(shaderID, 512, NULL, buffer);
    printf("Shader Compile Failed. Info:\n\n%s\n",buffer);
  }
}
#define GLM_FORCE_RADIANS //ensure we are using radians
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
const GLchar* vertexSource =
"#version 150 core\n"
"in vec3 position;"
"const vec3 inColor = vec3(0.f,0.7f,0.f);"
"in vec3 inNormal;"
"const vec3 inlightDir = normalize(vec3(1,0,0));"
"uniform mat4 model;"
"uniform mat4 view;"
"uniform mat4 proj;"

"out vec3 Color;"
"out vec3 normal;"
"out vec3 pos;"
"out vec3 eyePos;"
"out vec3 lightDir;"

"void main() {"
"   Color = inColor;"
"   vec4 pos4 = view * model * vec4(position,1.0);"
"   pos = pos4.xyz/pos4.w;"  //Homogeneous coordinate divide
"   vec4 norm4 = transpose(inverse(view*model)) * vec4(inNormal,0.0);"
"   normal = norm4.xyz;"
"   lightDir = (view * vec4(inlightDir,0)).xyz;"  //Transform light into to view space
"   gl_Position = proj * pos4;"
"}";


const GLchar* fragmentSource =
  "#version 150 core\n"
  "in vec3 Color;"
  "in vec3 normal;"
  "in vec3 pos;"
  "in vec3 eyePos;"
  "in vec3 lightDir;"
  "out vec4 outColor;"
  "const float ambient = .3;"
  "void main() {"
  "   vec3 N = normalize(normal);" //Re-normalized the interpolated normals
  "   vec3 diffuseC = Color*max(dot(lightDir,N),0.0);"
  "   vec3 ambC = Color*ambient;"
  "   vec3 reflectDir = reflect(-lightDir,N);"
  "   vec3 viewDir = normalize(-pos);"  //We know the eye is at 0,0
  "   float spec = max(dot(reflectDir,viewDir),0.0);"
  "   if (dot(lightDir,N) <= 0.0) spec = 0;"
  "   vec3 specC = vec3(.8,.8,.8)*pow(spec,4);"
  "   outColor = vec4(ambC+diffuseC+specC, 1.0);"
  "}";



int main(int argc, char *argv[]) {
	SDL_Init(SDL_INIT_VIDEO);  //Initialize Graphics (for OpenGL)

	//Print the version of SDL we are using
	SDL_version comp; SDL_version linked;
	SDL_VERSION(&comp); SDL_GetVersion(&linked);
	printf("\nCompiled against SDL version %d.%d.%d\n", comp.major, comp.minor, comp.patch);
	printf("Linked SDL version %d.%d.%d.\n", linked.major, linked.minor, linked.patch);

	//Ask SDL to get a recent version of OpenGL (3.2 or greater)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	//Create a window (offsetx, offsety, width, height, flags)
	SDL_Window* window = SDL_CreateWindow("My OpenGL Program", 100, 100,
	screen_width, screen_height, SDL_WINDOW_OPENGL);
	if (!window){printf("Could not create window: %s\n", SDL_GetError()); return 1;}
	float aspect = screen_width/(float)screen_height; //aspect ratio needs update on resize

	SDL_GLContext context = SDL_GL_CreateContext(window); //Bind OpenGL to the window

	if (gladLoadGLLoader(SDL_GL_GetProcAddress)){
		printf("OpenGL loaded\n");
		printf("Vendor:   %s\n", glGetString(GL_VENDOR));
		printf("Renderer: %s\n", glGetString(GL_RENDERER));
		printf("Version:  %s\n", glGetString(GL_VERSION));
	}
	else {
		printf("ERROR: Failed to initialize OpenGL context.\n");
		return -1;
	}


	string line;
	string v;//, valuesX[4475], valuesY[4475], valuesZ[4475];
	string uv;//, uCoord[4475], vCoord[4475];
	string uCoord, vCoord,wCoord;
	string valuesX, valuesY, valuesZ;
	float* modelData = new float[4475*8*2];
	float verts[4475*3];
	int vertCount = 0;
	float uvMap[4475*2];
	int uvCount = 0;
	float vertNorm[4475*3];
	int normCount = 0;
	int faceCount =0; 
	int filelength = 0;
	
	ifstream myfile ("models/low_poly_ship/ship_finished.obj");
	//string::size_type size;
	while(!myfile.eof()){
		myfile >> v >> valuesX>> valuesY>> valuesZ;
		filelength += 4;
		if (v[0] == 'v' && v[1] == 't'){
			if (uvCount < (4475*2)) {
				uvMap[uvCount] = strtof((valuesX).c_str(),0);
				uvMap[uvCount+1] = strtof((valuesY).c_str(),0);
				uvCount += 2;
			}
		} 
		else if (v[0] == 'v' && v[1] == 'n'){
			if (normCount < (4475*3)) {
				//myfile >> v >> valuesX>> valuesY>> valuesZ;
				vertNorm[normCount] = strtof((valuesX).c_str(),0);
				vertNorm[normCount+1] = strtof((valuesY).c_str(),0);
				vertNorm[normCount+2] = strtof((valuesZ).c_str(),0);
				normCount += 3;
			}
		} 
		else if (v[0] == 'v'){
			if (vertCount < (4475*3)) {
				//myfile >> v >> valuesX>> valuesY>> valuesZ;
				//cout << valuesX[n] << "\t" << valuesY[n] << "\t" << valuesZ[n] << endl;
				verts[vertCount] = strtof((valuesX).c_str(),0);
				verts[vertCount+1] = strtof((valuesY).c_str(),0);
				verts[vertCount+2] = strtof((valuesZ).c_str(),0);
				vertCount += 3;
			}
		}
	}

	//start of changes I made
	char faces[filelength + 5];
	string fileinfo;
	int faceindex =0; 
	ifstream newfile ("models/low_poly_ship/ship_finished.obj");
	while(!newfile.eof()){
		newfile >> fileinfo;
		faces[faceindex++] = fileinfo[0];
	}
	int modelIndex =0 ;
	int incVerty = 0;
	int incNormy = 0;
	int incMap = 0;
	
	int numVerts = 0;
	 
	for (int i =0; i < filelength; i++){
		if (faces[i] == 'f'){
			if (faceCount < 1168) {
				if (faces[i+5] != 'f' && faces[i+4] != 'f'){
					continue; 
				}
				//if f is followed by 3 verticies add them as normal
				else if(faces[i+4] == 'f'){
					numVerts += 3;
					for (int j =0; j < 3; j++){
						modelData[modelIndex++] = verts[incVerty++];
						modelData[modelIndex++] = verts[incVerty++];
						modelData[modelIndex++] = verts[incVerty++];
						modelData[modelIndex++] = vertNorm[incNormy++];
						modelData[modelIndex++] = vertNorm[incNormy++];
						modelData[modelIndex++] = vertNorm[incNormy++];
						modelData[modelIndex++] = uvMap[incMap++];
						modelData[modelIndex++] = uvMap[incMap++];
					}
				}
				//if f is followed by 4 vertices (a, b, c, d), add them as (a, b, c) & (b, c, d)
				else if(faces[i+5] == 'f'){
					numVerts += 6;
					for (int j =0; j < 3; j++){
						modelData[modelIndex++] = verts[incVerty++];
						modelData[modelIndex++] = verts[incVerty++];
						modelData[modelIndex++] = verts[incVerty++];
						modelData[modelIndex++] = vertNorm[incNormy++];
						modelData[modelIndex++] = vertNorm[incNormy++];
						modelData[modelIndex++] = vertNorm[incNormy++];
						modelData[modelIndex++] = uvMap[incMap++];
						modelData[modelIndex++] = uvMap[incMap++];
					}
					incVerty -= 6; 
					incNormy -= 6;
					incMap -= 4; 
					for (int j =0; j < 3; j++){
						modelData[modelIndex++] = verts[incVerty++];
						modelData[modelIndex++] = verts[incVerty++];
						modelData[modelIndex++] = verts[incVerty++];
						modelData[modelIndex++] = vertNorm[incNormy++];
						modelData[modelIndex++] = vertNorm[incNormy++];
						modelData[modelIndex++] = vertNorm[incNormy++];
						modelData[modelIndex++] = uvMap[incMap++];
						modelData[modelIndex++] = uvMap[incMap++];
					}
				}
				vertCount += 1;
			}
			faceCount++; 
		}
	}
	
	int numLines = numVerts * 8 ;
	//end of changes I made 
	
	/*
	int incVerty = 0;
	int incNormy = 0;
	int incMap = 0;
	for (int i = 0;i < 4475*8;i+=8) {
		modelData[i] = verts[incVerty++];
		modelData[i+1] = verts[incVerty++];
		modelData[i+2] = verts[incVerty++];
		modelData[i+3] = vertNorm[incNormy++];
		modelData[i+4] = vertNorm[incNormy++];
		modelData[i+5] = vertNorm[incNormy++];
		modelData[i+6] = uvMap[incMap++];
		modelData[i+7] = uvMap[incMap++];
	}
	*/
	
	//printShitMatey(verts, 4475*3);
	//printShitMatey(uvMap, 4475*2);
	//printShitMatey(vertNorm, 4475*3);

	

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	loadShader(vertexShader, vertexSource);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	loadShader(fragmentShader, fragmentSource);

	//Join the vertex and fragment shaders together into one program
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "outColor"); // set output
	glLinkProgram(shaderProgram); //run the linker


	GLuint vao;
	glGenVertexArrays(1, &vao); //Create a VAO
	glBindVertexArray(vao); //Bind the above created VAO to the current context
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, numLines*sizeof(float), modelData, GL_STATIC_DRAW);

	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), 0);
	//Attribute, vals/attrib., type, isNormalized, stride, offset
	glEnableVertexAttribArray(posAttrib);


	GLint normAttrib = glGetAttribLocation(shaderProgram, "inNormal");
	glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(normAttrib);


	glEnable(GL_DEPTH_TEST);

	SDL_Event windowEvent;
	bool quit = false;
	while (!quit){
		while (SDL_PollEvent(&windowEvent)){
			if (windowEvent.type == SDL_QUIT) quit = true; //Exit Game Loop
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE)
				quit = true; //Exit Game Loop
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_f){
				fullscreen = !fullscreen;
				SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
			}
		}
		// Clear the screen to default color
		glClearColor(.2f, 0.4f, 0.8f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float time = SDL_GetTicks()/1000.f;
		glm::mat4 model;
		//model = glm::rotate(model,time * 3.14f/2,glm::vec3(0.0f, 1.0f, 1.0f));
		//model = glm::rotate(model,time * 3.14f/4,glm::vec3(1.0f, 0.0f, 0.0f));
		GLint uniModel = glGetUniformLocation(shaderProgram, "model");
		glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

		glm::mat4 view = glm::lookAt(
		glm::vec3(6.0f, 0.0f, 4.0f),  //Cam Position
		glm::vec3(0.0f, 0.0f, 1.0f),  //Look at point
		glm::vec3(0.0f, 0.0f, 1.0f)); //Up
		GLint uniView = glGetUniformLocation(shaderProgram, "view");
		glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
		glm::mat4 proj = glm::perspective(3.14f/4, aspect, 0.01f, 100.0f);
		//FOV, aspect ratio, near, far
		GLint uniProj = glGetUniformLocation(shaderProgram, "proj");
		glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));



		glUseProgram(shaderProgram);
		glBindVertexArray(vao);  //Bind the VAO for the shaders we are using
		glDrawArrays(GL_TRIANGLES, 0, numVerts); //Number of vertices

		SDL_GL_SwapWindow(window); //Double buffering
	}
	glDeleteProgram(shaderProgram);
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	SDL_GL_DeleteContext(context);
	SDL_Quit();

	return 0;
}
