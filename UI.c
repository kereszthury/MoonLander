#include "UI.h"

#include "debugmalloc.h"

bool InButtonRect(const Button *button, Vector2 point){
    if (button->text == NULL) return false;
    bool x = button->parentPosition.x + button->localPosition.x - button->size.x / 2 < point.x && button->parentPosition.x + button->localPosition.x + button->size.x / 2 > point.x;
    bool y = button->parentPosition.y + button->localPosition.y - button->size.y / 2 < point.y && button->parentPosition.y + button->localPosition.y + button->size.y / 2 > point.y;
    return x && y;
}

Button* ButtonClick(ButtonArray *buttonList, Vector2 cursorPosition){
    for (int i = 0; i < buttonList->buttonCount; ++i) {
        if(InButtonRect(&buttonList->button[i], cursorPosition)) {
            return &buttonList->button[i];
        }
    }
    return NULL;
}

void SetButtonPresses(ButtonArray *buttonList, MouseStatus *mouseStatus){
    for (int i = 0; i < buttonList->buttonCount; ++i) {
        if (InButtonRect(&buttonList->button[i], (Vector2) {mouseStatus->x, mouseStatus->y})){
            if (mouseStatus->clicked) buttonList->button[i].buttonState = Pressed;
            else buttonList->button[i].buttonState = Highlighted;
        }
        else buttonList->button[i].buttonState = Base;
    }
}

void UpdatePanel(Panel *panel, MouseStatus *mouseStatus, float timeElapsed, const GameSettings *gameSettings){
    if (panel->enabled && panel->slideState == Stay) SetButtonPresses(&panel->buttons, mouseStatus);
    else if (panel->slideState == In){
        switch (panel->slideIn) {
            case TopToBottom:
                if (!panel->enabled) panel->currentPosition = (Vector2) {panel->screenPosition.x, -panel->size.y / 2};
                panel->currentPosition.x = panel->screenPosition.x;
                panel->currentPosition.y += panel->slideSpeed * 24000 / gameSettings->width * timeElapsed;
                if (panel->currentPosition.y >= panel->screenPosition.y){
                    panel->currentPosition.y = panel->screenPosition.y;
                    panel->slideState = Stay;
                }
                break;
            case BottomToTop:
                if (!panel->enabled) panel->currentPosition = (Vector2) {panel->screenPosition.x, gameSettings->height + panel->size.y / 2};
                panel->currentPosition.x = panel->screenPosition.x;
                panel->currentPosition.y -= panel->slideSpeed * 24000 / gameSettings->width * timeElapsed;
                if (panel->currentPosition.y <= panel->screenPosition.y){
                    panel->currentPosition.y = panel->screenPosition.y;
                    panel->slideState = Stay;
                }
                break;
            case LeftToRight:
                if (!panel->enabled) panel->currentPosition = (Vector2) {-panel->size.x / 2, panel->screenPosition.y};
                panel->currentPosition.y = panel->screenPosition.y;
                panel->currentPosition.x += panel->slideSpeed * 24000 / gameSettings->width * timeElapsed;
                if (panel->currentPosition.x >= panel->screenPosition.x){
                    panel->currentPosition.x = panel->screenPosition.x;
                    panel->slideState = Stay;
                }
                break;
            case RightToLeft:
                if (!panel->enabled) panel->currentPosition = (Vector2) {gameSettings->width + panel->size.x / 2, panel->screenPosition.y};
                panel->currentPosition.y = panel->screenPosition.y;
                panel->currentPosition.x -= panel->slideSpeed * 24000 / gameSettings->width * timeElapsed;
                if (panel->currentPosition.x <= panel->screenPosition.x){
                    panel->currentPosition.x = panel->screenPosition.x;
                    panel->slideState = Stay;
                }
                break;
        }
        if (!panel->enabled) panel->enabled = true;
    }
    else {
        switch (panel->slideOut) {
            case TopToBottom:
                panel->currentPosition.y += panel->slideSpeed * 24000 / gameSettings->width * timeElapsed;
                if (panel->currentPosition.y >= gameSettings->height + panel->size.y / 2 + 1){
                    panel->slideState = Stay;
                    panel->enabled = false;
                }
                break;
            case BottomToTop:
                panel->currentPosition.y -= panel->slideSpeed * gameSettings->scale * timeElapsed;
                if (panel->currentPosition.y <= -panel->size.y / 2 - 1){
                    panel->currentPosition.y = panel->screenPosition.y;
                    panel->slideState = Stay;
                    panel->enabled = false;
                }
                break;
            case LeftToRight:
                panel->currentPosition.x += panel->slideSpeed * 24000 / gameSettings->width * timeElapsed;
                if (panel->currentPosition.x >= gameSettings->width + panel->size.x / 2 + 1){
                    panel->slideState = Stay;
                    panel->enabled = false;
                }
                break;
            case RightToLeft:
                panel->currentPosition.x -= panel->slideSpeed * 24000 / gameSettings->width * timeElapsed;
                if (panel->currentPosition.x <= -panel->size.x / 2 - 1){
                    panel->slideState = Stay;
                    panel->enabled = false;
                }
                break;
        }
    }
}

