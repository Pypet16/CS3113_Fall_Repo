//
#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "Matrix.h"
#include "ShaderProgram.h"


#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

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

int main(int argc, char *argv[])
{
    
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    glViewport(0, 0, 640, 360);
    ShaderProgram program(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    
    
    
    Matrix projectionMatrix;
    Matrix modelMatrix;
    Matrix viewMatrix;
    projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
    glUseProgram(program.programID);
    
    float lastFrameTicks = 0.0f;
    float ticks, elapsed;
    
    //Corrdinates of the matrices
    float vertices[] = {-1.0, -1.0, 1.0, -1.0, 1.0, 1.0, -1.0, -1.0, 1.0, 1.0, -1.0, 1.0};
    
    float RiseOne = 0.0;
    float RiseTwo = 0.0;
    
    // These are the balls units
    float unitX = 0;
    float unitY = 0;
    float originAngle = 45.0;
    float angle = originAngle;
    float bSpeed = 3.0; //velocity
    
    
    SDL_Event event;
    bool done = false;
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
        
        //Code for moving both padals, as the both positions get updated with the keys W, S, UP, and Down keys
        //------------------------
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        if(keys[SDL_SCANCODE_UP] && keys[SDL_SCANCODE_W]){
            RiseOne += 0.1;
            RiseTwo += 0.1;
        }
        else if(keys[SDL_SCANCODE_DOWN] && keys[SDL_SCANCODE_S]){
            RiseOne -= 0.1;
            RiseTwo -= 0.1;
        }
        else if(keys[SDL_SCANCODE_UP] && keys[SDL_SCANCODE_S]){
            RiseOne += 0.1;
            RiseTwo -= 0.1;
        }
        else if(keys[SDL_SCANCODE_DOWN] && keys[SDL_SCANCODE_W]){
            RiseOne -= 0.1;
            RiseTwo += 0.1;
        }
        else if(keys[SDL_SCANCODE_UP]){
            RiseOne += 0.1;
        }
        else if(keys[SDL_SCANCODE_DOWN]){
            RiseOne -= 0.1;
        }

        else if(keys[SDL_SCANCODE_W]){
            RiseTwo += 0.1;
        }
        else if(keys[SDL_SCANCODE_S]){
            RiseTwo -= 0.1;
        }
        
        if(RiseOne > 1.5 || RiseOne < -1.5){
            if(RiseOne > 0){
                RiseOne = 1.5;
            }
            else{
                RiseOne = -1.5;
            }
        }
        
        if(RiseTwo > 1.5 || RiseTwo < -1.5){
            if(RiseTwo > 0){
                RiseTwo = 1.5;
            }
            else{
                RiseTwo = -1.5;
            }
        }
        
        // The code for rendering the padals
        //----------------------------
        modelMatrix.identity();
        modelMatrix.Translate(3.45, RiseOne, 0.0);
        modelMatrix.Scale(0.10, 0.5, 1);
        program.setModelMatrix(modelMatrix);
        
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        
        program.setProjectionMatrix(projectionMatrix);
        program.setViewMatrix(viewMatrix);
        
    
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        modelMatrix.identity();
        modelMatrix.Translate(-3.45, RiseTwo, 0.0);
        modelMatrix.Scale(0.10, 0.5, 1);
        program.setModelMatrix(modelMatrix);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        //This is the matrix responsible for the balls movement
        //------------------------------------------------------
        // These are the conditions for a player to win. After the ball goes through the gates, the x and y coordinates are reset, and the angle changes to go in the opposite direction.
        if(unitX > 3.55 || unitX < -3.55){
            if(unitX > 3.55){
                angle = (originAngle+90)*-1;
                unitX = 0;
                unitY = 0;
            }
            else if(unitX < -3.55){
                angle = originAngle;
                unitX = 0;
                unitY = 0;
            }
        }
        
        //--------------------------------------------------------
        // These are the conditions for the ball to bounce off the borders of the game, top and bottom
        if(unitY > 1.9){
            if(cos((angle)*3.14159265359/180) > 0){
                angle = angle * -1.0;
            }
            else if (cos((angle)*3.14159265359/180) < 0){
                angle = angle * -1.0;
            }
        }
        
        if (unitY < -1.9){
            if(cos((angle)*3.14159265359/180) > 0){
                angle = angle * -1;
            }
            else if(cos((angle)*3.14159265359/180) < 0){
                angle = angle*-1;
            }
        }
        //--------------------------------------------------------
        //paddle conditions to make the ball bounce off them
        if(unitX > 3.35 && unitX < 3.45){
            if(unitY < 0.5+RiseOne && unitY > -0.5+RiseOne){
                if(sin((angle)*3.14159265359/180) > 0){
                    angle = angle + 90;
                }
                if(sin((angle)*3.14159265359/180) < 0){
                    angle = angle - 90;
                }
            }
        }
        else if(unitX < -3.35 && -3.45 < unitX){
            if(unitY <= 0.5+RiseTwo && unitY > -0.5+RiseTwo){
                if(sin((angle)*3.14159265359/180) > 0){
                    angle = angle - 90;
                }
                if(sin((angle)*3.14159265359/180) < 0){
                    angle = angle + 90;
                }
            }
            

        }
        unitX += bSpeed * elapsed * cos((angle)*3.14159265359/180);
        unitY += bSpeed * elapsed * sin((angle)*3.14159265359/180);
        
        modelMatrix.identity();
        modelMatrix.Translate(unitX, unitY, 0.0);
        modelMatrix.Scale(0.10, 0.10, 0.25);
        program.setModelMatrix(modelMatrix);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program.positionAttribute);
        
        SDL_GL_SwapWindow(displayWindow);
        
    }
    
    SDL_Quit();
    return 0;
}
