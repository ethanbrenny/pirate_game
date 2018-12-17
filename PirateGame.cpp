using namespace std;
#include <string.h>
#include <iostream>
#include "glad/glad.h"  //Include order can matter here

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <cmath>
#include <time.h>

#define GLM_FORCE_RADIANS //ensure we are using radians
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb-master/stb_image.h"

#if defined(__APPLE__) || defined(__linux__)
 #include <SDL2/SDL.h>
 #include <SDL2/SDL_opengl.h>
#else
 #include <SDL.h>
 #include <SDL_opengl.h>
#endif
#include <fstream>

using std::ifstream;
float shipRotation = 0;
float* modelData = new float[4475*8];
float* waterData = new float[100000]; // 11232 * 8 vertices
  GLfloat waterSquare[] = {
   // X      Y     Z     R     G      B      U      V
     -0.25f,  0.25f,  0.0f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // first tri
     -.25f, -0.25f,   0.0f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
     0.0f, 0.0f,      0.0f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
     -0.25f, 0.25f,   0.0f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
     0.25f, 0.25f,    0.0f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
     0.0f, 0.0f,      0.0f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f,

     0.25f,  0.25f,   0.0f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // second tri
     0.25f, -0.25f,   0.0f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
     0.0f, 0.0f,      0.0f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
     0.25f, -0.25f,   0.0f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
     -0.25f, -0.25f,  0.0f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
     0.0f, 0.0f,      0.0f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f,

   };

float Obelisks[] = {-10.0,-10.0,-15.0,10.0};

int numLines = 0;
int numVerts = 0;
GLuint texture;
GLuint fb =1;

int height, width;
string textureName = "ship.ppm";

bool fullscreen = false;
int screen_width = 800;
int screen_height = 600;

glm::vec3 camPos = glm::vec3(6.5f, 0.0f, 3.0f);  //Cam Position
glm::vec3 shipPos = glm::vec3(0.0f, 0.0f, 1.0f);  //Look at point
glm::vec3 shipDir = glm::vec3(0.0f,1.0f,0.0f);
glm::vec3 camUp = glm::vec3(0.0f, 0.0f, 1.0f); //Up

float timer1;
clock_t startTime;

int sizeOfWater =0;
float last[] = {0.0, 0.0, 0.0, 0.0, 0.0};
float now[] = {0.0, 0.0, 0.0, 0.0, 0.0};
float frontSlope, sideSlope;
float oldSideSlope =0;
float oldFrontSlope = 0;

void makeWater(){

	float xstart = -3.75;
	float ystart = -3.75;
	int indexofwater =0;
	int squareIndex = 0;

	//first layer
	for (int i = 0; i < 16; i++){
		for (int j = 0; j < 16; j ++){
			squareIndex =0;
			for (int l =0; l< 12; l ++){
				waterData[indexofwater++] = waterSquare[squareIndex++] + xstart + 0.5*i;
				waterData[indexofwater++] = waterSquare[squareIndex++] + ystart + 0.5*j;
				for (int k =0; k < 6; k++){
					waterData[indexofwater++] = waterSquare[squareIndex++];
				}
				sizeOfWater += 8;
			}
		}
	}

	//second layer around the center double the size of triangles.
	for (int h =0; h < 9; h++){
		if(h != 4){
			xstart = -11.5 + 8 * (h%3);
			ystart = -11.5 + 8 * (h/3);
			for(int i =0; i < 8; i++){
				for (int j =0; j < 8; j++){
					squareIndex = 0;
					for (int l =0; l< 12; l ++){
						waterData[indexofwater++] = waterSquare[squareIndex++]*2.01 + xstart + i;
						waterData[indexofwater++] = waterSquare[squareIndex++]*2.01 + ystart + j;
						for (int k =0; k < 6; k++){
							waterData[indexofwater++] = waterSquare[squareIndex++];
						}
						sizeOfWater += 8;
					}
				}
			}
		}
	}

	//outer circle
	float outerPos[60*4];
	float innerPos[60*4];
	float dx, dy, xs, ys, distFromCent;
	int inde = 0;
	for (int side =0; side < 4; side++){
		if (side == 0){
			dy = 0;
			dx = 23.226/30;
			xs = -12.0;
			ys = -12.0;
		}
		else if (side == 1){
			dy = 23.226/30;
			dx = 0;
			xs = 12.0;
			ys = -12.0;
		}
		else if (side == 2){
			dy = 0;
			dx = -23.226/30;
			xs = 12.0;
			ys = 12.0;
		}
		else if (side == 3){
			dy = -23.226/30;
			dx = 0;
			xs = -12.0;
			ys = 12.0;
		}
		for (int point =0; point < 30; point++){
			innerPos[inde] = xs+dx*point;
			innerPos[inde+1] = ys+dy*point;
			distFromCent = sqrt(innerPos[inde]*innerPos[inde] + innerPos[inde+1]*innerPos[inde+1]);
			outerPos[inde] = innerPos[inde] * 30 / distFromCent;
			outerPos[inde + 1] = innerPos[inde+1] * 30 / distFromCent;
			inde+=2;
		}
	}

	for (int i =0; i < 120; i++){
		waterData[indexofwater++] = innerPos[i*2];
		waterData[indexofwater++] = innerPos[i*2 +1];
		for (int k =0; k < 6; k++){
      if (k == 3) {
        waterData[indexofwater++] = 1.0;
      }
      else{
        waterData[indexofwater++] = 0.0;
      }
		}
		waterData[indexofwater++] = outerPos[i*2];
		waterData[indexofwater++] = outerPos[i*2 +1];
		for (int k =0; k < 6; k++){
      if (k == 3) {
        waterData[indexofwater++] = 1.0;
      }
      else{
        waterData[indexofwater++] = 0.0;
      }
		}
		waterData[indexofwater++] = innerPos[(i*2+2)%240];
		waterData[indexofwater++] = innerPos[(i*2+3)%240];
		for (int k =0; k < 6; k++){
      if (k == 3) {
        waterData[indexofwater++] = 1.0;
      }
      else{
        waterData[indexofwater++] = 0.0;
      }
		}

		waterData[indexofwater++] = outerPos[i*2];
		waterData[indexofwater++] = outerPos[i*2 +1];
		for (int k =0; k < 6; k++){
      if (k == 3) {
        waterData[indexofwater++] = 1.0;
      }
      else{
        waterData[indexofwater++] = 0.0;
      }
		}
		waterData[indexofwater++] = outerPos[(i*2+2)%240];
		waterData[indexofwater++] = outerPos[(i*2+3)%240];
		for (int k =0; k < 6; k++){
      if (k == 3) {
        waterData[indexofwater++] = 1.0;
      }
      else{
        waterData[indexofwater++] = 0.0;
      }
		}
		waterData[indexofwater++] = innerPos[(i*2+2)%240];
		waterData[indexofwater++] = innerPos[(i*2+3)%240];
		for (int k =0; k < 6; k++){
      if (k == 3) {
        waterData[indexofwater++] = 1.0;
      }
      else{
        waterData[indexofwater++] = 0.0;
      }
		}
		sizeOfWater += 8*6;
	}

	return;
}

