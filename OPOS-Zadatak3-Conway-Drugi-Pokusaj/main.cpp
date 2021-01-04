#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include "CL/cl.h"
#include "utilities.h"
#include <iostream>
/*
* 
*/

void printArgs();

int main(int argc, char ** argv) {
	if (argc == 1)
		printArgs();
	cl_platform_id cpPlatform;				// OpenCL platform
	cl_device_id device_id;					// device ID
	cl_context context;						// context
	cl_command_queue queue;					// command queue
	cl_program program;						// program
	cl_kernel Kill_Pass_Resurrect_kernel;	// kernel
	cl_kernel Check_Oscilators_kernel;		// kernel

	cl_int err;

	// Bind to platform
	err = clGetPlatformIDs(1, &cpPlatform, NULL);

	// Get ID for the device
	err = clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);

	// Create a context  
	context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);

	// Create a command queue 
	queue = clCreateCommandQueue(context, device_id, 0, &err);

	char* kernelSource = readKernelSource("GameOfLife.cl");

	// Create the compute program from the source buffer
	program = clCreateProgramWithSource(context, 1, (const char**)&kernelSource, NULL, &err);

	// Build the program executable 
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);

	if (err)
	{
		// Determine the size of the log
		size_t log_size;
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

		// Allocate memory for the log
		char* log = new char[log_size];

		// Get the log
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

		// Print the log
		std::cout << log << std::endl;;

		delete[] log;
	}

	Kill_Pass_Resurrect_kernel = clCreateKernel(program, "GoL_Kill_Pass_Resurrect", &err);
	Check_Oscilators_kernel = clCreateKernel(program, "GoL_Check_Oscilators", &err);

	int imgWidth, imgHeight;
	unsigned char* imgPixels;
	readImage("image0.pgm", imgPixels, imgWidth, imgHeight);
	unsigned char *noviPixels = new unsigned char[imgHeight * imgWidth]();

	cl_mem gpu_imgPixels = clCreateBuffer(context, CL_MEM_READ_WRITE, imgWidth * imgHeight, NULL, &err);
	clEnqueueWriteBuffer(queue, gpu_imgPixels, CL_TRUE, 0, imgWidth * imgHeight, imgPixels, 0, NULL, NULL);
	clFlush(queue);
	clSetKernelArg(Kill_Pass_Resurrect_kernel, 0, sizeof(cl_mem), &gpu_imgPixels);
	clSetKernelArg(Kill_Pass_Resurrect_kernel, 1, sizeof(int), &imgHeight);
	clSetKernelArg(Kill_Pass_Resurrect_kernel, 2, sizeof(int), &imgWidth);
	
	clFlush(queue);

	size_t workLength[2] = { imgWidth, imgHeight };
	clEnqueueNDRangeKernel(queue, Kill_Pass_Resurrect_kernel, 2, NULL, workLength, NULL, 0, NULL, NULL);

	clFlush(queue);
	clEnqueueReadBuffer(queue, gpu_imgPixels, CL_TRUE, 0, static_cast<size_t>(imgWidth) * imgHeight, noviPixels, 0, NULL, NULL);
	clFlush(queue);
	writeImage("image1.pgm", noviPixels, imgWidth, imgHeight);

	//cleanup
	clReleaseMemObject(gpu_imgPixels);
	clReleaseKernel(Kill_Pass_Resurrect_kernel);
	clReleaseKernel(Check_Oscilators_kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(queue);
	clReleaseContext(context);

	return err;
}

void printArgs() {
	std::cout <<
R"(Ok, vidim da nisi naveo argumente, ali ima nekoliko argumenata koje mozes koristiti:
  -from <N> pokrece simulaciju kao da je vec uradjeno N-1 koraka
  -img <naziv> kristi img kao ulaznu sliku
  -w <W> postavlja sirinu polja (uzajamno iskljucivo za -img, -img ima veci prioritet)
  -h <H> postavlja visinu polja (isto kao -w, ukoliko nije prisutno za -h se koristi <W>)
  -up x,y oznacava da je celija u x-tom redu u y-toj koloni ziva, moze ih se navesti vise
)" << std::endl;
}