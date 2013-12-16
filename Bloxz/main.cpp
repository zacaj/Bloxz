#include "stdafx.h"
#include <SDL/SDL_opengl.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "image.cpp"
#include <string>
#include <vector>
#include <iostream>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utility>
using namespace std;
#include "glfont2.h"
#include <minwindef.h>
#include <SDL/SDL_timer.h>
typedef struct
{
	int type;
	int x, y;
} Blok;
Blok level[10][10];
#define BLOKSIZE 32
#define SPEED 1
Blok bad;
Blok blokAt(int x, int y)
{
	if (x >= 1 && x<9 && y >= 1 && y<9)
		return level[x][y];
	else
		return bad;
}
float min(float a, float b)
{
	if (a < b)
		return a;
	else return b;
}
float max(float a, float b)
{
	if (a > b)
		return a;
	else return b;
}
int temp;
int firing = 0;
void fire(int Side, int At, int p);
void checkexplode();
int mode = 0;
int lasttime = 0;
float fpsm = 1;
int side, at;
int next_, current;
TargaImage *imgs[6];
TargaImage *particle;
TargaImage *gradient;
TargaImage *background;
TargaImage *buttons;
TargaImage *buttontext;
TargaImage *menutext;
TargaImage *title;
TargaImage *help[8];
GLFont font;
GLFont sfont;
int menu = 1;
int score = 0, highscore[3];
int nBloxz = 0;
int timeleft = 0;
void rect(int x, int y, int x2, int y2)
{

	GLfloat squareVertices[] = {
		x, y,
		x + x2, y,
		x, y + y2,
		x + x2, y + y2,
	};
	GLfloat uvs[] = {
		0, 1,
		1, 1,
		0, 0,
		1, 0,
	};
	glVertexPointer(2, GL_FLOAT, 0, squareVertices);
	glTexCoordPointer(2, GL_FLOAT, 0, uvs);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
void rectline(int x, int y, int x2, int y2)
{

	GLfloat squareVertices[] = {
		x, y,
		x + x2, y,
		x + x2, y + y2,
		x, y + y2,
	};
	/*GLfloat uvs[]={
	0,1,
	1,1,
	1,0,
	0,0,
	};*/
	glVertexPointer(2, GL_FLOAT, 0, squareVertices);
	//glTexCoordPointer(2,GL_FLOAT,0,uvs);
	glDisable(GL_TEXTURE_2D);
	glDrawArrays(GL_LINE_LOOP, 0, 4);
	glEnable(GL_TEXTURE_2D);
}
typedef struct
{
	float x, y;
} vec2f;
typedef struct
{
	int x, y;
} vec2i;
vec2f *scoreVerts;

#define MAXPAR 1024*16
vec2f parp[MAXPAR];
vec2f parv[MAXPAR];
unsigned char parc[MAXPAR][4];
int nPar = 0;
float parvx[32][32];
float parvy[32][32];

#define DEG2RAD 0.017453292519943295769236907684886
void randPars(vec2f pos, float r, float g, float b)
{
	for (int i = 0; i<rand() % 8 + 28; i++)
	{
		if (nPar >= MAXPAR)
		{;
			return;
		}
		float angle = rand() % 360;
		angle *= DEG2RAD;
		parv[nPar].x = sin(angle)*((rand() % 40)*.1 + 1) / 256;
		parv[nPar].y = cos(angle)*((rand() % 40)*.1 + 1) / 256;
		parp[nPar].x = pos.x + parv[nPar].x;
		parp[nPar].y = pos.y + parv[nPar].y;
		parc[nPar][0] = r;
		parc[nPar][1] = g;
		parc[nPar][2] = b;
		parc[nPar][3] = 1.0;
		nPar++;
	}
}
char* pathToFile(char *name, char *ext)
{
	char *ret = new char[strlen(name) + strlen(ext) + 3];
	strcpy(ret, name);
	strcat(ret, ext);
	return ret;
}
char* i2s(int n)
{
	string temp;
	char t[50];
	temp.push_back('$');
	sprintf(t, "%i", n);

	temp.append(t);
	for (int i = 3; i<temp.size() - 1; i += 4)
		temp.insert(temp.end() - i, ',');
	temp.erase(temp.begin());
	if (temp[0] == '-' && temp[1] == ',')
		temp.erase(temp.begin() + 1);
	char *ret = new char[temp.size() + 1];
	for (int i = 0; i<temp.size(); i++)
		ret[i] = temp[i];
	ret[temp.size()] = '\0';
	return ret;
}
char* t2s(int n)
{
	string temp;
	char t[50];
	sprintf(t, "%i", n);
	temp.append(t);
	temp.erase(temp.end() - 1);
	temp.insert(temp.end() - 2, '.');
	char *ret = new char[temp.size() + 1];
	for (int i = 0; i<temp.size(); i++)
		ret[i] = temp[i];
	ret[temp.size()] = '\0';
	return ret;
}

typedef struct
{
	string name;
	int score;
} highscore_t;
highscore_t highscores[3][10];
int tx = -1, ty = -1;
int missed = 0;
int tim;
int blokn[7];
int blokd = 0;;
vec2f vert[8 * 8];
vec2f uvs[8 * 8];
int nVert = 0;
int gamemode = 0;
int start;
int totaltime = 0;
char *playername;
int hst = 0;
int getLow(int n)
{
	int i;
	for (i = 0; i<10; i++)
	if (highscores[n][i].score<0) break;
	if (i>0)
		return highscores[n][i - 1].score;
	else return -1;
}
void addHighscore()
{
	if (score>getLow(gamemode - 1));
	{
		int i = 9;
		while (i >= 0 && score>highscores[gamemode - 1][i].score) i--;
		if (i<9)
		{
			i++;
			for (int j = 9; j>i; j--)
			{
				highscores[gamemode - 1][j].score = highscores[gamemode - 1][j - 1].score;
				highscores[gamemode - 1][j].name = highscores[gamemode - 1][j - 1].name;
			}
			highscores[gamemode - 1][i].name = playername;
			highscores[gamemode - 1][i].score = score;
		}
		for (int i = 0; i<10; i++)
			printf("%i: %s %i\n", i, highscores[gamemode - 1][i].name.c_str(), highscores[gamemode - 1][i].score);
	}
}

void newlevel()
{
	nBloxz = 0;
	glLoadIdentity();
	for (int j = 0; j<10; j++)
	for (int i = 0; i<10; i++)
	{
		level[i][j].type = 0;
		level[i][j].x = (i - 5)*BLOKSIZE;
		level[i][j].y = (j - 5)*BLOKSIZE;
	}

	for (int i = 0; i<7; i++)
		blokn[i] = 0;
	next_ = rand() % 6 + 1;
	current = rand() % 6 + 1;
	for (int j = 1; j<9; j++)
	for (int i = 1; i<9; i++)
	{
		while (1)
		{
			level[i][j].type = rand() % 6 + 1;
			if (blokAt(i - 1, j).type != level[i][j].type)
			if (blokAt(i + 1, j).type != level[i][j].type)
			if (blokAt(i, j - 1).type != level[i][j].type)
			if (blokAt(i, j + 1).type != level[i][j].type)
				break;
		}
		blokn[level[i][j].type]++;
		nBloxz++;
	}
	mode = 0;
	nPar = 0;
	score = 0;
	blokd = 0;
	missed = 0;
	side = 0;
	at = 0;
	firing = 0;
	if (gamemode == 3)
	{
		timeleft = 30000;
	}
	else
		timeleft = 0;
	totaltime = 0;
}
int menucur, menuold = -1;
float menuat = 0;
void saveHighscore()
{
	FILE *fp;
	fp = fopen("hiscore.dat", "w");
	if (fp != NULL)
	{
		fprintf(fp, "%i\n", highscore[0]);
		fprintf(fp, "%i\n", highscore[1]);
		fprintf(fp, "%i\n", highscore[2]);
		for (int j = 0; j<10; j++)
		for (int i = 0; i<10; i++)
			fprintf(fp, "%i %i,%i\n", level[i][j].type, level[i][j].x, level[i][j].y);
		fprintf(fp, "%i\n", nBloxz);
		for (int i = 0; i<7; i++)
			fprintf(fp, "%i\n", blokn[i]);
		fprintf(fp, "%i,%i\n", current, next_);
		fprintf(fp, "%i\n", mode);
		fprintf(fp, "%i\n", score);
		fprintf(fp, "%i\n", blokd);
		fprintf(fp, "%i\n", menu);
		fprintf(fp, "%i\n", gamemode);
		fprintf(fp, "%i\n", missed);
		fprintf(fp, "%i,%i\n", side, at);
		fprintf(fp, "%i\n", firing);
		fprintf(fp, "%i\n", temp);
		fprintf(fp, "%i\n", timeleft);
		fprintf(fp, "%s\n", playername);
		for (int i = 0; i<3; i++)
		for (int j = 0; j<10; j++)
			fprintf(fp, "%s %i\n", highscores[i][j].name.c_str(), highscores[i][j].score);
		fclose(fp);
	}
	else
		printf("HISCORE NO SAVE");
}
void init()
{
	srand(time(NULL));
	FILE *fp;
	fp = fopen("hiscore.dat", "r");
	if (fp != NULL)
	{
		fscanf(fp, "%i\n", &highscore[0]);
		fscanf(fp, "%i\n", &highscore[1]);
		fscanf(fp, "%i\n", &highscore[2]);
		for (int j = 0; j<10; j++)
		for (int i = 0; i<10; i++)
		{
			fscanf(fp, "%i %i,%i\n", &level[i][j].type, &level[i][j].x, &level[i][j].y);
		}
		fscanf(fp, "%i\n", &nBloxz);
		for (int i = 0; i<7; i++)
			fscanf(fp, "%i\n", &blokn[i]);
		fscanf(fp, "%i,%i\n", &current, &next_);
		fscanf(fp, "%i\n", &mode);
		fscanf(fp, "%i\n", &score);
		fscanf(fp, "%i\n", &blokd);
		fscanf(fp, "%i\n", &menu);
		fscanf(fp, "%i\n", &gamemode);
		fscanf(fp, "%i\n", &missed);
		fscanf(fp, "%i,%i\n", &side, &at);
		fscanf(fp, "%i\n", &firing);
		fscanf(fp, "%i\n", &temp);
		fscanf(fp, "%i\n", &timeleft);
		printf("dg\n");
		char t[100];
		fscanf(fp, "%s\n", &t);
		playername = new char[strlen(t)];
		strcpy(playername, t);
		printf("loaded %s\n", playername);
		for (int i = 0; i<3; i++)
		for (int j = 0; j<10; j++)
		{
			fscanf(fp, "%s %i\n", &t, &highscores[i][j].score);
			highscores[i][j].name = t;
		}

		fclose(fp);
		nPar = 0;
	}
	
	if (current ==0 || fp==NULL)
	{
		playername = "Player";
		for (int i = 0; i<3; i++)
		for (int j = 0; j<10; j++)
		{
			highscores[i][j].name = "Player";
			printf("%s\n", highscores[i][j].name.c_str());
			highscores[i][j].score = -1;
		}
		//	fscanf(fp,"%s\n",&playername);
		printf("loaded %s\n", playername);
		saveHighscore();
		newlevel();
	}
	glMatrixMode(GL_PROJECTION);
	glOrtho(-128 - BLOKSIZE, 128 + BLOKSIZE, 128 + BLOKSIZE + 100, -128 - BLOKSIZE - 60, -128, 128);
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, .01);
	level[9][9].type = -1;
	char str[20];
	for (int i = 1; i<7; i++)
	{
		sprintf(str, "%i", i);
		imgs[i - 1] = LoadTargaImage(pathToFile(str, ".tga"));
	}
	for (int i = 1; i<9; i++)
	{
		sprintf(str, "h%i", i);
		help[i - 1] = LoadTargaImage(pathToFile(str, ".tga"));
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	/*glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(0,1);
	glVertex2f(level[i][j].x+1,level[i][j].y+1);
	glTexCoord2f(1,1);
	glVertex2f(level[i][j].x+BLOKSIZE-1,level[i][j].y+1);
	glTexCoord2f(0,0);
	glVertex2f(level[i][j].x+1,level[i][j].y+BLOKSIZE-1);
	glTexCoord2f(1,0);
	glVertex2f(level[i][j].x+BLOKSIZE-1,level[i][j].y+BLOKSIZE-1);
	glEnd();*/
	particle = LoadTargaImage(pathToFile("particle", ".tga"));
	gradient = LoadTargaImage(pathToFile("gradient", ".tga"));
	background = LoadTargaImage(pathToFile("background", ".tga"));
	buttons = LoadTargaImage(pathToFile("buttons", ".tga"));
	buttontext = LoadTargaImage(pathToFile("buttontext", ".tga"));
	menutext = LoadTargaImage(pathToFile("menutext", ".tga"));
	title = LoadTargaImage(pathToFile("bloxz", ".tga"));
	GLuint *fontID = new GLuint[2];
	glGenTextures(2, fontID);
	font.Create(pathToFile("font", ".glf"), fontID[0]);
	sfont.Create(pathToFile("smallfont", ".glf"), fontID[1]);

	for (int y = 0; y<32; y++)
	for (int x = 0; x<32; x++)
	{
		parvx[x][y] = sin(atan2(x - 16, y - 16)) * 16 * .05;
		parvy[x][y] = cos(atan2(x - 16, y - 16)) * 16 * .05;
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	for (float i = 65; i<92; i += 25 / 8)
	{
		vert[nVert].x = i;
		vert[nVert].y = 260;
		uvs[nVert].x = 0;
		uvs[nVert].y = 0;
		nVert++;
		vert[nVert].x = i;
		vert[nVert].y = 170 - sin((i - 65)*3.1415 / 2 / 25) * 25;
		uvs[nVert].x = 0;
		uvs[nVert].y = 1;
		nVert++;
		vert[nVert].x = i + 25 / 8;
		vert[nVert].y = 260;
		uvs[nVert].x = 1;
		uvs[nVert].y = 0;
		nVert++;
		vert[nVert].x = i + 25 / 8;
		vert[nVert].y = 170 - sin((i + 25 / 8 - 65)*3.1415 / 2 / 25) * 25;
		uvs[nVert].x = 1;
		uvs[nVert].y = 1;
		nVert++;
	}

}
int last = SDL_GetTicks();
char name[100];
void update()
{            /* OpenGL animation code goes here */
	glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0, -30, 0);
	if (menu == 0 || menu == 4)
	{
		glColor3f(0, 0, 1);
		Bind(particle);
		for (int i = 1; i<9; i++)
		{
			rect(-149, -160 + i*BLOKSIZE + 11, 10, 10);
			rect(-160 + i*BLOKSIZE + 11, -149, 10, 10);
			rect(138, -160 + i*BLOKSIZE + 11, 10, 10);
			rect(-160 + i*BLOKSIZE + 11, 138, 10, 10);
		}
		for (int i = 0; i<nPar; i++)
		{
			parp[i].x += parv[i].x;
			parp[i].y += parv[i].y;
			if (parc[i][3]<.01 || parp[i].x<-170 || parp[i].x>170 || parp[i].y<-200 || parp[i].y>200)
			{
				nPar--;
				parp[i] = parp[nPar];
				parv[i] = parv[nPar];
				parc[i][0] = parc[nPar][0];
				parc[i][1] = parc[nPar][1];
				parc[i][2] = parc[nPar][2];
				parc[i][3] = parc[nPar][3];
			}
			if (blokAt((int)(parp[i].x + 160) / 32, (int)(parp[i].y + 160) / 32).type>0)
			{
				float tx, ty, tx2, ty2;
				tx = (parp[i].x + 160) - ((int)(parp[i].x + 160) / 32) * 32;
				ty = (parp[i].y + 160) - ((int)(parp[i].y + 160) / 32) * 32;
				tx2 = (parp[i].x + 160) - ((int)(parp[i].x + 160 + 32) / 32) * 32;
				ty2 = (parp[i].y + 160) - ((int)(parp[i].y + 160 + 32) / 32) * 32;
				tx = min(fabs(tx), fabs(tx2));
				ty = min(fabs(ty), fabs(ty2));
				if (tx<ty)
				{
					parv[i].x = -parv[i].x;
					parv[i].x *= 1;
					parv[i].y *= 2;
				}
				else
				{
					parv[i].y = -parv[i].y;
					parv[i].x *= 2;
					parv[i].y *= 1;
				}
				parv[i].x *= .8;
				parv[i].y *= .8;
				parc[i][3] *= .6;
				parp[i].x = -500;
			}
			parc[i][3] -= .01;
		}
	}
	glColor3f(1, 1, 1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, parp);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, parc);
	//glEnable(GL_POINT_SPRITE_OES);
	// glTexEnvf(GL_POINT_SPRITE_OES,GL_COORD_REPLACE_OES,GL_TRUE);
	glPointSize(1.5);
	glEnable(GL_POINT_SMOOTH);
	glDrawArrays(GL_POINTS, 0, nPar);
	// glDisable(GL_POINT_SPRITE_OES);
	// glTexEnvf(GL_POINT_SPRITE_OES,GL_COORD_REPLACE_OES,GL_FALSE);
	glDisableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glColor3f(1, 1, 1);
	bool moved = 0;
	if (tx != -1 || ty != -1) tim++;
	for (int j = 0; j<10; j++)
	for (int i = 0; i<10; i++)
	{
		if (level[i][j].type <= 0)
			continue;
		if ((tx != -1 || ty != -1) && tim>6 && !firing && !menu)
		if (tx == i || ty == j)
			glColor4f(1, 1, 1, 1);
		else
			glColor4f(.5, .5, .5, 1);

		Bind(imgs[level[i][j].type - 1]);
		/* switch(level[i][j].type)
		{
		case 0:
		glColor3f(.3,.3,.3);
		break;
		case 1:
		glColor3f(1,0,0);
		break;
		case 2:
		glColor3f(1,.5,0);
		break;
		case 3:
		glColor3f(1,1,0);
		break;
		case 4:
		glColor3f(0,1,0);
		break;
		case 5:
		glColor3f(0,0,1);
		break;
		case 6:
		glColor3f(1,0,1);
		break;
		case 7:
		glColor3f(1,1,1);
		break;
		}*/
		if (mode && !menu)
		{
			if (level[i][j].x<(i - 5)*BLOKSIZE)
			{
				moved = 1;
				level[i][j].x += SPEED;
			}
			else if (level[i][j].x>(i - 5)*BLOKSIZE)
			{
				moved = 1;
				level[i][j].x -= SPEED;
			}
			if (level[i][j].y<(j - 5)*BLOKSIZE)
			{
				moved = 1;
				level[i][j].y += SPEED;
			}
			else if (level[i][j].y>(j - 5)*BLOKSIZE)
			{
				moved = 1;
				level[i][j].y -= SPEED;
			}
		}
		if (level[i][j].type>0)
		{
			rect(level[i][j].x, level[i][j].y, BLOKSIZE, BLOKSIZE);
		}
	}
	if (!moved && mode == 1 && !menu)
	{
		fire(side, at, 0);
	}
	else if (!moved && mode == 2 && !menu)
	{
		mode = 0;
		checkexplode();
	}
	/* glColor3f(0,0,0);
	rect(-160,-160,160,-128);
	rect(-160,160,160,128);
	rect(-160,160,-128,-160);
	rect(160,160,128,-160);*/
	glLoadIdentity();
	Bind(gradient);
	float mult = 1.f - (float)min(firing, 40) / 40.f;
	if (!firing)
	{
		switch (current)
		{
		case 1:
			glColor4f(1, 117 / 255.f, 0, .5);
			break;
		case 2:
			glColor4f(247 / 255.f, 192 / 255.f, 0, .5);
			break;
		case 3:
			glColor4f(0, 203 / 255.f, 67 / 255.f, .5);
			break;
		case 4:
			glColor4f(0, 126 / 255.f, 240 / 255.f, .5);
			break;
		case 5:
			glColor4f(168 / 255.f, 0, 240 / 255.f, .5);
			break;
		case 6:
			glColor4f(248 / 255.f, 0, 0, .5);
			break;
		}
		rect(-160, 170, 230, 90);
		rect(97, 145, 68, 115);
		glVertexPointer(2, GL_FLOAT, 0, vert);
		glTexCoordPointer(2, GL_FLOAT, 0, uvs);
		glTranslatef(5, 0, 0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, nVert);
		glTranslatef(-5, 0, 0);
	}
	else if (menu != 4)
	{

		float old[3];
		float cur[3];
		switch (temp)
		{
		case 1:
			old[0] = 1;
			old[1] = 117 / 255.f;
			old[2] = 0;
			break;
		case 2:
			old[0] = 247 / 255.f;
			old[1] = 192 / 255.f;
			old[2] = 0;
			break;
		case 3:
			old[0] = 0;
			old[1] = 203 / 255.f;
			old[2] = 67 / 255.f;
			break;
		case 4:
			old[0] = 0;
			old[1] = 126 / 255.f;
			old[2] = 240 / 255.f;
			break;
		case 5:
			old[0] = 168 / 255.f;
			old[1] = 0;
			old[2] = 240 / 255.f;
			break;
		case 6:
			old[0] = 248 / 255.f;
			old[1] = 0;
			old[2] = 0;
			break;
		}
		switch (current)
		{
		case 1:
			cur[0] = 1;
			cur[1] = 117 / 255.f;
			cur[2] = 0;
			break;
		case 2:
			cur[0] = 247 / 255.f;
			cur[1] = 192 / 255.f;
			cur[2] = 0;
			break;
		case 3:
			cur[0] = 0;
			cur[1] = 203 / 255.f;
			cur[2] = 67 / 255.f;
			break;
		case 4:
			cur[0] = 0;
			cur[1] = 126 / 255.f;
			cur[2] = 240 / 255.f;
			break;
		case 5:
			cur[0] = 168 / 255.f;
			cur[1] = 0;
			cur[2] = 240 / 255.f;
			break;
		case 6:
			cur[0] = 248 / 255.f;
			cur[1] = 0;
			cur[2] = 0;
			break;
		}
		glColor4f(cur[0] * (1 - mult) + old[0] * mult, cur[1] * (1 - mult) + old[1] * mult, cur[2] * (1 - mult) + old[2] * mult, .5);
		rect(-160, 170, 230, 90);
		rect(97, 145, 68, 115);
		glVertexPointer(2, GL_FLOAT, 0, vert);
		glTexCoordPointer(2, GL_FLOAT, 0, uvs);
		glTranslatef(5, 0, 0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, nVert);
		glTranslatef(-5, 0, 0);
	}
	glColor4f(1, 1, 1, 1);
	if (!firing)
	{
		Bind(imgs[current - 1]);
		rect(102, 160, 48, 48);
		Bind(imgs[next_ - 1]);
		rect(114, 220, 24, 24);
	}
	else
	{
		Bind(imgs[temp - 1]);
		rect(102, 160, 48, 48);
		Bind(imgs[next_ - 1]);
		rect(114, 220, 24, 24);
		Bind(imgs[current - 1]);
		rect((114 - 102)*mult + 102, (220 - 160)*mult + 160, (48 - 24)*(1 - mult) + 24, (48 - 24)*(1 - mult) + 24);
	}
	glColor3f(1, 1, 1);
	/* glBindTexture(GL_TEXTURE_2D,0);
	glVertexPointer(2,GL_FLOAT,0,scoreVerts);
	glTranslatef(0,288-BLOKSIZE,0);
	glScalef(1.066,-1.066,1.066);
	//  glDrawArrays(GL_LINES,0,80);
	glScalef(-1,1,1);*/
	// glDrawArrays(GL_LINES,0,80);
	font.Begin();
	glLoadIdentity();
	glScalef(1, -1, 1);
	glColor3f(1, 1, 1);
	font.DrawString("SCORE", .9, -120, -210);
	if (gamemode != 3)
		font.DrawString("HIGHSCORE", .64, -145, -185);
	else
		font.DrawString("TIME LEFT", .64, -125, -185);
	// char str[10];
	// sprintf(str,"%i",score);
	font.DrawString(i2s(score), .9, 85 - font.GetStringWidth(i2s(score), .9), -210);
	// sprintf(str,"%i",highscore);
	if (gamemode != 3)
		font.DrawString(i2s(highscore[gamemode - 1]), .64, 85 - font.GetStringWidth(i2s(highscore[gamemode - 1]), .64), -185);
	else
		font.DrawString(t2s(timeleft), .64, 85 - font.GetStringWidth(t2s(timeleft), .64), -185);
	if (gamemode == 3 && timeleft<0)
	{
		menu = 4;
		timeleft = 0;
		mode = 0;
		firing = 0;
	}
	if (gamemode == 3 && menu==0)
	{
		timeleft -= SDL_GetTicks() - last;
		totaltime += SDL_GetTicks() - last;
		last = SDL_GetTicks();
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glLoadIdentity();
	glColor4f(1, 1, 1, .8);
	rect(-153, 236, 4, 15);
	rect(-145, 236, 4, 15);
	// SwapBuffers(hDC);
	//  while(clock()-lasttime<CLOCKS_PER_SEC/60);
	// lasttime=clock();
	if (menu>0 && menu != 4)
	{
		if (menuold == -1)
		{
			menuold = current;
			menucur = next_;
			menuat = 0;
		}
		glColor4f(0, 0, 0, .8);
		rect(-160, -220, 320, 480);

		float old[3];
		float cur[3];
		switch (menuold)
		{
		case 1:
			old[0] = 1;
			old[1] = 117 / 255.f;
			old[2] = 0;
			break;
		case 2:
			old[0] = 247 / 255.f;
			old[1] = 192 / 255.f;
			old[2] = 0;
			break;
		case 3:
			old[0] = 0;
			old[1] = 203 / 255.f;
			old[2] = 67 / 255.f;
			break;
		case 4:
			old[0] = 0;
			old[1] = 126 / 255.f;
			old[2] = 240 / 255.f;
			break;
		case 5:
			old[0] = 168 / 255.f;
			old[1] = 0;
			old[2] = 240 / 255.f;
			break;
		case 6:
			old[0] = 248 / 255.f;
			old[1] = 0;
			old[2] = 0;
			break;
		}
		switch (menucur)
		{
		case 1:
			cur[0] = 1;
			cur[1] = 117 / 255.f;
			cur[2] = 0;
			break;
		case 2:
			cur[0] = 247 / 255.f;
			cur[1] = 192 / 255.f;
			cur[2] = 0;
			break;
		case 3:
			cur[0] = 0;
			cur[1] = 203 / 255.f;
			cur[2] = 67 / 255.f;
			break;
		case 4:
			cur[0] = 0;
			cur[1] = 126 / 255.f;
			cur[2] = 240 / 255.f;
			break;
		case 5:
			cur[0] = 168 / 255.f;
			cur[1] = 0;
			cur[2] = 240 / 255.f;
			break;
		case 6:
			cur[0] = 248 / 255.f;
			cur[1] = 0;
			cur[2] = 0;
			break;
		}
		glColor4f(cur[0] * (1 - menuat) + old[0] * menuat, cur[1] * (1 - menuat) + old[1] * menuat, cur[2] * (1 - menuat) + old[2] * menuat, .65);
		menuat += .012;
		if (menuat>1)
		{
			menuat = 0;
			menucur = menuold;
			do
			menuold = rand() % 6 + 1;
			while (menuold == menucur);
		}
		Bind(background);
		rect(-160, -220, 320, 480);
		glColor4f(1, 1, 1, 1);
		Bind(title);
		rect(-140, -210, 280, 135);
	}
	if (menu == 1)
	{
		glColor3f(1, 1, 1);
		Bind(buttons);
		rect(-160, -220, 320, 480);
		Bind(buttontext);
		rect(-160, -220, 320, 480);
		Bind(menutext);
		rect(-160, -220, 320, 480);
		glScalef(1, -1, 1);
		sfont.Begin();
		sfont.DrawString("TAP TO RESUME", .65, 50, -240);
	}
	else if (menu == 2)
	{
		glColor4f(1, 1, 1, 1);
		if (score>getLow(gamemode - 1) || 1)
			rectline(-22, -28, 150, 30);
		font.Begin();
		glScalef(1, -1, 1);
		glColor3f(1, 1, 1);
		font.Begin();
		font.DrawString(name,.75, -22, 28);
		if (nBloxz<1)
			font.DrawStringC("COMPLETE!", 1.8, 0, -120);
		else
		{
			font.DrawString("GAME", 2, -120, -90);
			font.DrawString("OVER", 2, -80, -160);
		}
		sfont.Begin();
		sfont.DrawString("TAP TO CONTINUE", .65, 40, -240);
		if (score == highscore[gamemode - 1])
			sfont.DrawStringC("NEW HIGHSCORE!", .9, 55, 60);
		char str[20];
		sprintf(str, "BLOXZ CLEARED:   %i", blokd);
		sfont.DrawStringC(str, .75, 52, -38);
		sprintf(str, "FINAL SCORE:   %i", score);
		sfont.DrawStringC(str, .75, 52, -6);
		switch (gamemode)
		{
		case 1:
			sfont.DrawStringC("GAME TYPE: NORMAL", .75, 52, -70);
			break;
		case 2:
			sfont.DrawStringC("GAME TYPE: HARD", .75, 52, -70);
			break;
		case 3:
			sfont.DrawStringC("GAME TYPE: TIME", .75, 52, -70);
			break;
		}
	}
	else if (menu == 3)
	{
		int i;
		for (i = 0; i<10; i++)
		if (highscores[hst][i].score<0) break;
		if (i != 0)
		{
			glColor4f(0, 0, 0, .5);
			glBindTexture(GL_TEXTURE_2D, 0);
			rect(-85, -35, 150, i * 25 + 3);
			glScalef(1, -1, 1);
			glColor3f(1, 1, 1);
			sfont.Begin();
		}
		else
		{
			glScalef(1, -1, 1);
			glColor3f(1, 1, 1);
			sfont.Begin();
			sfont.DrawStringC("NO SCORES", 1, -7, -30);
		}
		//sfont.DrawString("TAP TO CONTINUE",.70,35,-240);
		for (i = 0; i<10; i++)
		{
			if (highscores[hst][i].score<0) break;
			sfont.DrawString(highscores[hst][i].name.c_str(), .95, -80, -i * 25 + 35);
			sfont.DrawStringR(i2s(highscores[hst][i].score), .95, 60, -i * 25 + 35);
		}
		sfont.DrawString("<", 2, -144, -200);
		sfont.DrawString(">", 2, 112, -200);
		sfont.DrawStringC("MENU", 1.25, 0, -215);
		font.Begin();
		switch (hst)
		{
		case 0:
			font.DrawStringC("NORMAL", .8, -7, 77);
			break;
		case 1:
			font.DrawStringC("HARD", .8, -7, 77);
			break;
		case 2:
			font.DrawStringC("TIMED", .8, -7, 77);
			break;
		}
	}
	else if (menu == 4)
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		glColor4f(0, 0, 0, .7);
		if (nBloxz == 0)
			rect(-100, -75, 200, 128);
		else
			rect(-100, -130, 200, 188);
		font.Begin();
		glScalef(1, -1, 1);
		glColor3f(1, 1, 1);
		if (nBloxz == 0)
			font.DrawStringC("COMPLETE", 1.18, 0, 65);
		else
		{
			font.DrawString("GAME", 1.75, -95, 125);
			font.DrawString("OVER", 1.75, -65, 75);
		}
		sfont.Begin();
		sfont.DrawStringC("TAP TO CONTINUE", 1, 0, -10);
	}
	else if (menu == 5)
	{
		glColor3f(1, 1, 1);
		Bind(help[hst]);
		rect(-160, -220, 320, 480);
		glScalef(1, -1, 1);
		sfont.Begin();
		sfont.DrawString("TAP TO CONTINUE", .65, 40, -240);
		sfont.DrawString("X", 1.25, -155, -220);
	}
	else
	{
		if (firing) firing++;
		menuold = -1;
		menuat = 0;
	}
}

