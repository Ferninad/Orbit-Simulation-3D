#include "common.h"
#include "cmath"
#include "vector"

bool Init();
void CleanUp();
void Run();
void Setup();
void Rots();
void Convert();
void Draw();
void Simulate();
vector<vector<double>> MultMatrixs(vector<vector<double>> mat1, vector<vector<double>> mat2);
void DrawCircle(SDL_Point center, int radius, SDL_Color color);

SDL_Window *window;
SDL_GLContext glContext;
SDL_Surface *gScreenSurface = nullptr;
SDL_Renderer *renderer = nullptr;
SDL_Rect pos;

int screenWidth = 500;
int screenHeight = 500;
bool run = false;
double mag = 0;
double zoom;
double posx = 0;
double posy = 0;
double mpp = 5000000000; //Mass per pixel, radius
double rate = 0;
double timeStep;
int followObject = -1;
int px = 0;
int py = 0;
double yang = 0;
double xang = 0;
double zang = 0;
double xper = 1;
double yper = 1;
double zper = 1;
int step = 1;

vector<vector<double>> objects;
vector<vector<double>> pps;
vector<vector<double>> trail;
vector<vector<double>> tps;
vector<vector<double>> rotx;
vector<vector<double>> roty;
vector<vector<double>> rotz;
vector<vector<double>> projection;

bool Init()
{
    if (SDL_Init(SDL_INIT_NOPARACHUTE & SDL_INIT_EVERYTHING) != 0)
    {
        SDL_Log("Unable to initialize SDL: %s\n", SDL_GetError());
        return false;
    }
    else
    {
        //Specify OpenGL Version (4.2)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_Log("SDL Initialised");
    }

    //Create Window Instance
    window = SDL_CreateWindow(
        "Game Engine",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        screenWidth,
        screenHeight,   
        SDL_WINDOW_OPENGL);

    //Check that the window was succesfully created
    if (window == NULL)
    {
        //Print error, if null
        printf("Could not create window: %s\n", SDL_GetError());
        return false;
    }
    else{
        gScreenSurface = SDL_GetWindowSurface(window);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        SDL_Log("Window Successful Generated");
    }
    //Map OpenGL Context to Window
    glContext = SDL_GL_CreateContext(window);

    return true;
}

int main()
{
    //Error Checking/Initialisation
    if (!Init())
    {
        printf("Failed to Initialize");
        return -1;
    }

    // Clear buffer with black background
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    //Swap Render Buffers
    SDL_GL_SwapWindow(window);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    Run();

    CleanUp();
    return 0;
}

