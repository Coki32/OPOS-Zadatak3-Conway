#pragma once

char* readKernelSource(const char* filename);

void readImage(const char* filename, unsigned char*& array, int& width, int& height);

void writeImage(const char* filename, const unsigned char* array, const int width, const int height);
