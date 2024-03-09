#include <SDL.h>
#include <SDL_image.h>

#include "Lander.h"
#include "debugmalloc.h"

void ReStartLander(Lander *lander, const GameSettings *gameSettings, LanderMode mode){
    Object obj = {{0, 0}, {20,0},
                  {-8 * gameSettings->scale, 8 * gameSettings->scale},
                  90,  (LanderWeight + FuelWeight), {0, 0}};
    lander->obj = obj;
    lander->fuelPercent = 1.0f;
    lander->mode = mode;
}

Lander* InitLander(SDL_Renderer *renderer, const GameSettings* gameSettings){
    Lander *lander = (Lander*) malloc(sizeof (Lander));
    SDL_Texture *texture = IMG_LoadTexture(renderer, "probe.png");
    if(texture == NULL) {
        SDL_Log("Cannot load lander texture: %s", SDL_GetError());
        exit(42);
    }
    lander->texture = texture;

    ReStartLander(lander, gameSettings, Display);

    return lander;
}

void RotateLander(Lander *lander, int rotateDirection){
    if (lander->obj.rotation < -180) lander->obj.rotation += 360;
    else if (lander->obj.rotation > 180) lander->obj.rotation -= 360;
    lander->obj.rotation += (1.5f * rotateDirection);
}

void UpdateLander(Lander *lander, const GameSettings *gameSettings, GameStatus *gameStatus, float timeElapsed, DataList **landData, ReplayData *replayData){
    lander->obj.mass = (LanderWeight + FuelWeight * lander->fuelPercent);

    if (lander->mode == Play){
        RotateLander(lander, gameStatus->rotateDirection);

        if (gameStatus->engineOn && lander->fuelPercent > 0) {
            Vector2 thrustForce = {Thrust * sinf(lander->obj.rotation / 180 * PI),
                                   -Thrust * cosf(lander->obj.rotation / 180 * PI)};
            AddForce(&lander->obj, thrustForce);
            lander->fuelPercent -= timeElapsed / 20 * (1 + gameSettings->difficulty);
            if (lander->fuelPercent < 0) lander->fuelPercent = 0;
        }

        Vector2 gravity = {0, Gravity * (LanderWeight + lander->fuelPercent * FuelWeight)};
        AddForce(&lander->obj, gravity);
    }
    else if (lander->mode != Replay){
        float initPositionX = -8 * gameSettings->scale;
        float initPositionY = 8 * gameSettings->scale;
        lander->obj.rotation = 90;
        if (lander->obj.position.x > gameSettings->width || lander->obj.position.y > initPositionY){
            lander->obj.position.x = 2 * initPositionX;
            lander->obj.position.y = initPositionY;
            lander->obj.velocity = (Vector2) {25,0};
        }
        if (lander->mode == SetUpPlay || lander->mode == SetUpReplay) {
            if (lander->obj.position.x > 0){
                Vector2 acceleration = {100 * LanderWeight, 0};
                AddForce(&lander->obj, acceleration);
            }
            else if (lander->obj.position.x > initPositionX){
                if (lander->mode == SetUpPlay) ReStartLander(lander, gameSettings, Play);
                else ReStartLander(lander, &replayData->gameSettings, Replay);
            }
        }
    }
    else if (*landData != NULL && gameStatus->landerStatus == Landing) {
        LandData data = ReadOutData(landData);
        lander->obj = data.lander;
        gameStatus->engineOn = data.engineOn;
    }

    if (lander->mode != Replay) {
        Move(&lander->obj, timeElapsed, gameSettings);
        RemoveForce(&lander->obj, lander->obj.sumF);
    }
}