void translateShip(int size, float xtrans, float ytrans, float ztrans){
	float bob = 0.06;
	float goof = 0.01;
	now[0] = sin(shipPos.x + xtrans + shipPos.y + ytrans + timer1*2.1) * bob;
	now[1] = sin(shipPos.x + xtrans + 0.5 + shipPos.y + ytrans + 0.5 + timer1*2.1) * goof;
	now[2] = sin(shipPos.x + xtrans - 0.5 + shipPos.y + ytrans - 0.5 + timer1*2.1) * goof;
	now[3] = sin(shipPos.x + xtrans + 0.5 + shipPos.y + ytrans + 0.5 + timer1*2.1) * goof;
	now[4] = sin(shipPos.x + xtrans - 0.5 + shipPos.y + ytrans - 0.5 + timer1*2.1) * goof;

	float frontSlope = now[1] - now[2];
	float sideSlope = now[3] - now[4];

	float xfromcent , oldxfromcent;
	float yfromcent , oldyfromcent;

	for(int x =0; x < size; x += 8){
		oldxfromcent = modelData[x] - (shipPos.x);
		oldyfromcent = modelData[x+1] - (shipPos.y);

		modelData[x] += xtrans;
		modelData[x+1] += ytrans;

		xfromcent = modelData[x] - (shipPos.x + xtrans);
		yfromcent = modelData[x+1] - (shipPos.y + ytrans);

		modelData[x+2] -= last[0] + oldFrontSlope*oldxfromcent + oldSideSlope*oldyfromcent;
		modelData[x+2] += now[0] + frontSlope*xfromcent + sideSlope*yfromcent;

	}
	for (int i =0; i < 5 ; i++){
		last[i] = now[i];
	}
	oldFrontSlope = frontSlope;
	oldSideSlope = sideSlope;


    shipPos[0] += xtrans;
    shipPos[1] += ytrans;
    shipPos[2] += ztrans;

    camPos[0] += xtrans;
    camPos[1] += ytrans;
    camPos[2] += ztrans;

	return;
}

void makeWave(float xpos, float ypos, int index){
	float zpos =0;
  float tot = 0;
	float waveHeight = 0.1;
	float waterHeight = 0.1;
	float waveSpeed = 2.1;

	float xdist = xpos - shipPos.x;
	float ydist = ypos - shipPos.y;
	float distance = (xdist * xdist + ydist * ydist);

	if(distance > 0){
		waveHeight = 0.1*(0.5*cos(3.1415*distance/900) + 0.5);
	}

	zpos = sin(xpos + ypos + timer1*waveSpeed) * waveHeight + waterHeight;
	//zpos += sin((xpos - ypos + timer1)*5) * 0.02;
	//z pos
	waterData[index+2] = zpos;

	float tangent = cos(xpos + ypos + timer1*waveSpeed)*waveHeight;

	if(tangent < 0.01 && tangent >= 0.0){
		tangent = 0.01;
	}
	else if (tangent > -0.01 && tangent <= 0.0){
		tangent = -0.01;
	}

	tangent = 1/tangent;
  //waterData[5] = 1;
  //tot = sqrt((tangent*tangent *2) + waterData[5]);
	//x normal
	//waterData[index+3] = tangent/tot;

	//y normal
	//waterData[index+4] = tangent/tot;
  //waterData[index+5] = 1/tot;
}

