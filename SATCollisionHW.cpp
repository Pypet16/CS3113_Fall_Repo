//
//  SATCollisionHW.cpp
//  NYUCodebase
//
//  Created by Peter Smondyrev on 12/6/16.
//  Copyright © 2016 Ivan Safrin. All rights reserved.
//

#include <stdio.h>
#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "Matrix.h"
#include "ShaderProgram.h"
#include "vector"

#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
using namespace std;


#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

class Vector{
public:
    float x;
    float y;
    float z;
    Vector(){}
    Vector(float x1, float y1, float z1){
        x = x1;
        y = y1;
        z = z1;
    }
};


bool wallColli(vector<Vector>& recty){
    for(int i = 0; i < recty.size(); i++){
        if(recty[i].x > 3.55){
            return true;
        }
        if(recty[i].x < -3.55){
            return true;
        }
    }
    return false;
}

bool ceilingColli(vector<Vector>& recty){
    for(int i = 0; i < recty.size(); i++){
        if(recty[i].y > 2.0){
            return true;
        }
        if(recty[i].x < -2.0){
            return true;
        }
    }
    return false;
}

bool testSATSeparationForEdge(float edgeX, float edgeY, const std::vector<Vector> &points1, const std::vector<Vector> &points2) {
    float normalX = -edgeY;
    float normalY = edgeX;
    float len = sqrtf(normalX*normalX + normalY*normalY);
    normalX /= len;
    normalY /= len;
    
    std::vector<float> e1Projected;
    std::vector<float> e2Projected;
    
    for(int i=0; i < points1.size(); i++) {
        e1Projected.push_back(points1[i].x * normalX + points1[i].y * normalY);
    }
    for(int i=0; i < points2.size(); i++) {
        e2Projected.push_back(points2[i].x * normalX + points2[i].y * normalY);
    }
    
    std::sort(e1Projected.begin(), e1Projected.end());
    std::sort(e2Projected.begin(), e2Projected.end());
    
    float e1Min = e1Projected[0];
    float e1Max = e1Projected[e1Projected.size()-1];
    float e2Min = e2Projected[0];
    float e2Max = e2Projected[e2Projected.size()-1];
    float e1Width = fabs(e1Max-e1Min);
    float e2Width = fabs(e2Max-e2Min);
    float e1Center = e1Min + (e1Width/2.0);
    float e2Center = e2Min + (e2Width/2.0);
    float dist = fabs(e1Center-e2Center);
    float p = dist - ((e1Width+e2Width)/2.0);
    
    if(p < 0) {
        return true;
    }
    return false;
}

bool checkSATCollision(const std::vector<Vector> &e1Points, const std::vector<Vector> &e2Points) {
    for(int i=0; i < e1Points.size(); i++) {
        float edgeX, edgeY;
        
        if(i == e1Points.size()-1) {
            edgeX = e1Points[0].x - e1Points[i].x;
            edgeY = e1Points[0].y - e1Points[i].y;
        } else {
            edgeX = e1Points[i+1].x - e1Points[i].x;
            edgeY = e1Points[i+1].y - e1Points[i].y;
        }
        
        bool result = testSATSeparationForEdge(edgeX, edgeY, e1Points, e2Points);
        if(!result) {
            return false;
        }
    }
    for(int i=0; i < e2Points.size(); i++) {
        float edgeX, edgeY;
        
        if(i == e2Points.size()-1) {
            edgeX = e2Points[0].x - e2Points[i].x;
            edgeY = e2Points[0].y - e2Points[i].y;
        } else {
            edgeX = e2Points[i+1].x - e2Points[i].x;
            edgeY = e2Points[i+1].y - e2Points[i].y;
        }
        bool result = testSATSeparationForEdge(edgeX, edgeY, e1Points, e2Points);
        if(!result) {
            return false;
        }
    }
    return true;
}


SDL_Window* displayWindow;

GLuint LoadTexture(const char *image_path){
    SDL_Surface *surface = IMG_Load(image_path);
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    SDL_FreeSurface(surface);
    
    return textureID;
}


void changeVertices(float vertices[], vector<Vector> recty){
    vertices[0] = recty[0].x;
    vertices[1] = recty[0].y;
    
    vertices[2] = recty[1].x;
    vertices[3] = recty[1].y;
    
    vertices[4] = recty[2].x;
    vertices[5] = recty[2].y;
    
    vertices[6] = recty[0].x;
    vertices[7] = recty[0].y;
    
    vertices[8] = recty[2].x;
    vertices[9] = recty[2].y;

    vertices[10] = recty[3].x;
    vertices[11] = recty[3].y;


}


