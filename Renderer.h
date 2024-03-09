#ifndef MoonLander_RENDERER_H
#define MoonLander_RENDERER_H

#include "General.h"
#include "Terrain.h"
#include "Physics.h"
#include "Particles.h"
#include "Lander.h"
#include "UI.h"

void RenderScreen(SDL_Renderer *renderer, const GameSettings *settings, const TerrainData *terrainData, int frame, Lander *lander, const GameStatus *gameStatus, ParticleList *particles, ParticleList *stars, Panel panels[], TTF_Font *font);

#endif