void translateWater(int size, float xtrans, float ytrans, float ztrans){
	timer1 = (clock() - startTime) / float(CLOCKS_PER_SEC);
	for(int x =0; x < size; x += 8){
		//positions
		waterData[x] += xtrans;
		waterData[x+1] += ytrans;

		//hiehgt
		makeWave(waterData[x], waterData[x+1], x);

		//uv
		waterData[x + 6] = waterData[x]*0.5 + waterData[x+2]*0.7;
		waterData[x + 7] = waterData[x+1]*0.5 + waterData[x+2]*0.7;
	}
}

void rotateShip(int size, float xcen, float ycen, float zcen, float rotAngle, float xzrot, float yzrot){
	translateShip(size, -xcen, -ycen, -zcen);

	float xp , yp, zp, xnp, ynp, znp;
	for(int x =0; x < size; x += 8){
		xp = modelData[x];
		yp = modelData[x+1];
		zp = modelData[x+2];

		xnp = modelData[x] + modelData[x+3];
		ynp = modelData[x+1] + modelData[x+4];
		znp = modelData[x+2] + modelData[x+5];

		//flat rotate
		modelData[x] = xp * cos(rotAngle) - yp*sin(rotAngle);
		modelData[x+1] = yp * cos(rotAngle) + xp*sin(rotAngle);
		modelData[x+3] = (xnp * cos(rotAngle) - ynp*sin(rotAngle)) - modelData[x];
		modelData[x+4] = (ynp * cos(rotAngle) + xnp*sin(rotAngle)) - modelData[x+1];

		/*
		xp = modelData[x];
		yp = modelData[x+1];
		zp = modelData[x+2];

		xnp = modelData[x] + modelData[x+3];
		ynp = modelData[x+1] + modelData[x+4];
		znp = modelData[x+2] + modelData[x+5];

		//xz rotate
		modelData[x] = xp * cos(xzrot) - zp*sin(xzrot);
		modelData[x+2] = zp * cos(xzrot) + xp*sin(xzrot);
		modelData[x+3] = (xnp * cos(xzrot) - znp*sin(xzrot)) - modelData[x];
		modelData[x+5] = (znp * cos(xzrot) + xnp*sin(xzrot)) - modelData[x+2];

		xp = modelData[x];
		yp = modelData[x+1];
		zp = modelData[x+2];

		xnp = modelData[x] + modelData[x+3];
		ynp = modelData[x+1] + modelData[x+4];
		znp = modelData[x+2] + modelData[x+5];

		//yz rotate
		modelData[x+1] = yp * cos(yzrot) - zp*sin(yzrot);
		modelData[x+2] = zp * cos(yzrot) + yp*sin(yzrot);
		modelData[x+4] = (ynp * cos(xzrot) - znp*sin(yzrot)) - modelData[x+1];
		modelData[x+5] = (znp * cos(xzrot) + ynp*sin(yzrot)) - modelData[x+2];
		*/
	}

	translateShip(size, xcen, ycen, zcen);
}

unsigned char* loadImage(string tName, int& img_w, int& img_h){

   //Open the texture image file
   ifstream ppmFile;
   ppmFile.open(tName.c_str());
   if (!ppmFile){
      printf("ERROR: Texture file '%s' not found.\n",tName.c_str());
      exit(1);
   }

   //Check that this is an ASCII PPM (first line is P3)
   string PPM_style;
   ppmFile >> PPM_style; //Read the first line of the header
   if (PPM_style != "P3") {
      printf("ERROR: PPM Type number is %s. Not an ASCII (P3) PPM file!\n",PPM_style.c_str());
      exit(1);
   }

   //Read in the texture width and height
   ppmFile >> img_w >> img_h;
   unsigned char* img_data = new unsigned char[4*img_w*img_h];

   //Check that the 3rd line is 255 (ie., this is an 8 bit/pixel PPM)
   int maximum;
   ppmFile >> maximum;
   if (maximum != 255) {
      printf("ERROR: Maximum size is (%d) not 255.\n",maximum);
      exit(1);
   }

   //TODO:
   //while(ppmFile>> ...){
   //    //Store the RGB pixel data from the file into an array
   //}
   int red = 0, blue = 0, green = 0,alpha =0;
   int i = 0;
   while (ppmFile>>red>>green>>blue>>alpha) {
     img_data[i++] = red;  //Red
     img_data[i++] = green;  //Green
     img_data[i++] = blue;  //Blue
     img_data[i++] = alpha;  //Alpha
   }
   return img_data;
}

