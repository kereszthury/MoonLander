#include <SDL_ttf.h>
#include "Renderer.h"
#include "debugmalloc.h"

void DrawLander(SDL_Renderer *renderer, float scale, const Lander *moonLander);
void DrawParticles(SDL_Renderer *renderer, ParticleList *particles, const GameSettings *settings);
void DrawTerrain(SDL_Renderer *renderer, const Vector2 *points, const GameSettings *settings, SDL_Color color);
void DrawLandingMarkers(SDL_Renderer *renderer, const LandingSite *landingSites, const GameSettings *gameSettings);
void RenderPanels(SDL_Renderer *renderer, Panel *panels, TTF_Font *font);
void RenderText(SDL_Renderer *renderer, char text[], float scale, Vector2 position, TTF_Font *font, SDL_Color color, TextAlignment alignment);
void DrawIndicators(SDL_Renderer *renderer, const GameStatus *gameStatus, Panel *gamePanel, float gameScale);

void RenderScreen(SDL_Renderer *renderer, const GameSettings *settings, const TerrainData *terrainData, int frame, Lander *lander, const GameStatus *gameStatus, ParticleList *particles, ParticleList *stars, Panel panels[], TTF_Font *font){
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    DrawParticles(renderer, stars, settings);
    DrawTerrain(renderer, terrainData->backdropPoints, settings, (SDL_Color){150, 150, 150, 255});
    DrawTerrain(renderer, terrainData->terrainPoints, settings, (SDL_Color){255, 255, 255, 255});
    DrawParticles(renderer, particles, settings);
    if (gameStatus->landerStatus == SuccessfullyLanded || gameStatus->landerStatus == Landing) DrawLander(renderer, settings->scale, lander);
    if(frame % 50 < 25) DrawLandingMarkers(renderer, terrainData->landingSites, settings);

    RenderPanels(renderer, panels, font);
    if (panels[GameView].enabled) DrawIndicators(renderer, gameStatus, &panels[GameView], settings->scale);

    SDL_RenderPresent(renderer);
}

void DrawLander(SDL_Renderer *renderer, float scale, const Lander *moonLander){
    SDL_Rect dest = {moonLander->obj.position.x, moonLander->obj.position.y, 8 * scale, 7 * scale}; // The lander's texture resolution is 800x700
    SDL_RenderCopyEx(renderer, moonLander->texture, NULL, &dest, moonLander->obj.rotation, NULL, SDL_FLIP_NONE);
}

void DrawParticles(SDL_Renderer *renderer, ParticleList *particles, const GameSettings *settings){
    for (int i = 0; i < ParticleListLength(particles); ++i) {
        filledCircleRGBA(renderer, GetParticle(particles, i)->particle.object.position.x, GetParticle(particles, i)->particle.object.position.y,
                         GetParticle(particles, i)->particle.radius * settings->scale / 10, GetParticle(particles, i)->particle.color.r,
                         GetParticle(particles, i)->particle.color.g, GetParticle(particles, i)->particle.color.b,
                         GetParticle(particles, i)->particle.color.a);
    }
}

void DrawTerrain(SDL_Renderer *renderer, const Vector2 *points, const GameSettings *settings, SDL_Color color){
    for (int i = 0; i < settings->width; i++){
        int lineHeight = GetTerrainHeight(i, points);
        vlineRGBA(renderer, i, settings->height, lineHeight, color.r, color.g, color.b, color.a);
    }
}

void DrawLandingMarkers(SDL_Renderer *renderer, const LandingSite *landingSites, const GameSettings *gameSettings){
    int landSiteCount = 3 - gameSettings->difficulty;
    for (int i = 0; i < landSiteCount; i++) {
        for (int j = 3; j > 0; --j) {
            hlineRGBA(renderer, landingSites[i].start.x + gameSettings->width / gameSettings->terrainResolution /2 /(4 - j),
                      landingSites[i].end.x - gameSettings->width / gameSettings->terrainResolution / 2 / (4 - j),
                      landingSites[i].start.y + 5 * j, 255, 0, 0, 255);
        }
    }
}

