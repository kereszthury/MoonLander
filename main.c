#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <SDL_image.h>
#include <stdbool.h>
#include <SDL_ttf.h>
#include <dirent.h>

#include "General.h"
#include "Renderer.h"
#include "Terrain.h"
#include "Physics.h"
#include "Particles.h"
#include "Lander.h"
#include "UI.h"
#include "FileManager.h"

#include "debugmalloc.h"

Uint32 timer(Uint32 ms, void *param) {
    SDL_Event ev;
    ev.type = SDL_USEREVENT;
    SDL_PushEvent(&ev);
    return ms;
}

void sdl_init(char const *header, int width, int height, SDL_Window **pWindow, SDL_Renderer **pRenderer);
void SetWindowFullscreen(SDL_Window *window, GameSettings *gameSettings);
void HandleButtonEvents(Button *button, GameSettings *gameSettings, View *currentView, Lander *lander, TerrainData *terrainData, bool *quit, int *replayCount, int *replayPage, Panel *replayPanel, ReplayData *replayData, DataList **landData, TerrainData *replayTerrain);
void HandleInput(SDL_Event event, GameStatus *gameStatus, MouseStatus *mouseStatus, View *currentView);
void SlidePanels(Panel panels[], View currentView);
void AppendNumericData(char **destination, char string[], float value, int digits);
void UpdateDisplayInfo(char **texts, Lander *lander, Panel *panels, float time);
void SetEndText(const GameStatus *gameStatus, Panel *panels, char *scoreText);
int CalculateScore(const GameSettings *gameSettings, float time, float fuel);
void ClearReplayData(DataList **dataList);
void ClearUp(ParticleList* particleList, ParticleList* stars, TerrainData* terrainData, TerrainData *replayTerrain, SDL_TimerID timerId, Lander* lander, Panel panels[], TTF_Font *font, char *texts[], DataList **dataList, char *replayNames);

