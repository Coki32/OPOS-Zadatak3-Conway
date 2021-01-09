#pragma once
#include <vector>
#include <memory>
#include <string>
struct Pixel
{
	unsigned char r, g, b;
};


char* readKernelSource(const char* filename);

void readImage(const char* filename, Pixel*& array, int& width, int& height);
void writeImage(const char* filename, const Pixel* array, const int width, const int height);

std::shared_ptr<std::vector<std::string>> charPtrArrayToVector(char** inputArray, int length);
std::pair<int, int> parseIntPair(const std::string& intPair);