void CleanUp()
{
    //Free up resources
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Run()
{
    bool gameLoop = true;
    srand(time(NULL));

    Rots();
    Setup();
    while (gameLoop)
    {   
        zoom = pow(2, mag);
        timeStep = pow(2, rate);
        if(followObject == -1){
            trail.clear();
            tps.clear();
        }
        if(run)
            Simulate();
        projection.clear();
        projection.push_back({xper, 0, 0});
        projection.push_back({0, yper, 0});
        Convert();
        if(followObject != -1){
            posx = -1 * pps[followObject][0];
            posy = -1 * pps[followObject][1];
        }
        Draw();
        
        SDL_RenderPresent(renderer);
        pos.x = 0;
        pos.y = 0;
        pos.w = screenWidth;
        pos.h = screenHeight;
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(renderer, &pos);
    
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                gameLoop = false;
            }
            if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                        gameLoop = false;
                        break;
                    case SDLK_SPACE:
                        run = !run;
                        break;
                    case SDLK_r:
                        mag++;
                        break;
                    case SDLK_f:
                        mag--;
                        break;
                    case SDLK_w:
                        posy += (screenHeight / 20) / zoom;
                        followObject = -1;
                        break;
                    case SDLK_s:
                        posy -= (screenHeight / 20) / zoom;
                        followObject = -1;
                        break;
                    case SDLK_a:
                        posx += (screenWidth / 20) / zoom;
                        followObject = -1;
                        break;
                    case SDLK_d:
                        posx -= (screenWidth / 20) / zoom;
                        followObject = -1;
                        break;
                    case SDLK_q:
                        rate--;
                        trail.clear();
                        break;
                    case SDLK_e:
                        rate++;
                        trail.clear();
                        break;
                    case SDLK_c:
                        followObject = -1;
                        break;
                    case SDLK_x:
                        followObject++;
                        if(followObject > objects.size() - 1)
                            followObject = 0;
                        trail.clear();
                        break;
                    case SDLK_z:
                        followObject--;
                        if(followObject < 0)
                            followObject = objects.size() - 1;
                        trail.clear();
                        break;
                    case SDLK_UP:
                        xang += .01;
                        if(xang > 2*M_PI)
                            xang -= 2 * M_PI;
                        break;
                    case SDLK_DOWN:
                        xang -= .01;
                        if(xang < 0)
                            xang += 2 * M_PI;
                        break;
                    case SDLK_RIGHT:
                        yang += .01;
                        if(yang > 2*M_PI)
                            yang -= 2 * M_PI;
                        break;
                    case SDLK_LEFT:
                        yang -= .01;
                        if(yang < 0)
                            yang += 2 * M_PI;
                        break;
                    case SDLK_t:
                        xper += .01;
                        break;
                    case SDLK_g:
                        xper -= .01;
                        break;
                    case SDLK_y:
                        yper += .01;
                        break;
                    case SDLK_h:
                        yper -= .01;
                        break; 
                    case SDLK_u:
                        zper += .01;
                        break;
                    case SDLK_j:
                        zper -= .01;
                        break;
                    default:
                        break;
                }
            }

            if (event.type == SDL_KEYUP)
            {
                switch (event.key.keysym.sym)
                {
                default:
                    break;
                }
            }
        }
    }
}

void Setup(){
    objects.push_back({10000, -177, 0, 0, 0, -.03252, 0}); //mass x y z vx vy vz
    objects.push_back({10000, -176, 0, 0, 0, -.02072, 0});
    objects.push_back({25000000, -175, 0, 0, 0, -.06183, 0});
    objects.push_back({100, -75.25, 0, 0, 0, -.09253, 0});
    objects.push_back({10000, -75, 0, 0, 0, -.09433, 0});
    objects.push_back({10000, 0, 0, -75, 0, -.09433, 0});
    objects.push_back({10000, -50, 0, 0, 0, -.11553, 0});
    objects.push_back({10000, -25, 0, 0, 0, -.16339, 0});
    objects.push_back({10000000000, 0, 0, 0, 0, 0, 0});
    
    for(int i = 0; i < 20; i++){
        double mass = static_cast<double>(rand()) / RAND_MAX * 20000 + 5000;
        double dist = static_cast<double>(rand()) / RAND_MAX * 100 - 375;
        double v = sqrt(((6.674 / pow(10, 11)) * (10000000000 + mass)) / abs(dist));
        objects.push_back({mass, dist, 0, 0, 0, -1 * (v * (1.05 - (.1 * static_cast<double>(rand())/RAND_MAX))), 0});
    }
    for(int i = 0; i < 20; i++){
        double mass = static_cast<double>(rand()) / RAND_MAX * 20000 + 5000;
        double dist = static_cast<double>(rand()) / RAND_MAX * 100 + 275;
        double v = sqrt(((6.674 / pow(10, 11)) * (10000000000 + mass)) / abs(dist));
        objects.push_back({mass, dist, 0, 0, 0, v * (1.05 - (.1 * static_cast<double>(rand())/RAND_MAX)), 0});
    }
    for(int i = 0; i < 20; i++){
        double mass = static_cast<double>(rand()) / RAND_MAX * 20000 + 5000;
        double dist = static_cast<double>(rand()) / RAND_MAX * 100 - 375;
        double v = sqrt(((6.674 / pow(10, 11)) * (10000000000 + mass)) / abs(dist));
        objects.push_back({mass, 0, dist, 0, v * (1.05 - (.1 * static_cast<double>(rand())/RAND_MAX)), 0, 0});
    }
    for(int i = 0; i < 20; i++){
        double mass = static_cast<double>(rand()) / RAND_MAX * 20000 + 5000;
        double dist = static_cast<double>(rand()) / RAND_MAX * 100 + 275;
        double v = sqrt(((6.674 / pow(10, 11)) * (10000000000 + mass)) / abs(dist));
        objects.push_back({mass, 0, dist, 0, -1 * v * (1.05 - (.1 * static_cast<double>(rand())/RAND_MAX)), 0, 0});
    }
}