int main(int argc, char *argv[]) {
    // Create window
    SDL_Window *window;
    SDL_Renderer *renderer;

    // Init settings, make fullscreen window
    GameSettings gameSettings = ReadInSettings();
    sdl_init("Moon Lander", gameSettings.width, gameSettings.height, &window, &renderer);
    SetWindowFullscreen(window, &gameSettings);
    gameSettings.scale = gameSettings.width / 24000.0f * 100 / gameSettings.terrainResolution * 20;

    srand(time(NULL));
    SDL_TimerID timerID = SDL_AddTimer(20, timer, NULL);
    MouseStatus mouseStatus = { false, false, 0, 0};

    // Init Lander
    Lander *lander = InitLander(renderer, &gameSettings);

    // Init terrain
    TerrainData terrainData = { NULL, NULL, NULL };
    InitTerrain(&terrainData, &gameSettings);

    // Init particles
    ParticleList *particles = NULL;
    ParticleList *stars = NULL;
    CreateStars(100, &gameSettings, &stars, false);

    // Init ui elements
    Panel panels[UIPanels];
    InitPanels(&gameSettings, panels);

    // Load in font
    TTF_Init();
    TTF_Font *font = TTF_OpenFont("Anita semi square.ttf", 940 / 24000.0f * gameSettings.width);
    if (!font) {
        SDL_Log("Cannot open font: %s\n", TTF_GetError());
        exit(69);
    }

    // Init GameStatus
    GameStatus gameStatus = {false, false, false, false, false, 0, Landing};
    View currentView = MenuView;
    Button *pressedButton = NULL;
    int score;
    float gameTime = 0; // Used for score calculation and replays
    // All texts have to be initialised NULL for AppendNumericData method
    char *texts[NumberOfTextIDs] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL };

    // Init replay management
    ReplayData replayData;
    TerrainData replayTerrain = { NULL, NULL, NULL };
    DataList *dataList = NULL;
    char *replayNames = NULL;
    int replayCount;
    int replayPage = 1;

    int frame = 0;
    SDL_Event event;
    bool quit = false;
    while (SDL_WaitEvent(&event) && !quit) {
        SDL_GetMouseState(&mouseStatus.x, &mouseStatus.y);
        switch (event.type) {
            case SDL_USEREVENT:
                SlidePanels(panels, currentView);
                pressedButton = UpdatePanels(panels, &gameSettings, &mouseStatus);

                HandleButtonEvents(pressedButton, &gameSettings, &currentView, lander, &terrainData, &quit, &replayCount, &replayPage, &panels[ReplaysView], &replayData, &dataList, &replayTerrain);

                if (currentView == SetUpView) {
                    if (lander->mode == Play) {
                        currentView = GameView;
                        ClearReplayData(&dataList);
                    }
                    else if (lander->mode == Replay) {
                        currentView = ReplayView;
                    }
                    gameTime = 0;
                }
                if (currentView == MenuView) gameStatus.engineOn = false;

                GameSettings *currentSettings = lander->mode != Replay ? &gameSettings : &replayData.gameSettings;
                TerrainData *currentTerrain = lander->mode != Replay ? &terrainData : &replayTerrain;
                gameStatus.paused = currentView == PauseView;

                if (!gameStatus.paused && (gameStatus.landerStatus == Landing || lander->mode != Play)){
                    UpdateLander(lander, &gameSettings, &gameStatus, .02f, &dataList, &replayData);
                    UpdateDisplayInfo(texts, lander, panels, gameTime);
                    if (lander->mode != Display) gameTime += .02f;
                    if (lander->mode == Play){
                        LandData data = {lander->obj, gameStatus.engineOn && lander->fuelPercent > 0};
                        AddData(&dataList, data);
                    }

                    gameStatus.landerStatus = CheckCollisions(currentTerrain, &lander->obj, currentSettings, &gameStatus);
                    if (gameStatus.engineOn && lander->fuelPercent > 0 || lander->mode == SetUpPlay || lander->mode == SetUpReplay) {
                        SpawnThrust(3, &particles, currentSettings->scale, lander, 10);
                    }
                    if (gameStatus.landerStatus != Landing && lander->mode == Play){
                        if (gameStatus.landerStatus >= 2) {
                            panels[EndView].texts.uiText[0].text = "Failure";
                            if (gameStatus.landerStatus != 2) {
                                Vector2 centerOfLander = {lander->obj.position.x + gameSettings.scale * 4,
                                                          lander->obj.position.y + gameSettings.scale * 3.5f};
                                SpawnExplosion(250, &particles, centerOfLander);
                            }
                        }
                        else {
                            panels[EndView].texts.uiText[0].text = "Success";
                            score = CalculateScore(&gameSettings, gameTime, lander->fuelPercent);
                            if (score > gameSettings.highScore) gameSettings.highScore = score;
                            AppendNumericData(&texts[ScoreText], "Score: ", (float) score, 3);
                        }

                        char replayName[21];
                        gcvt(time(NULL), 20, replayName);
                        SaveReplay(replayName, (ReplayData) {gameSettings, gameStatus.landerStatus == SuccessfullyLanded }, &terrainData, &dataList);

                        SetEndText(&gameStatus, panels, texts[ScoreText]);
                        currentView = EndView;
                    }
                    if (gameStatus.landerStatus != Landing && lander->mode == Replay && !replayData.finish){
                        if (gameStatus.landerStatus != 2) {
                            Vector2 centerOfLander = {lander->obj.position.x + gameSettings.scale * 4,
                                                      lander->obj.position.y + gameSettings.scale * 3.5f};
                            SpawnExplosion(250, &particles, centerOfLander);
                        }
                        replayData.finish = true;
                    }
                }

                if (currentView == MenuView){
                    AppendNumericData(&texts[HighScoreText], "Highscore: ", (float) gameSettings.highScore, 5);
                    panels[MenuView].texts.uiText[0].text = texts[HighScoreText];
                }

                frame = (frame + 1) % 50;
                if(frame % 10 < 1 && !gameStatus.paused) CreateStars(rand() % 2, &gameSettings, &stars, true);
                if(!gameStatus.paused || gameStatus.landerStatus != Landing) {
                    UpdateParticleList(&stars, currentSettings, 0.02f);
                    UpdateParticleList(&particles, currentSettings, 0.02f);
                }

                mouseStatus.buttonPressed = false;
                RenderScreen(renderer, currentSettings, currentTerrain, frame, lander, &gameStatus, particles, stars, panels, font);
                break;
            case SDL_KEYUP:
            case SDL_KEYDOWN:
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                HandleInput(event, &gameStatus, &mouseStatus, &currentView);
                break;
            case SDL_QUIT: quit = true;
                break;
        }
    }

    SaveSettings(gameSettings);

    // Free up memory
    ClearUp(particles, stars, &terrainData, &replayTerrain, timerID, lander, panels, font, texts, &dataList, replayNames);
    SDL_Quit();

    return 0;
}

