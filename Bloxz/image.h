/*
 *  image.h
 *  GLES
 *
 *  Created by Zak Fry on 2/14/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

enum TGATypes
{
	TGA_NODATA = 0,
	TGA_INDEXED = 1,
	TGA_RGB = 2,
	TGA_GRAYSCALE = 3,
	TGA_INDEXED_RLE = 9,
	TGA_RGB_RLE = 10,
	TGA_GRAYSCALE_RLE = 11
};
#include <SDL/SDL_opengl.h>

// Image Data Formats
#define	IMAGE_RGB       0
#define IMAGE_RGBA      1
#define IMAGE_LUMINANCE 2

// Image data types
#define	IMAGE_DATA_UNSIGNED_BYTE 0

// Pixel data transfer from file to screen:
// These masks are AND'd with the imageDesc in the TGA header,
// bit 4 is left-to-right ordering
// bit 5 is top-to-bottom
#define BOTTOM_LEFT  0x00	// first pixel is bottom left corner
#define BOTTOM_RIGHT 0x10	// first pixel is bottom right corner
#define TOP_LEFT     0x20	// first pixel is top left corner
#define TOP_RIGHT    0x30	// first pixel is top right corner

// TGA header
typedef struct
{
	unsigned char  IDLength;
	unsigned char  colorMapType;
	unsigned char  imageTypeCode;
	unsigned char  colorMapSpec[5];
	unsigned short xOrigin;
	unsigned short yOrigin;
	unsigned short wIDth;
	unsigned short height;
	unsigned char  bpp;
	unsigned char  imageDesc;
} tgaheader_t;

typedef struct
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
} rgba_t;

typedef struct
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
} rgb_t;

typedef struct
{
		unsigned char  m_colorDepth;
		unsigned char  m_imageDataType;
		unsigned char  m_imageDataFormat;
		unsigned char *m_pImageData;
		unsigned short m_wIDth;
		unsigned short m_height;
		unsigned long  m_imageSize;
		GLuint ID;
} TargaImage;
/*@interface TargaImage : NSObject
{
	unsigned char  m_colorDepth;
	unsigned char  m_imageDataType;
	unsigned char  m_imageDataFormat;
	unsigned char *m_pImageData;
	unsigned short m_wIDth;
	unsigned short m_height;
	unsigned long  m_imageSize;
}

@property unsigned char  m_colorDepth;
@property unsigned char  m_imageDataType;
@property unsigned char  m_imageDataFormat;
@property unsigned char *m_pImageData;
@property unsigned short m_wIDth;
@property unsigned short m_height;
@property unsigned long  m_imageSize;

@end*/

void SwapRedBlue(TargaImage *img);

TargaImage* LoadTargaImage(const char *filename);
void Release(TargaImage *img);
void Bind(TargaImage* img);
// flips image vertically
int FlipVertical(TargaImage *img);

// converts RGB format to RGBA format and vice versa
int ConvertRGBAToRGB(TargaImage *img);
int ConvertRGBToRGBA(TargaImage *img,unsigned char alphaValue);
rgba_t GetPixelAt(TargaImage *img,int x,int y);