int main(int argc, char *argv[]){
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    glViewport(0, 0, 640, 360);
    ShaderProgram program(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    
    vector<Vector> rectOne;
    vector<Vector> rectTwo;
    vector<Vector> rectThree;
    float rotAngle = 45.0;
    float angle = (rotAngle)*(3.14159265359/180);
    float tempX = -0.5;
    float tempY = -0.5;
    
    float x1 = (tempX*cosf(angle)) - (tempY*sinf(angle));
    float y1 = (tempX*sinf(angle)) + (tempY*cosf(angle));
    rectOne.push_back(Vector(x1, y1, 1.0));
    
    tempX = 0.75;
    x1 = (tempX*cosf(angle)) - (tempY*sinf(angle));
    y1 = (tempX*sinf(angle)) + (tempY*cosf(angle));
    rectOne.push_back(Vector(x1, y1, 1.0));
    
    tempY = 0.5;
    x1 = (tempX*cosf(angle)) - (tempY*sinf(angle));
    y1 = (tempX*sinf(angle)) + (tempY*cosf(angle));
    rectOne.push_back(Vector(x1, y1, 1.0));
    
    tempX = -0.5;
    x1 = (tempX*cosf(angle)) - (tempY*sinf(angle));
    y1 = (tempX*sinf(angle)) + (tempY*cosf(angle));
    rectOne.push_back(Vector(x1, y1, 1.0));
    
    //------------------------------------------
    
    tempX = 1.5;  tempY = -0.75;
    rotAngle = 25.0;
    angle = (rotAngle)*(3.14159265359/180);
    
    x1 = (tempX*cosf(angle)) - (tempY*sinf(angle));
    y1 = (tempX*sinf(angle)) + (tempY*cosf(angle));
    rectTwo.push_back(Vector(x1, y1, 1.0));
    
    tempY = 0.25;
    x1 = (tempX*cosf(angle)) - (tempY*sinf(angle));
    y1 = (tempX*sinf(angle)) + (tempY*cosf(angle));
    rectTwo.push_back(Vector(x1, y1, 1.0));
    
    tempX = 2.25;
    x1 = (tempX*cosf(angle)) - (tempY*sinf(angle));
    y1 = (tempX*sinf(angle)) + (tempY*cosf(angle));
    rectTwo.push_back(Vector(x1, y1, 1.0));

    tempY = -0.75;
    x1 = (tempX*cosf(angle)) - (tempY*sinf(angle));
    y1 = (tempX*sinf(angle)) + (tempY*cosf(angle));
    rectTwo.push_back(Vector(x1, y1, 1.0));
    
    //-------------------------------------------
    tempX = -2.8;  tempY = -0.4;
    rotAngle = 10.0;
    angle = (rotAngle)*(3.14159265359/180);
    x1 = (tempX*cosf(angle)) - (tempY*sinf(angle));
    y1 = (tempX*sinf(angle)) + (tempY*cosf(angle));
    rectThree.push_back(Vector(x1, y1, 1.0));
    
    tempY = 0.8;
    x1 = (tempX*cosf(angle)) - (tempY*sinf(angle));
    y1 = (tempX*sinf(angle)) + (tempY*cosf(angle));
    rectThree.push_back(Vector(x1, y1, 1.0));
    
    tempX = -1.9;
    x1 = (tempX*cosf(angle)) - (tempY*sinf(angle));
    y1 = (tempX*sinf(angle)) + (tempY*cosf(angle));
    rectThree.push_back(Vector(x1, y1, 1.0));
    
    tempY = -0.4;
    x1 = (tempX*cosf(angle)) - (tempY*sinf(angle));
    y1 = (tempX*sinf(angle)) + (tempY*cosf(angle));
    rectThree.push_back(Vector(x1, y1, 1.0));
    
    //----------------------------------
    
    Matrix projectionMatrix;
    Matrix modelMatrix;
    Matrix viewMatrix;
    program.setProjectionMatrix(projectionMatrix);
    projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
    glUseProgram(program.programID);
    program.setProjectionMatrix(projectionMatrix);
    program.setViewMatrix(viewMatrix);
    
    float vertices[] ={-1.0, -1.0, 1.0, -1.0, 1.0, 1.0, -1.0, -1.0, 1.0, 1.0, -1.0, 1.0} ;
    float lastFrameTicks = 0.0f;
    float ticks, elapsed;
    SDL_Event event;
    bool done = false;
    float directionOne = -1.0;
    float directionTwo = 1.0;
    float directionThree = 1.0;
    
    float yDirectionOne = 1.0;
    
    //-------- The loop that renders and uses the SAT collision function.
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }
        glClear(GL_COLOR_BUFFER_BIT);
        ticks = (float)SDL_GetTicks()/1000.0f;
        elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        //-----------------------------
        //reset Vertices value
        
        if(wallColli(rectTwo)){
            directionOne = directionOne* -1.0;
        }
        if(wallColli(rectThree)){
            directionTwo = directionTwo* -1.0;
        }
        
        if(checkSATCollision(rectOne, rectTwo)){
            directionOne = directionOne* -1.0;
            directionThree = directionThree * -1.0;
        }
        if(checkSATCollision(rectOne, rectThree)){
            directionTwo = directionTwo* -1.0;
            directionThree = directionThree * -1.0;

        }
        if(checkSATCollision(rectTwo, rectThree)){
            directionOne = directionOne* -1.0;
            directionTwo = directionTwo* -1.0;
        }
        
        for(int i =0; i < rectOne.size(); i++){
            rectOne[i].x += 1.0* elapsed * directionThree;
        }
        
        for(int i =0; i < rectTwo.size(); i++){
            rectTwo[i].x += 1.0* elapsed * directionOne;
        }
        for(int i =0; i < rectThree.size(); i++){
            rectThree[i].x += 1.0* elapsed * directionTwo;
        }
        
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        changeVertices(vertices, rectOne);
        modelMatrix.identity();
        program.setModelMatrix(modelMatrix);
        
        
        glEnableVertexAttribArray(program.positionAttribute);
        

        glDrawArrays(GL_TRIANGLES, 0, 6); // draws our rectangles for the 3 rects in this program
        
        
        
        //we change values in vertices to take into account of each rectangle that is rendered in the program
        changeVertices(vertices, rectTwo);
        modelMatrix.identity();
        program.setModelMatrix(modelMatrix);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        changeVertices(vertices, rectThree);
        modelMatrix.identity();
        program.setModelMatrix(modelMatrix);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        SDL_GL_SwapWindow(displayWindow);
        
    }
    
    SDL_Quit();
    return 0;
    
    
    return 0;
}