void RefreshReplayView(int page, Panel *replayPanel){
    FILE *file = fopen("replayFiles.txt", "rt");

    for (int i = 0; i < (page - 1) * 9; ++i) {
        char read[257];
        fscanf(file, "%s", read);
    }

    for (int i = 0; i < 9; ++i) {
        free(replayPanel->buttons.button[i].text);
        char read[257];
        char *text = NULL;
        if (fscanf(file, "%s", read) == 1){
            text = (char*) malloc(sizeof(char) * (strlen(read) + 1));
            strcpy(text, read);
            replayPanel->buttons.button[i].text = text;
        }
        else replayPanel->buttons.button[i].text = NULL;
    }

    fclose(file);

    free(replayPanel->texts.uiText[1].text);
    char *pageText = (char*) malloc(sizeof(char) * 11);
    gcvt((float) page, 5, pageText);
    replayPanel->texts.uiText[1].text = pageText;
}

void AppendNumericData(char **destination, char string[], float value, int digits){
    free(*destination);
    int numLength = 11;
    char number[numLength];
    gcvt(value, digits, number);
    *destination = (char *) malloc((strlen(string) + numLength) * sizeof (char));
    strcpy(*destination, string);
    strcat(*destination, number);
}

void UpdateDisplayInfo(char **texts, Lander *lander, Panel *panels, float time){
    AppendNumericData(&texts[XSpeedText], "Horizontal speed: ", lander->obj.velocity.x, 3);
    AppendNumericData(&texts[YSpeedText], "Vertical speed: ", lander->obj.velocity.y, 3);
    AppendNumericData(&texts[RotationText], "Rotation: ", lander->obj.rotation, 3);
    AppendNumericData(&texts[FuelText], "Fuel: ", lander->fuelPercent * 100, 3);
    AppendNumericData(&texts[TimeText], "Time: ", time, 5);

    panels[GameView].texts.uiText[0].text = texts[XSpeedText];
    panels[GameView].texts.uiText[1].text = texts[YSpeedText];
    panels[GameView].texts.uiText[2].text = texts[RotationText];
    panels[GameView].texts.uiText[3].text = texts[FuelText];
    panels[GameView].texts.uiText[4].text = texts[TimeText];
}

int CalculateScore(const GameSettings *gameSettings, float time, float fuel){
    float difficultyModifier = 1;
    if (gameSettings->difficulty == Easy) difficultyModifier = .75f;
    else if (gameSettings->difficulty == Hard) difficultyModifier = 1.25f;
    if (gameSettings->terrainResolution == SmallMapSize) difficultyModifier *= .75f;
    else if (gameSettings->terrainResolution == LargeMapSize) difficultyModifier *= 1.25f;
    return pow(difficultyModifier * (fuel + 100), 2) / time;
}

void SetEndText(const GameStatus *gameStatus, Panel *panels, char *scoreText){
    switch (gameStatus->landerStatus) {
        case SuccessfullyLanded:
            panels[EndView].texts.uiText[1].text = scoreText;
            break;
        case OutOfBounds:
            panels[EndView].texts.uiText[1].text = "Lander left the mission area";
            break;
        case NotSiteCrash:
            panels[EndView].texts.uiText[1].text = "Lander crashed on uneven terrain";
            break;
        case WrongAngleCrash:
            panels[EndView].texts.uiText[1].text = "Lander tilt was too high";
            break;
        case HighSpeedCrash:
            panels[EndView].texts.uiText[1].text = "Lander speed was too high";
            break;
    }
}