float* makeObject(string fileName, int amtVerts, int totShapes) {
  numVerts = 0;
  numLines = 0;
  string line;
	string v;//, valuesX[4475], valuesY[4475], valuesZ[4475];
	string uv;//, uCoord[4475], vCoord[4475];
	string uCoord, vCoord,wCoord;
	string valuesX, valuesY, valuesZ;
	float verts[amtVerts*3];
	float uvMap[amtVerts*2];
    int vertCount = 0;
    int lineCount = 0;
	int uvCount = 0;
	float vertNorm[amtVerts*3];
	int normCount = 0;
	int faceCount =0;
	int filelength = 0;
	float* newData = new float[amtVerts*8*2];

	ifstream myfile (fileName.c_str());
	//string::size_type size;
	while(!myfile.eof()){
		myfile >> v >> valuesX>> valuesY>> valuesZ;
		filelength += 4;
		if (v[0] == 'v' && v[1] == 't'){
			if (uvCount < (amtVerts*2)) {
				uvMap[uvCount] = strtof((valuesX).c_str(),0);
				uvMap[uvCount+1] = strtof((valuesY).c_str(),0);
				uvCount += 2;
			}
		}
		else if (v[0] == 'v' && v[1] == 'n'){
			if (normCount < (amtVerts*3)) {
				//myfile >> v >> valuesX>> valuesY>> valuesZ;
				vertNorm[normCount] = strtof((valuesX).c_str(),0);
				vertNorm[normCount+1] = strtof((valuesY).c_str(),0);
				vertNorm[normCount+2] = strtof((valuesZ).c_str(),0);
				normCount += 3;
			}
		}
		else if (v[0] == 'v'){
			if (vertCount < (amtVerts*3)) {
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
	ifstream newfile (fileName.c_str());
	while(!newfile.eof()){
		newfile >> fileinfo;
		faces[faceindex++] = fileinfo[0];
	}
	int modelIndex =0 ;
	int incVerty = 0;
	int incNormy = 0;
	int incMap = 0;


	for (int i =0; i < filelength; i++){
		if (faces[i] == 'f'){
			if (faceCount < 1168) {
				if (faces[i+5] != 'f' && faces[i+4] != 'f' && faces[i+4] != 's' && faces[i+5] != 's'){
					continue;
				}
				//if f is followed by 3 verticies add them as normal
				else if(faces[i+4] == 'f' || faces[i+4] == 's'){
					numVerts += 3;
					for (int j =0; j < 3; j++){
						newData[modelIndex++] = verts[incVerty++];
						newData[modelIndex++] = verts[incVerty++];
						newData[modelIndex++] = verts[incVerty++];
						newData[modelIndex++] = vertNorm[incNormy++];
						newData[modelIndex++] = vertNorm[incNormy++];
						newData[modelIndex++] = vertNorm[incNormy++];
						newData[modelIndex++] = uvMap[incMap++];
						newData[modelIndex++] = 1-uvMap[incMap++];
					}
				}
				//if f is followed by 4 vertices (a, b, c, d), add them as (a, b, c) & (b, c, d)
				else if(faces[i+5] == 'f' || faces[i+5] == 's'){
					numVerts += 6;
					for (int j =0; j < 3; j++){ //
						newData[modelIndex++] = verts[incVerty++];
						newData[modelIndex++] = verts[incVerty++];
						newData[modelIndex++] = verts[incVerty++];
						newData[modelIndex++] = vertNorm[incNormy++];
						newData[modelIndex++] = vertNorm[incNormy++];
						newData[modelIndex++] = vertNorm[incNormy++];
						newData[modelIndex++] = uvMap[incMap++];
						newData[modelIndex++] = 1-uvMap[incMap++];
					}
					incVerty -= 3;
					incNormy -= 3;
					incMap -= 2;
					for (int j =0; j < 3; j++){
            if (j == 2) {
              incVerty -= 12;
    					incNormy -= 12;
              incMap -= 8;
            }
						newData[modelIndex++] = verts[incVerty++];
						newData[modelIndex++] = verts[incVerty++];
						newData[modelIndex++] = verts[incVerty++];
						newData[modelIndex++] = vertNorm[incNormy++];
						newData[modelIndex++] = vertNorm[incNormy++];
						newData[modelIndex++] = vertNorm[incNormy++];
						newData[modelIndex++] = uvMap[incMap++];
						newData[modelIndex++] = 1-uvMap[incMap++];
					}
          incVerty += 9;
          incNormy += 9;
          incMap += 6;
				}
			}
			faceCount++;
		}
	}
  numLines = numVerts * 8;
  return newData;
}

void makeShip() {
  numVerts = 0;
  numLines = 0;
  string line;
	string v;//, valuesX[4475], valuesY[4475], valuesZ[4475];
	string uv;//, uCoord[4475], vCoord[4475];
	string uCoord, vCoord,wCoord;
	string valuesX, valuesY, valuesZ;
	float verts[4475*3];
	float uvMap[4475*2];
    int vertCount = 0;
    int lineCount = 0;
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


	for (int i =0; i < filelength; i++){
		if (faces[i] == 'f'){
			if (faceCount < 1168) {
				if (faces[i+5] != 'f' && faces[i+4] != 'f' && faces[i+4] != 's' && faces[i+5] != 's'){
					continue;
				}
				//if f is followed by 3 verticies add them as normal
				else if(faces[i+4] == 'f' || faces[i+4] == 's'){
					numVerts += 3;
					for (int j =0; j < 3; j++){
						modelData[modelIndex++] = verts[incVerty++];
						modelData[modelIndex++] = verts[incVerty++];
						modelData[modelIndex++] = verts[incVerty++];
						modelData[modelIndex++] = vertNorm[incNormy++];
						modelData[modelIndex++] = vertNorm[incNormy++];
						modelData[modelIndex++] = vertNorm[incNormy++];
						modelData[modelIndex++] = uvMap[incMap++];
						modelData[modelIndex++] = 1-uvMap[incMap++];
					}
				}
				//if f is followed by 4 vertices (a, b, c, d), add them as (a, b, c) & (b, c, d)
				else if(faces[i+5] == 'f' || faces[i+5] == 's'){
					numVerts += 6;
					for (int j =0; j < 3; j++){ //
						modelData[modelIndex++] = verts[incVerty++];
						modelData[modelIndex++] = verts[incVerty++];
						modelData[modelIndex++] = verts[incVerty++];
						modelData[modelIndex++] = vertNorm[incNormy++];
						modelData[modelIndex++] = vertNorm[incNormy++];
						modelData[modelIndex++] = vertNorm[incNormy++];
						modelData[modelIndex++] = uvMap[incMap++];
						modelData[modelIndex++] = 1-uvMap[incMap++];
					}
					incVerty -= 3;
					incNormy -= 3;
					incMap -= 2;
					for (int j =0; j < 3; j++){
            if (j == 2) {
              incVerty -= 12;
    					incNormy -= 12;
              incMap -= 8;
            }
						modelData[modelIndex++] = verts[incVerty++];
						modelData[modelIndex++] = verts[incVerty++];
						modelData[modelIndex++] = verts[incVerty++];
						modelData[modelIndex++] = vertNorm[incNormy++];
						modelData[modelIndex++] = vertNorm[incNormy++];
						modelData[modelIndex++] = vertNorm[incNormy++];
						modelData[modelIndex++] = uvMap[incMap++];
						modelData[modelIndex++] = 1-uvMap[incMap++];
					}
          incVerty += 9;
          incNormy += 9;
          incMap += 6;
				}
			}
			faceCount++;
		}
	}
  numLines = numVerts * 8 ;
}

void drawObject(float* data,GLuint shaderP, GLuint vao, GLuint vbo[],int numVer,int numLin) {
  // numLines and numVerts
  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); //Set the first vbo as the active  buffer
  glBufferData(GL_ARRAY_BUFFER, numLin*sizeof(float), data, GL_DYNAMIC_DRAW);
  GLint posAttrib = glGetAttribLocation(shaderP, "position");
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), 0);

	glEnableVertexAttribArray(posAttrib);

	GLint normAttrib = glGetAttribLocation(shaderP, "inNormal");
	glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(normAttrib);

	GLint texAttrib = glGetAttribLocation(shaderP, "inTexcoord");
  glEnableVertexAttribArray(texAttrib);
  glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));

  GLint colAttrib = glGetAttribLocation(shaderP, "inColor");
  glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), 0);
  glEnableVertexAttribArray(colAttrib);
  glUseProgram(shaderP); //Set the active shader program
  glBindVertexArray(vao);  //Bind the VAO for the shaders we are using
  glDrawArrays(GL_TRIANGLES, 0, numVer); //Number of vertices
  /*
  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); //Set the first vbo as the active  buffer
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  GLint posAttrib = glGetAttribLocation(shaderP, "position");
  glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), 0);
                  //Attribute, vals/attrib., type, isNormalized, stride, offset
  glEnableVertexAttribArray(posAttrib);

  GLint colAttrib = glGetAttribLocation(shaderP, "inColor");
  glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE,
                        8*sizeof(float), (void*)(3*sizeof(float)));
  glEnableVertexAttribArray(colAttrib);
  glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
  //glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW); //upload normals to vbo
  GLint normAttrib = glGetAttribLocation(shaderP, "inNormal");
  glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(normAttrib);

  glUseProgram(shaderP); //Set the active shader program
  glBindVertexArray(vao);  //Bind the VAO for the shaders we are using
  glDrawArrays(GL_TRIANGLES, 0, 36); //Number of vertices
  */
}