Button* UpdatePanels(Panel panels[], GameSettings *gameSettings, MouseStatus *mouseStatus){
    Button *button = NULL;
    for (int i = 0; i < UIPanels; ++i) {
        UpdatePanel(&panels[i], mouseStatus, 0.02f, gameSettings);
        if (mouseStatus->buttonPressed && panels[i].enabled){
            button = ButtonClick(&panels[i].buttons, (Vector2) {mouseStatus->x, mouseStatus->y});
            if (button != NULL) mouseStatus->buttonPressed = false;
        }
    }
    return button;
}

void AddButton(Button button, Panel *panel){
    if (panel->buttons.button == NULL){
        panel->buttons.button = (Button *) malloc(sizeof(Button));
        panel->buttons.buttonCount = 1;
    }
    else {
        panel->buttons.buttonCount++;
        panel->buttons.button = (Button *) realloc(panel->buttons.button, panel->buttons.buttonCount * sizeof (Button));
    }
    panel->buttons.button[panel->buttons.buttonCount - 1] = button;
}

void AddText(UIText uiText, Panel *panel){
    if (panel->texts.uiText == NULL){
        panel->texts.uiText = (UIText *) malloc(sizeof(UIText));
        panel->texts.textCount = 1;
    }
    else {
        panel->texts.textCount++;
        panel->texts.uiText = (UIText *) realloc(panel->texts.uiText, panel->texts.textCount * sizeof (UIText));
    }
    panel->texts.uiText[panel->texts.textCount - 1] = uiText;
}

void ClearPanel(Panel *panel){
    free(panel->buttons.button);
    free(panel->texts.uiText);
}

Panel InitMenuView(const GameSettings *gameSettings){
    Vector2 panelSize = {gameSettings->width * .25f, gameSettings->height * .6f};
    Vector2 panelPosition =   {gameSettings->width / 4, gameSettings->height / 2};

    Panel p = (Panel) {{NULL, 0}, {NULL, 0}, 0, 0,
                       panelPosition, panelSize,{50,50,50,225},
                    LeftToRight, RightToLeft, Stay, 150, false };

    Button land = { Land, 0, -panelSize.y * .35f, panelPosition,
                       panelSize.x *.75f,panelSize.y * .1f, "Land",
                       {255,255,255,255},
                       {175,175,175,255},
                       {100,100,100,255},
                       {125,125,125,255},
                       {0,0,0,255},
                       .25f * gameSettings->width * 0.01f, Base, .5f};
    AddButton(land, &p);

    Button settings = { Settings, 0, -panelSize.y * .15f, panelPosition,
                    panelSize.x *.75f,panelSize.y * .1f, "Settings",
                    {255,255,255,255},
                    {175,175,175,255},
                    {100,100,100,255},
                    {125,125,125,255},
                    {0,0,0,255},
                    .25f * gameSettings->width * 0.01f, Base, .5f};
    AddButton(settings, &p);

    Button replays = { Replays, 0, panelSize.y * .05f, panelPosition,
                        panelSize.x *.75f,panelSize.y * .1f, "Replays",
                        {255,255,255,255},
                        {175,175,175,255},
                        {100,100,100,255},
                        {125,125,125,255},
                        {0,0,0,255},
                        .25f * gameSettings->width * 0.01f, Base, .5f};
    AddButton(replays, &p);

    Button exit = { ExitGame, 0, panelSize.y * .25f, panelPosition,
                       panelSize.x *.75f,panelSize.y * .1f, "Exit",
                       {255,255,255,255},
                       {175,175,175,255},
                       {100,100,100,255},
                       {125,125,125,255},
                       {0,0,0,255},
                       .25f * gameSettings->width * 0.01f, Base, .5f};
    AddButton(exit, &p);

    UIText highScore = {"", -panelSize.x * .38f, panelSize.y * .45f, Left, .3f, {200, 200, 200, 200}};
    AddText(highScore, &p);

    return p;
}

