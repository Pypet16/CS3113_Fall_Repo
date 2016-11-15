//
//  platformGame.cpp
//  NYUCodebase
//
//  Created by Peter Smondyrev on 11/7/16.
//  Copyright © 2016 Ivan Safrin. All rights reserved.
//

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


#define fixedTimeStep 0.0166666f
#define Max_TimeSteps 6
#define mapWidth 130
#define mapHeight 20
#define tileSize 23

#define SPRITE_COUNT_X 30
#define SPRITE_COUNT_Y 16


unsigned char levelData[mapHeight][mapWidth];

SDL_Window* displayWindow;

enum objectType{player, dangerous,  object, item};

class gameChar{
    float x;
    float y;
    float witdth;
    float height;
    
    //positions of indeces of character matrix
    float left;
    float right;
    float top;
    float bottom;
    
    float direction;
    float velocityX = 3.0;
    float velocityY;
    float accelerationX;
    float accelerationY;
    float gravity = -9.8;
    float fixedElapsed;
    objectType unit;
    GLuint textChar;
    
    bool finishLine;
    bool collideTop;
    bool collideBottom;
    bool collideLeft;
    bool collideRight;
public:
    
    bool colliRight(){
        float posy = float(int(y));
        float posx = float(int(x)+1);
        if(int(levelData[int(posy)][int(posx)]) == 194 && int(levelData[int(posy)][int(posx)-2]) ==194 ){
            return false;
        }
        if(int(levelData[int(posy)][int(posx)]) != 194){
            if(right > posx && right < posx + 1.0){
                    return true;
            }
        }
        if(int(levelData[int(posy)+1][int(posx)]) != 194){
            if(right > posx && right < posx + 1.0){
                return true;
            }
        }
        return false;
    }
    
    bool colliLeft(){
        float posy = float(int(y));
        float posx = float(int(x));
        if(int(levelData[int(posy)][int(posx)]) == 194 && int(levelData[int(posy+1.0)][int(posx)]) ==194 ){
            return false;
        }
        if(int(levelData[int(posy)][int(posx)]) != 194){
            
            if(left > posx && left < posx + 1.0){
                return true;
            }
        }
        
        if(int(levelData[int(posy+1.0)][int(posx)]) != 194){
            if(left > posx && left < posx + 1.0){
                    return true;
            }
        }
        
        
        return false;

    }
    
    bool colliTop(){
        float posy = float(int(y));
        float posx = float(int(x));
        if(int(levelData[int(posy)][int(posx)]) == 194 && int(levelData[int(posy)][int(posx)+1]) ==194){
            return false;
        }
        
        if(int(levelData[int(posy)][int(posx)]) != 194){
            if(top < posy && top > posy-1.0){
                return true;
            }
        }
        if(int(levelData[int(posy)][int(posx)+1]) != 194){
            if(top < posy && top > posy-1.0){
                return true;
            }

        }
        
        return false;
    }
    void lose(){
        finishLine = true;
    }
    
    bool colliBot(){
        float posy = float(int(y)+1);
        float posx = float(int(x));
        if(int(levelData[int(posy)][int(posx)]) == 194 && int(levelData[int(posy)][int(posx)+1]) ==194){
            return false;
        }
        if(int(levelData[int(posy)][int(posx)]) != 194){
            if(bottom > posy && bottom < posy + 1.0){
                return true;
            }
        }
        if(int(levelData[int(posy)][int(posx)+1]) != 194){
            if(bottom > posy && bottom < posy + 1.0){
                return true;
            }
        }

        return false;
    }
    
    
    // x position = x* 0.2,  y position = 2.0 - 0.2*y
    bool returnFinishStatus(){return finishLine;}
    
    
    void jump(){
        if(velocityY == 0.0){
            velocityY = 10.0;
        }
        
    }
    void updatePos(){
        left = x;
        right = x+1.0;
        top = y;
        bottom = y+1.0;
    }
    //this function is responsible for changing the finish status for the game.
    void intereactingGoal(){
        if(int(levelData[int(y)][int(x)+1]) == 130){
            finishLine = true;
        }
        if(int(levelData[int(y)+1][int(x)]) == 130){
            finishLine = true;
        }

    }
    
