#ifndef IMAGE_H
#define IMAGE_H

struct ImageData
{
	// Raw image data
	unsigned char* data;
	unsigned int dataSize;
	int width;
	int height;
	int colorChannels;

	// Pixel offset info
	int w_offset;
	int h_offset;
	int activeTexture;

	// Tile placement info
	int depth;
	int row;
	int col;
	int face;
	int eye;

	// Done downloading?
	bool complete;

	//stbi_free is just a wrapper around free() so this is safe to do
	void Free()
	{
		if (data != NULL)
			free(data);
		free(this);
	}
};
#endif