Panel InitGameView(const GameSettings *gameSettings){
    Vector2 panelSize = {gameSettings->width * .22f, gameSettings->height * .1f };
    Vector2 panelPosition = {gameSettings->width * .95f, gameSettings->height * .05f};

    Panel p = (Panel) {{NULL, 0}, {NULL, 0}, 0, 0,
                       panelPosition,
                       panelSize,
                       {50,50,50,225},
                       TopToBottom, BottomToTop, Stay, 20, false };

    UIText xSpeed = {NULL, -panelSize.x * .4f, -panelSize.y * .3f, Left, .2f, {200, 200, 200, 200}};
    AddText(xSpeed, &p);

    UIText ySpeed = {NULL, -panelSize.x * .4f, -panelSize.y * .1f, Left, .2f, {200, 200, 200, 200}};
    AddText(ySpeed, &p);

    UIText rotation = {NULL, -panelSize.x * .4f, panelSize.y * .1f, Left, .2f, {200, 200, 200, 200}};
    AddText(rotation, &p);

    UIText fuel = {NULL, -panelSize.x * .4f, panelSize.y * .3f, Left, .2f, {200, 200, 200, 200}};
    AddText(fuel, &p);

    UIText time = {NULL, -panelSize.x * .1f, panelSize.y * .3f, Left, .2f, {200,200,200,200}};
    AddText(time, &p);

    return p;
}

Panel InitPauseView(const GameSettings *gameSettings){
    Vector2 panelSize = {gameSettings->width * .15f, gameSettings->height * .4f};
    Vector2 panelPosition = {gameSettings->width / 2, gameSettings->height / 2};

    Panel p = (Panel) {{NULL, 0}, {NULL, 0}, 0, 0,
                       panelPosition, panelSize,{50,50,50,225},
                       BottomToTop, BottomToTop, Stay, 600, false };

    Button unpause = { Continue, 0, -panelSize.y * .05f, panelPosition,
                       panelSize.x *.75f,panelSize.y * .15f, "Continue",
                       {255,255,255,255},
                       {175,175,175,255},
                       {100,100,100,255},
                       {125,125,125,255},
                       {0,0,0,255},
                       .25f * gameSettings->width * 0.01f, Base, .5f};
    AddButton(unpause, &p);

    Button back = { BackToMenu, 0, panelSize.y * .25f, panelPosition,
                       panelSize.x *.75f,panelSize.y * .15f, "Leave",
                       {255,255,255,255},
                       {175,175,175,255},
                       {100,100,100,255},
                       {125,125,125,255},
                       {0,0,0,255},
                       .25f * gameSettings->width * 0.01f, Base, .5f};
    AddButton(back, &p);

    UIText text = {"PAUSED", 0, -panelSize.y * .3f, Middle, .7f, {200, 200, 200, 200}};
    AddText(text, &p);

    return p;
}

Panel InitEndView(const GameSettings *gameSettings){
    Vector2 panelSize = {gameSettings->width * .2f, gameSettings->height * .3f };
    Vector2 panelPosition = {gameSettings->width / 2, gameSettings->height / 2};

    Panel p = (Panel) {{NULL, 0}, {NULL, 0}, 0, 0,
                       panelPosition, panelSize, {50,50,50,225},
                       RightToLeft, LeftToRight, Stay, 300, false };

    Button back = { BackToMenu, 0, panelSize.y * .3f, panelPosition,
                    panelSize.x *.45f,panelSize.y * .15f, "Leave",
                    {255,255,255,255},
                    {175,175,175,255},
                    {100,100,100,255},
                    {125,125,125,255},
                    {0,0,0,255},
                    .25f * gameSettings->width * 0.01f, Base, .5f};
    AddButton(back, &p);

    Button retry = { Land, 0, 0, panelPosition,
                    panelSize.x *.45f,panelSize.y * .15f, "Retry",
                    {255,255,255,255},
                    {175,175,175,255},
                    {100,100,100,255},
                    {125,125,125,255},
                    {0,0,0,255},
                    .25f * gameSettings->width * 0.01f, Base, .5f};
    AddButton(retry, &p);


    UIText endStatus = {NULL, 0, -panelSize.y * .35f, Middle, .5f, {200,200,200,255}};
    AddText(endStatus, &p);

    UIText infoDisplay = {NULL, 0, -panelSize.y * .2f, Middle, .2f, {255,255,255,255}};
    AddText(infoDisplay, &p);

    return p;
}

