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
intptr_t uiCurrentSelected;
intptr_t uiLastSelected;

uint32_t uiCurrentMouseState;
uint32_t uiLastMouseState;
int uiMouseX, uiMouseY;

SDL_Rect uiCurrentWindow;
int uiSingleLineHeight = 30;
int uiLineSpacing = 5;
int uiTotalHeight;

void uiUpdateMouseState()
{
	uiLastMouseState = uiCurrentMouseState;
	uiCurrentMouseState = SDL_GetMouseState(&uiMouseX, &uiMouseY);
}

int uiCanSelect() { return uiCurrentSelected == 0 && uiLastSelected == 0; }

void uiSelect(void* ptr)
{
	uiCurrentSelected = (intptr_t)ptr;
}

void uiInit(SDL_Renderer* target)
{
	uiTarget = target;
	uiCurrentSelected = uiLastSelected = 0;
	uiUpdateMouseState();
}

void uiTerminate()
{
	
}

void uiSetTarget(SDL_Renderer* renderer)
{
	uiTarget = renderer;
}

void uiBeginFrame()
{
	uiLastSelected = uiCurrentSelected;
	uiCurrentSelected = 0;
	uiUpdateMouseState();
}

void uiAddLines(int n)
{
	uiTotalHeight += n * (uiSingleLineHeight + uiLineSpacing);
}

void uiAddLine() { uiAddLines(1); }

int uiBeginWindow(SDL_Rect* rect)
{
	uiTotalHeight = 0;
	memcpy(&uiCurrentWindow, rect, sizeof(SDL_Rect));
	
	return 1;
}

int uiInArea(SDL_Rect* r, int x, int y)
{
	int x_min = r->x, x_max = r->x + r->w;
	int y_min = r->y, y_max = r->y + r->h;
	return (x >= x_min && x < x_max && y >= y_min && y < y_max);
}

int uiSlider(float* value, float min, float max, float step)
{
	const int SLIDER_WIDTH = 10;
	const int LINE_THICKNESS = 5;
	
	float startValue = *value;
	int valueChanged = 0;
	float valuePercentage = ((*value)-min) / (max - min);
	
	SDL_Rect position = uiCurrentWindow;
	position.y += uiTotalHeight;
	position.h = uiSingleLineHeight;
	
	SDL_Rect lineRect = {
		position.x, position.y + uiSingleLineHeight/2,
		position.w, LINE_THICKNESS
	};
	SDL_Rect totalRect = {
		position.x - SLIDER_WIDTH/2, position.y,
		position.w + SLIDER_WIDTH/2, uiSingleLineHeight
	};
	
	SDL_Color sliderColor = {230, 230, 230, 255};

	if(uiCurrentMouseState & SDL_BUTTON_LEFT)
	{
		if(uiCurrentSelected == 0 && (uiLastSelected == (intptr_t)value || uiInArea(&totalRect, uiMouseX, uiMouseY)))
		{
			uiSelect(value);
		}
	}
	else if(uiCurrentSelected == (intptr_t)value)
	{
		uiSelect(NULL);
	}
	
	if(uiCurrentSelected == (intptr_t)value)
	{
		valuePercentage = (float)(uiMouseX - position.x) / (float)(position.w);
		
		if(valuePercentage < 0)
			valuePercentage = 0;
		else if(valuePercentage > 1)
			valuePercentage = 1;
		
		(*value) = valuePercentage * (max - min);
		(*value) = floorf((*value) * (1.f/step)) * step + min;
		
		if(*value != startValue)
		{
			valueChanged = 1;
		}
	}
	
	SDL_Rect sliderRect = {
		position.x + valuePercentage * position.w, position.y,
		SLIDER_WIDTH, uiSingleLineHeight
	};
	
	SDL_SetRenderDrawColor(uiTarget, 100, 100, 100, 255);
	SDL_RenderFillRect(uiTarget, &lineRect);
	SDL_SetRenderDrawColor(uiTarget, sliderColor.r, sliderColor.g, sliderColor.b, sliderColor.a);
	SDL_RenderFillRect(uiTarget, &sliderRect);
	
	uiAddLine();
	return valueChanged;
}

int uiButton()
{
	SDL_Rect position = uiCurrentWindow;
	position.h = uiSingleLineHeight;
	position.y += uiTotalHeight;
	
	SDL_SetRenderDrawColor(uiTarget, 100, 100, 100, 255);
	SDL_RenderFillRect(uiTarget, &position);
	
	uiAddLine();
	
	if((uiLastMouseState & SDL_BUTTON_LEFT) == 0 && (uiCurrentMouseState & SDL_BUTTON_LEFT) != 0)
	{
		if(uiCanSelect() && uiInArea(&position, uiMouseX, uiMouseY))
		{
			return 1;
		}
	}
	return 0;
}