void fire(int Side, int At, int p)
{
	side = Side;
	at = At;
	int f = 1, l = 0;
	if (mode == 0)
	{
		switch (side)
		{
		case 0:
			f = 1;
			while (blokAt(at, f).type == 0 && f<10) f++;
			if (f == 10)
				return;
			level[at][f - 1].x = (at - 5)*BLOKSIZE;
			level[at][f - 1].y = min(30, p - 16) - 190;
			level[at][f - 1].type = current;
			blokn[current]++;
			nBloxz++;
			temp = current;
			current = next_;
			next_ = rand() % 6 + 1;
			mode = 1;
			break;
		case 1:
			f = 8;
			while (blokAt(f, at).type == 0 && f>0) f--;
			if (f == 0)
				return;
			level[f + 1][at].x = max(288, p - 16) - 160;//-BLOKSIZE;
			level[f + 1][at].y = (at - 5)*BLOKSIZE;
			level[f + 1][at].type = current;
			blokn[current]++;
			temp = current;
			current = next_;
			next_ = rand() % 6 + 1;
			mode = 1;
			nBloxz++;
			break;
		case 2:
			f = 8;
			while (blokAt(at, f).type == 0 && f>0) f--;
			if (f == 0)
				return;
			level[at][f + 1].x = (at - 5)*BLOKSIZE;
			level[at][f + 1].y = max(316, p - 16) - 190;//-BLOKSIZE;
			level[at][f + 1].type = current;
			blokn[current]++;
			temp = current;
			current = next_;
			next_ = rand() % 6 + 1;
			mode = 1;
			nBloxz++;
			break;
		case 3:
			f = 1;
			while (blokAt(f, at).type == 0 && f<10) f++;
			if (f == 10)
				return;
			level[f - 1][at].x = min(0, p - 16) - 160;
			level[f - 1][at].y = (at - 5)*BLOKSIZE;
			level[f - 1][at].type = current;
			blokn[current]++;
			temp = current;
			current = next_;
			next_ = rand() % 6 + 1;
			mode = 1;
			nBloxz++;
			break;
		}
	}
	else if (mode == 1)
	{
		switch (side)
		{
		case 0:
			f = 1;
			l = 0;
			while (blokAt(at, f).type == 0) f++;
			printf("f=%i\n", f);
			l = f;
			while (blokAt(at, l).type>0) l++;
			printf("l=%i\n", l);
			for (int i = l; i >= f; i--)
				level[at][i] = level[at][i - 1];
			level[at][f - 1].type = 0;
			mode = 2;
			break;
		case 1:
			f = 8;
			l = 9;
			while (blokAt(f, at).type == 0) f--;
			printf("f=%i\n", f);
			l = f;
			while (blokAt(l, at).type>0) l--;
			printf("l=%i\n", l);
			for (int i = l; i <= f; i++)
				level[i][at] = level[i + 1][at];
			level[f + 1][at].type = 0;
			mode = 2;
			break;
		case 2:
			f = 8;
			l = 9;
			while (blokAt(at, f).type == 0) f--;
			printf("f=%i\n", f);
			l = f;
			while (blokAt(at, l).type>0) l--;
			printf("l=%i\n", l);
			for (int i = l; i <= f; i++)
				level[at][i] = level[at][i + 1];
			level[at][f + 1].type = 0;
			mode = 2;
			break;
		case 3:
			f = 1;
			l = 0;
			while (blokAt(f, at).type == 0) f++;
			printf("f=%i\n", f);
			l = f;
			while (blokAt(l, at).type>0) l++;
			printf("l=%i\n", l);
			for (int i = l; i >= f; i--)
				level[i][at] = level[i - 1][at];
			level[f - 1][at].type = 0;
			mode = 2;
			break;
		}
	}
	int i;
	for (i = 0; i<7; i++)
	if (blokn[i]>0)
		break;
	while (blokn[next_]<1 && i != 7)
		next_ = rand() % 6 + 1;

	if (mode == 1)
		firing = 1;
}
void checkexplode()
{
	//[boom prepareToPlay];
	bool explode[8][8];
	for (int j = 0; j<8; j++)
	for (int i = 0; i<8; i++)
		explode[i][j] = 0;
	int mult = 0, bloxz = 0;
	for (int j = 1; j<9; j++)
	for (int i = 1; i<9; i++)
	{
		if (level[i][j].type <= 0)
			continue;
		int n = 0;
		if (blokAt(i, j - 1).type == level[i][j].type)
			n++;
		if (blokAt(i, j + 1).type == level[i][j].type)
			n++;
		if (blokAt(i - 1, j).type == level[i][j].type)
			n++;
		if (blokAt(i + 1, j).type == level[i][j].type)
			n++;
		if (n >= 2)
		{
			bool nw = 1;
			if (explode[i - 1][j - 1])
				nw = 0;
			else
				bloxz += 1 + !nw;
			explode[i - 1][j - 1] = 1;
			if (blokAt(i, j - 1).type == level[i][j].type)
			{
				if (explode[i - 1][j - 2])
					nw = 0;
				/*else
				bloxz+=1+!nw;*/
				bloxz++;
				explode[i - 1][j - 2] = 1;
			}
			if (blokAt(i, j + 1).type == level[i][j].type)
			{
				if (explode[i - 1][j - 1 + 1])
					nw = 0;
				/* else
				bloxz+=1+!nw;*/
				bloxz++;
				explode[i - 1][j + 1 - 1] = 1;
			}
			if (blokAt(i - 1, j).type == level[i][j].type)
			{
				if (explode[i - 1 - 1][j - 1])
					nw = 0;
				/*  else
				bloxz+=1+!nw;*/
				bloxz++;
				explode[i - 1 - 1][j - 1] = 1;
			}
			if (blokAt(i + 1, j).type == level[i][j].type)
			{
				if (explode[i - 1 + 1][j - 1])
					nw = 0;
				/* else
				bloxz+=1+!nw;*/
				bloxz++;
				explode[i + 1 - 1][j - 1] = 1;
			}
			if (nw)
				mult++;
			if (n>2)
				bloxz += n - 2;
		}
	}
	/* for(int j=7;j>=0;j--)
	for(int i=0;i<8;i++)
	if(explode[i][j])
	{
	//   mode=2;
	/*  if(j==0)
	{
	level[i+1][j+1].y-=BLOKSIZE;
	level[i+1][j+1].type=rand()%6+1;
	}
	else
	{
	level[i+1][j+1]=level[i+1][j-1+1];
	explode[i][j-1]=1;
	}*//*
	level[i+1][j+1].type=0;
	}*/
	for (int j = 9; j >= 0; j--)
	for (int i = 0; i<10; i++)
	if (i == 0 || i == 9 || j == 0 || j == 9)
	{
		if (level[i][j].type>0)
		{
			for (int y = 0; y<32; y++)
			for (int x = 0; x<32; x++)
			if (nPar<MAXPAR)
			{
				parp[nPar].x = (i - 5)*BLOKSIZE + x;
				parp[nPar].y = (j - 5)*BLOKSIZE + y;
				rgba_t color = GetPixelAt(imgs[level[i][j].type - 1], x * 2, 63 - y * 2);
				if (i == 0)
				{
					parv[nPar].x = -SPEED + (float)(abs(y - 16)) / 150;
					parv[nPar].y = (rand() % 2 - 1)*.1;
				}
				else if (j == 0)
				{
					parv[nPar].y = -SPEED + (float)(abs(x - 16)) / 150;
					parv[nPar].x = (rand() % 2 - 1)*.1;;
				}
				else if (i == 9)
				{
					parv[nPar].x = SPEED - (float)(abs(y - 16)) / 150;
					parv[nPar].y = (rand() % 2 - 1)*.1;
				}
				else if (j == 9)
				{
					parv[nPar].y = SPEED - (float)(abs(x - 16)) / 150;
					parv[nPar].x = (rand() % 2 - 1)*.1;;
				}
				parc[nPar][0] = color.r;
				parc[nPar][1] = color.g;
				parc[nPar][2] = color.b;
				parc[nPar][3] = color.a;
				nPar++;
			}
			else break;

			blokn[level[i][j].type]--;
			nBloxz--;
			level[i][j].type = 0;
		}
	}
	else if (explode[i - 1][j - 1])
	{
		if (level[i][j].type>0)
		{
			for (int y = 0; y<32; y++)
			for (int x = 0; x<32; x++)
			if (nPar<MAXPAR)
			{
				parp[nPar].x = (i - 5)*BLOKSIZE + x;
				parp[nPar].y = (j - 5)*BLOKSIZE + y;
				rgba_t color = GetPixelAt(imgs[level[i][j].type - 1], x * 2, 63 - y * 2);
				parv[nPar].x = parvx[x][y];
				parv[nPar].y = parvy[x][y];
				parc[nPar][0] = color.r;
				parc[nPar][1] = color.g;
				parc[nPar][2] = color.b;
				parc[nPar][3] = color.a;
				nPar++;
			}
			else break;
			blokn[level[i][j].type]--;
			level[i][j].type = 0;
			nBloxz--;
			blokd++;
			missed = 0;
		}
	}
	timeleft += (bloxz*mult - missed) * 315;
	score += bloxz*mult * 100;
	score -= missed * 100;
	//if(bloxz)
	//	[boom play];
	if (score>highscore[gamemode - 1])
		highscore[gamemode - 1] = score;
	if ((bloxz == 0 && gamemode == 2) || nBloxz<1 || (gamemode == 3 && timeleft<0))
	{
		menu = 4;
		for (int i = 0; i<rand() % 5 + 10; i++)
		{
			float old[3];
			switch (rand() % 6 + 1)
			{
			case 1:
				old[0] = 1;
				old[1] = 117 / 255.f;
				old[2] = 0;
				break;
			case 2:
				old[0] = 247 / 255.f;
				old[1] = 192 / 255.f;
				old[2] = 0;
				break;
			case 3:
				old[0] = 0;
				old[1] = 203 / 255.f;
				old[2] = 67 / 255.f;
				break;
			case 4:
				old[0] = 0;
				old[1] = 126 / 255.f;
				old[2] = 240 / 255.f;
				break;
			case 5:
				old[0] = 168 / 255.f;
				old[1] = 0;
				old[2] = 240 / 255.f;
				break;
			case 6:
				old[0] = 248 / 255.f;
				old[1] = 0;
				old[2] = 0;
				break;
			}
			vec2f pos;
			pos.x = rand() % 320 - 160;
			pos.y = rand() % 480 - 220;
			randPars(pos, old[0], old[1], old[2]);
		}
		timeleft = 0;
	}
	if (bloxz == 0) missed++;
	int i;
	for (i = 0; i<7; i++)
	if (blokn[i]>0)
		break;
	if (i != 7 && blokn[current]<1)
	{
		current = next_;
		next_ = rand() % 6 + 1;
		for (i = 0; i<7; i++)
		if (blokn[i]>0)
			break;
		while (blokn[next_]<1 && i != 7)
			next_ = rand() % 6 + 1;
	}
	while (blokn[current]<1 && i != 7)
		current = rand() % 6 + 1;
	for (int i = 0; i<10; i++)
	{
		level[9][i].type = 0;
		level[0][i].type = 0;
		level[i][9].type = 0;
		level[i][0].type = 0;
	}
	firing = 0;
	int q = 1;
	/*switch(side)
	{
	case 0:*/
	if (gamemode == 4)
	{
		for (int i = 1; i<9; i++)
		{
			for (int j = 8; j >= 1; j--)
			if (level[i][j].type == 0)
			{
				mode = 2;
				int k = j - 1;
				while (k >= 0 && level[i][k].type == 0) k--;
				if (k<1)
				{
					while (1)
					{
						level[i][j].type = rand() % 6 + 1;
						if (blokAt(i - 1, j).type != level[i][j].type)
						if (blokAt(i + 1, j).type != level[i][j].type)
						if (blokAt(i, j - 1).type != level[i][j].type)
						if (blokAt(i, j + 1).type != level[i][j].type)
							break;
					}
					blokn[level[i][j].type]++;
					nBloxz++;
					level[i][j].y = -128 - (q++)*BLOKSIZE;
				}
				else
				{
					level[i][j] = level[i][k];
					level[i][k].type = 0;
				}
			}
			q = 1;
		}
	}
	//while(1);
	/* break;
	case 1:
	for(int j=1;j<9;j++)
	{
	for(int i=1;i<=8;i++)
	if(level[i][j].type==0)
	{
	mode=2;
	int k=i+1;
	while(k<=9 && level[k][j].type==0) k++;
	if(k>8)
	{
	while(1)
	{
	level[i][j].type=rand()%6+1;
	if(blokAt(i-1,j).type!=level[i][j].type)
	if(blokAt(i+1,j).type!=level[i][j].type)
	if(blokAt(i,j-1).type!=level[i][j].type)
	if(blokAt(i,j+1).type!=level[i][j].type)
	break;
	}
	level[i][j].x=128+(q++)*BLOKSIZE-BLOKSIZE;
	}
	else
	{
	level[i][j]=level[k][j];
	level[k][j].type=0;
	}
	}
	q=1;
	}
	break;
	case 2:
	for(int i=1;i<9;i++)
	{
	for(int j=1;j<=8;j++)
	if(level[i][j].type==0)
	{
	mode=2;
	int k=j+1;
	while(k<=9 && level[i][k].type==0) k++;
	if(k>8)
	{
	while(1)
	{
	level[i][j].type=rand()%6+1;
	if(blokAt(i-1,j).type!=level[i][j].type)
	if(blokAt(i+1,j).type!=level[i][j].type)
	if(blokAt(i,j-1).type!=level[i][j].type)
	if(blokAt(i,j+1).type!=level[i][j].type)
	break;
	}
	level[i][j].y=128+(q++)*BLOKSIZE-BLOKSIZE;
	}
	else
	{
	level[i][j]=level[i][k];
	level[i][k].type=0;
	}
	}
	q=1;
	}
	break;
	case 3:
	for(int j=1;j<9;j++)
	{
	for(int i=8;i>=1;i--)
	if(level[i][j].type==0)
	{
	mode=2;
	int k=i-1;
	while(k>=0 && level[k][j].type==0) k--;
	if(k<1)
	{
	while(1)
	{
	level[i][j].type=rand()%6+1;
	if(blokAt(i-1,j).type!=level[i][j].type)
	if(blokAt(i+1,j).type!=level[i][j].type)
	if(blokAt(i,j-1).type!=level[i][j].type)
	if(blokAt(i,j+1).type!=level[i][j].type)
	break;
	}
	level[i][j].x=-128-(q++)*BLOKSIZE;
	}
	else
	{
	level[i][j]=level[k][j];
	level[k][j].type=0;
	}
	}
	q=1;
	}
	break;
	}*/
}