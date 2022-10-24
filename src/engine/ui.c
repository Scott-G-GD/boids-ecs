//
//  ui.c
//  engine
//
//  Created by Scott on 21/10/2022.
//

#include "ui.h"
#include <math.h>
#include <stdint.h>

SDL_Renderer* uiTarget;
typedef struct ui_selection_t {
	uintptr_t item;
	short keepSelected;
	enum {
		UI_SELECT_DEFAULT,
		UI_SELECT_TEXT
	} selectKind;
	union {
		struct {
			long start, end;
		} text_select;
	};
} ui_selection_t;

ui_selection_t uiSelection;

uint32_t uiCurrentMouseState;
uint32_t uiLastMouseState;
int uiMouseX, uiMouseY;

SDL_Rect uiCurrentWindow;
int uiIndentLevel;
int uiSingleLineHeight = 30;
int uiLineSpacing = 5;
int uiWindowPadding = 10;
int uiTotalHeight;
int uiRenderSameLine;

SDL_Rect uiNextLineRect;

struct ui_style_t {
	TTF_Font* font;
	int indentPixels;
} uiStyle;

void uiAddPixels(int n);

void uiUpdateMouseState()
{
	uiLastMouseState = uiCurrentMouseState;
	uiCurrentMouseState = SDL_GetMouseState(&uiMouseX, &uiMouseY);
}

int uiCanSelect(void* ptr)
{
	intptr_t item = (intptr_t)ptr;
	
	return uiSelection.item == item || uiSelection.item == 0;
}

void uiSelect(void* ptr, short keepActive)
{
	uiSelection.item = (intptr_t)ptr;
	uiSelection.keepSelected = keepActive;
}

short uiIsSelected(void* ptr)
{
	return uiSelection.item == (intptr_t)ptr;
}

void uiInit(SDL_Renderer* target)
{
	uiTarget = target;
	uiSelection.item = 0;
	memset(&uiStyle, 0, sizeof(struct ui_style_t));
	uiUpdateMouseState();
	uiStyle.indentPixels = 10;
}

void uiTerminate()
{
	
}

void uiSetFont(TTF_Font* font)
{
	uiStyle.font = font;
}

// check if a mouse button was pressed on this ui frame
static inline
short uiMouseButtonPressed(uint32_t buttonMask)
{
	return
	(buttonMask & uiCurrentMouseState) != 0 && // button is down
	(buttonMask & uiLastMouseState) == 0; // button was down
}

// check if a mouse button was released on this ui frame
static inline
short uiMouseButtonReleased(uint32_t buttonMask)
{
	return
	(buttonMask & uiLastMouseState) != 0 && // button was down
	(buttonMask & uiCurrentMouseState) == 0; // button is up
}

// check if a mouse button is currently pressed
static inline
short uiMouseButtonDown(uint32_t buttonMask)
{
	return uiCurrentMouseState & buttonMask;
}

// change the SDL render target
void uiSetTarget(SDL_Renderer* renderer)
{
	uiTarget = renderer;
}

// begin a new frame
void uiBeginFrame()
{
	if(uiSelection.keepSelected == 0)
		uiSelection.item = 0;
	uiUpdateMouseState();
}

// push any number of lines
void uiSkipLines(int n)
{
	while(n-- >= 0)
	{
		uiNextLine();
	}
}

// push a new line
void uiNextLine()
{
	if(uiRenderSameLine > 0)
	{
		uiRenderSameLine--;
		uiNextLineRect.x += uiNextLineRect.w;
	}
	else
	{
		uiAddPixels(uiSingleLineHeight + uiLineSpacing);
	}
}

// push the next line down by a number of pixels
void uiAddPixels(int px)
{
	uiTotalHeight += px;
	uiNextLineRect = uiCurrentWindow;
	uiNextLineRect.h = uiSingleLineHeight;
	uiNextLineRect.y += uiTotalHeight;
	uiNextLineRect.w -= uiIndentLevel * uiStyle.indentPixels * 2;
	uiNextLineRect.x += uiIndentLevel * uiStyle.indentPixels;
}

// draw the next n elements on the same line
void uiSameLine(int n)
{
	if(n > 1)
	{
		uiRenderSameLine += n-1;
		
		uiNextLineRect.w /= n;
	}
}

// start a window fitting within the given rectangle
int uiBeginWindow(SDL_Rect* rect, int* isActive)
{
	uiTotalHeight = 0;
	uiIndentLevel = 0;
	memcpy(&uiCurrentWindow, rect, sizeof(SDL_Rect));
	
	if(*isActive)
	{
		SDL_SetRenderDrawColor(uiTarget, 20, 20, 20, 255);
		SDL_RenderFillRect(uiTarget, rect);
	}
	
	uiNextLineRect = uiCurrentWindow;
	uiNextLineRect.h = uiSingleLineHeight;
	int toggled = uiButton();
	
	uiCurrentWindow.x += uiWindowPadding;
	uiCurrentWindow.y += uiWindowPadding;
	uiCurrentWindow.w -= uiWindowPadding*2;
	uiCurrentWindow.h -= uiWindowPadding*2;
	
	uiNextLineRect.x = uiCurrentWindow.x;
	uiNextLineRect.w = uiCurrentWindow.w;
	uiNextLineRect.y = uiCurrentWindow.y + uiTotalHeight;
	uiNextLineRect.h = uiSingleLineHeight;
	
	if(toggled)
	{
		int last = *isActive;
		*isActive = !(*isActive);
		return last;
	}
	
	return (*isActive);
}

