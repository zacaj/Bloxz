#import <OpenGLES/ES1/gl.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "image.c"
#include <string>
#include <utility>
using namespace std;
#include "glfont2.h"
using namespace glfont;

typedef struct
	{
		int type;
		int x,y;
	} Blok;
Blok level[10][10];
#define BLOKSIZE 32
#define SPEED 1
Blok bad;
Blok blokAt(int x,int y)
{
    if(x>=1 && x<9 && y>=1 && y<9)
        return level[x][y];
    else
        return bad;
}
int temp;
int firing=0;
void fire(int Side,int At,int p);
void checkexplode();
int mode=0;
int lasttime=0;
float fpsm=1;
int side, at;
int next,current;
TargaImage *imgs[6];
TargaImage *particle;
TargaImage *gradient;
GLFont font;
int score=0,highscore=0;
void rect(int x,int y,int x2,int y2)
{
    
	GLfloat squareVertices[] = {
		x,y,
		x+x2,y,
		x,y+y2,
		x+x2,y+y2,
    };
	GLfloat uvs[]={
		0,1,
		1,1,
		0,0,
		1,0,
	};
    glVertexPointer(2, GL_FLOAT, 0, squareVertices);
	glTexCoordPointer(2,GL_FLOAT,0,uvs);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}  
typedef struct
	{
		float x,y;
	} vec2f;
typedef struct
	{
		int x,y;
	} vec2i;
vec2f *scoreVerts;

#define MAXPAR 1024*9
vec2f parp[MAXPAR];
vec2f parv[MAXPAR];
unsigned char parc[MAXPAR][4];
int nPar=0;
float parvx[32][32];
float parvy[32][32];
void glColor3f(float r,float g,float b)
{
	glColor4f(r,g,b,1);
}
char* pathToFile(char *name,char *ext)
{
	NSString *path=[[NSBundle mainBundle] pathForResource: [[NSString alloc] initWithCString: name] ofType: [[NSString alloc] initWithCString: ext]];
	return (char*)[path cStringUsingEncoding:1];
}
int tx=-1,ty=-1;
int missed=0;
int tim;
int blokn[7];

