#ifndef MoonLander_GENERAL_H
#define MoonLander_GENERAL_H

#define LanderWeight 2134 /* kg */
#define FuelWeight 4000 /* kg */
#define Gravity 1.62 /* m/s2 */
#define Thrust 42000 /* N */
#define PI 3.1415926536

#include <stdbool.h>

typedef enum Difficulty { Easy = 0, Normal = 1, Hard = 2 } Difficulty;

typedef struct Vector2 {
    float x, y;
} Vector2;

typedef struct GameSettings{
    int width, height, terrainResolution; float scale; Difficulty difficulty; int highScore;
} GameSettings;

typedef enum LanderStatus {Landing = 0, SuccessfullyLanded = 1, OutOfBounds = 2, NotSiteCrash = 3, WrongAngleCrash = 4, HighSpeedCrash = 5} LanderStatus;
typedef struct GameStatus{
    bool paused, engineOn, xSpeedOk, ySpeedOk, rotationOk; int rotateDirection; LanderStatus landerStatus;
} GameStatus;

typedef struct MouseStatus{
    bool clicked, buttonPressed; int x, y;
} MouseStatus;

typedef enum TextIdentifiers { ScoreText, XSpeedText, YSpeedText, RotationText, FuelText, TimeText, HighScoreText, NumberOfTextIDs} TextIdentifier;

#endif
