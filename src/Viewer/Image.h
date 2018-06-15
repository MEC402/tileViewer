#ifndef IMAGE_H
#define IMAGE_H

struct ImageData
{
	// Raw image data
	//std::unique_ptr<unsigned char*> data;
	unsigned char* data;
	unsigned int dataSize;
	int colorChannels;

	// Pixel offset info
	int w_offset;
	int h_offset;
	int activeTexture;

	int depth;
	int row;
	int col;
	int face;
	int eye;

	// Done downloading?
	bool complete;

	void Free()
	{
		if (data != NULL)
			free(data);
		free(this);
	}
};
#endif