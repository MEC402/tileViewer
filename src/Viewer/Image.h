#pragma once
struct ImageData
{
	unsigned char* data;
	unsigned int dataSize;
	int w_offset;
	int h_offset;
	int activeTexture;
	bool complete;
};