Panel InitSettingsView(const GameSettings *gameSettings){
    Vector2 panelSize = {gameSettings->width * .3f, gameSettings->height * .6f};
    Vector2 panelPosition = {gameSettings->width / 3, gameSettings->height / 2};

    Panel p = (Panel) {{NULL, 0}, {NULL, 0}, 0, 0,
                       panelPosition, panelSize, {50,50,50,225},
                       LeftToRight, RightToLeft, Stay, 300, false };

    Button small = { SmallMap, -panelSize.x * .25f, -panelSize.y * .15f, panelPosition,
                    panelSize.x *.3f,panelSize.y * .1f, "Small",
                    {255,255,255,255},
                    {175,175,175,255},
                    {100,100,100,255},
                    {125,125,125,255},
                    {0,0,0,255},
                    .25f * gameSettings->width * 0.01f, Base, .5f};
    AddButton(small, &p);

    Button medium = { MediumMap, -panelSize.x * .25f, 0, panelPosition,
                     panelSize.x *.3f,panelSize.y * .1f, "Medium",
                     {255,255,255,255},
                     {175,175,175,255},
                     {100,100,100,255},
                     {125,125,125,255},
                     {0,0,0,255},
                     .25f * gameSettings->width * 0.01f, Base, .5f};
    AddButton(medium, &p);

    Button large = { LargeMap, -panelSize.x * .25f, panelSize.y * .15f, panelPosition,
                     panelSize.x *.3f,panelSize.y * .1f, "Large",
                     {255,255,255,255},
                     {175,175,175,255},
                     {100,100,100,255},
                     {125,125,125,255},
                     {0,0,0,255},
                     .25f * gameSettings->width * 0.01f, Base, .5f};
    AddButton(large, &p);

    Button easy = { SetEasy, panelSize.x * .25f, -panelSize.y * .15f, panelPosition,
                      panelSize.x *.3f,panelSize.y * .1f, "Easy",
                      {255,255,255,255},
                      {175,175,175,255},
                      {100,100,100,255},
                      {125,125,125,255},
                      {0,0,0,255},
                      .25f * gameSettings->width * 0.01f, Base, .5f};
    AddButton(easy, &p);

    Button normal = { SetNormal, panelSize.x * .25f, 0, panelPosition,
                     panelSize.x *.3f,panelSize.y * .1f, "Normal",
                     {255,255,255,255},
                     {175,175,175,255},
                     {100,100,100,255},
                     {125,125,125,255},
                     {0,0,0,255},
                     .25f * gameSettings->width * 0.01f, Base, .5f};
    AddButton(normal, &p);

    Button hard = { SetHard, panelSize.x * .25f, panelSize.y * .15f, panelPosition,
                      panelSize.x *.3f,panelSize.y * .1f, "Hard",
                      {255,255,255,255},
                      {175,175,175,255},
                      {100,100,100,255},
                      {125,125,125,255},
                      {0,0,0,255},
                      .25f * gameSettings->width * 0.01f, Base, .5f};
    AddButton(hard, &p);

    Button back = { BackToMenu, panelSize.x * .3f, panelSize.y * .4f, panelPosition,
                    panelSize.x *.25f,panelSize.y * .1f, "Back",
                    {255,255,255,255},
                    {175,175,175,255},
                    {100,100,100,255},
                    {125,125,125,255},
                    {0,0,0,255},
                    .25f * gameSettings->width * 0.01f, Base, .5f};
    AddButton(back, &p);

    UIText settings = {"SETTINGS", -panelSize.x * .15f, -panelSize.y * .4f, Middle, .7f, {220, 220, 220, 255}};
    AddText(settings, &p);

    UIText mapSize = {"Map size", -panelSize.x * .25f, -panelSize.y * .25f, Middle, .3f, {220, 220, 220, 255}};
    AddText(mapSize, &p);

    UIText difficulty = {"Difficulty", panelSize.x * .25f, -panelSize.y * .25f, Middle, .3f, {220, 220, 220, 255}};
    AddText(difficulty, &p);

    return p;
}

