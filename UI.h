#ifndef MOONLANDER_UI_H
#define MOONLANDER_UI_H

#include <SDL.h>

#include "General.h"

#define UIPanels 8

typedef enum ButtonEvents { Land, Settings, Replays, ReplayLand, ExitGame, Continue, Leave, BackToMenu, SmallMap, MediumMap, LargeMap, SetEasy, SetNormal, SetHard, NextPage, PreviousPage, None } ButtonEvent;
typedef enum ButtonStates { Base, Highlighted, Pressed } ButtonState;

typedef struct Button{
    ButtonEvent buttonEvent; Vector2 localPosition, parentPosition, size; char *text; SDL_Color baseColor, highlightedColor, pressedColor, borderColor, textColor; float borderSize; ButtonState buttonState; float textScale;
} Button;

typedef struct ButtonArray {
    Button *button; int buttonCount;
} ButtonArray;

typedef enum TextAlignments { Middle, Left } TextAlignment;

typedef struct UIText{
    char *text; Vector2 localPosition; TextAlignment alignment; float scale; SDL_Color color;
} UIText;

typedef struct TextArray{
    UIText *uiText; int textCount;
} TextArray;

typedef enum SlideDirections { TopToBottom, BottomToTop, LeftToRight, RightToLeft } SlideDirection;
typedef enum SlideStates { In, Out, Stay } SlideState;

typedef struct Panel{
    ButtonArray buttons; TextArray texts; Vector2 currentPosition, screenPosition, size; SDL_Color backgroundColor; SlideDirection slideIn, slideOut; SlideState slideState; float slideSpeed; bool enabled;
} Panel;

typedef enum Views{ MenuView = 0, GameView = 1, PauseView = 2, EndView = 3, SettingsView = 4, ReplaysView = 5, ReplayView = 6, TitleView = 7, SetUpView = -1 } View;

Button* UpdatePanels(Panel panels[], GameSettings *gameSettings, MouseStatus *mouseStatus);
void ClearPanel(Panel *panel);
void InitPanels(const GameSettings *gameSettings, Panel panels[]);

#endif