void attachTexture(unsigned char* img,int a, int b) {
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, a, b, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
 	glGenerateMipmap(GL_TEXTURE_2D);
}

float* drawObelisk(float xLoc, float yLoc, GLuint shaderP, GLuint vao, GLuint vbo[]) {
  float leftX = xLoc - 2.5;
  float rightX = xLoc + 2.5;
  float forwardY = yLoc + 2.5;
  float backwardY = yLoc - 2.5;
  float upZ = 20;
  float curZ = -1;
  float Obelisk[] = {
   // X      Y     Z     R     G      B      U      V
   xLoc, yLoc,      upZ, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // forw background
   leftX, forwardY, curZ,0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
   rightX,forwardY, curZ, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
   rightX,backwardY,curZ,0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
   leftX, backwardY,curZ,0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
   xLoc,yLoc,  upZ, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,

   xLoc, yLoc,       upZ, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // forw background
   leftX, forwardY,  curZ,0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
   leftX, backwardY, curZ, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
   rightX,backwardY, curZ,0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
   rightX, forwardY, curZ,0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
   xLoc,yLoc,  upZ,  0.0f, -1.0f, 0.0f, 0.0f, 1.0f
 };
 drawObject(Obelisk,shaderP, vao, vbo,96,96);
}

void drawObelisks(GLuint shaderP, GLuint vao, GLuint vbo[]) {
  if (abs(Obelisks[0] - shipPos.x) + abs(Obelisks[1] - shipPos.y) > 40) {
    Obelisks[0] = shipPos.x + (shipDir.x*20);
    Obelisks[1] = shipPos.y + (shipDir.y*20);
    cout<<" Obelisk boy the tormenter "<<Obelisks[0]<<", "<<Obelisks[1]<<endl;
  } else if (abs(Obelisks[2] - shipPos.x) + abs(Obelisks[3] - shipPos.y) > 40) {
    Obelisks[2] = shipPos.x + (shipDir.x*20);
    Obelisks[3] = shipPos.y + (shipDir.y*20);
  }
  drawObelisk(Obelisks[0],Obelisks[1],shaderP,vao,vbo);
  drawObelisk(Obelisks[2],Obelisks[3],shaderP,vao,vbo);
  // shipPos.x shipPos.y shipPos.z

}