Panel InitReplaysView(const GameSettings *gameSettings){
    Vector2 panelSize = {gameSettings->width * .5f, gameSettings->height * .6f};
    Vector2 panelPosition = {gameSettings->width / 3, gameSettings->height / 2};

    Panel p = (Panel) {{NULL, 0}, {NULL, 0}, 0, 0,
                       panelPosition, panelSize, {50,50,50,225},
                       LeftToRight, RightToLeft, Stay, 300, false };

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            Button replaySlot = { ReplayLand, i * panelSize.x * .3f + -panelSize.x * .3f, j * panelSize.y * .2f + -panelSize.y * .2f, panelPosition,
                                  panelSize.x *.22f,panelSize.y * .1f, NULL,
                                  {50,50,50,100},
                                  {175,175,175,100},
                                  {100,100,100,100},
                                  {125,125,125,100},
                                  {150,150,150,255},
                                  .1f * gameSettings->width * 0.01f, Base, .25f};
            AddButton(replaySlot, &p);
        }
    }

    Button back = { BackToMenu, panelSize.x * .35f, panelSize.y * .4f, panelPosition,
                    panelSize.x *.15f,panelSize.y * .1f, "Back",
                    {255,255,255,255},
                    {175,175,175,255},
                    {100,100,100,255},
                    {125,125,125,255},
                    {0,0,0,255},
                    .1f * gameSettings->width * 0.01f, Base, .5f};
    AddButton(back, &p);

    Button prev = { PreviousPage, panelSize.x * .2f, -panelSize.y * .4f, panelPosition,
                    panelSize.x *.08f,panelSize.y * .1f, "<-",
                    {255,255,255,255},
                    {175,175,175,255},
                    {100,100,100,255},
                    {125,125,125,255},
                    {0,0,0,255},
                    .1f * gameSettings->width * 0.01f, Base, .5f};
    AddButton(prev, &p);

    Button next = { NextPage, panelSize.x * .4f, -panelSize.y * .4f, panelPosition,
                    panelSize.x *.08f,panelSize.y * .1f, "->",
                    {255,255,255,255},
                    {175,175,175,255},
                    {100,100,100,255},
                    {125,125,125,255},
                    {0,0,0,255},
                    .1f * gameSettings->width * 0.01f, Base, .5f};
    AddButton(next, &p);

    UIText replays = {"REPLAYS", -panelSize.x * .30f, -panelSize.y * .4f, Middle, .7f, {220, 220, 220, 255}};
    AddText(replays, &p);

    UIText page = {NULL, panelSize.x * .3f, -panelSize.y * .4f, Middle, .5f, {220, 220, 220, 255}};
    AddText(page, &p);

    return p;
}

Panel InitReplayView(const GameSettings *gameSettings){
    Vector2 panelSize = {gameSettings->width * .2f, gameSettings->height * .15f};
    Vector2 panelPosition = {gameSettings->width * .85f, gameSettings->height * .15f};

    Panel p = (Panel) {{NULL, 0}, {NULL, 0},0, 0,
                       panelPosition, panelSize, {50,50,50,225},
                       TopToBottom, BottomToTop, Stay, 100, false };

    Button back = { BackToMenu, panelSize.x * .3f, panelSize.y * .3f, panelPosition,
                    panelSize.x *.3f,panelSize.y * .2f, "Back",
                    {255,255,255,255},
                    {175,175,175,255},
                    {100,100,100,255},
                    {125,125,125,255},
                    {0,0,0,255},
                    .1f * gameSettings->width * 0.01f, Base, .3f};
    AddButton(back, &p);

    UIText text = {"Replay", -panelSize.x * .43f, -panelSize.y * .28f, Left, .75f, {255, 0, 0, 125}};
    AddText(text, &p);

    return p;
}

Panel InitTitle(const GameSettings *gameSettings){
    Vector2 panelSize = {gameSettings->width * .3f, gameSettings->height * .2f};
    Vector2 panelPosition =   {gameSettings->width / 4, gameSettings->height * .1f};

    Panel p = (Panel) {{NULL, 0}, {NULL, 0}, 0, 0,
                 panelPosition, panelSize, {0,0,0,0},
                 LeftToRight, RightToLeft, Stay, 150, false };

    UIText title = {"Moon Lander", 0, 0, Middle, 1.5f, {255, 255, 255, 255}};
    AddText(title, &p);

    return p;
}

void InitPanels(const GameSettings *gameSettings, Panel panels[]){
    panels[MenuView] = InitMenuView(gameSettings);
    panels[GameView] = InitGameView(gameSettings);
    panels[PauseView] = InitPauseView(gameSettings);
    panels[EndView] = InitEndView(gameSettings);
    panels[SettingsView] = InitSettingsView(gameSettings);
    panels[ReplaysView] = InitReplaysView(gameSettings);
    panels[ReplayView] = InitReplayView(gameSettings);
    panels[TitleView] = InitTitle(gameSettings);
}