#define _CRT_SECURE_NO_WARNINGS
#include "utilities.h"
#include <cstdio>
#include <fstream>

char* readKernelSource(const char* filename)
{
	std::fstream kernelFile(filename);
	std::string content((std::istreambuf_iterator<char>(kernelFile)),std::istreambuf_iterator<char>());
	char* kernelCharArray = new char[content.size()+1]();//() postavi 0
	std::memcpy(kernelCharArray, content.c_str(), content.size());
	return kernelCharArray;
}

void readImage(const char* filename, Pixel*& array, int& width, int& height)
{
	FILE* fp = fopen(filename, "rb"); /* b - binary mode */
	char junk[1024] = { 0 };
	if (!fscanf(fp, "P6\n%*[^\n]\n%d %d\n255\n", &width, &height)) {
		throw "error";
	}
	Pixel* image = new Pixel[(size_t)width * height];
	fread(image, sizeof(Pixel), (size_t)width * (size_t)height, fp);
	fclose(fp);
	array = image;
}

void writeImage(const char* filename, const Pixel* array, const int width, const int height)
{
	FILE* fp = fopen(filename, "wb"); /* b - binary mode */
	fprintf(fp, "P6\n# Komentar je sad obavezan\n%d %d\n255\n", width, height);
	fwrite(array, sizeof(Pixel), (size_t)width * (size_t)height, fp);
	fclose(fp);
}

std::shared_ptr<std::vector<std::string>> charPtrArrayToVector(char** inputArray, int length)
{
	auto result = std::make_shared<std::vector<std::string>>();
	result->reserve(length);
	for (int i = 0; i < length; ++i)
		result->emplace_back(inputArray[i]);
	return result;
}

std::pair<int, int> parseIntPair(const std::string& intPair)
{
	auto commaIdx = intPair.find_first_of(',');
	return std::make_pair(std::stoi(intPair.substr(0, commaIdx)), std::stoi(intPair.substr(commaIdx+1)));
}
