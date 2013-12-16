/*
 *  image.c
 *  GLES
 *
 *  Created by Zak Fry on 2/14/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#include "stdafx.h"
#include "image.h"
#include <stdio.h>
#include <stdlib.h>

void SwapRedBlue(TargaImage *img)
{
	switch (img->m_colorDepth)
	{
		case 4:
		{
			unsigned char temp;
			rgba_t* source = (rgba_t*)img->m_pImageData;
            int pixel;
			for (pixel = 0; pixel < (img->m_wIDth * img->m_height); ++pixel)
			{
				temp = source[pixel].b;
				source[pixel].b = source[pixel].r;
				source[pixel].r = temp;
			}
		} break;
		case 3:
		{
			unsigned char temp;
			rgb_t* source = (rgb_t*)img->m_pImageData;
            int pixel;
			for (pixel = 0; pixel < (img->m_wIDth * img->m_height); ++pixel)
			{
				temp = source[pixel].b;
				source[pixel].b = source[pixel].r;
				source[pixel].r = temp;
			}
		} break;
		default:
			// ignore other color depths
			break;
	}
}
rgba_t GetPixelAt(TargaImage *img,int x,int y)
{
    rgba_t ret;
    unsigned char *pixel=img->m_pImageData+(y*img->m_wIDth+x)*img->m_colorDepth;
    ret.r=*pixel++;
    ret.g=*pixel++;
    ret.b=*pixel++;
    ret.a=*pixel++;
    return ret;
}
TargaImage* LoadTargaImage(const char *filename)
{
	TargaImage *img;
	img=(TargaImage*)malloc(sizeof(TargaImage));
	FILE *pFile = fopen(filename, "rb");

	if (!pFile)
	{
		printf("ERROR: File %s doesnt exist\n",filename);
		return 0;
	}
	tgaheader_t tgaHeader;

	// read the TGA header
	fread(&tgaHeader, 1, sizeof(tgaheader_t), pFile);

	// see if the image type is one that we support (RGB, RGB RLE, GRAYSCALE, GRAYSCALE RLE)
	if ( ((tgaHeader.imageTypeCode != TGA_RGB) && (tgaHeader.imageTypeCode != TGA_GRAYSCALE) &&
		  (tgaHeader.imageTypeCode != TGA_RGB_RLE) && (tgaHeader.imageTypeCode != TGA_GRAYSCALE_RLE)) ||
		tgaHeader.colorMapType != 0)
	{
		printf("ERROR: Filetype not supported\n",filename);
		fclose(pFile);
		return 0;
	}

	// get image wIDth and height
	img->m_wIDth = tgaHeader.wIDth;
	img->m_height = tgaHeader.height;

	// colormode -> 3 = BGR, 4 = BGRA
	int colorMode = tgaHeader.bpp / 8;

	// we don't handle less than 24 bit
	//if (colorMode < 3)
	{
		//fclose(pFile);
		//NSLog(@"ERROR: <24b",filename);
		//return 0;
	}

	img->m_imageSize =img-> m_wIDth * img->m_height * colorMode;

	// allocate memory for TGA image data
	img->m_pImageData=(unsigned char*)malloc(sizeof(unsigned char)*img->m_imageSize);

	// skip past the ID if there is one
	if (tgaHeader.IDLength > 0)
		fseek(pFile, SEEK_CUR, tgaHeader.IDLength);

	// read image data
	if (tgaHeader.imageTypeCode == TGA_RGB || tgaHeader.imageTypeCode == TGA_GRAYSCALE)
	{
		fread(img->m_pImageData, 1, img->m_imageSize, pFile);
	}
	else
	{
		// this is an RLE compressed image
		unsigned char ID;
		unsigned char length;
		rgba_t color = { 0, 0, 0, 0 };
		unsigned int i = 0;

		while (i < img->m_imageSize)
		{
			ID = fgetc(pFile);

			// see if this is run length data
			if (ID >= 128)// & 0x80)
			{
				// find the run length
				length = (unsigned char)(ID - 127);

				// next 3 (or 4) bytes are the repeated values
				color.b = (unsigned char)fgetc(pFile);
				color.g = (unsigned char)fgetc(pFile);
				color.r = (unsigned char)fgetc(pFile);

				if (colorMode == 4)
					color.a = (unsigned char)fgetc(pFile);

				// save everything in this run
				while (length > 0)
				{
					img->m_pImageData[i++] = color.b;
					img->m_pImageData[i++] = color.g;
					img->m_pImageData[i++] = color.r;

					if (colorMode == 4)
						img->m_pImageData[i++] = color.a;

					--length;
				}
			}
			else
			{
				// the number of non RLE pixels
				length = (unsigned char)(ID + 1);

				while (length > 0)
				{
					color.b = (unsigned char)fgetc(pFile);
					color.g = (unsigned char)fgetc(pFile);
					color.r = (unsigned char)fgetc(pFile);

					if (colorMode == 4)
						color.a = (unsigned char)fgetc(pFile);

					img->m_pImageData[i++] = color.b;
					img->m_pImageData[i++] = color.g;
					img->m_pImageData[i++] = color.r;

					if (colorMode == 4)
						img->m_pImageData[i++] = color.a;

					--length;
				}
			}
		}
	}

	fclose(pFile);

	switch(tgaHeader.imageTypeCode)
	{
		case TGA_RGB:
		case TGA_RGB_RLE:
			if (3 == colorMode)
			{
				img->m_imageDataFormat = IMAGE_RGB;
				img->m_imageDataType = IMAGE_DATA_UNSIGNED_BYTE;
				img->m_colorDepth = 24/8;
			}
			else
			{
				img->m_imageDataFormat = IMAGE_RGBA;
				img->m_imageDataType = IMAGE_DATA_UNSIGNED_BYTE;
				img->m_colorDepth = 32/8;
			}
			break;

		case TGA_GRAYSCALE:
		case TGA_GRAYSCALE_RLE:
			img->m_imageDataFormat = IMAGE_LUMINANCE;
			img->m_imageDataType = IMAGE_DATA_UNSIGNED_BYTE;
			img->m_colorDepth = 8/8;
			break;
	}

	if ((tgaHeader.imageDesc & TOP_LEFT) == TOP_LEFT)
		FlipVertical(img);

	// swap the red and blue components in the image data
	SwapRedBlue(img);
	glGenTextures(1,&img->ID);
	glBindTexture(GL_TEXTURE_2D,img->ID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	switch(img->m_imageDataFormat)
	{
	    case IMAGE_RGB:
            glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,img->m_wIDth,img->m_height,0,GL_RGB,GL_UNSIGNED_BYTE,img->m_pImageData);
            break;
        case IMAGE_RGBA:
            glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,img->m_wIDth,img->m_height,0,GL_RGBA,GL_UNSIGNED_BYTE,img->m_pImageData);
            break;
        case IMAGE_LUMINANCE:
            glTexImage2D(GL_TEXTURE_2D,0,GL_ALPHA,img->m_wIDth,img->m_height,0,GL_ALPHA,GL_UNSIGNED_BYTE,img->m_pImageData);
            break;
	}
	return img;
}
void Bind(TargaImage* img)
{
    glBindTexture(GL_TEXTURE_2D,img->ID);
}
int FlipVertical(TargaImage *img)
{
	if (!img->m_pImageData)
		return 0;

	if (img->m_colorDepth == 32)
	{
		rgba_t* tmpBits =(rgba_t*)malloc(sizeof(rgba_t)*img->m_wIDth);
		if (!tmpBits)
			return 0;

		int lineWIDth = img->m_wIDth * 4;

		rgba_t* top = (rgba_t*)img->m_pImageData;
		rgba_t* bottom = (rgba_t*)(img->m_pImageData + lineWIDth*(img->m_height-1));
        int i;
		for (i = 0; i < (img->m_height / 2); ++i)
		{
			memcpy(tmpBits, top, lineWIDth);
			memcpy(top, bottom, lineWIDth);
			memcpy(bottom, tmpBits, lineWIDth);

			top = (rgba_t*)((unsigned char*)top + lineWIDth);
			bottom = (rgba_t* )((unsigned char*)bottom - lineWIDth);
		}

		free(tmpBits);
		tmpBits = 0;
	}
	else if (img->m_colorDepth == 24)
	{
		rgb_t* tmpBits =(rgb_t*)malloc(sizeof(rgb_t)*img->m_wIDth);
		if (!tmpBits)
			return 0;

		int lineWIDth =img-> m_wIDth * 3;

		rgb_t* top = (rgb_t*)img->m_pImageData;
		rgb_t* bottom = (rgb_t*)(img->m_pImageData + lineWIDth*(img->m_height-1));
        int i;
		for (i = 0; i < (img->m_height / 2); ++i)
		{
			memcpy(tmpBits, top, lineWIDth);
			memcpy(top, bottom, lineWIDth);
			memcpy(bottom, tmpBits, lineWIDth);

			top = (rgb_t*)((unsigned char*)top + lineWIDth);
			bottom = (rgb_t*)((unsigned char*)bottom - lineWIDth);
		}

		free(tmpBits);
		tmpBits = 0;
	}

	return 1;
}

void Release(TargaImage *img)
{
	free(img->m_pImageData);
	img->m_pImageData = NULL;
}

int ConvertRGBToRGBA(TargaImage *img,unsigned char alphaValue)
{
	if ((img->m_colorDepth == 24) && (img->m_imageDataFormat == IMAGE_RGB))
	{
		rgba_t *newImage =(rgba_t*)malloc(sizeof(rgba_t)*(img->m_wIDth *img-> m_height));

		if (!newImage)
			return 0;

		rgba_t *dest = newImage;
		rgb_t *src = (rgb_t*)img->m_pImageData;
        int x;
		for (x = 0; x < img->m_height; x++)
		{
		    int y;
			for (y = 0; y <img-> m_wIDth; y++)
			{
				dest->r = src->r;
				dest->g = src->g;
				dest->b = src->b;
				dest->a = alphaValue;

				++src;
				++dest;
			}
		}

		free(img->m_pImageData);
		img->m_pImageData = (unsigned char*)newImage;

		img->m_colorDepth = 32;
		img->m_imageDataType = IMAGE_DATA_UNSIGNED_BYTE;
		img->m_imageDataFormat = IMAGE_RGBA;

		return 1;
	}

	return 0;
}

int ConvertRGBAToRGB(TargaImage *img)
{
	if ((img->m_colorDepth == 32) && (img->m_imageDataFormat == IMAGE_RGBA))
	{
		rgb_t *newImage =(rgb_t*)malloc(sizeof(rgb_t)*(img->m_wIDth * img->m_height));

		if (!newImage)
			return 0;

		rgb_t *dest = newImage;
		rgba_t *src = (rgba_t*)img->m_pImageData;
        int x;
		for (x = 0; x < img->m_height; x++)
		{
		    int y;
			for (y = 0; y < img->m_wIDth; y++)
			{
				dest->r = src->r;
				dest->g = src->g;
				dest->b = src->b;

				++src;
				++dest;
			}
		}

		free(img->m_pImageData);
		img->m_pImageData = (unsigned char*)newImage;

		img->m_colorDepth = 24;
		img->m_imageDataType = IMAGE_DATA_UNSIGNED_BYTE;
		img->m_imageDataFormat = IMAGE_RGB;

		return 1;
	}

	return 0;
}

