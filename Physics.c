#include "Physics.h"

#include "debugmalloc.h"

void AddForce(Object *object, Vector2 force){
    object->sumF.x += force.x;
    object->sumF.y += force.y;
}

void SetAcceleration(Object *object){
    object->acceleration.x = object->sumF.x / object->mass;
    object->acceleration.y = object->sumF.y / object->mass;
}

void SetVelocity(Object *object, float timeElapsed){
    object->velocity.x += object->acceleration.x * timeElapsed;
    object->velocity.y += object->acceleration.y * timeElapsed;
}

void Move(Object *object, float timeElapsed, const GameSettings *gameSettings){
    SetAcceleration(object);
    if (object->acceleration.x != 0 || object->acceleration.y != 0) SetVelocity(object, timeElapsed);

    object->position.x += object->velocity.x * gameSettings->scale / 100;
    object->position.y += object->velocity.y * gameSettings->scale / 100;
}

void RemoveForce(Object *object, Vector2 force){
    Vector2 neg = {-force.x, -force.y};
    AddForce(object, neg);
}

bool Collision(Vector2 point, const Vector2 *terrainPoints, int screenWidth){
    if (point.x < 1 || point.x > screenWidth ) return false;
    if (GetTerrainHeight(point.x, terrainPoints) < point.y) return true;
    return false;
}

LanderStatus CheckCollisions(const TerrainData *terrainData, Object *moonLander, const GameSettings *settings, GameStatus *gameStatus){
    Vector2 collisionPoints[] = {{20, 700}, {230, 700}, {570, 700}, {790, 700},
                                 {50, 530}, {750, 530}, {180, 330}, {620, 330},
                                 {180, 130}, {620, 130}, {300, 0}, {500, 0}, {400, 650}};
    // Check collisions for the 13 possible collision points
    for (int i = 0; i < 13; i++) {
        float difficultyModifier = 1;
        if (settings->difficulty == Easy) difficultyModifier = 1.25f;
        else if (settings->difficulty == Hard) difficultyModifier = .75f;

        gameStatus->xSpeedOk = moonLander->velocity.x < 2 * difficultyModifier && moonLander->velocity.x > -2 * difficultyModifier;
        gameStatus->ySpeedOk = moonLander->velocity.y <= 5 * difficultyModifier;
        gameStatus->rotationOk = moonLander->rotation < 8 * difficultyModifier && moonLander->rotation > -8 * difficultyModifier;

        // Calculate the point's absolute coordinate
        float x = (collisionPoints[i].x / 100 * settings->scale - 4 * settings->scale) * cosf(moonLander->rotation / 180 * PI)
                - (collisionPoints[i].y / 100 * settings->scale - 3.5 * settings->scale) * sinf(moonLander->rotation / 180 * PI)
                + moonLander->position.x + 4 * settings->scale;
        float y = (collisionPoints[i].x / 100 * settings->scale - 4 * settings->scale) * sinf(moonLander->rotation / 180 * PI)
                + (collisionPoints[i].y / 100 * settings->scale - 3.5 * settings->scale) * cosf(moonLander->rotation / 180 * PI)
                + moonLander->position.y + 3.5 * settings->scale;
        Vector2 point = {x, y}; // The lander's resolution is 800x700, rotated at the centre -> 4*scale, 3.5*scale
        bool collide = Collision(point, terrainData->terrainPoints, settings->width);

        if (collide){
            int landSiteCount = 3 - settings->difficulty;
            for (int j = 0; j < landSiteCount; j++) {
                if (moonLander->position.x >= terrainData->landingSites[j].start.x && moonLander->position.x + 7 * settings->scale <= terrainData->landingSites[j].end.x){
                    if (!gameStatus->rotationOk) return WrongAngleCrash;
                    if (!gameStatus->ySpeedOk || !gameStatus->xSpeedOk) return HighSpeedCrash;
                    moonLander->rotation = 0;
                    moonLander->position.y = terrainData->landingSites[j].start.y - 7 * settings->scale;
                    return SuccessfullyLanded;
                }
            }
            return NotSiteCrash;
        }
        else if(moonLander->position.x + settings->scale * 8 < 0 || moonLander->position.x > settings->width) return OutOfBounds;
    }
    return Landing;
}