void drawBackground(GLuint shaderP, GLuint vao, GLuint vbo[]) {
  float distance_away = 28.2;
  float curX = shipPos.x;
  float curY = shipPos.y;
  float curZ = shipPos.z;
  float rightX = curX + distance_away;
  float leftX = curX - distance_away;
  float forwardY = curY + distance_away;
  float backwardY = curY - distance_away;
  float upZ = curZ + 40;
  float midZ = curZ + 20;
  curZ -= 15;
  float background[] = {
   // X      Y     Z     R     G      B      U      V
   leftX, forwardY,  upZ, 0.0f, -1.0f, 0.0f, 0.25f, 1.0f, // forw background
   leftX, forwardY,  curZ,0.0f, -1.0f, 0.0f, 0.25f, 0.0f,
   rightX,forwardY,  upZ, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
   rightX,forwardY,  curZ,0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
   leftX, forwardY,  curZ,0.0f, -1.0f, 0.0f, 0.25f, 0.0f,
   rightX,forwardY,  upZ, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,

   leftX, backwardY,  upZ, 0.0f, -1.0f, 0.0f, 0.5f, 1.0f, //  left background
   leftX, backwardY,  curZ,0.0f, -1.0f, 0.0f, 0.5f, 0.0f,
   leftX,  forwardY,  upZ, 0.0f, -1.0f, 0.0f, 0.25f, 1.0f,
   leftX,  forwardY,  curZ,0.0f, -1.0f, 0.0f, 0.25f, 0.0f,
   leftX, backwardY,  curZ,0.0f, -1.0f, 0.0f, 0.5f, 0.0f,
   leftX,  forwardY,  upZ, 0.0f, -1.0f, 0.0f, 0.25f, 1.0f,

   leftX, backwardY,  upZ, 0.0f, -1.0f, 0.0f, 0.5f, 1.0f, //  back background
   leftX, backwardY,  curZ,0.0f, -1.0f, 0.0f, 0.5f, 0.0f,
   rightX,backwardY,  upZ, 0.0f, -1.0f, 0.0f, 0.75f, 1.0f,
   rightX,backwardY,  curZ,0.0f, -1.0f, 0.0f, 0.75f, 0.0f,
   leftX, backwardY,  curZ,0.0f, -1.0f, 0.0f, 0.5f, 0.0f,
   rightX,backwardY,  upZ, 0.0f, -1.0f, 0.0f, 0.75f, 1.0f,

   rightX,  forwardY,  curZ,0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
   rightX, backwardY,  curZ,0.0f, -1.0f, 0.0f, 0.75f, 0.0f,
   rightX,  forwardY,  upZ, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
   rightX, backwardY,  upZ, 0.0f, -1.0f, 0.0f, 0.75f, 1.0f, //  right background
   rightX, backwardY,  curZ,0.0f, -1.0f, 0.0f, 0.75f, 0.0f,
   rightX,  forwardY,  upZ, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f
   };
  drawObject(background,shaderP, vao, vbo,192,192);
}

void printShitMatey(float ex[],int length) {
  for (int i=0;i<length;i+=3) {
    cout<<" vals at ("<<i<<"): "<<ex[i]<<endl;
  }
}

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

/*void rotateShipVertically(float amt) // amt + or - determines direction of shift
{
  shipRotation += amt;
  //float vecX = 0;
  //float vecY = 0;

  glm::normalize(shipDir);
  glm::vec3 vecToPoint = glm::vec3(0.0f,0.0f,0.0f);
  float direct = 0.0;
  for (int i = 0;i < 89856; i+=8) {
    //shipRotation += changeIt;
    vecToPoint.x = waterData[i] - shipPos.x;
    vecToPoint.y = waterData[i+1] - shipPos.y;
    glm::normalize(vecToPoint);
    direct = glm::dot(vecToPoint,shipDir);
    if (direct > 0) {
      //waterData[i+2] += ;
    } else {
      //waterData[i+2] = 0;
    }
  }
}*/

