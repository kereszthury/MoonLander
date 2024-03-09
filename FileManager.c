#include <dirent.h>
#include "FileManager.h"

#include "debugmalloc.h"

GameSettings ReadInSettings(){
    FILE* file;
    GameSettings gameSettings = (GameSettings) {1920, 1080, 22, -1, Normal };
    file = fopen("data.b", "rb");
    if (file == NULL) return gameSettings;
    fread(&gameSettings, sizeof (GameSettings), 1, file);
    fclose(file);
    return gameSettings;
}

void SaveSettings(GameSettings gameSettings){
    FILE* file;
    file = fopen("data.b", "wb");
    fwrite(&gameSettings, sizeof (GameSettings), 1, file);
    fclose(file);
}

void SaveReplay(char *name, ReplayData replayData, TerrainData *terrainData, DataList **dataList){
    DIR* dir = opendir("Replays");
    if (!dir) mkdir("Replays");

    char path[strlen(name) + 8];
    strcpy(path, "Replays/");
    strcat(path, name);
    strcat(path, "land");

    FILE* file;
    file = fopen(path, "wb");

    if (file != NULL){
        fwrite(&replayData, sizeof (ReplayData), 1, file);
        for (int i = 0; i < replayData.gameSettings.terrainResolution; ++i) {
            fwrite(&terrainData->terrainPoints[i], sizeof (Vector2), 1, file);
            fwrite(&terrainData->backdropPoints[i], sizeof (Vector2), 1, file);
        }
        for (int i = 0; i < (3 - replayData.gameSettings.difficulty); ++i) {
            fwrite(&terrainData->landingSites[i], sizeof (LandingSite), 1, file);
        }
        while (*dataList != NULL){
            LandData data = ReadOutData(dataList);
            fwrite(&data, sizeof (LandData), 1, file);
        }
        fclose(file);
    }

    closedir(dir);
}

void LoadReplay(char *name, ReplayData *replayData, TerrainData *terrainData, DataList **dataList){
    char path[strlen(name) + 8];
    strcpy(path, "Replays/");
    strcat(path, name);

    while (*dataList != NULL){
        ReadOutData(dataList);
    }

    FILE* file = fopen(path, "rb");

    if (file != NULL){
        fread(replayData, sizeof (ReplayData), 1, file);

        FreeTerrain(terrainData);

        Vector2* terrainPoints = (Vector2*) malloc(replayData->gameSettings.terrainResolution * sizeof (Vector2));
        Vector2* backdropPoints = (Vector2*) malloc(replayData->gameSettings.terrainResolution * sizeof (Vector2));
        LandingSite* landingSites = (LandingSite*) malloc((3 - replayData->gameSettings.difficulty) * sizeof (LandingSite));
        terrainData->terrainPoints = terrainPoints;
        terrainData->backdropPoints = backdropPoints;
        terrainData->landingSites = landingSites;

        for (int i = 0; i < replayData->gameSettings.terrainResolution; ++i) {
            fread(&terrainData->terrainPoints[i], sizeof (Vector2), 1, file);
            fread(&terrainData->backdropPoints[i], sizeof (Vector2), 1, file);
        }
        for (int i = 0; i < (3 - replayData->gameSettings.difficulty); ++i) {
            fread(&terrainData->landingSites[i], sizeof (LandingSite), 1, file);
        }

        LandData data;
        while (!feof(file)){
            fread(&data, sizeof (LandData), 1, file);
            AddData(dataList, data);
        }
        fclose(file);
    }
}

int DataListLength(const DataList *list){
    if (list == NULL) return 0;
    int length = 1;
    while (list->next != NULL){
        length++;
        list = list->next;
    }
    return length;
}

DataList* GetData(DataList *list, int index){
    if (index == 0) return list;
    return GetData(list->next, index - 1);
}

void AddData(DataList **list, LandData d){
    DataList *newElement = (DataList *) malloc(sizeof (DataList));
    newElement->landData = d;
    newElement->next = NULL;
    if (*list == NULL) *list = newElement;
    else{
        DataList *lastElement = GetData(*list, DataListLength(*list) - 1);
        lastElement->next = newElement;
    }
}

// Cannot read from empty list
LandData ReadOutData(DataList **list){
    DataList *read = *list;
    LandData data = read->landData;
    *list = read->next;
    free(read);
    return data;
}

int ReadInReplays(){
    DIR *d = opendir("Replays");
    struct dirent *dir;
    int count = 0;

    if (d) {
        FILE* headerFile;
        headerFile = fopen("replayFiles.txt", "wt");
        while ((dir = readdir(d)) != NULL) {
            if (!(!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))){
                fprintf(headerFile, "%s\n", dir->d_name);
                count++;
            }
        }
        closedir(d);
        fclose(headerFile);
    }

    return count;
}