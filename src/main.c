#include "vec.h"
#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define swapp(x, y) { \
    SDL_Point tmp = x; \
    x = y; \
    y = tmp; }

typedef struct
{
    float x, slope;
} RTI;

Vec3f barycentric_coefficients(Vec3f points[3], Vec3f p)
{
    Vec3f ba = vec_sub(points[1], points[0]);
    Vec3f ca = vec_sub(points[2], points[0]);

    Vec3f pa = vec_sub(p, points[0]);
    Vec3f pb = vec_sub(p, points[1]);
    Vec3f pc = vec_sub(p, points[2]);

    Vec3f n = vec_cross(ba, ca);
    float s_abc = vec_len(n);
    n = vec_normalize(n);

    Vec3f pbc = vec_cross(pb, pc);
    float s_pbc = vec_dot(n, pbc);

    Vec3f apc = vec_cross(pa, ca);
    float s_apc = vec_dot(n, apc);

    Vec3f abp = vec_cross(ba, pa);
    float s_abp = vec_dot(n, abp);

    return (Vec3f){
        s_pbc / s_abc,
        s_apc / s_abc,
        s_abp / s_abc
    };
}

void fill_edges(int top, int bot, RTI *l1, RTI *l2, Vec3f pts[3], Vec3f texcoords[3], SDL_Texture *tex, SDL_Renderer *rend)
{
    for (int y = top; y < bot; ++y)
    {
        int min = roundf(l1->x > l2->x ? l2->x : l1->x);
        int max = roundf(l1->x < l2->x ? l2->x : l1->x);

        for (int i = min; i < max; ++i)
        {
            if (i < 0)
            {
                i = 0;
                continue;
            }

            if (i >= 800) break;

            Vec3f p = { i, y, 0 };
            Vec3f coeffs = barycentric_coefficients(pts, p);

            Vec3f uv = vec_addv(vec_addv(vec_mulf(texcoords[2], coeffs.z), vec_mulf(texcoords[1], coeffs.y)), vec_mulf(texcoords[0], coeffs.x));
            /* int ix = uv.x * 100; */
            /* int iy = uv.y * 100; */

            /* if (ix % 5 == 0) */
            /*     SDL_SetRenderDrawColor(rend, 0, 0, 0, 255); */
            /* else */
            /*     SDL_SetRenderDrawColor(rend, 255, 255, 255, 255); */

            SDL_Rect src = { uv.x * 100.f, uv.y * 100.f, 1, 1 };
            SDL_Rect dst = { i, y, 1, 1 };
            SDL_RenderCopy(rend, tex, &src, &dst);
        }

        l1->x += 1.f / l1->slope;
        l2->x += 1.f / l2->slope;
    }
}

void draw_tri(SDL_Renderer *rend, SDL_Point p[3], Vec3f texcoords[3], SDL_Texture *tex)
{
    SDL_Point p0 = p[0], p1 = p[1], p2 = p[2];

    if (p0.y > p1.y) swapp(p0, p1);
    if (p0.y > p2.y) swapp(p0, p2);
    if (p1.y > p2.y) swapp(p1, p2);

    SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
    /* SDL_RenderDrawLine(rend, p0.x, p0.y, p1.x, p1.y); */
    /* SDL_RenderDrawLine(rend, p1.x, p1.y, p2.x, p2.y); */
    /* SDL_RenderDrawLine(rend, p2.x, p2.y, p0.x, p0.y); */

    RTI r02 = { p0.x, (float)(p2.y - p0.y) / (p2.x - p0.x) };
    RTI r01 = { p0.x, (float)(p1.y - p0.y) / (p1.x - p0.x) };
    RTI r12 = { p1.x, (float)(p2.y - p1.y) / (p2.x - p1.x) };

    Vec3f pts[3] = {
        (Vec3f){ p0.x, p0.y, 0 },
        (Vec3f){ p1.x, p1.y, 0 },
        (Vec3f){ p2.x, p2.y, 0 }
    };

    fill_edges(p0.y, p1.y, &r02, &r01, pts, texcoords, tex, rend);
    fill_edges(p1.y, p2.y, &r02, &r12, pts, texcoords, tex, rend);
}

void mainloop(SDL_Window *win, SDL_Renderer *rend)
{
    bool running = true;
    SDL_Event evt;

    SDL_Point tri[3] = {
        (SDL_Point){ 300, 300 },
        (SDL_Point){ 300, 600 },
        (SDL_Point){ 500, 600 }
    };

    Vec3f texcoords[3] = {
        (Vec3f){ 0.f, 0.f, 0.f },
        (Vec3f){ 0.f, 1.f, 0.f },
        (Vec3f){ 1.f, 1.f, 0.f }
    };

    SDL_Point tri2[3] = {
        (SDL_Point){ 400, 300 },
        (SDL_Point){ 600, 300 },
        (SDL_Point){ 600, 600 }
    };

    Vec3f texcoords2[3] = {
        (Vec3f){ 0.f, 0.f, 0.f },
        (Vec3f){ 1.f, 0.f, 0.f },
        (Vec3f){ 1.f, 1.f, 0.f }
    };

    SDL_Texture *image = IMG_LoadTexture(rend, "image.png");

    while (running)
    {
        while (SDL_PollEvent(&evt))
        {
            switch (evt.type)
            {
            case SDL_QUIT:
                running = false;
                break;
            }
        }

        SDL_RenderClear(rend);

        draw_tri(rend, tri, texcoords, image);
        draw_tri(rend, tri2, texcoords2, image);

        SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
        SDL_RenderPresent(rend);
    }
}

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window *w = SDL_CreateWindow("Texture mapping", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 800, SDL_WINDOW_SHOWN);
    SDL_Renderer *r = SDL_CreateRenderer(w, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    mainloop(w, r);

    SDL_DestroyRenderer(r);
    SDL_DestroyWindow(w);

    SDL_Quit();
}

