#include "fnae.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX_ASSETS 600
static Texture images[MAX_ASSETS];
static SDL_Renderer *renderer;

int assets_init(SDL_Renderer *r) {
    renderer = r;
    for (int i=0;i<MAX_ASSETS;i++) images[i].tex=NULL;
    return 0;
}

SDL_Texture *asset_image(int id) {
    if (id < 0 || id >= MAX_ASSETS) return NULL;
    if (images[id].tex) return images[id].tex;
    char path[512];
    snprintf(path,sizeof(path),"assets/images/%d.png",id);
    SDL_Surface *s = IMG_Load(path);
    if (!s) return NULL;
    images[id].w=s->w; images[id].h=s->h;
    images[id].tex=SDL_CreateTextureFromSurface(renderer,s);
    SDL_FreeSurface(s);
    return images[id].tex;
}

void draw_asset(SDL_Renderer *r, int id, int fit) {
    SDL_Texture *t=asset_image(id); if(!t) return;
    int w,h; SDL_QueryTexture(t,NULL,NULL,&w,&h);
    SDL_Rect dst={0,0,w,h};
    if(fit){ int rw,rh; SDL_GetRendererOutputSize(r,&rw,&rh); dst=(SDL_Rect){0,0,rw,rh}; }
    SDL_RenderCopy(r,t,NULL,&dst);
}

void assets_shutdown(void){
    for(int i=0;i<MAX_ASSETS;i++) if(images[i].tex) SDL_DestroyTexture(images[i].tex);
}