void Convert(){
    pps.clear();
    tps.clear();
    vector<vector<double>> xyz;
    vector<double> temp;
    temp.clear();
    Rots();
    for(int i = 0; i < objects.size(); i++){
        xyz.clear();
        temp.clear();
        for(int j = 1; j < 4; j++){
            temp.push_back(objects[i][j]);
            xyz.push_back(temp);
            temp.clear();
        }
        vector<vector<double>> rotated = MultMatrixs(roty, xyz);
        rotated = MultMatrixs(rotx, rotated);
        rotated = MultMatrixs(rotz, rotated);
        vector<vector<double>> xys = MultMatrixs(projection, rotated);
        for(int j = 0; j < xys.size(); j++){
            temp.push_back(xys[j][0]);
        }
        pps.push_back(temp);
    }
    for(int i = 0; i < trail.size(); i++){
        xyz.clear();
        temp.clear();
        for(int j = 0; j < 3; j++){
            temp.push_back(trail[i][j]);
            xyz.push_back(temp);
            temp.clear();
        }
        vector<vector<double>> rotated = MultMatrixs(roty, xyz);
        rotated = MultMatrixs(rotx, rotated);
        rotated = MultMatrixs(rotz, rotated);
        vector<vector<double>> xys = MultMatrixs(projection, rotated);
        for(int j = 0; j < xys.size(); j++){
            temp.push_back(xys[j][0]);
        }
        tps.push_back(temp);
    }
}

