#ifndef IMAGE_H
#define IMAGE_H
struct ImageData
{
	// Raw image data
	unsigned char* data;
	unsigned int dataSize;

	// Pixel offset info
	int w_offset;
	int h_offset;
	int activeTexture;

	int depth;
	int row;
	int col;
	int face;

	// Done downloading?
	bool complete;
};
#endif