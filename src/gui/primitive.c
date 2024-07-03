#include "app.h"

void
YU_SetRenderDrawColor(SDL_Renderer *renderer, SDL_Color c)
{
	SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
}

/**
 * Allocates thick points and calls SDL_RenderLines
 */
void
YU_DrawLinesThick(SDL_Renderer *renderer, SDL_FPoint p1, SDL_FPoint p2, int thick, SDL_Color c)
{
	SDL_FPoint *points = malloc(sizeof(SDL_FPoint)*thick);
	int i = 0;
	while (i < thick)
	{
		points[i].x = p1.x;
		points[i].y = p1.y + i;
		i++;
	}
	YU_SetRenderDrawColor(g_inst.r, c);
	SDL_RenderLines(renderer, points, thick);
	free(points);
}

void 
DrawFilledCircle(SDL_Renderer * renderer, int32_t centreX, int32_t centreY, int32_t radius)
{
    const int32_t diameter = (radius * 2);

    int32_t x = (radius - 1);
    int32_t y = 0;
    int32_t tx = 1;
    int32_t ty = 1;
    int32_t error = (tx - diameter);

    while (x >= y)
    {
        // Draw horizontal lines to fill the circle
        SDL_RenderLine(renderer, centreX - x, centreY - y, centreX + x, centreY - y);
        SDL_RenderLine(renderer, centreX - x, centreY + y, centreX + x, centreY + y);
        SDL_RenderLine(renderer, centreX - y, centreY - x, centreX + y, centreY - x);
        SDL_RenderLine(renderer, centreX - y, centreY + x, centreX + y, centreY + x);

        if (error <= 0)
        {
            ++y;
            error += ty;
            ty += 2;
        }

        if (error > 0)
        {
            --x;
            tx += 2;
            error += (tx - diameter);
        }
    }
}

void 
DrawCircle(SDL_Renderer *renderer, int32_t centreX, int32_t centreY, int32_t radius)
{
   const int32_t diameter = (radius * 2);

   int32_t x = (radius - 1);
   int32_t y = 0;
   int32_t tx = 1;
   int32_t ty = 1;
   int32_t error = (tx - diameter);

   while (x >= y)
   {
      //  Each of the following renders an octant of the circle
      SDL_RenderPoint(renderer, centreX + x, centreY - y);
      SDL_RenderPoint(renderer, centreX + x, centreY + y);
      SDL_RenderPoint(renderer, centreX - x, centreY - y);
      SDL_RenderPoint(renderer, centreX - x, centreY + y);
      SDL_RenderPoint(renderer, centreX + y, centreY - x);
      SDL_RenderPoint(renderer, centreX + y, centreY + x);
      SDL_RenderPoint(renderer, centreX - y, centreY - x);
      SDL_RenderPoint(renderer, centreX - y, centreY + x);

      if (error <= 0)
      {
         ++y;
         error += ty;
         ty += 2;
      }

      if (error > 0)
      {
         --x;
         tx += 2;
         error += (tx - diameter);
      }
   }
}