// check if point (x,y) is in r
int uiInArea(SDL_Rect* r, int x, int y)
{
	int x_min = r->x, x_max = r->x + r->w;
	int y_min = r->y, y_max = r->y + r->h;
	return (x >= x_min && x < x_max && y >= y_min && y < y_max);
}

void uiDrawText(const char* text, SDL_Rect dstrect, SDL_Color colour)
{
	int height = TTF_FontHeight(uiStyle.font);
	double fontToLineHeight = (double)height/(double)uiSingleLineHeight;
	
	SDL_Rect srcrect = { 0, 0, dstrect.w * fontToLineHeight, dstrect.h * fontToLineHeight };
	SDL_Surface* surface = TTF_RenderText_Solid_Wrapped(uiStyle.font, text, colour, srcrect.w);
	
	dstrect.w = surface->w / fontToLineHeight;
	
	if(surface == NULL)
	{
		SDL_Log("Failed to render text to surface:\n%s", SDL_GetError());
		return;
	}
	
	SDL_Texture* tex = SDL_CreateTextureFromSurface(uiTarget, surface);
	SDL_FreeSurface(surface);
	
	if(tex == NULL)
		return;
	
	SDL_RenderCopy(uiTarget, tex, &srcrect, &dstrect);
	
	SDL_DestroyTexture(tex);
}

// draw an interactive slider with a min, max and step
int uiSlider(float* value, float min, float max, float step)
{
	const int SLIDER_WIDTH = 10;
	const int LINE_THICKNESS = 5;
	
	float startValue = *value;
	int valueChanged = 0;
	float valuePercentage = ((*value)-min) / (max - min);
	
	SDL_Rect position = uiNextLineRect;
	position.x += SLIDER_WIDTH/2;
	position.w -= SLIDER_WIDTH;
	
	SDL_Rect lineRect = {
		position.x, position.y + uiSingleLineHeight/2,
		position.w, LINE_THICKNESS
	};
	
	SDL_Color sliderColor = {230, 230, 230, 255};

	if(uiMouseButtonPressed(SDL_BUTTON_LEFT))
	{
		if(uiCanSelect(value) && uiInArea(&position, uiMouseX, uiMouseY))
		{
			uiSelect(value, 1);
		}
	}
	else if(uiIsSelected(value) && !uiMouseButtonDown(SDL_BUTTON_LEFT))
	{
		uiSelection.keepSelected = 0;
	}
	
	if(uiIsSelected(value))
	{
		valuePercentage = (float)(uiMouseX - position.x) / (float)(position.w);
		
		if(valuePercentage < 0)
			valuePercentage = 0;
		else if(valuePercentage > 1)
			valuePercentage = 1;
		
		(*value) = valuePercentage * (max - min);
		(*value) = roundf((*value) * (1.f/step)) * step + min;
		valuePercentage = ((*value) - min) / (max-min);
		
		
		if(*value != startValue)
		{
			valueChanged = 1;
		}
	}
	
	SDL_Rect sliderRect = {
		position.x + valuePercentage * position.w - SLIDER_WIDTH/2, position.y,
		SLIDER_WIDTH, uiSingleLineHeight
	};
	
	SDL_SetRenderDrawColor(uiTarget, 100, 100, 100, 255);
	SDL_RenderFillRect(uiTarget, &lineRect);
	
	if(valuePercentage >= 0 && valuePercentage <= 1)
	{
		SDL_SetRenderDrawColor(uiTarget, sliderColor.r, sliderColor.g, sliderColor.b, sliderColor.a);
		SDL_RenderFillRect(uiTarget, &sliderRect);
	}
	
	uiNextLine();
	
	return valueChanged;
}

// draw a clickable button
int uiButton()
{
	SDL_Rect position = uiNextLineRect;
	short clicked = 0;
	
	if(uiMouseButtonPressed(SDL_BUTTON_LEFT))
	{
		if(uiCanSelect(0) && uiInArea(&position, uiMouseX, uiMouseY))
		{
			uiSelect(0, 0);
			clicked = 1;
		}
	}
	
	SDL_SetRenderDrawColor(uiTarget, 100, 100, 100, 255);
	SDL_RenderFillRect(uiTarget, &position);
	
	uiNextLine();

	return clicked;
}

void uiLabel(const char* label)
{
	SDL_Colour white = {255, 255, 255, 255};
	uiDrawText(label, uiNextLineRect, white);
	uiNextLine();
}

void uiLabelNext(const char* label, float ratio)
{
	int y = uiNextLineRect.y;
	uiNextLineRect.w = uiCurrentWindow.w * ratio;
	uiLabel(label);
	uiTotalHeight -= uiSingleLineHeight + uiLineSpacing;
	uiNextLineRect.y = y;
	uiNextLineRect.x += uiCurrentWindow.w * ratio;
	uiNextLineRect.w = uiCurrentWindow.w * (1-ratio);
	uiNextLineRect.h = uiSingleLineHeight;
}

void uiHeader(const char* label)
{
	uiSubIndent();
	uiAddPixels(uiSingleLineHeight/2);
	uiLabel(label);
	uiAddIndent();
}

void uiSetIndent(int n)
{
	uiIndentLevel = n;
	uiIndentLevel = uiIndentLevel >= 0 ? uiIndentLevel : 0;
	// force update of next line rect
	uiAddPixels(0);
}

void uiAddIndent()
{
	uiSetIndent(uiIndentLevel + 1);
}

void uiSubIndent()
{
	uiSetIndent(uiIndentLevel - 1);
}
