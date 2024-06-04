

#define TGA_NONE			0
#define TGA_PAL				1
#define TGA_RGB				2
#define TGA_BW				3
#define TGA_PAL_RLE			9
#define TGA_RGB_RLE			10
#define TGA_BW_RLE			11
//#define TGA_XZ			32
//#define TGA_XZ1			33

//#define TGAORIGIN_LEFT

#define TGA_INVERSELEFTRIGHT 0x10
#define TGA_INVERSETOPBOTTOM 0x20

#include <pshpack1.h>

typedef struct tga_file_header_s
{
	unsigned char iIDLen;
	unsigned char eColorMapType;
	unsigned char eImageType;
	struct { //color map specification
		unsigned short iStart;
		unsigned short iLength;
		unsigned char iEntrySize;
	} cm;
	struct { //image specification
		unsigned short xOrigin;
		unsigned short yOrigin;
		unsigned short iWidth;
		unsigned short iHeight;
		unsigned char iPixelSize;
		unsigned char flags;
	} img;
} tga_file_header_t;

#include <poppack.h>