    //function that updates x and y positions.
    void update(float& elapsed, float& sign, Matrix& view, float& movementX){
        intereactingGoal();
        if(int(levelData[int(y)+1][int(x)]) == 194  && velocityY == 0.0){
            velocityY += -0.0000001;
        }
        if(velocityY > 0){
            y-= velocityY*elapsed - (0.5*gravity*(elapsed*elapsed));
            velocityY += gravity*elapsed;
        }
        if(velocityY < 0){
            y-= velocityY*elapsed - 0.5*gravity*(elapsed*elapsed);
            velocityY += gravity*elapsed;
        }
        if(int(levelData[int(y)][int(x)+1]) == 194){
            x += velocityX*elapsed*sign;
            movementX -= 0.6*elapsed*sign;
        }
        
        //We are on;y using these two, as the object can't reach the ceiling and the aim of the game is to move forward.
        collideRight = colliRight();
        collideBottom = colliBot();

        if(collideRight == true){
            x = float(int(left))-0.00001;
            movementX += 0.6*elapsed*sign;
        }
        
        if(collideBottom == true){
            velocityY = 0;
            y = float(int(y));
        }
        
        collideRight = false;
        collideBottom = false;
        updatePos();

        
    }
    void checkTime(float elapsed){
        fixedElapsed = elapsed;
        if(fixedElapsed > fixedTimeStep*Max_TimeSteps){
            fixedElapsed = fixedTimeStep*Max_TimeSteps;
        }
        
    }
    void setEntityPosition(float placeX, float placeY){
        x = placeX;
        y = placeY;
        velocityY = 0.0;
        finishLine = false;
        direction = 1.0;
        left = x;
        right = x+1.0;
        top = y;
        bottom = top+1.0;
        
    }
    
    
    
    void draw(GLuint& text,ShaderProgram& program, Matrix& model){
        float spriteWidth = 1.0/((float)SPRITE_COUNT_X);
        float spriteHeight = 1.0/((float)SPRITE_COUNT_Y);
        
        float u = 0.0;
        float v = 0.0;
        float indx;
        float indy;
        
        u = ((float)(20%SPRITE_COUNT_X));
        v = (float)(20/SPRITE_COUNT_X);
        
        indx = u*spriteWidth;
        indy = (v*spriteHeight) + spriteHeight;
        float vertices[] = {-3.55, 1.8, -3.35, 1.8, -3.35, 2.0, -3.55, 1.8, -3.35, 2.0, -3.55, 2.0};
        float textCord[] = {indx, indy, indx+spriteWidth, indy, indx+spriteWidth, indy-spriteHeight,
            indx, indy, indx+spriteWidth, indy-spriteHeight, indx, indy-spriteHeight
        };
        glBindTexture(GL_TEXTURE_2D, text);
        model.Translate(x*0.2, (y)*(-0.2), 0.0);
        
        program.setModelMatrix(model);
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, textCord);
        
        glEnableVertexAttribArray(program.positionAttribute);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);

        
        
    }
    
    
};


//--------------------------------------------
// file reading functions,  I excluded the header function as I am using a tile map with already pre defined width and height.  These two are the only ones to make things simpler.
bool readLayerData(std::ifstream &stream) {
    string line;
    while(getline(stream, line)) {
        if(line == "") {
            break; }
        istringstream sStream(line);
        string key,value;
        getline(sStream, key, '=');
        getline(sStream, value);
        if(key == "data") {
            for(int y=0; y < mapHeight; y++) {
                getline(stream, line);
                istringstream lineStream(line);
                string tile;
                for(int x=0; x < mapWidth; x++) {
                    getline(lineStream, tile, ',');
                    unsigned char val =  (unsigned char)atoi(tile.c_str());
                    
                    if(val > 0) {
                        // be careful, the tiles in this format are indexed from 1 not 0
                        levelData[y][x] = val-1;
                    } else {
                        levelData[y][x] = 0;
                    }
                }
            } }
    }
    return true;
}

// dangerous spots 70

bool readEntityData(std::ifstream &stream, gameChar& obj) {
    string line;
    string type;
    while(getline(stream, line)) {
        if(line == "") { break; }
        istringstream sStream(line);
        string key,value;
        getline(sStream, key, '=');
        getline(sStream, value);
        if(key == "type") {
            type = value;
            
        } else if(key == "location"){
            istringstream lineStream(value);
            string xPosition, yPosition;
            getline(lineStream, xPosition, ',');
            getline(lineStream, yPosition, ',');
            float placeX = atoi(xPosition.c_str());
            float placeY = atoi(yPosition.c_str())-1.0;
            obj.setEntityPosition(placeX, placeY);
        }
    }
    return true;
}
// This function is only concerned with getting our coordinates for teh layers and the object layer
void readFromFile(gameChar& chr){
    ifstream infile;
    infile.open("/Users/Peter/Desktop/PlatformMap.txt");
    string line;
    while (getline(infile, line)) {
        if(line == "[layer]") {
            readLayerData(infile);
         }
        else if(line == "[Object Layer 1]"){
            readEntityData(infile, chr);
        }
    }
}

