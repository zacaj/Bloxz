// Bloxz.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <SDL/SDL.h>

#include "main.h"
#include <SDL/SDL_video.h>
#include <string.h>
float min(float,float);
float max(float,float);
extern int tx, ty, tim;
extern int firing;
extern int menu;
extern int hst;
extern int gamemode;
extern int score, highscore[3];
extern char name[100];
extern int timeleft;
extern char *playername;
int nameat = 0;
struct  vec2
{
	int x, y;
	vec2()
	{
		x = y = 0;
	}
	vec2(int _x, int _y)
	{
		x = _x*(320.f / 480);
		y = _y*(320.f / 480);
	}
};
int main(int argc, char* argv[])
{
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window *window = SDL_CreateWindow("Bloxz", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 480, 480.f/(320.f/480.f), SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
	SDL_GL_CreateContext(window);
	bool done = 0;
	init();
	
	int start = SDL_GetTicks();
	while (!done)
	{
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				done = 1;
				break;
			case SDL_KEYDOWN:
				if (e.key.keysym.sym == SDLK_ESCAPE)
					done = 1;
				else if (menu == 2)
				{
					if (e.key.keysym.sym == SDLK_BACKSPACE)
					{
						name[--nameat] = 0;
					}
					if (e.key.keysym.sym == SDLK_RETURN)
					{
						menu = 1;
						playername = new char(strlen(name) + 2);
						strcpy(playername, name);
						addHighscore();
						newlevel();
					}
					else if (e.key.keysym.sym < 128 && e.key.keysym.sym>=32)
					{
						name[nameat++] = e.key.keysym.sym;
						name[nameat] = 0;
					}
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
			{
				if (firing) break;;
				vec2 point(e.button.x, e.button.y);
				if (point.x<32 || point.x>320 - 32)
				{
					tx = -1;
					ty = (point.y - 30) / 32;
				}
				else if (point.y<62 || (point.y>316 && point.y < 390))
				{
					ty = -1;
					tx = (point.x) / 32;
				}
				else
				{
					tx = ty = -1;
				}
				if (tx == 0 || tx == 9)
					tx = -1;
				if (ty == 0 || ty == 9)
					ty = -1;
				tim = 0;
			}
				break;
			case SDL_MOUSEMOTION:
				if (e.motion.state)
				{
					if (firing) break;
					vec2 point(e.motion.x, e.motion.y);
					if (point.x<32 || point.x>320 - 32)
					{
						tx = -1;
						ty = (point.y - 30) / 32;
					}
					else if (point.y<62 || (point.y>316 && point.y < 390))
					{
						ty = -1;
						tx = (point.x) / 32;
					}
					else
					{
						tx = ty = -1;
					}
					if (tx == 0 || tx == 9)
						tx = -1;
					if (ty == 0 || ty == 9)
						ty = -1;
				}
				break;
			case SDL_MOUSEBUTTONUP:
			{
				if (firing) break;
				vec2 point(e.button.x, e.button.y);
				if (menu == 0)
				{
					if (point.x<32 && point.y>62 && point.y<316)
						fire(3, (point.y - 30) / 32, point.x);
					else if (point.x>32 * 9 && point.y > 62 && point.y<316)
						fire(1, (point.y - 30) / 32, point.x);
					if (point.y<62 && point.x>32 && point.x<316)
						fire(0, point.x / 32, point.y);
					else if (point.y>316 && point.x>32 && point.x < 32 * 9 && point.y<390)
						fire(2, point.x / 32, point.y);
					if (point.x<26 && point.y>454)
					{
						menu = 1;
					}
				}
				else if (menu == 1)
				{
					if (point.x>160 && gamemode != 0)
						menu = 0;
					else
					{
						if (point.y<330 && point.y>160)
						{
							menu = 0;
							if (point.y < 225)
								gamemode = 1;
							else if (point.y<280)
								gamemode = 2;
							else
								gamemode = 3;
							newlevel();
						}
						if (point.y>360 && point.y<405)
							menu = 3;
						if (point.y>415 && point.y<465)
						{
							menu = 5;
							hst = 0;
						}
					}
				}
				else if (menu == 2)
				{
					if (point.y>320)
					{
//						if (textbox == nil)
						menu = 1;
						playername = new char(strlen(name) + 2);
						strcpy(playername, name);
						addHighscore();
						newlevel();
					}
				}
				else if (menu == 3)
				{
					if (point.x > 16 && point.x<48 && point.y>445 && point.y < 477)
					{
						hst--;
						if (hst<0)
							hst = 2;
					}
					else if (point.x>272 && point.x<304 && point.y>445 && point.y<477)
					{
						hst++;
						if (hst>2)
							hst = 0;
					}
					else if (point.x > 128 && point.x<192 && point.y>445 && point.y < 477)
					{
						menu = 1;
					}
				}
				else if (menu == 4)//gameover
				{
					name[0] = 0;
					strcpy(name, playername);
					nameat = strlen(name);
					menu = 2;
				}
				else if (menu == 5)//high
				{
					if (point.x<26 && point.y>454)
					{
						menu = 1;
					}
					else
					{
						hst++;
						if (hst > 7)
							menu = 1;
					}
				}
				printf("%f,%f\n", point.x, point.y);
				tx = ty = -1;
			}
				break;
			}
		}
		update();
		SDL_GL_SwapWindow(window);
		int elapsed = (SDL_GetTicks() - start);
		SDL_Delay(max(0, 17 - elapsed));
		start = SDL_GetTicks();
	}
	saveHighscore();
	SDL_Quit();
	return 0;
}

