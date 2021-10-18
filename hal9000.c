/*
    James William Fletcher (james@voxdsp.com)
        October 2021

    A simple game using the Faux Renderer:
    https://gist.github.com/mrbid/5e7a272c9225b3d1a0c72c8a26073840

    Requires:
        - sdl_extra.h:  https://gist.github.com/mrbid/f2192b4796f7092520df2b0c7b02d15b
        - vec.h:        https://gist.github.com/mrbid/77a92019e1ab8b86109bf103166bd04e
    
    Compile: clang hal9000.c -Ofast -lSDL2 -lm -o hal9000
*/

#include <SDL2/SDL.h>
#include <stdio.h>
#include <time.h>
#include "sdl_extra.h"

#define NOSSE
#define SEIR_RAND
#include "vec.h"

char name[]="hal9000";
char status[128];

#define WIN_INDENT 33
SDL_Window* w = NULL;
SDL_Surface* s = NULL;
Uint32 w2 = 0, h2 = 0, t = 0;
float fw2 = 0.f, fh2 = 0.f, fw = 0.f, fh = 0.f;
float eyedist = 90.f, rsmag = 0.f;
Uint32 mx=0, my=0;

time_t rs = 0;
vec hal = (vec){0.f, 0.f, 100.f};
Uint8 hal_health = 100;
Uint8 gauss_live = 0;
float gnd = 0.f; // gauss normal distance

Uint8 halt = 0;

typedef struct
{
    vec pos;
    vec dir;
    float hue;
    float sat;
} comet;

#define TOTAL_COMETS 128
comet comets[TOTAL_COMETS];

int cmpfunc(const void* a, const void* b)
{
    return ((comet*)a)->pos.z < ((comet*)b)->pos.z;
}

int collision(vec v)
{
    for(int i = 0; i < TOTAL_COMETS; i++)
        if(comets[i].hue != -0.1337f && vDist(v, comets[i].pos) < 10.f)
            return i;
    return -1;
}

int findGauss()
{
    for(int i = 0; i < TOTAL_COMETS; i++)
        if(comets[i].hue == -0.1337f)
            return i;
    return -1; // bad (should check for this to avoid memory under-run, but, also should never happen so~)
}

void project(vec* v, float eyedist)
{   // credit to Keith Ditchburn (K.J.Ditchburn@tees.ac.uk)
    // for teaching me this trick 11 years ago to date.
    v->x = ((eyedist * (v->x - fw2)) / (v->z + eyedist)) + fw2;
    v->y = ((eyedist * (v->y - fh2)) / (v->z + eyedist)) + fh2;
}

float scale(float z)
{
    return (100.f-z)*1.3f;
}

void resetcomet(int i)
{
    // particle emitter
    comets[i].pos.x = hal.x;
    comets[i].pos.y = hal.y;
    comets[i].pos.z = hal.z;
    vRuvBT(&comets[i].dir);
    comets[i].dir.z = -randf();
    // I'm not normalising so dir now becomes a random velocity (dir+mag).

    comets[i].hue = randf();
    comets[i].sat = randf();
}

void reset()
{
    rs = time(0);
    sprintf(status, "Hal is not happy, projectiles incoming, prepare yourself.");
    gauss_live = 0;
    hal_health = 100;
    hal.x = fw2;
    hal.y = fh2;
    hal.z = 150.f;
    for(int i = 0; i < TOTAL_COMETS; i++)
        resetcomet(i);
    comets[0].hue = -0.1337f; // this will be our gauss
    halt = 0;
}

void setScreenMetrics()
{
    w2 = s->w/2, h2 = s->h/2;
    fw2 = (float)w2, fh2 = (float)h2;
    fw = (float)s->w, fh = (float)s->h;
    rsmag = rsqrtss(w2*w2 + h2*h2); // reciprocal screen magnitude
}