#define GLM_FORCE_RADIANS //ensure we are using radians
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

const GLchar* vertexSource =
"#version 150 core\n"
"in vec3 position;"
"in vec3 inNormal;"
"in vec3 inColor;"
"in vec2 inTexcoord;"
"const vec3 inlightDir = normalize(vec3(0,0,1));"
"uniform mat4 model;"
"uniform mat4 view;"
"uniform mat4 proj;"

"out vec3 Color;"
"out vec3 normal;"
"out vec3 pos;"
"out vec3 eyePos;"
"out vec3 lightDir;"
"out vec2 texcoord;"

"void main() {"
"   Color = inColor;"
"   vec4 pos4 = view * model * vec4(position,1.0);"
"   pos = pos4.xyz/pos4.w;"  //Homogeneous coordinate divide
"   vec4 norm4 = transpose(inverse(view*model)) * vec4(inNormal,0.0);"
"   normal = norm4.xyz;"
"   lightDir = (view * vec4(inlightDir,0)).xyz;"  //Transform light into to view space
"   gl_Position = proj * pos4;"
"   texcoord = inTexcoord;"
"}";


const GLchar* fragmentSource =
  "#version 150 core\n"
  "uniform sampler2D tex0;"
  "in vec3 normal;"
  "in vec3 pos;"
  "in vec3 eyePos;"
  "in vec3 lightDir;"
  "in vec2 texcoord;"
  "out vec3 outColor;"
  "const float ambient = .3;"
  "void main() {"
  "   vec3 Color = vec3(1,1,1);"
  "   vec3 N = normalize(normal);" //Re-normalized the interpolated normals
  "   vec3 diffuseC = .5*Color*max(dot(lightDir,N),0.0);"
  "   vec3 ambC = Color*ambient;"
  "   vec3 reflectDir = reflect(-lightDir,N);"
  "   vec3 viewDir = normalize(eyePos-pos);"  //We know the eye is at 0,0
  "   float spec = max(dot(reflectDir,viewDir),0.0);"
  "   if (dot(lightDir,N) <= 0.0) spec = 0;"
  "   vec3 specC = vec3(.8,.8,.8)*pow(spec,4);"
  "   outColor = texture(tex0, texcoord).rgb * (ambC+0.4*diffuseC);"
  "}";

