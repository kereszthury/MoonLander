#ifndef MoonLander_PARTICLES_H
#define MoonLander_PARTICLES_H

#include "Physics.h"
#include "Lander.h"

typedef struct Particle{
    Object object; SDL_Color color; float decayTime; float existence; int radius;
} Particle;

typedef struct ParticleList{
    Particle particle; struct ParticleList *next;
} ParticleList;

int ParticleListLength(const ParticleList *particleList);
ParticleList* GetParticle(ParticleList *particleList, int index);
void AddParticle(ParticleList **particleList, Particle p);
void RemoveParticle(ParticleList **particleList, int index);
void UpdateParticleList(ParticleList **particleList, const GameSettings *gameSettings, float timeElapsed);
void ClearParticleList(ParticleList *particleList);
void CreateStars(int stars, const GameSettings *gameSettings, ParticleList **particleList, bool spawnOnEdge);
void SpawnExplosion(int particles, ParticleList **particleList, Vector2 position);
void SpawnThrust(int particles, ParticleList **particleList, float landerScale, const Lander *lander, int maxAngle);

#endif