void Draw(){
    for(int i = 0; i < pps.size(); i++){
        if(followObject == i){
            int x = static_cast<int>((pps[i][0] + posx)*zoom + screenWidth/2 - (ceil(objects[i][0] / mpp * zoom) + 1)/2) - ceil(((ceil(objects[i][0] / mpp * zoom) + 1)/2) * 1.25) - 1;
            int y = static_cast<int>((pps[i][1] + posy)*zoom + screenHeight/2 - (ceil(objects[i][0] / mpp * zoom) + 1)/2) - ceil(((ceil(objects[i][0] / mpp * zoom) + 1)/2) * 1.25) - 1;
            pos.x = x;
            pos.y = y;
            pos.w = ceil(objects[i][0] / mpp * zoom) + 1 + 2 * round(static_cast<int>((pps[i][0] + posx)*zoom + screenWidth/2 - (ceil(objects[i][0] / mpp * zoom) + 1)/2) - x);
            pos.h = 1;
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRect(renderer, &pos);
            
            pos.x = x + ceil(objects[i][0] / mpp * zoom) + 1 + 2 * round(static_cast<int>((pps[i][0] + posx)*zoom + screenWidth/2 - (ceil(objects[i][0] / mpp * zoom) + 1)/2) - x) - 1;
            pos.y = y;
            pos.w = 1;
            pos.h = ceil(objects[i][0] / mpp * zoom) + 1 + 2 * round(static_cast<int>((pps[i][1] + posy)*zoom + screenHeight/2 - (ceil(objects[i][0] / mpp * zoom) + 1)/2) - y);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRect(renderer, &pos);

            pos.x = x;
            pos.y = y;
            pos.w = 1;
            pos.h = ceil(objects[i][0] / mpp * zoom) + 1 + 2 * round(static_cast<int>((pps[i][1] + posy)*zoom + screenHeight/2 - (ceil(objects[i][0] / mpp * zoom) + 1)/2) - y);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRect(renderer, &pos);

            pos.x = x;
            pos.y = y + ceil(objects[i][0] / mpp * zoom) + 1 + 2 * round(static_cast<int>((pps[i][1] + posy)*zoom + screenHeight/2 - (ceil(objects[i][0] / mpp * zoom) + 1)/2) - y) - 1;
            pos.w = ceil(objects[i][0] / mpp * zoom) + 1 + 2 * round(static_cast<int>((pps[i][0] + posx)*zoom + screenWidth/2 - (ceil(objects[i][0] / mpp * zoom) + 1)/2) - x);
            pos.h = 1;
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRect(renderer, &pos);
        }
        SDL_Point center = {static_cast<int>(round((pps[i][0] + posx)*zoom + screenWidth/2)), static_cast<int>(round((pps[i][1] + posy)*zoom + screenHeight/2))};
        int radius = static_cast<int>(round((ceil(objects[i][0] / mpp * zoom) + 1)/2));
        SDL_Color color = {255, 255, 255, 255};
        if(center.x >= 0-radius && center.x < screenWidth+radius && center.y >= 0-radius && center.y < screenHeight+radius && radius > 4)
            DrawCircle(center, radius, color);
        else{
            pos.x = static_cast<int>((pps[i][0] + posx)*zoom + screenWidth/2 - (ceil(objects[i][0] / mpp * zoom) + 1)/2);
            pos.y = static_cast<int>((pps[i][1] + posy)*zoom + screenHeight/2 - (ceil(objects[i][0] / mpp * zoom) + 1)/2);
            pos.w = ceil(objects[i][0] / mpp * zoom) + 1;
            pos.h = ceil(objects[i][0] / mpp * zoom) + 1;
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRect(renderer, &pos);
        }
    }
    int x, y, px, py;
    px = 0;
    py = 0;
    for(int i = 0; i < tps.size(); i++){
        x = static_cast<int>((tps[i][0] + posx)*zoom + screenWidth/2);
        y = static_cast<int>((tps[i][1] + posy)*zoom + screenHeight/2);
        if(!(x == px && y == py) || x > screenWidth || x < 0 || y > screenHeight || y < 0){
            pos.x = x;
            pos.y = y;
            pos.w = 1;
            pos.h = 1;
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRect(renderer, &pos);
        }
    }
}

void Simulate(){
    for(int i = 0; i < objects.size(); i++){
        for(int j = 0; j < objects.size(); j++){
            if(i != j){
                if(sqrt(pow(objects[j][1] - objects[i][1], 2) + pow(objects[j][2] - objects[i][2], 2) + pow(objects[j][3] - objects[i][3], 2)) < (objects[i][0] / mpp)/2 + (objects[j][0] / mpp)/2){
                    double vx = (objects[i][0] * objects[i][4] + objects[j][0] * objects[j][4]) / (objects[i][0] + objects[j][0]);
                    double vy = (objects[i][0] * objects[i][5] + objects[j][0] * objects[j][5]) / (objects[i][0] + objects[j][0]);
                    double vz = (objects[i][0] * objects[i][6] + objects[j][0] * objects[j][6]) / (objects[i][0] + objects[j][0]);
                    objects[i][0] += objects[j][0];
                    objects[i][4] = vx;
                    objects[i][5] = vy;
                    objects[i][6] = vz;
                    objects.erase(objects.begin()+j);
                    if(j < i){
                        i--;
                        j--;
                    }
                    else
                        j--;
                }
            }
        }
    }
    for(int i = 0; i < objects.size(); i++){
        int trailLength = 10000 / timeStep;
        if(i == followObject){
            if(trail.size() == trailLength){
                for(int k = 0; k < trail.size()-1; k++){
                        trail[k] = trail[k+1];
                }
                trail[trail.size()-1] = {objects[i][1], objects[i][2], objects[i][3]};
            }
            else
                trail.push_back({objects[i][1], objects[i][2], objects[i][3]});
        }
        for(int j = 0; j < objects.size(); j++){
            if(i != j){
                double Fg = ((6.674 / pow(10, 11)) * objects[i][0] * objects[j][0]) / pow(sqrt(pow(objects[j][1] - objects[i][1], 2) + pow(objects[j][2] - objects[i][2], 2) + pow(objects[j][3] - objects[i][3], 2)), 2) * timeStep;

                double x = objects[j][1] - objects[i][1];
                double y = objects[j][2] - objects[i][2];
                double z = objects[j][3] - objects[i][3];
                double ang = atan2(z, sqrt(pow(x, 2) + pow(y, 2)));
                if(ang < 0)
                    ang += 2*M_PI;
                double vz = Fg * sin(ang);
                double vxy = Fg * cos(ang);
                ang = atan2(y, x);
                if(ang < 0)
                    ang += 2*M_PI;
                double vx = vxy * cos(ang);
                double vy = vxy * sin(ang);
                objects[i][4] += vx / objects[i][0];
                objects[i][5] += vy / objects[i][0];
                objects[i][6] += vz / objects[i][0];
            }
        }
    }
    
    for(int i = 0; i < objects.size(); i++){
        objects[i][1] += objects[i][4] * timeStep;
        objects[i][2] += objects[i][5] * timeStep;
        objects[i][3] += objects[i][6] * timeStep;
    }
}

