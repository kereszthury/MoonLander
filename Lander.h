#ifndef MoonLander_LANDER_H
#define MoonLander_LANDER_H

#include "General.h"
#include "Physics.h"
#include "FileManager.h"

typedef enum LanderModes { Display, SetUpPlay, Play, SetUpReplay, Replay } LanderMode;

typedef struct Lander {
    Object obj; SDL_Texture *texture; float fuelPercent; LanderMode mode;
} Lander;

void ReStartLander(Lander *lander, const GameSettings *gameSettings, LanderMode mode);
Lander* InitLander(SDL_Renderer *renderer, const GameSettings* gameSettings);
void UpdateLander(Lander *lander, const GameSettings *gameSettings, GameStatus *gameStatus, float timeElapsed, DataList **landData, ReplayData *replayData);

#endif