void HandleButtonEvents(Button *button, GameSettings *gameSettings, View *currentView, Lander *lander, TerrainData *terrainData, bool *quit, int *replayCount, int *replayPage, Panel *replayPanel, ReplayData *replayData, DataList **landData, TerrainData *replayTerrain){
    if (button == NULL) return;
    switch (button->buttonEvent) {
        case Land:
            *currentView = SetUpView;
            lander->mode = SetUpPlay;
            break;
        case Settings: *currentView = SettingsView;
            break;
        case Replays:
            *currentView = ReplaysView;
            *replayCount = ReadInReplays();
            RefreshReplayView(*replayPage, replayPanel);
            break;
        case ReplayLand:
            *currentView = SetUpView;
            lander->mode = SetUpReplay;
            LoadReplay(button->text, replayData, replayTerrain, landData);
            break;
        case ExitGame: *quit = true;
            break;
        case Continue: *currentView = GameView;
            break;
        case Leave:
            *currentView = MenuView;
            ReStartLander(lander, gameSettings, Display);
            break;
        case BackToMenu:
            if (*currentView == EndView) InitTerrain(terrainData, gameSettings);
            *currentView = MenuView;
            if (lander->mode != Display) ReStartLander(lander, gameSettings, Display);
            break;
        case SmallMap:
            if (gameSettings->terrainResolution != SmallMapSize) {
                gameSettings->terrainResolution = SmallMapSize;
                InitTerrain(terrainData, gameSettings);
            }
            break;
        case MediumMap:
            if (gameSettings->terrainResolution != MediumMapSize) {
                gameSettings->terrainResolution = MediumMapSize;
                InitTerrain(terrainData, gameSettings);
            }
            break;
        case LargeMap:
            if (gameSettings->terrainResolution != LargeMapSize) {
                gameSettings->terrainResolution = LargeMapSize;
                InitTerrain(terrainData, gameSettings);
            }
            break;
        case SetEasy:
            if (gameSettings->difficulty != Easy){
                gameSettings->difficulty = Easy;
                InitTerrain(terrainData, gameSettings);
            }
            break;
        case SetNormal:
            if (gameSettings->difficulty != Normal){
                gameSettings->difficulty = Normal;
                InitTerrain(terrainData, gameSettings);
            }
            break;
        case SetHard:
            if (gameSettings->difficulty != Hard){
                gameSettings->difficulty = Hard;
                InitTerrain(terrainData, gameSettings);
            }
            break;
        case NextPage:
            if (*replayPage < (*replayCount - 1) / 9 + 1) ++*replayPage;
            RefreshReplayView(*replayPage, replayPanel);
            break;
        case PreviousPage:
            if (*replayPage > 1) --*replayPage;
            RefreshReplayView(*replayPage, replayPanel);
            break;
        default:
            break;
    }
}

void SlidePanels(Panel panels[], View currentView){
    if (!panels[currentView].enabled) panels[currentView].slideState = In;
    switch (currentView) {
        case MenuView:
            if (!panels[TitleView].enabled) panels[TitleView].slideState = In;
            if (panels[GameView].enabled) panels[GameView].slideState = Out;
            if (panels[EndView].enabled) panels[EndView].slideState = Out;
            if (panels[PauseView].enabled) panels[PauseView].slideState = Out;
            if (panels[SettingsView].enabled) panels[SettingsView].slideState = Out;
            if (panels[ReplaysView].enabled) panels[ReplaysView].slideState = Out;
            if (panels[ReplayView].enabled) panels[ReplayView].slideState = Out;
            break;
        case GameView:
            if (panels[TitleView].enabled) panels[TitleView].slideState = Out;
            if (panels[MenuView].enabled) panels[MenuView].slideState = Out;
            if (panels[PauseView].enabled) panels[PauseView].slideState = Out;
            break;
        case EndView:
            if (panels[GameView].enabled) panels[GameView].slideState = Out;
            break;
        case PauseView:
            break;
        case SettingsView:
            if (panels[MenuView].enabled) panels[MenuView].slideState = Out;
            break;
        case ReplaysView:
            if (panels[MenuView].enabled) panels[MenuView].slideState = Out;
            break;
        case ReplayView:
            if (panels[TitleView].enabled) panels[TitleView].slideState = Out;
            if (panels[ReplaysView].enabled) panels[ReplaysView].slideState = Out;
            break;
        default:
            for (int i = 0; i < UIPanels; ++i) {
                if (panels[i].enabled) panels[i].slideState = Out;
            }
            break;
    }
}

