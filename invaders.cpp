#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "Matrix.h"
#include "ShaderProgram.h"
#include "vector"

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
//----------------------------------- Text draw function
void DrawText(ShaderProgram *program, int fontTexture, std::string text, const float& size, const float& spacing) {
    float texture_size = 1.0/16.0f;
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    for(int i=0; i < text.size(); i++) {
        float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
        float texture_y = (float)(((int)text[i]) / 16) / 16.0f;
        vertexData.insert(vertexData.end(), {
            ((size+spacing) * i) + (-0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (-0.5f * size), -0.5f * size,
            ((size+spacing) * i) + (0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (0.5f * size), -0.5f * size,
            ((size+spacing) * i) + (0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (-0.5f * size), -0.5f * size,
        });
        texCoordData.insert(texCoordData.end(), {
            texture_x, texture_y,
            texture_x, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x + texture_size, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x, texture_y + texture_size,
        }); }
    glUseProgram(program->programID);
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glDrawArrays(GL_TRIANGLES, 0, text.size() * 6.0);
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

//----------------------------------------------------------

class Entity{
    float positionx, positiony;
    int spriteCountX, spriteCountY;
    int index;
    int varyVar = 1.0;
    float movement_speed = 1.5;
    float tank_speed = 35.0;
    GLuint mytexture;
    float bulletLife;
    bool spriteLife;
public:
    Entity(const int& ind, const int& pox, const int& poy):spriteCountX(pox), spriteCountY(poy), index(ind){spriteLife = true;}
    Entity():positionx(0.0), positiony(0.0){spriteLife = true;};
    void setPosition(const float& x, const float& y){
        positionx = x;
        positiony = y;
    }
    
    
    void move(Matrix& model, float elap){
        positionx += movement_speed*elap;
        model.Scale(0.2, 0.2, 1.0);
        model.Translate(positionx, positiony, 0.0);
    }
    
    void draw(ShaderProgram* program){
        float spriteWidth = 1.0/((float)spriteCountX);
        float spriteHeight = 1.0/((float)spriteCountY);
        float u = ((float)(int(index)%spriteCountX));
        float v = (float)(int(index)/spriteCountX);
        
        float indx = u*spriteWidth;
        float indy = (v*spriteHeight) + spriteHeight;
        
        float vertices[] = {-1.0, -1.0, 1.0, -1.0, 1.0, 1.0, -1.0, -1.0, 1.0, 1.0, -1.0, 1.0};
        float textCord[] = {indx, indy, indx+spriteWidth, indy, indx+spriteWidth, indy-spriteHeight,
            indx, indy, indx+spriteWidth, indy-spriteHeight, indx, indy-spriteHeight
        };
        glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, textCord);
        
        
        glEnableVertexAttribArray(program->positionAttribute);
        glEnableVertexAttribArray(program->texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program->positionAttribute);
        glDisableVertexAttribArray(program->texCoordAttribute);
    }
    bool borderCheck(){
        return ((positionx > 16.8)||(positionx)<-16.8);
    }
    bool gameover(){
        if(positiony < -7.8){
            return true;
        }
        return false;
    }
    bool alive(){
        return spriteLife;
    }
    
    void updateY(){
        positiony -= 2.0;
        movement_speed = movement_speed*-1.0;
    }
    
    //------------------------------
    // code for the bullets
    
    void interesect(Entity& target){
        
    }
    //-------------------------------
    // code for the tank
    void tankMove(Matrix model, float& elap, const float& direction, float& origin){
        origin += tank_speed*elap*direction;
        model.Translate(origin, -8.5, 0.0);
    }
    
    Entity(GLuint& text, float& positionx, const float& positiony, const float&elap):mytexture(text), positionx(positionx), positiony(positiony), bulletLife(elap){spriteLife = true;}
    void bulletMove(Matrix& model,float& elapsed){
        bulletLife += elapsed;
        positiony += 2.0*elapsed;
        model.Translate(positionx, positiony, 0.0);
    }
    bool bulletExp(){
        return (bulletLife > 3.0);
    }
    
    void BulletDraw(ShaderProgram* program, Matrix& model){
        
        glBindTexture(GL_TEXTURE_2D, mytexture);
        
        model.Translate(positionx, positiony, 0.0);
        float vertices[] = {-1.0, -1.0, 1.0, -1.0, 1.0, 1.0, -1.0, -1.0, 1.0, 1.0, -1.0, 1.0};
        float textCord[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        
        glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, textCord);
        
        
        glEnableVertexAttribArray(program->positionAttribute);
        glEnableVertexAttribArray(program->texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program->positionAttribute);
        glDisableVertexAttribArray(program->texCoordAttribute);
    }

    void bulletCollide(Entity& enemy){
        //dimensions of bullet
        if(positiony > enemy.positiony && positiony <enemy.positiony+0.2){
            if(positionx > (enemy.positionx) && positionx < (enemy.positionx)+0.2){
                enemy.spriteLife = false;
                spriteLife = false;
                return;
            }
            else if(positionx+0.2 >enemy.positionx && positionx + 0.2 < enemy.positionx+0.2){
                enemy.spriteLife = false;
                spriteLife = false;
                return;
            }
            else if(((positionx+positionx+0.2)/2) > enemy.positionx && ((positionx+positionx+0.2)/2) <enemy.positionx+0.2){
                enemy.spriteLife = false;
                spriteLife = false;
                return;
            }
        }
        else if(positiony+0.2 > enemy.positiony && positiony+0.2 < enemy.positiony+0.2){
            if(positionx > (enemy.positionx) && positionx < (enemy.positionx)+0.2){
                enemy.spriteLife = false;
                spriteLife = false;
                return;
            }
            else if(positionx+0.2 >enemy.positionx && positionx + 0.2 < enemy.positionx+0.2){
                enemy.spriteLife = false;
                spriteLife = false;
                return;
            }
            else if(((positionx+positionx+0.2)/2) > enemy.positionx && ((positionx+positionx+0.2)/2) <enemy.positionx+0.2){
                enemy.spriteLife = false;
                spriteLife = false;
                return;
            }
        }
        if(((positiony+positiony+0.2)/2) > enemy.positiony && ((positiony+positiony+0.2)/2) < enemy.positiony+0.2){
            if(positionx > (enemy.positionx) && positionx < (enemy.positionx)+0.2){
                enemy.spriteLife = false;
                spriteLife = false;
                return;
            }
            else if(positionx+0.2 >enemy.positionx && positionx + 0.2 < enemy.positionx+0.2){
                enemy.spriteLife = false;
                spriteLife = false;
                return;
            }
            else if(((positionx+positionx+0.2)/2) > enemy.positionx && ((positionx+positionx+0.2)/2) <enemy.positionx+0.2){
                enemy.spriteLife = false;
                spriteLife = false;
                return;
            }
        }
    }
    
};




//------------------------------------------------------------


void update(bool& status, const Uint8 *keys){
    if(keys[SDL_SCANCODE_RETURN]){
        status = true;
    }
}

void guardBorder(float& position){
    if(position > 16.8){
        position = 16.8;
    }
    else if(position < -16.8){
        position = -16.8;
    }
}

int getIndex(std::vector<int>& arr, float& direction){
    int min;
    if(direction > 0.0){
        min = arr[0];
        for(int i = 0; i < arr.size(); i++){
            if(min < arr[i]%6 ){
                min = arr[i];
            }
        }
    }
    else if(direction < 0.0){
        min = arr[0]-5;
        for(int i = 0; i < arr.size(); i++){
            if(min > (arr[i]-5)%6){
                min = arr[i];
            }
        }
    }
    return min;
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
    ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    
    Matrix projectionMatrix;
    Matrix modelMatrix;
    Matrix viewMatrix;
    projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
    glUseProgram(program.programID);
    
    
    GLuint text = LoadTexture("characters_3.png");
    
    //Objects for this game
    std::vector<Entity> myEnemies;
    Entity tank(11, 8, 4);
    std::vector<Entity> bullets;
    
    float startx = -5.8;
    for(int i = 0; i < 12; i++){
        myEnemies.push_back(Entity(3, 8, 4));
        if(i < 5){
            myEnemies[i].setPosition(startx, 8.0);
            startx += 2.2;
        }
        if(i == 5){
            myEnemies[i].setPosition(startx, 8.0);
            startx = -5.8;
        }
        else if (i >5){
            myEnemies[i].setPosition(startx, 6.0);
            startx += 2.2;
        }
    }
    
    startx = -5.8;
    for(int i = 0; i < 12; i++){
        myEnemies.push_back(Entity(16, 8, 4));
        if(i < 5){
            myEnemies[i+12].setPosition(startx, 4.0);
            startx += 2.2;
        }
        if(i == 5){
            myEnemies[i+12].setPosition(startx, 4.0);
            startx = -5.8;
        }
        else if (i >5){
            myEnemies[i+12].setPosition(startx, 2.0);
            startx += 2.2;
        }
    }
    startx = -5.8;
    for(int i = 0; i < 12; i++){
        myEnemies.push_back(Entity(24, 8, 4));
        if(i < 5){
            myEnemies[i+24].setPosition(startx, 0.0);
            startx += 2.2;
        }
        if(i == 5){
            myEnemies[i+24].setPosition(startx, 0.0);
            startx = -5.8;
        }
        else if (i >5){
            myEnemies[i+24].setPosition(startx, -2.0);
            startx += 2.2;
        }
    }
    
    float lastFrameTicks = 0.0f;
    float ticks, elapsed;
    float tankOrigin = 0.0;
    //-----------------------------------------------------
    SDL_Event event;
    bool done = false;
    bool enterStatus = false;
    GLuint font = LoadTexture("font2.png");
    GLuint bullet = LoadTexture("fire00.png");
    
    std::string name = "Welcome";
    std::string name1 = "Space Invaders!";
    std::string objective1 = "Black Friday Edition";
    std::string objective2 = "Knock the alien shoppers out";
    std::string startGame = "Press Enter to Start";
    
    std::vector<int> positionArr ={5, 11, 17, 23, 29, 35};
    
    program.setProjectionMatrix(projectionMatrix);
    program.setViewMatrix(viewMatrix);
    float direct = 1.0;
    
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }
        ticks = (float)SDL_GetTicks()/1000.0f;
        elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        glClear(GL_COLOR_BUFFER_BIT);
        
        for(int i = 0; i < myEnemies.size(); i++){
            if(myEnemies[i].gameover()){
                done = true;
                break;
            }
        }
        
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        // This is menu state code, the enterStatus is updated depending on the update function
        if(enterStatus == false){
            modelMatrix.identity();
            modelMatrix.Translate(-1.5, 1.5, 0.0);
            program.setModelMatrix(modelMatrix);
            DrawText(&program, font, name, 0.5f, 0.05f);
            
            modelMatrix.identity();
            modelMatrix.Translate(-1.6, 1.2, 0.0);
            program.setModelMatrix(modelMatrix);
            DrawText(&program, font, name1, 0.25f, 0.01f);
            
            modelMatrix.identity();
            modelMatrix.Translate(-1.8, 0.0, 0.0);
            program.setModelMatrix(modelMatrix);
            DrawText(&program, font, objective1, 0.20f, 0.001f);
            
            modelMatrix.identity();
            modelMatrix.Translate(-2.4, -0.25, 0.0);
            program.setModelMatrix(modelMatrix);
            DrawText(&program, font, objective2, 0.20f, 0.001f);
            
            modelMatrix.identity();
            modelMatrix.Translate(-1.7, -0.6, 0.0);
            program.setModelMatrix(modelMatrix);
            DrawText(&program, font, startGame, 0.20f, 0.001f);
            
            update(enterStatus, keys);
            
        }
        // This is for the entities in the game, or the game menu state
        //-----------------------------------------------------
        else{
            modelMatrix.identity();
            modelMatrix.Scale(0.2, 0.2, 0.0);
            modelMatrix.Translate(tankOrigin, -8.5, 0.0);
            if(keys[SDL_SCANCODE_D]){
                direct = 1.0;
                tank.tankMove(modelMatrix, elapsed, direct, tankOrigin);
            }
            else if(keys[SDL_SCANCODE_A]){
                direct = -1.0;
                tank.tankMove(modelMatrix, elapsed, direct, tankOrigin);
            }
            else if(keys[SDL_SCANCODE_SPACE]){
                if(bullets.size()!=1){
                    bullets.push_back(Entity(bullet, tankOrigin, -8.3, 0.0));
                }
            }
            
            guardBorder(tankOrigin);
            
            program.setModelMatrix(modelMatrix);
            tank.draw(&program);
            
            if(bullets.size() != 0){
                for(int i = 0; i< myEnemies.size(); i++){
                    if(!bullets[0].alive()){
                        i = int(myEnemies.size());
                    }
                    bullets[0].bulletCollide(myEnemies[i]);
                }
            }
            // The bullets rendering
            if(bullets.size() != 0){
                if(bullets[0].bulletExp()){ //bulletExp is simply bulletExpiration
                    bullets.clear();
                }
                else{
                    modelMatrix.identity();
                    modelMatrix.Scale(0.1, 0.2, 1.0);
                    glBindTexture(GL_TEXTURE_2D, bullet);
                    bullets[0].bulletMove(modelMatrix, elapsed);
                    program.setModelMatrix(modelMatrix);
                    bullets[0].BulletDraw(&program, modelMatrix);
                }
            }
            // Enemies movement
            
            if(myEnemies[getIndex(positionArr, direct)].borderCheck()){
                for(int i = 0; i<myEnemies.size(); i++){
                    myEnemies[i].updateY();
                }
                direct = direct* -1.0;
            }
            for(int i = 0; i<myEnemies.size(); i++){
                modelMatrix.identity();
                glBindTexture(GL_TEXTURE_2D, text);
                myEnemies[i].move(modelMatrix, elapsed);
                program.setModelMatrix(modelMatrix);
                if(myEnemies[i].alive()){
                    myEnemies[i].draw(&program);
                }
            }
            
        }
        SDL_GL_SwapWindow(displayWindow);
        
    }
    
    SDL_Quit();
    return 0;
}
