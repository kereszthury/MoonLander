#ifndef MoonLander_PHYSICS_H
#define MoonLander_PHYSICS_H
#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

#include "General.h"
#include "Terrain.h"

typedef struct Object{
    Vector2 acceleration; Vector2 velocity; Vector2 position; float rotation; float mass; Vector2 sumF;
} Object;

void Move(Object *object, float timeElapsed, const GameSettings *gameSettings);
void AddForce(Object *object, Vector2 force);
void RemoveForce(Object *object, Vector2 force);

LanderStatus CheckCollisions(const TerrainData *terrainData, Object *moonLander, const GameSettings *settings, GameStatus *gameStatus);

#endif