void HandleInput(SDL_Event event, GameStatus *gameStatus, MouseStatus *mouseStatus, View *currentView){
    switch (event.type) {
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
                case SDLK_LEFT: if (*currentView == GameView) gameStatus->rotateDirection = -1;
                    break;
                case SDLK_RIGHT: if (*currentView == GameView) gameStatus->rotateDirection = 1;
                    break;
                case SDLK_ESCAPE:
                    if (*currentView == GameView || *currentView == PauseView){
                        gameStatus->paused = !gameStatus->paused;
                        if (gameStatus->paused) *currentView = PauseView;
                        else *currentView = GameView;
                    }
                    break;
                case SDLK_UP: if (*currentView == GameView) gameStatus->engineOn = true;
                    break;
            }
            break;
        case SDL_KEYUP:
            switch (event.key.keysym.sym) {
                case SDLK_LEFT:
                case SDLK_RIGHT: gameStatus->rotateDirection = 0;
                    break;
                case SDLK_UP: gameStatus->engineOn = false;
                    break;
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            mouseStatus->clicked = true;
            break;
        case SDL_MOUSEBUTTONUP:
            mouseStatus->clicked = false;
            mouseStatus->buttonPressed = true;
            break;
    }
}

void sdl_init(const char *header, int width, int height, SDL_Window **pWindow, SDL_Renderer **pRenderer) {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        SDL_Log("Cannot start SDL: %s", SDL_GetError());
        exit(1);
    }
    SDL_Window *window = SDL_CreateWindow(header, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0);
    if (window == NULL) {
        SDL_Log("Cannot create window: %s", SDL_GetError());
        exit(1);
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (renderer == NULL) {
        SDL_Log("Cannot create viewer: %s", SDL_GetError());
        exit(1);
    }
    SDL_RenderClear(renderer);

    *pWindow = window;
    *pRenderer = renderer;
}

void SetWindowFullscreen(SDL_Window *window, GameSettings *gameSettings){
    SDL_DisplayMode dm;
    if (SDL_GetDesktopDisplayMode(0, &dm) != 0) SDL_Log("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
    gameSettings->width = dm.w;
    gameSettings->height = dm.h;
    SDL_SetWindowSize(window, dm.w, dm.h);
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

void ClearReplayData(DataList **dataList){
    while (*dataList != NULL) ReadOutData(dataList);
}

void ClearUp(ParticleList* particleList, ParticleList* stars, TerrainData* terrainData, TerrainData *replayTerrain, SDL_TimerID timerId, Lander* lander, Panel panels[], TTF_Font *font, char *texts[], DataList **dataList, char *replayNames){
    ClearParticleList(particleList);
    ClearParticleList(stars);
    FreeTerrain(terrainData);
    FreeTerrain(replayTerrain);
    SDL_DestroyTexture(lander->texture);
    free(lander);
    for (int i = 0; i < 9; ++i) {
        free(panels[ReplaysView].buttons.button[i].text);
    }
    free(panels[ReplaysView].texts.uiText[1].text);
    for (int i = 0; i < UIPanels; ++i) {
        ClearPanel(&panels[i]);
    }
    for (int i = 0; i < NumberOfTextIDs; ++i) {
        free(texts[i]);
    }
    ClearReplayData(dataList);
    free(replayNames);
    remove("replayFiles.txt");
    TTF_CloseFont(font);
    SDL_RemoveTimer(timerId);
}