//"   outColor = vec4(ambC+diffuseC+specC, 1.0);"

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

	//Allocate Texture 0
	cout << "textures loaded\n";
	GLuint tex0;
	glGenTextures(1, &tex0);
	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//load texture image
	int wi, hi, nrChannels;
	//unsigned char* imgData = stbi_load("./models/low_poly_ship/123.png", &wi, &hi, &nrChannels, 0);
	unsigned char* imgData = loadImage("ship.ppm",wi,hi);
	printf("Loaded Image of size (%d,%d)\n",wi,hi);
  int obe1, obe2;
    //unsigned char* imgData = stbi_load("./models/low_poly_ship/123.png", &wi, &hi, &nrChannels, 0);
  unsigned char* obeImg = loadImage("banana.ppm",obe1,obe2);


	int v1, v2;
	//unsigned char* imgData = stbi_load("./models/low_poly_ship/123.png", &wi, &hi, &nrChannels, 0);
	unsigned char* skyImg = loadImage("sky2.ppm",v1,v2);

    int watX,watY;
    //unsigned char* imgData = stbi_load("./models/low_poly_ship/123.png", &wi, &hi, &nrChannels, 0);
    unsigned char* watImg = loadImage("glass.ppm",watX,watY);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, wi, hi, 0, GL_RGB, GL_UNSIGNED_BYTE, imgData);
	glGenerateMipmap(GL_TEXTURE_2D);

	modelData = makeObject("models/low_poly_ship/ship_finished.obj",4475,1168); // This is surprising but this makes a ship
	cout << "ship made \n";
	//errors in this area

	//Build a Vertex Array Object. This stores the VBO and attribute mappings in one object
	GLuint vao;
	glGenVertexArrays(1, &vao); //Create a VAO
	glBindVertexArray(vao); //Bind the above created VAO to the current context

	//Allocate memory on the graphics card to store geometry (vertex buffer object)
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, numLines*sizeof(float), modelData, GL_DYNAMIC_DRAW);

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

	cout << "color made \n";


	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), 0);
	//Attribute, vals/attrib., type, isNormalized, stride, offset
	glEnableVertexAttribArray(posAttrib);
	cout << "potition made \n";

	GLint normAttrib = glGetAttribLocation(shaderProgram, "inNormal");
	glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(normAttrib);
	cout << "normals made \n";

	GLint texAttrib = glGetAttribLocation(shaderProgram, "inTexcoord");
    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));
   	cout << "texture made \n";

   GLint colAttrib = glGetAttribLocation(shaderProgram, "inColor");
   glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), 0);
   glEnableVertexAttribArray(colAttrib);


	glEnable(GL_DEPTH_TEST);

	SDL_Event windowEvent;
	bool quit = false;

	SDL_SetRelativeMouseMode(SDL_TRUE);

	float xsens = -0.002;
	float ysens = -0.002;
	float distToShip = 6.0;
	float needToMove = 0.0;
    float shipSpeed = 0.01;
    makeWater();
    cout << sizeOfWater << endl;

	while (!quit){
		while (SDL_PollEvent(&windowEvent)){
			if (windowEvent.type == SDL_QUIT) quit = true; //Exit Game Loop
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE)
				quit = true; //Exit Game Loop
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_f){
				fullscreen = !fullscreen;
				SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
			}

			glm::vec3 camForward = shipPos - camPos;
			camForward = glm::normalize(camForward);
			glm::vec3 camLeft = glm::cross(camForward,camUp);
			camLeft = glm::normalize(camLeft);
			glm::vec3 camBack = camPos - shipPos;
			camBack = glm::normalize(camBack);
			glm::vec3 camRight = glm::cross(camUp, camForward);
			camRight = glm::normalize(camRight);
			glm::vec3 newPos;


			float rotSpeed = 0.01;
			//left
			glm::normalize(shipDir);
			if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_a){
				float xval, yval;
				xval = shipDir.x * cos(rotSpeed) - shipDir.y*sin(rotSpeed);
				yval = shipDir.y * cos(rotSpeed) + shipDir.x*sin(rotSpeed);
				shipDir.x = xval;
				shipDir.y = yval;

				rotateShip(4475*8*2, shipPos.x, shipPos.y, shipPos.z, rotSpeed, 0, 0);
			}
			//right
			if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_d){
				float xval, yval;
				xval = shipDir.x * cos(-rotSpeed) - shipDir.y*sin(-rotSpeed);
				yval = shipDir.y * cos(-rotSpeed) + shipDir.x*sin(-rotSpeed);
				shipDir.x = xval;
				shipDir.y = yval;

				rotateShip(4475*8*2, shipPos.x, shipPos.y, shipPos.z, -rotSpeed, 0 , 0);
			}

			if (windowEvent.type == SDL_MOUSEMOTION){
				//scale left vector
				glm::vec3 scaledRight = camRight * 0.1f;
				//add left vector to current position

				camPos += camLeft * float(windowEvent.motion.xrel)*xsens;
			}

		}
		distToShip = (camPos.x - shipPos.x)*(camPos.x - shipPos.x) + (camPos.y - shipPos.y) * (camPos.y - shipPos.y);
		glm::vec3 toShip= shipPos - camPos;
		toShip.z = 0;
		glm::normalize(toShip);
		//fix camera
		if (distToShip > 42.4){
			camPos += toShip * 0.006f;
			//cout << distToShip <<endl;
		}
		else if (distToShip > 42.28){
			camPos += toShip * 0.001f;
			//cout << distToShip <<endl;
		}
		/*
		else if (distToShip < 42.1){
			camPos -= toShip * 0.01f;
		}
		else if (distToShip < 42.22){
			camPos -= toShip * 0.001f;
		}
		*/

		translateShip(4475*8*2,shipDir.x*shipSpeed,shipDir.y*shipSpeed, shipDir.z*shipSpeed);
		translateWater(sizeOfWater,shipDir.x*shipSpeed,shipDir.y*shipSpeed, shipDir.z*shipSpeed);
		//rotateShip(4475*8*2, 0, 0, 0, 0.02);
		glBindBuffer(GL_ARRAY_BUFFER,vbo);
		glBufferData(GL_ARRAY_BUFFER, numLines*sizeof(float),modelData,GL_DYNAMIC_DRAW);
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
		camPos,  //Cam Position
		shipPos,  //Look at point
		camUp); //Up

		GLint uniView = glGetUniformLocation(shaderProgram, "view");
		glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
		glm::mat4 proj = glm::perspective(3.14f/4, aspect, 0.01f, 100.0f);
		//FOV, aspect ratio, near, far
		GLint uniProj = glGetUniformLocation(shaderProgram, "proj");
		glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));



		glUseProgram(shaderProgram);
		glBindVertexArray(vao);  //Bind the VAO for the shaders we are using
		attachTexture(imgData,wi,hi);
		drawObject(modelData,shaderProgram, vao, &vbo,numVerts, numLines);
    attachTexture(obeImg,obe1,obe2);
    drawObelisks(shaderProgram, vao, &vbo);
		attachTexture(watImg,watX,watY);
		drawObject(waterData,shaderProgram, vao, &vbo,sizeOfWater, sizeOfWater);
		attachTexture(skyImg,v1,v2);
		drawBackground(shaderProgram,vao,&vbo);
		//glDrawArrays(GL_TRIANGLES, 0, numVerts); //Number of vertices
		//glDrawElements(GL_TRIANGLES, 0, numVerts/3, 0); //Number of vertices

		SDL_GL_SwapWindow(window); //Double buffering
	}
    delete [] watImg;
	delete [] imgData;
    delete [] skyImg;
	glDeleteProgram(shaderProgram);
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	SDL_GL_DeleteContext(context);
	SDL_Quit();

	return 0;
}