//--------------------------------------------
//These are the functions that are involved with rendering the textures

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

void draw(GLuint& texture, ShaderProgram& program, Matrix model){
    float spriteWidth = 1.0/((float)SPRITE_COUNT_X);
    float spriteHeight = 1.0/((float)SPRITE_COUNT_Y);
    
    float u = 0.0;
    float v = 0.0;
    float indx;
    float indy;
    float height = 0.0;
    float width = 0.0;
    
    float vertices[] = {-3.55, 1.8, -3.35, 1.8, -3.35, 2.0, -3.55, 1.8, -3.35, 2.0, -3.55, 2.0};
    
    
    for(int y =0; y < mapHeight; y++){
        for(int x = 0; x < mapWidth; x++){
            model.identity();
            model.Translate(width, height, 0.0);
            glBindTexture(GL_TEXTURE_2D, texture);
            if(int(levelData[y][x]) != 194){
                u = ((float)(int(levelData[y][x])%SPRITE_COUNT_X));
                v = (float)(int(levelData[y][x])/SPRITE_COUNT_X);
            
                indx = u*spriteWidth;
                indy = (v*spriteHeight) + spriteHeight;
                float textCord[] = {indx, indy, indx+spriteWidth, indy, indx+spriteWidth, indy-spriteHeight,
                indx, indy, indx+spriteWidth, indy-spriteHeight, indx, indy-spriteHeight
                };
            
                program.setModelMatrix(model);
                glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
                glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, textCord);
            
                glEnableVertexAttribArray(program.positionAttribute);
                glEnableVertexAttribArray(program.texCoordAttribute);
            
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }
            width += 0.2;
        }
        width = 0.0;
        height -= 0.2;
    }
    model.identity();

}
//Where the magic happens and where the tiles come to life
//----------------------------------------------------------------
int main(int argc, char *argv[]){
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    glViewport(0, 0, 640, 360);
    ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    
    
    gameChar character; // our game Character
    readFromFile(character);
    
    float movementx = 0;
    Matrix projectionMatrix;
    Matrix modelMatrix;
    Matrix viewMatrix;
    program.setProjectionMatrix(projectionMatrix);
    program.setViewMatrix(viewMatrix);
    program.setModelMatrix(modelMatrix);
    projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
    glUseProgram(program.programID);
    
    vector<float> vecCord;
    vector<float> texCord;
    
    float lastFrameTicks = 0.0f;
    float ticks, elapsed;
    
    GLuint text = LoadTexture("spritesheet.png");
    SDL_Event event;
    bool done = false;
    while (!done && !character.returnFinishStatus()) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }
        glClear(GL_COLOR_BUFFER_BIT);
        ticks = (float)SDL_GetTicks()/1000.0f;
        elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;

        
        //---------------------------------------------
        // Game Controls, There are only two controls in this game, as you can only move forward and jump.
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        float sign = 0.0;
        if(keys[SDL_SCANCODE_D] && keys[SDL_SCANCODE_W]){
            character.jump();
            sign = 1.0;
        }
        if(keys[SDL_SCANCODE_D]){
            sign = 1.0;
        }
        else if(keys[SDL_SCANCODE_W]){
            character.jump();
        }
        //The rule of the game is to never turn back.  This is also used if the player is trapped in a pitFall trap signaling their resignation from the game.
        if(keys[SDL_SCANCODE_A]){
            character.lose();
        }
        
        character.update(elapsed, sign, viewMatrix, movementx);

        //---------------------------------------------
        viewMatrix.identity();
        viewMatrix.Translate(1.0, 0.0, 0.0);
        viewMatrix.Translate(movementx, 0.0, 0.0);
        program.setProjectionMatrix(projectionMatrix);
        program.setViewMatrix(viewMatrix);
        
        modelMatrix.identity();
        
        program.setModelMatrix(modelMatrix);
        
        character.draw(text, program, modelMatrix);
        draw(text, program, modelMatrix);
        modelMatrix.identity();
        
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        SDL_GL_SwapWindow(displayWindow);
        
    }
    
    SDL_Quit();
    return 0;

    
    return 0;
}
