#define _CRT_SECURE_NO_WARNINGS
#include "utilities.h"
#include <cstdio>
#include <fstream>
char* readKernelSource(const char* filename)
{
	long length;
	std::fstream kernelFile(filename);
	std::string content((std::istreambuf_iterator<char>(kernelFile)),std::istreambuf_iterator<char>());
	char* kernelCharArray = new char[content.size()+1]();//() postavi 0
	std::memcpy(kernelCharArray, content.c_str(), content.size());
	return kernelCharArray;
}

void readImage(const char* filename, unsigned char*& array, int& width, int& height)
{
	FILE* fp = fopen(filename, "rb"); /* b - binary mode */
	if (!fscanf(fp, "P5\n%d %d\n255\n", &width, &height)) {
		throw "error";
	}
	unsigned char* image = new unsigned char[(size_t)width * height];
	fread(image, sizeof(unsigned char), (size_t)width * (size_t)height, fp);
	fclose(fp);
	array = image;
}

void writeImage(const char* filename, const unsigned char* array, const int width, const int height)
{
	FILE* fp = fopen(filename, "wb"); /* b - binary mode */
	fprintf(fp, "P5\n%d %d\n255\n", width, height);
	fwrite(array, sizeof(unsigned char), (size_t)width * (size_t)height, fp);
	fclose(fp);
}