vec2f vert[8*8];
vec2f uvs[8*8];
int nVert=0;
void init()
{
    glMatrixMode(GL_PROJECTION);
    glOrthof(-128-BLOKSIZE,128+BLOKSIZE,128+BLOKSIZE+100,-128-BLOKSIZE-60,-128,128);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	for(int j=0;j<10;j++)
        for(int i=0;i<10;i++)
        {
            level[i][j].type=0;
            level[i][j].x=(i-5)*BLOKSIZE;
            level[i][j].y=(j-5)*BLOKSIZE;
        }
	for(int i=0;i<7;i++)
		blokn[i]=0;
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER,.01);
    level[9][9].type=-1;
    printf("start\n");
    next=rand()%6+1;
    current=rand()%6+1;
    for(int j=1;j<9;j++)
        for(int i=1;i<9;i++)
        {
            while(1)
            {
                level[i][j].type=rand()%6+1;
                printf("%i %i,%i\n",level[i][j].type,i,j);
                if(blokAt(i-1,j).type!=level[i][j].type)
					if(blokAt(i+1,j).type!=level[i][j].type)
						if(blokAt(i,j-1).type!=level[i][j].type)
							if(blokAt(i,j+1).type!=level[i][j].type)
								break;
            }
			blokn[level[i][j].type]++;
        }
	for(int i=0;i<7;i++)
		printf("%i:%i\n",i,blokn[i]);
    char str[20];
    for(int i=1;i<7;i++)
    {
        sprintf(str,"%i",i);
        imgs[i-1]=LoadTargaImage(pathToFile(str,"tga"));
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
    particle=LoadTargaImage(pathToFile("particle","tga"));
    gradient=LoadTargaImage(pathToFile("gradient","tga"));
    GLuint fontID;
    glGenTextures(1,&fontID);
    font.Create(pathToFile("font","glf"),fontID);
    for(int y=0;y<32;y++)
        for(int x=0;x<32;x++)
        {
			/* parvx[x][y]=x-15;
			 parvy[x][y]=y-15;
			 parvx[x][y]*=(15-abs(x-16));
			 parvx[x][y]*=.001;
			 parvy[x][y]*=(15-abs(y-16));
			 parvy[x][y]*=.001;*/
            parvx[x][y]=sin(atan2(x-16,y-16))*16*.05;
            parvy[x][y]=cos(atan2(x-16,y-16))*16*.05;
        }
		
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	
	for(float i=65;i<92;i+=25/8)
	{
		vert[nVert].x=i;
		vert[nVert].y=260;
		uvs[nVert].x=0;
		uvs[nVert].y=0;
		nVert++;
		vert[nVert].x=i;
		vert[nVert].y=170-sin((i-65)*M_PI/2/25)*25;
		uvs[nVert].x=0;
		uvs[nVert].y=1;
		nVert++;
		vert[nVert].x=i+25/8;
		vert[nVert].y=260;
		uvs[nVert].x=1;
		uvs[nVert].y=0;
		nVert++;
		vert[nVert].x=i+25/8;
		vert[nVert].y=170-sin((i+25/8-65)*M_PI/2/25)*25;
		uvs[nVert].x=1;
		uvs[nVert].y=1;
		nVert++;
	}
	
		}
		void update()
		{            /* OpenGL animation code goes here */
			
            glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glLoadIdentity();
            glTranslatef(0,-30,0);
			// glTranslatef(BLOKSIZE/2,BLOKSIZE,0);
			
            glColor3f(0,0,1);
            Bind(particle);
            for(int i=1;i<9;i++)
            {
                rect(-149,-160+i*BLOKSIZE+11,10,10);
                rect(-160+i*BLOKSIZE+11,-149,10,10);
                rect(138,-160+i*BLOKSIZE+11,10,10);
                rect(-160+i*BLOKSIZE+11,138,10,10);
            }
            if(nPar<MAXPAR)
            {
                parp[nPar].x=155;
                parp[nPar].y=-155;
                parv[nPar].x=(rand()%20-10)*.1;
                parv[nPar].y=(rand()%20-10)*.1;
                parc[nPar][0]=1;
                parc[nPar][1]=0;
                parc[nPar][2]=0;
                parc[nPar][3]=1;
                nPar++;
            }
            for(int i=0;i<nPar;i++)
            {
                parp[i].x+=parv[i].x;
                parp[i].y+=parv[i].y;
                if(parc[i][3]<.01 || parp[i].x<-170 || parp[i].x>170 || parp[i].y<-200 || parp[i].y>200)
                {
                    nPar--;
                    parp[i]=parp[nPar];
                    parv[i]=parv[nPar];
                    parc[i][0]=parc[nPar][0];
                    parc[i][1]=parc[nPar][1];
                    parc[i][2]=parc[nPar][2];
                    parc[i][3]=parc[nPar][3];
                }
                if(blokAt((int)(parp[i].x+160)/32,(int)(parp[i].y+160)/32).type>0)
                {
                    float tx,ty,tx2,ty2;
                    tx=(parp[i].x+160)-((int)(parp[i].x+160)/32)*32;
                    ty=(parp[i].y+160)-((int)(parp[i].y+160)/32)*32;
                    tx2=(parp[i].x+160)-((int)(parp[i].x+160+32)/32)*32;
                    ty2=(parp[i].y+160)-((int)(parp[i].y+160+32)/32)*32;
                    tx=min(fabs(tx),fabs(tx2));
                    ty=min(fabs(ty),fabs(ty2));
                    if(tx<ty)
                    {
                        parv[i].x=-parv[i].x;
                        parv[i].x*=1;
                        parv[i].y*=2;
                    }
                    else
                    {
                        parv[i].y=-parv[i].y;
                        parv[i].x*=2;
                        parv[i].y*=1;
                    }
                    parv[i].x*=.8;
                    parv[i].y*=.8;
                    parc[i][3]*=.6;
                }
                parc[i][3]-=.01;
            }
            glColor3f(1,1,1);
            //glBindTexture(GL_TEXTURE_2D,0);
            glEnableClientState(GL_COLOR_ARRAY);
            glVertexPointer(2,GL_FLOAT,0,parp);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            glColorPointer(4,GL_UNSIGNED_BYTE,0,parc);
            glEnable(GL_POINT_SPRITE_OES);
            glTexEnvf(GL_POINT_SPRITE_OES,GL_COORD_REPLACE_OES,GL_TRUE);
            glPointSize(4);
           // glEnable(GL_POINT_SMOOTH);
            glDrawArrays(GL_POINTS,0,nPar);
            glDisable(GL_POINT_SPRITE_OES);
            glTexEnvf(GL_POINT_SPRITE_OES,GL_COORD_REPLACE_OES,GL_FALSE);
			glDisableClientState(GL_COLOR_ARRAY);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glColor3f(1,1,1);
            bool moved=0;
			if(tx!=-1 || ty!=-1) tim++;
            for(int j=0;j<10;j++)
                for(int i=0;i<10;i++)
                {
					if(level[i][j].type<=0)
						continue;
					if((tx!=-1 || ty!=-1) && tim>6 && !firing)
						if(tx==i || ty==j)
							glColor4f(1,1,1,1);
						else
							glColor4f(.5,.5,.5,1);
					
                    Bind(imgs[level[i][j].type-1]);
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
                    if(mode)
                    {
                        if(level[i][j].x<(i-5)*BLOKSIZE)
                        {
                            moved=1;
                            level[i][j].x+=SPEED;
                        }
                        else if(level[i][j].x>(i-5)*BLOKSIZE)
                        {
                            moved=1;
                            level[i][j].x-=SPEED;
                        }
                        if(level[i][j].y<(j-5)*BLOKSIZE)
                        {
                            moved=1;
                            level[i][j].y+=SPEED;
                        }
                        else if(level[i][j].y>(j-5)*BLOKSIZE)
                        {
                            moved=1;
                            level[i][j].y-=SPEED;
                        }
                    }
                    if(level[i][j].type>0)
                    {
						rect(level[i][j].x,level[i][j].y,BLOKSIZE,BLOKSIZE);
                    }
                }
            if(!moved && mode==1)
            {
                fire(side,at,0);
            }
            else if(!moved && mode==2)
            {
                mode=0;
                checkexplode();
            }
			/* glColor3f(0,0,0);
			 rect(-160,-160,160,-128);
			 rect(-160,160,160,128);
			 rect(-160,160,-128,-160);
			 rect(160,160,128,-160);*/
            glLoadIdentity();
            Bind(gradient);
			float mult=1.f-(float)min(firing,40)/40.f;
			if(!firing)
			{
				switch(current)
				{
					case 1:
						glColor4f(1,117/255.f,0,.5);
						break;
					case 2:
						glColor4f(247/255.f,192/255.f,0,.5);
						break;
					case 3:
						glColor4f(0,203/255.f,67/255.f,.5);
						break;
					case 4:
						glColor4f(0,126/255.f,240/255.f,.5);
						break;
					case 5:
						glColor4f(168/255.f,0,240/255.f,.5);
						break;
					case 6:
						glColor4f(248/255.f,0,0,.5);
						break;
				}
				rect(-160,170,230,90);
				rect(97,145,68,115);
				glVertexPointer(2,GL_FLOAT,0,vert);
				glTexCoordPointer(2,GL_FLOAT,0,uvs);
				glTranslatef(5,0,0);
				glDrawArrays(GL_TRIANGLE_STRIP,0,nVert);
				glTranslatef(-5,0,0);
			}
			else
			{
				
				float old[3];
				float cur[3];
				switch(temp)
				{
					case 1:
						old[0]=1;
						old[1]=117/255.f;
						old[2]=0;
						break;
					case 2:
						old[0]=247/255.f;
						old[1]=192/255.f;
						old[2]=0;
						break;
					case 3:
						old[0]=0;
						old[1]=203/255.f;
						old[2]=67/255.f;
						break;
					case 4:
						old[0]=0;
						old[1]=126/255.f;
						old[2]=240/255.f;
						break;
					case 5:
						old[0]=168/255.f;
						old[1]=0;
						old[2]=240/255.f;
						break;
					case 6:
						old[0]=248/255.f;
						old[1]=0;
						old[2]=0;
						break;
				}
				switch(current)
				{
					case 1:
						cur[0]=1;
						cur[1]=117/255.f;
						cur[2]=0;
						break;
					case 2:
						cur[0]=247/255.f;
						cur[1]=192/255.f;
						cur[2]=0;
						break;
					case 3:
						cur[0]=0;
						cur[1]=203/255.f;
						cur[2]=67/255.f;
						break;
					case 4:
						cur[0]=0;
						cur[1]=126/255.f;
						cur[2]=240/255.f;
						break;
					case 5:
						cur[0]=168/255.f;
						cur[1]=0;
						cur[2]=240/255.f;
						break;
					case 6:
						cur[0]=248/255.f;
						cur[1]=0;
						cur[2]=0;
						break;
				}
				glColor4f(cur[0]*(1-mult)+old[0]*mult,cur[1]*(1-mult)+old[1]*mult,cur[2]*(1-mult)+old[2]*mult,.5);
				rect(-160,170,230,90);
				rect(97,145,68,115);
				glVertexPointer(2,GL_FLOAT,0,vert);
				glTexCoordPointer(2,GL_FLOAT,0,uvs);
				glTranslatef(5,0,0);
				glDrawArrays(GL_TRIANGLE_STRIP,0,nVert);
				glTranslatef(-5,0,0);
			}	
            glColor4f(1,1,1,1);
			if(!firing)
			{
				Bind(imgs[current-1]);
				rect(102,160,48,48);
				Bind(imgs[next-1]);
				rect(114,220,24,24);
			}
			else
			{
				Bind(imgs[temp-1]);
				rect(102,160,48,48);
				Bind(imgs[next-1]);
				rect(114,220,24,24);
				Bind(imgs[current-1]);
				rect((114-102)*mult+102,(220-160)*mult+160,(48-24)*(1-mult)+24,(48-24)*(1-mult)+24);
			}
            glColor3f(1,1,1);
			/* glBindTexture(GL_TEXTURE_2D,0);
			 glVertexPointer(2,GL_FLOAT,0,scoreVerts);
			 glTranslatef(0,288-BLOKSIZE,0);
			 glScalef(1.066,-1.066,1.066);
			 //  glDrawArrays(GL_LINES,0,80);
			 glScalef(-1,1,1);*/
			// glDrawArrays(GL_LINES,0,80);
            font.Begin();
            glScalef(1,-1,1);
            glColor3f(1,1,1);
            font.DrawString("SCORE",.9,-120,-210);
            font.DrawString("HIGHSCORE",.64,-145,-185);
            char str[10];
            sprintf(str,"%i",score);
            font.DrawString(str,.9,85-font.GetStringWidth(str,.9),-210);
            sprintf(str,"%i",highscore);
            font.DrawString(str,.64,85-font.GetStringWidth(str,.64),-185);
           // SwapBuffers(hDC);
          //  while(clock()-lasttime<CLOCKS_PER_SEC/60);
			// lasttime=clock();
			if(firing) firing++;
        }
