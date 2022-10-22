//
//  ui.h
//  engine
//
//  Created by Scott on 21/10/2022.
//

#ifndef ui_h
#define ui_h

#include <SDL2/SDL.h>

extern void uiInit(SDL_Renderer* target);
extern void uiTerminate();

extern void uiSetTarget(SDL_Renderer* renderer);

extern void uiBeginFrame();

extern int uiBeginWindow(SDL_Rect* rect, int* isActive);

extern void uiEndWindow();

extern void uiAddLines(int n);
extern void uiAddLine();
extern void uiAddPixels(int px);
extern void uiSameLine(int n);

extern int uiSlider(float* value, float min, float max, float step);
extern int uiButton();

#endif /* ui_h */