vector<vector<double>> MultMatrixs(vector<vector<double>> mat1, vector<vector<double>> mat2){
    vector<vector<double>> result;
    vector<double> temp;
    double a = 0;
    for(int j = 0; j < mat1.size(); j++){
        for(int k = 0; k < mat2[0].size(); k++){
            for(int i = 0; i < mat1[j].size(); i++){
                a+= mat1[j][i] * mat2[i][k];
            }
            temp.push_back(a);
            a = 0;
        }
        result.push_back(temp);
        temp.clear();
    }
    return result;
}

void Rots(){
    vector<double> temp;
    rotx.clear();
    roty.clear();
    rotz.clear();

    temp.clear();
    temp.push_back(1);
    temp.push_back(0);
    temp.push_back(0);
    rotx.push_back(temp);
    temp.clear();
    temp.push_back(0);
    temp.push_back(cos(xang));
    temp.push_back(-1*sin(xang));
    rotx.push_back(temp);
    temp.clear();
    temp.push_back(0);
    temp.push_back(sin(xang));
    temp.push_back(cos(xang));
    rotx.push_back(temp);
    temp.clear();

    temp.push_back(cos(yang));
    temp.push_back(0);
    temp.push_back(sin(yang));
    roty.push_back(temp);
    temp.clear();
    temp.push_back(0);
    temp.push_back(1);
    temp.push_back(0);
    roty.push_back(temp);
    temp.clear();
    temp.push_back(-1*sin(yang));
    temp.push_back(0);
    temp.push_back(cos(yang));
    roty.push_back(temp);
    temp.clear();

    temp.push_back(cos(zang));
    temp.push_back(-1*sin(zang));
    temp.push_back(0);
    rotz.push_back(temp);
    temp.clear();
    temp.push_back(sin(zang));
    temp.push_back(cos(zang));
    temp.push_back(0);
    rotz.push_back(temp);
    temp.clear();
    temp.push_back(0);
    temp.push_back(0);
    temp.push_back(1);
    rotz.push_back(temp);
    temp.clear();
}

void DrawCircle(SDL_Point center, int radius, SDL_Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    double x = center.x - radius;
    double y = center.y;
    if(x < 0)
        x = 0;
    double endx = center.x + radius;
    if(endx >= screenWidth)
        endx = screenWidth - 1;
    for(x; x < endx; x+=1){
        double h = sqrt((radius + x - center.x) * (radius - x + center.x)) + center.y;
        pos.x = round(x);
        pos.y = y - (round(h) - y);
        pos.w = 1;
        pos.h = (round(h) - y) * 2;
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &pos);
    }
}