void fire(int Side,int At,int p)
{
	if(mode==0)
		firing=1;
    side=Side;
    at=At;
    printf("%i,%i\n",side,at);
    int f=1,l=0;
    if(mode==0)
    {
        switch(side)
        {
            case 0:
                f=1;
                while(blokAt(at,f).type==0 && f<10) f++;
				if(f==10)
					return;
                level[at][f-1].x=(at-5)*BLOKSIZE;
                level[at][f-1].y=min(30,p)-190-8;
                level[at][f-1].type=current;
				blokn[current]++;
				temp=current;
                current=next;
                next=rand()%6+1;
                mode=1;
                break;
            case 1:
                f=8;
                while(blokAt(f,at).type==0 && f>0) f--;
				if(f==0)
					return;
                level[f+1][at].x=max(128,p)-160-8;//-BLOKSIZE;
                level[f+1][at].y=(at-5)*BLOKSIZE;
                level[f+1][at].type=current;
				blokn[current]++;
				temp=current;
                current=next;
                next=rand()%6+1;
                mode=1;
                break;
            case 2:
                f=8;
                while(blokAt(at,f).type==0 && f>0) f--;
				if(f==0)
					return;
                level[at][f+1].x=(at-5)*BLOKSIZE;
                level[at][f+1].y=max(316,p)-190-8;//-BLOKSIZE;
                level[at][f+1].type=current;
				blokn[current]++;
				temp=current;
                current=next;
                next=rand()%6+1;
                mode=1;
                break;
            case 3:
                f=1;
                while(blokAt(f,at).type==0 && f<10) f++;
				if(f==10)
					return;
                level[f-1][at].x=min(0,p)-160-8;
                level[f-1][at].y=(at-5)*BLOKSIZE;
                level[f-1][at].type=current;
				blokn[current]++;
				temp=current;
                current=next;
                next=rand()%6+1;
                mode=1;
                break;
        }
    }
    else if(mode==1)
    {
        switch(side)
        {
            case 0:
                f=1;
                l=0;
                while(blokAt(at,f).type==0) f++;
                printf("f=%i\n",f);
                l=f;
                while(blokAt(at,l).type>0) l++;
                printf("l=%i\n",l);
                for(int i=l;i>=f;i--)
                    level[at][i]=level[at][i-1];
                level[at][f-1].type=0;
                mode=2;
                break;
            case 1:
                f=8;
                l=9;
                while(blokAt(f,at).type==0) f--;
                printf("f=%i\n",f);
                l=f;
                while(blokAt(l,at).type>0) l--;
                printf("l=%i\n",l);
                for(int i=l;i<=f;i++)
                    level[i][at]=level[i+1][at];
                level[f+1][at].type=0;
                mode=2;
                break;
            case 2:
                f=8;
                l=9;
                while(blokAt(at,f).type==0) f--;
                printf("f=%i\n",f);
                l=f;
                while(blokAt(at,l).type>0) l--;
                printf("l=%i\n",l);
                for(int i=l;i<=f;i++)
                    level[at][i]=level[at][i+1];
                level[at][f+1].type=0;
                mode=2;
                break;
            case 3:
                f=1;
                l=0;
                while(blokAt(f,at).type==0) f++;
                printf("f=%i\n",f);
                l=f;
                while(blokAt(l,at).type>0) l++;
                printf("l=%i\n",l);
                for(int i=l;i>=f;i--)
                    level[i][at]=level[i-1][at];
                level[f-1][at].type=0;
                mode=2;
                break;
        }
    }
	while(blokn[next]<1)
		next=rand()%6+1;
}
void checkexplode()
{
    bool explode[8][8];
    for(int j=0;j<8;j++)
        for(int i=0;i<8;i++)
            explode[i][j]=0;
    int mult=0,bloxz=0;
    for(int j=1;j<9;j++)
        for(int i=1;i<9;i++)
        {
            if(level[i][j].type<=0)
                continue;
            int n=0;
            if(blokAt(i,j-1).type==level[i][j].type)
                n++;
            if(blokAt(i,j+1).type==level[i][j].type)
                n++;
            if(blokAt(i-1,j).type==level[i][j].type)
                n++;
            if(blokAt(i+1,j).type==level[i][j].type)
                n++;
            if(n>=2)
            {
                bool nw=1;
                if(explode[i-1][j-1])
                    nw=0;
                else
                    bloxz+=1+!nw;
                explode[i-1][j-1]=1;
                if(blokAt(i,j-1).type==level[i][j].type)
                {
                    if(explode[i-1][j-2])
                        nw=0;
                    /*else
					 bloxz+=1+!nw;*/
					bloxz++;
                    explode[i-1][j-2]=1;
                }
                if(blokAt(i,j+1).type==level[i][j].type)
                {
                    if(explode[i-1][j-1+1])
                        nw=0;
                   /* else
					bloxz+=1+!nw;*/
					bloxz++;
                    explode[i-1][j+1-1]=1;
                }
                if(blokAt(i-1,j).type==level[i][j].type)
                {
                    if(explode[i-1-1][j-1])
                        nw=0;
                  /*  else
				   bloxz+=1+!nw;*/
					bloxz++;
                    explode[i-1-1][j-1]=1;
                }
                if(blokAt(i+1,j).type==level[i][j].type)
                {
					if(explode[i-1+1][j-1])
                        nw=0;
                   /* else
                        bloxz+=1+!nw;*/
					bloxz++;
                    explode[i+1-1][j-1]=1;
                }
                if(nw)
                    mult++;
				if(n>2)
					bloxz+=n-2;
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
    for(int j=9;j>=0;j--)
        for(int i=0;i<10;i++)
            if(i==0 || i==9 || j==0 || j==9)
            {
				if(level[i][j].type>0)
                    for(int y=0;y<32;y++)
                        for(int x=0;x<32;x++)
                            if(nPar<MAXPAR)
                            {
                                parp[nPar].x=(i-5)*BLOKSIZE+x;
                                parp[nPar].y=(j-5)*BLOKSIZE+y;
                                rgba_t color=GetPixelAt(imgs[level[i][j].type-1],x*2,63-y*2);
                                if(i==0)
                                {
                                    parv[nPar].x=-SPEED+(float)(fabs(y-16))/150;
                                    parv[nPar].y=(rand()%2-1)*.1;
                                }
                                else if(j==0)
                                {
                                    parv[nPar].y=-SPEED+(float)(fabs(x-16))/150;
                                    parv[nPar].x=(rand()%2-1)*.1;;
                                }
                                else if(i==9)
                                {
                                    parv[nPar].x=SPEED-(float)(fabs(y-16))/150;
                                    parv[nPar].y=(rand()%2-1)*.1;
                                }
                                else if(j==9)
                                {
                                    parv[nPar].y=SPEED-(float)(fabs(x-16))/150;
                                    parv[nPar].x=(rand()%2-1)*.1;;
                                }
                                parc[nPar][0]=color.r;
                                parc[nPar][1]=color.g;
                                parc[nPar][2]=color.b;
                                parc[nPar][3]=color.a;
                                nPar++;
                            }
				blokn[level[i][j].type]--;
                level[i][j].type=0;
            }
            else if(explode[i-1][j-1])
            {
                if(level[i][j].type>0)
                    for(int y=0;y<32;y++)
                        for(int x=0;x<32;x++)
                            if(nPar<MAXPAR)
                            {
                                parp[nPar].x=(i-5)*BLOKSIZE+x;
                                parp[nPar].y=(j-5)*BLOKSIZE+y;
                                rgba_t color=GetPixelAt(imgs[level[i][j].type-1],x*2,63-y*2);
                                parv[nPar].x=parvx[x][y];
                                parv[nPar].y=parvy[x][y];
                                parc[nPar][0]=color.r;
                                parc[nPar][1]=color.g;
                                parc[nPar][2]=color.b;
                                parc[nPar][3]=color.a;
                                nPar++;
                            }
				blokn[level[i][j].type]--;
                level[i][j].type=0;
				missed=0;
            }
    score+=bloxz*mult;
	score-=missed;
    if(score>highscore)
        highscore=score;
	if(bloxz==0) missed++;
	for(int i=0;i<7;i++)
		printf("%i:%i\n",i,blokn[i]);
	while(blokn[current]<1)
		current=rand()%6+1;
    for(int i=0;i<10;i++)
    {
        level[9][i].type=0;
        level[0][i].type=0;
        level[i][9].type=0;
        level[i][0].type=0;
    }
	firing=0;
    /*int q=1;
	 switch(side)
	 {
	 case 0:
	 for(int i=1;i<9;i++)
	 {
	 for(int j=8;j>=1;j--)
	 if(level[i][j].type==0)
	 {
	 mode=2;
	 int k=j-1;
	 while(k>=0 && level[i][k].type==0) k--;
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
	 level[i][j].y=-128-(q++)*BLOKSIZE;
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