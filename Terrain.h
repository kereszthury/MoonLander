#ifndef MoonLander_TERRAIN_H
#define MoonLander_TERRAIN_H

#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

#include "General.h"

#define SmallMapSize 14
#define MediumMapSize 22
#define LargeMapSize 40

typedef struct LandingSite{
    Vector2 start, end;
} LandingSite;

typedef struct TerrainData{
    Vector2 *terrainPoints, *backdropPoints; LandingSite *landingSites;
} TerrainData;

int GetTerrainHeight(int x, const Vector2* terrainPoints);
bool Collision(Vector2 point, const Vector2 *terrainPoints, int screenWidth);
void InitTerrain(TerrainData *terrainData, GameSettings *gameSettings);
void FreeTerrain(TerrainData *terrainData);

#endif
