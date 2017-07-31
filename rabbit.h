/*  GIMP header image file format (RGB): /Users/leozhou/projects/the_pulsating_heart/rabbit.h  */

static unsigned int width = 6;
static unsigned int height = 7;

/*  Call this macro repeatedly.  After each use, the pixel data can be extracted  */

#define HEADER_PIXEL(data,pixel) {\
pixel[0] = (((data[0] - 33) << 2) | ((data[1] - 33) >> 4)); \
pixel[1] = ((((data[1] - 33) & 0xF) << 4) | ((data[2] - 33) >> 2)); \
pixel[2] = ((((data[2] - 33) & 0x3) << 6) | ((data[3] - 33))); \
data += 4; \
}
static char *header_data =
	"````DY_0````DY_0````````````DY_0````DY_0````````````DY_0````DY_0"
	"````````````L;WNL;WNL;WN````#95!````!!!!L;WN!!!!`````Y%$Y_0DY_0D"
	"Y_0DY_0DY_0D`Y%$Y_0DY_0DY_0DY_0DY_0D`Y%$";