void RenderButtons(SDL_Renderer *renderer, Panel panel, TTF_Font *font);
void RenderPanels(SDL_Renderer *renderer, Panel panels[], TTF_Font *font){
    for (int i = 0; i < UIPanels; ++i) {
        if(panels[i].enabled) {
            boxRGBA(renderer, panels[i].currentPosition.x - panels[i].size.x / 2, panels[i].currentPosition.y - panels[i].size.y / 2,
                    panels[i].currentPosition.x + panels[i].size.x / 2, panels[i].currentPosition.y + panels[i].size.y / 2,
                    panels[i].backgroundColor.r, panels[i].backgroundColor.g, panels[i].backgroundColor.b,
                    panels[i].backgroundColor.a);

            RenderButtons(renderer, panels[i], font);
            for (int j = 0; j < panels[i].texts.textCount; ++j) {
                UIText text = panels[i].texts.uiText[j];
                Vector2 position = {text.localPosition.x + panels[i].currentPosition.x, text.localPosition.y + panels[i].currentPosition.y};
                RenderText(renderer, text.text, text.scale, position, font, text.color, text.alignment);
            }
        }
    }
}

void RenderButtons(SDL_Renderer *renderer, Panel panel, TTF_Font *font){
    float panelX = panel.currentPosition.x;
    float panelY = panel.currentPosition.y;
    for (int j = 0; j < panel.buttons.buttonCount; ++j) {
        Button current = panel.buttons.button[j];
        if (current.text == NULL) continue;
        SDL_Color color;
        switch (current.buttonState) {
            case Pressed: color = current.pressedColor;
                break;
            case Highlighted: color = current.highlightedColor;
                break;
            default: color = current.baseColor;
                break;
        }
        // Render border
        boxRGBA(renderer, panelX + current.localPosition.x - current.size.x / 2 - current.borderSize,
                panelY + current.localPosition.y - current.size.y / 2 - current.borderSize,
                panelX + current.localPosition.x + current.size.x / 2 + current.borderSize,
                panelY + current.localPosition.y + current.size.y / 2 + current.borderSize,
                current.borderColor.r, current.borderColor.g, current.borderColor.b, current.borderColor.a);
        // Render button
        boxRGBA(renderer, panelX + current.localPosition.x - current.size.x / 2,
                panelY + current.localPosition.y - current.size.y / 2,
                panelX + current.localPosition.x + current.size.x / 2,
                panelY + current.localPosition.y + current.size.y / 2,
                color.r, color.g, color.b, color.a);

        Vector2 textPosition = {current.localPosition.x + panelX, current.localPosition.y + panelY};
        RenderText(renderer, current.text, current.textScale, textPosition, font, current.textColor, Middle);
    }
}

void RenderText(SDL_Renderer *renderer, char text[], float scale, Vector2 position, TTF_Font *font, SDL_Color color, TextAlignment alignment){
    if (text == NULL) return;
    SDL_Surface *display;
    SDL_Texture *displayTexture;
    display = TTF_RenderUTF8_Solid(font, text, color);
    displayTexture = SDL_CreateTextureFromSurface(renderer, display);
    float posX = position.x - display->w / 2 * scale;
    if (alignment == Left) posX = position.x;
    SDL_Rect destination = { posX, position.y - display->h / 2 * scale, display->w * scale, display->h * scale};
    SDL_RenderCopy(renderer, displayTexture, NULL, &destination);
    SDL_FreeSurface(display);
    SDL_DestroyTexture(displayTexture);
}

void DrawIndicators(SDL_Renderer *renderer, const GameStatus *gameStatus, Panel *gamePanel, float gameScale){
    if(gameStatus->xSpeedOk) filledCircleRGBA(renderer, gamePanel->currentPosition.x - gamePanel->size.x * .45f, gamePanel->currentPosition.y - gamePanel->size.y * .3f, .75f * gameScale, 0, 255, 0, 255);
    if(gameStatus->ySpeedOk) filledCircleRGBA(renderer, gamePanel->currentPosition.x - gamePanel->size.x * .45f, gamePanel->currentPosition.y - gamePanel->size.y * .1f, .75f * gameScale, 0, 255, 0, 255);
    if(gameStatus->rotationOk) filledCircleRGBA(renderer, gamePanel->currentPosition.x - gamePanel->size.x * .45f, gamePanel->currentPosition.y + gamePanel->size.y * .1f, .75f * gameScale, 0, 255, 0, 255);
}