void main_loop()
{
    // input
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_KEYDOWN:
            {
                if(event.key.keysym.sym == SDLK_r)
                    reset();
                
                if(event.key.keysym.sym == SDLK_s && halt == 0)
                    sprintf(status, "Hal Health: %d - Time Taken: %.2f minutes.", hal_health, ((double)(time(0)-rs))/60);
            }
            break;

            case SDL_WINDOWEVENT:
            {
                if(event.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    SDL_FreeSurface(s);
                    s = SDL_GetWindowSurface(w);
                    setScreenMetrics();
                    hal.x = fw2;
                    hal.y = fh2;
                }
            }
            break;

            case SDL_MOUSEMOTION:
            {
                mx = event.motion.x, my = event.motion.y;
            }
            break;

            case SDL_MOUSEBUTTONDOWN:
            {
                if(event.button.button == SDL_BUTTON_LEFT && halt == 0)
                {
                    if(gauss_live == 1)
                    {
                        gauss_live = 0;
                    }
                    else if(gnd > 0.073f)
                    {
                        const int gid = findGauss();
                        comets[gid].dir.x = comets[gid].pos.x - fw2;
                        comets[gid].dir.y = comets[gid].pos.y - fh2;
                        comets[gid].dir.z = 33.f;
                        vNorm(&comets[gid].dir);
                        gauss_live = 1;
                        sprintf(status, "Nice shot.");
                    }
                    else
                        sprintf(status, "You cant shoot directly at hal, you need to bounce off one of his projectiles.");
                }
            }
            break;
            
            case SDL_QUIT:
            {
                drawText(NULL, "*K", 0, 0, 0);
                orb(NULL, 0, 0, 0, 0, -1);
                SDL_FreeSurface(s);
                SDL_DestroyWindow(w);
                exit(0);
            }
            break;
        }
    }

    // time interpolator
    static float dts = 0.01666666754f;
    static Uint32 lt = 0;
    float dt = (t-lt)*dts;
    if(dt > 1.0f)
        dt = 1.0f;
    lt = t;

    // begin render
    SDL_FillRect(s, &s->clip_rect, 0);

    // draw hal
    orb(s, vec_ftoi(hal.x), vec_ftoi(hal.y), 0.f, 1.f, 320);

    // halt if win
    if(halt == 1)
    {
        // overlay
        drawText(s, status, 10, 10, 1);

        // finish render
        SDL_UpdateWindowSurface(w);

        // halt execution
        return;
    }

    // for each comet
    for(int i = 0; i < TOTAL_COMETS; i++)
    {
        // process gauss
        if(comets[i].hue == -0.1337f)
        {
            if(gauss_live == 1)
            {
                vec n;
                vMulS(&n, comets[i].dir, 9.3f * dt);
                vAdd(&comets[i].pos, comets[i].pos, n);
                if(comets[i].pos.z > 100.f)
                {
                    gauss_live = 0;
                    if(vDist(comets[i].pos, hal) < 160.f)
                    {
                        hal_health -= 1;
                        if(hal_health == 0)
                        {
                            sprintf(status, "You killed hal in: %.2f minutes. Press R to start a new game.", ((double)(time(0)-rs))/60);
                            halt = 1;
                        }
                        else
                            sprintf(status, "You hit. Hal integrity: %d.", hal_health);
                    }
                    else
                        sprintf(status, "You missed.");
                }

                const int cid = collision(comets[i].pos);
                if(cid != -1)
                {
                    resetcomet(cid);
                    vRuv(&comets[i].dir);
                    comets[i].dir.z = randf();
                    sprintf(status, "You got a bounce.");
                }

                vec pr;
                vCopy(&pr, comets[i].pos);
                project(&pr, eyedist);

                float sc = scale(comets[i].pos.z);
                if(sc < 0.f)
                sc = 0.f;

                orb(s, vec_ftoi(pr.x), vec_ftoi(pr.y), 0.1f, 1.0f, sc);
                continue;
            }
            else
                continue;
        }
        
        // simulate comet
        vec n;
        vMulS(&n, comets[i].dir, 0.3f * dt);
        vAdd(&comets[i].pos, comets[i].pos, n);
        if(comets[i].pos.z < -4.f)
            resetcomet(i);

        // draw comet
        vec pr;
        vCopy(&pr, comets[i].pos);
        project(&pr, eyedist);
        float sc = scale(vec_ftoi(comets[i].pos.z));
        if(sc < 0.f)
            sc = 0.f;
        orb(s, vec_ftoi(pr.x), vec_ftoi(pr.y), comets[i].hue, comets[i].sat, sc);
    }

    // z-sort comets
    qsort(comets, TOTAL_COMETS, sizeof(comet), cmpfunc);

    // targeting orb / gauss
    if(gauss_live == 0)
    {
        if(mx > WIN_INDENT && mx < s->w-WIN_INDENT && my > WIN_INDENT && my < s->h-WIN_INDENT)
        {
            const float xm = (fw2 - mx);
            const float ym = (fh2 - my);
            const float d = sqrtps(xm*xm + ym*ym);
            gnd = d*rsmag;

            const int gid = findGauss();
            comets[gid].pos.x = mx;
            comets[gid].pos.y = my;
            comets[gid].pos.z = 0.f;

            vec pr;
            vCopy(&pr, comets[gid].pos);
            project(&pr, eyedist);
            orb(s, vec_ftoi(pr.x), vec_ftoi(pr.y), 0.1f, gnd, 100.f+(100.f*gnd));
        }
    }

    // overlay
    drawText(s, status, 10, 10, 1);

    // finish render
    SDL_UpdateWindowSurface(w);
}

int main(int argc, char** argv)
{
    printf("hal9000 by James William Fletcher (james@voxdsp.com)\n\n");
    printf("hal9000 has gone haywire, he is firing plasma at you, luckily for you the plasma is harmless but hal does not know this. You have to disable hal but you cannot shoot directly at him! You have to shoot your plasma, at hal's plasma to make it bounce off in the direction of hal. You must destroy hal as fast as you can, the winner is the player with the lowest time taken.. or if playing alone you should aim to beat your last best time.\n\n");
    printf("Press S to see your time taken and hal's current health.\n");
    printf("Press R to reset the game.\n\n");

    srand(time(0));
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS) < 0)
    {
        fprintf(stderr, "ERROR: SDL_Init(): %s\n", SDL_GetError());
        return 1;
    }

    w = SDL_CreateWindow(name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 768, SDL_WINDOW_SHOWN);
    s = SDL_GetWindowSurface(w);
    setScreenMetrics();
    SDL_ShowCursor(0);
    SDL_Surface* icon = SDL_RGBA32Surface(16, 16);
    orb(icon, 7, 7, 0.f, 1.f, 50);
    SDL_SetWindowIcon(w, icon);
    SDL_FreeSurface(icon);
    reset();
    
    Uint32 lt = SDL_GetTicks() + 16000, fc = 0;
    while(1)
    {
        t = SDL_GetTicks();

        main_loop();
        SDL_Delay(16); // just over 60fps (more like 50 fps with some occasional frame drops ~40-50)
        
        fc++;
        if(t > lt)
        {
            printf("FPS: %u\n", fc/16);
            fc = 0;
            lt = t + 16000;
        }
    }

    return 0;
}
