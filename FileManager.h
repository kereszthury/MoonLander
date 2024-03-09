#ifndef MOONLANDER_FILEMANAGER_H
#define MOONLANDER_FILEMANAGER_H

#include <stdio.h>
#include "General.h"
#include "Terrain.h"
#include "Physics.h"

typedef struct ReplayData{
    GameSettings gameSettings; bool finish;
} ReplayData;

typedef struct LandData{
    Object lander; bool engineOn;
} LandData;

typedef struct DataList{
    LandData landData;
    struct DataList *next;
} DataList;

GameSettings ReadInSettings();
void SaveSettings(GameSettings gameSettings);

void AddData(DataList **list, LandData d);
LandData ReadOutData(DataList **list);
void SaveReplay(char name[], ReplayData replayData, TerrainData *terrainData, DataList **dataList);
void LoadReplay(char *name, ReplayData *replayData, TerrainData *terrainData, DataList **dataList);
int ReadInReplays();

#endif
