//
//  ui.h
//  engine
//
//  Created by Scott on 21/10/2022.
//

#ifndef ui_h
#define ui_h

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

extern void uiInit(SDL_Renderer* target);
extern void uiTerminate();

extern void uiSetTarget(SDL_Renderer* renderer);

extern void uiSetFont(TTF_Font* font);

extern void uiBeginFrame();

extern int uiBeginWindow(SDL_Rect* rect, int* isActive);

extern void uiSkipLines(int n);
extern void uiNextLine();
extern void uiSameLine(int n);

extern int uiSlider(float* value, float min, float max, float step);
extern int uiButton();
extern void uiLabel(const char* label);
extern void uiLabelNext(const char* label, float ratio);
extern void uiHeader(const char* label);
extern void uiAddIndent();
extern void uiSubIndent();
extern void uiSetIndent(int n);

#endif /* ui_h */
