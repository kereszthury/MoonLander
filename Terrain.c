#include <stdbool.h>

#include "Terrain.h"
#include "debugmalloc.h"

void GenerateLandingSites(int *sites, int numberOfSites, int terrainResolution);
bool EndOfLandsite(int pointID, const int *sites, int numberOfSites);

void GenerateTerrain(Vector2 *terrainPoints, LandingSite *landingSites, const GameSettings *settings, int minHeight, int maxHeight, float roughness) {
    int landSiteCount = 3 - settings->difficulty;
    int landIds[landSiteCount];
    if (landingSites != NULL) GenerateLandingSites(landIds, landSiteCount, settings->terrainResolution);

    for (int i = 0; i < settings->terrainResolution; i++){
        terrainPoints->x = i * settings->width / (settings->terrainResolution - 1);
        if (landingSites != NULL && EndOfLandsite(i, landIds, landSiteCount)) {
            terrainPoints->y = (terrainPoints - 1)->y;
            LandingSite lSite = {*(terrainPoints - 1), *terrainPoints};
            *landingSites = lSite;
            ++landingSites;
        }
        else terrainPoints->y = settings->height - (rand() % (maxHeight - minHeight) * roughness + minHeight);
        ++terrainPoints;
    }
}

int GetTerrainHeight(int x, const Vector2 *terrainPoints){
    while (terrainPoints->x < x) ++terrainPoints;
    Vector2 p2 = *terrainPoints;
    --terrainPoints;
    Vector2  p1 = *terrainPoints;

    return p1.y - (x - p1.x) * ((p1.y - p2.y) / (p2.x - p1.x));
}

void GenerateLandingSites(int *sites, int numberOfSites, int terrainResolution){
    for (int i = 0; i < numberOfSites; i++){
        int sitePlace = rand() % (terrainResolution - 3) + 1;
        bool siteNearby = false;
        for (int j = 0; j < i; ++j) {
            if (abs(sites[j] - sitePlace) < (terrainResolution / 2 / numberOfSites) + 1){
                siteNearby = true;
                break;
            }
        }
        if (!siteNearby) sites[i] = sitePlace;
        else i--;
    }
}

bool EndOfLandsite(int pointID, const int *sites, int numberOfSites){
    for (int i = 0; i < numberOfSites; ++i) {
        if (pointID - 1 == *sites) return  true;
        ++sites;
    }
    return false;
}

void FreeTerrain(TerrainData *terrainData){
    free(terrainData->terrainPoints);
    terrainData->terrainPoints = NULL;
    free(terrainData->backdropPoints);
    terrainData->backdropPoints = NULL;
    free(terrainData->landingSites);
    terrainData->landingSites = NULL;
}

void InitTerrain(TerrainData *terrainData, GameSettings *gameSettings){
    FreeTerrain(terrainData);
    gameSettings->scale = gameSettings->width / 24000.0f * 100 / gameSettings->terrainResolution * 20;
    Vector2* terrainPoints = (Vector2*) malloc(gameSettings->terrainResolution * sizeof (Vector2));
    Vector2* backdropPoints = (Vector2*) malloc(gameSettings->terrainResolution * sizeof (Vector2));
    LandingSite* landingSites = (LandingSite*) malloc((3 - gameSettings->difficulty) * sizeof (LandingSite));
    terrainData->terrainPoints = terrainPoints;
    terrainData->backdropPoints = backdropPoints;
    terrainData->landingSites = landingSites;
    GenerateTerrain(terrainData->terrainPoints, terrainData->landingSites, gameSettings, gameSettings->height / 10.0f, gameSettings->height / 4.0f, 1.2f);
    GenerateTerrain(terrainData->backdropPoints, NULL, gameSettings, gameSettings->height / 4.5f, gameSettings->height / 1.8f, 0.5f);
}