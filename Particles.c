#include "Particles.h"

#include "debugmalloc.h"

// Returns true if particle has decayed
bool UpdateParticle(Particle *particle, const GameSettings *gameSettings, float timeElapsed){
    Move(&particle->object, timeElapsed, gameSettings);
    particle->existence += timeElapsed;
    if (particle->decayTime >= 0){
        Uint8 alpha = (Uint8) (255 * (1 - powf(particle->existence / particle->decayTime, 2)));
        if (alpha < 0) alpha = 0;
        particle->color.a = alpha;
    }

    Vector2 position = particle->object.position;
    if (particle->decayTime < 0 && (position.x < 0 || position.y < 0
            || position.x > gameSettings->width || position.y > gameSettings->height)) return true;
    if (particle->decayTime >= 0) return particle->existence > particle->decayTime;
    return false;
}

int ParticleListLength(const ParticleList *particleList){
    if (particleList == NULL) return 0;
    int length = 1;
    while (particleList->next != NULL){
        length++;
        particleList = particleList->next;
    }
    return length;
}

ParticleList* GetParticle(ParticleList *particleList, int index){
    if (index == 0) return particleList;
    return GetParticle(particleList->next, index - 1);
}

void AddParticle(ParticleList **particleList, Particle p){
    ParticleList *newElement = (ParticleList*) malloc(sizeof (ParticleList));
    newElement->particle = p;
    newElement->next = NULL;
    if (*particleList == NULL) *particleList = newElement;
    else{
        ParticleList *lastElement = GetParticle(*particleList, ParticleListLength(*particleList) - 1);
        lastElement->next = newElement;
    }
}

void RemoveParticle(ParticleList **particleList, int index){
    ParticleList *finder = *particleList;
    ParticleList *previous = NULL;
    for (int i = 0; i < index; ++i) {
        if (finder == NULL) return;
        previous = finder;
        if (finder->next == NULL) return;
        finder = finder->next;
    }
    if (index < 1){
        if(ParticleListLength(*particleList) == 1) *particleList = NULL;
        else *particleList = (*particleList)->next;
    }
    else if (finder != NULL){
        if (previous != NULL) previous->next = finder->next;
        else *particleList = finder->next;
    }
    free(finder);
}

void UpdateParticleList(ParticleList **particleList, const GameSettings *gameSettings, float timeElapsed){
    for (int i = 0; i < ParticleListLength(*particleList); ++i) {
        if(UpdateParticle(&GetParticle(*particleList, i)->particle, gameSettings, timeElapsed)) {
            RemoveParticle(particleList, i);
        }
    }
}

void ClearParticleList(ParticleList *particleList){
    if (particleList != NULL && particleList->next != NULL) ClearParticleList(particleList->next);
    free(particleList);
}

void CreateStars(int stars, const GameSettings *gameSettings, ParticleList **particleList, bool spawnOnEdge){
    if (stars < 1) return;
    for (int i = 0; i < stars; ++i) {
        Particle p = {(Object){{0, 0}, {10,0},
                               {0, rand() % gameSettings->height},
                               0,  1, {0, 0}},
                      (SDL_Color){255, 255, 255, rand() % 155 + 100},
                      -1, 0, 2};
        if (!spawnOnEdge) p.object.position.x = rand() % gameSettings->width;
        AddParticle(particleList, p);
    }
}

void SpawnExplosion(int particles, ParticleList **particleList, Vector2 position){
    if (particles < 1) return;
    for (int i = 0; i < particles; ++i) {
        Particle p = {(Object){{0, 0}, {rand() % 10 * cos(rand() % 180 / PI),rand() % 10 * sin(rand() % 180 / PI)},
                               position,
                               0,  1, {0, 0}},
                      (SDL_Color){175 + rand() % 80, 75 + rand() % 75, 0, 255},
                      (rand() % 10 + 1) / 2, 0, 5};
        AddParticle(particleList, p);
    }
}

void SpawnThrust(int particles, ParticleList **particleList, float landerScale, const Lander *lander, int maxAngle){
    if (particles < 1) return;
    for (int i = 0; i < particles; ++i) {
        float x = -3 * landerScale * sinf(lander->obj.rotation / 180 * PI) + lander->obj.position.x + 4 * landerScale;
        float y = 3 * landerScale * cosf(lander->obj.rotation / 180 * PI) + lander->obj.position.y + 3.5 * landerScale;
        Particle p = {(Object){0, 0,
                               lander->obj.velocity.x + (10 + rand() % 10) * cos((lander->obj.rotation + 90 + rand() % (2 * maxAngle) - maxAngle) / 180 * PI),
                               lander->obj.velocity.y + (10 + rand() % 10) * sin((lander->obj.rotation + 90 + rand() % (2 * maxAngle) - maxAngle) / 180 * PI),
                               (Vector2) {x, y},
                               0,  1, {0, 0}},
                      (SDL_Color){175 + rand() % 80, 75 + rand() % 75, 0, 255},
                      (rand() % 5 + 1) / 2, 0, 3};
        AddParticle(particleList, p);
    }
}