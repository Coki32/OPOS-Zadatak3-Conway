#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include "CL/cl.h"
#include "utilities.h"
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

#define _SPAMMER_BUILD 0

const char* const kernelFileName = "GameOfLife.cl";

int globalMaxWorkSize[2] = { 1024,1024 };//default max hardcoded

struct Arguments {
	int from, to;
	std::string inputImage;
	bool usesInputImage;
	int createWidth, createHeight;
	int overrideGlobalMax;
	std::shared_ptr<std::vector<std::pair<int, int>>> additionalCells;
	bool wordy;
	bool savingDiskSpace;
	Arguments() : from(0), to(100), inputImage(""), usesInputImage(false), createHeight(0), createWidth(0),
				  additionalCells(nullptr), overrideGlobalMax(0), wordy(false), savingDiskSpace(false)
	{}
};

void printArgs();
Arguments parseArguments(int argc, char** argv);

int main(int _argc, char** _argv) {//_ da mi se ne pojavi kad napisem a
	Arguments actualArguments;
	if (_argc == 1) {
		printArgs();
		return 1;
	}
	else {//blok je visak ali ga VS ruzno formatira ako nema bloka
		try {
			actualArguments = parseArguments(_argc, _argv);
		}
		catch (const std::invalid_argument&) {
			std::cerr << "Neispravan format broja za argument! Svi su cijeli brojevi, pazi to" << std::endl;
			return -1;
		}
		catch (const std::exception& drugaGreskica) {
			std::cerr << drugaGreskica.what() << std::endl;
			return -2;
		}
	}

	if (!actualArguments.usesInputImage && (actualArguments.createWidth == 0 || actualArguments.createHeight == 0)) {
		std::cerr << "Nema ni slike ni argumenata za sirinu, ne moze to tako...." << std::endl;
		return -3;
	}

	cl_kernel Kill_Pass_Resurrect_kernel;	// kernel

#pragma region OpenCL init kopiran
	cl_platform_id cpPlatform;				// OpenCL platform
	cl_device_id device_id;					// device ID
	cl_context context;						// context
	cl_command_queue queue;					// command queue
	cl_program program;						// program

	cl_int err;

	size_t max_work_item_sizes[3] = { 0,0,0 };

	// Bind to platform
	err = clGetPlatformIDs(1, &cpPlatform, NULL);

	// Get ID for the device
	err = clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);

	// Create a context  
	context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);

	// Create a command queue 
	queue = clCreateCommandQueue(context, device_id, 0, &err);

	char* kernelSource = readKernelSource(kernelFileName);

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

		return -3;
	}

	Kill_Pass_Resurrect_kernel = clCreateKernel(program, "GoL_Kill_Pass_Resurrect", &err);
#pragma endregion

#pragma region Inicijalizacije i alokacije
	int imgWidth, imgHeight, imgSize;
	Pixel* previous = nullptr, * next = nullptr;//Ne znam kako se OpenCL slaze s pametnim pokazivacima i necu da saznam
	cl_mem gpu_previous, gpu_next;
	if (actualArguments.usesInputImage) {
		readImage(actualArguments.inputImage.c_str(), previous, imgWidth, imgHeight);
	}
	else {
		imgWidth = actualArguments.createWidth;
		imgHeight = actualArguments.createHeight;
		previous = new Pixel[(size_t)imgWidth * imgHeight]();
	}
	imgSize = imgWidth * imgHeight;//velicina kao broj piksela, ne u bajtovima, mnozi sa sizeof(Pixel)!
	next = new Pixel[imgSize]();//rezervisi odmah i ovo
	//odmah rezervisi prostor na grafickoj za prvi
	gpu_previous = clCreateBuffer(context, CL_MEM_READ_WRITE, imgSize * sizeof(Pixel), NULL, &err);
	gpu_next = clCreateBuffer(context, CL_MEM_READ_WRITE, imgSize * sizeof(Pixel), NULL, &err);

	if (actualArguments.overrideGlobalMax == 0) {// nije postavljen -ogm flag
		clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(max_work_item_sizes), max_work_item_sizes, NULL);
		globalMaxWorkSize[0] = max_work_item_sizes[0];
		globalMaxWorkSize[1] = max_work_item_sizes[1];
	}
	else {//postavljen -ogm flag
		globalMaxWorkSize[0] = actualArguments.overrideGlobalMax;
		globalMaxWorkSize[1] = actualArguments.overrideGlobalMax;
	}
#pragma endregion

	if (actualArguments.wordy)
		std::cout << "Ok, radim sa slikom dimenzija [" << imgWidth << ", " << imgHeight << "]" << std::endl;

	std::cout << "Ivice se odbacuju. Ako je celija na ivici ziva ostace ziva i samo ce smetati onima okolo" << std::endl;

	if (actualArguments.additionalCells != nullptr)
		for (auto it = actualArguments.additionalCells->begin(); it != actualArguments.additionalCells->end(); ++it) {
			auto& [x, y] = *it;
			if (x<0 || x>imgWidth || y<0 || y>imgHeight)
				std::cerr << "Ne mogu postaviti piksel na poziciji [" << x << ", " << y << "] jer je van granica slike!" << std::endl;
			else
				previous[x * imgWidth + y] = { 255, 255, 255 };
		}

	//napravi folder prije pocetka jer spama tonu slika...
	std::filesystem::create_directories("./out");

	for (int i = actualArguments.from; i < actualArguments.to; ++i) {
		if(actualArguments.wordy)
			std::cout << "Pokrecem " << i << ". iteraciju igre!" << std::endl;

		auto out = std::string("./out/iteration").append(std::to_string(i)).append(".ppm");

		clEnqueueWriteBuffer(queue, gpu_previous, CL_TRUE, 0, imgSize * sizeof(Pixel), previous, 0, NULL, NULL);
		clFinish(queue);
		clSetKernelArg(Kill_Pass_Resurrect_kernel, 0, sizeof(cl_mem), &gpu_previous);//iz cega
		clSetKernelArg(Kill_Pass_Resurrect_kernel, 1, sizeof(cl_mem), &gpu_next);//u sta
		clSetKernelArg(Kill_Pass_Resurrect_kernel, 2, sizeof(int), &imgHeight);
		clSetKernelArg(Kill_Pass_Resurrect_kernel, 3, sizeof(int), &imgWidth);

		if (imgHeight <= globalMaxWorkSize[0] && imgWidth <= globalMaxWorkSize[1]) {
			size_t workLength[2] = { (size_t)imgHeight, (size_t)imgWidth };
#ifdef _SPAMMER_BUILD
			//std::cout << "\tGrupe:" << std::endl
			//	<< "\t\tgroup[0] = " << workLength[0] << std::endl
			//	<< "\t\tgroup[1] = " << workLength[1] << std::endl;
#endif // _SPAMMER_BUILD
			clEnqueueNDRangeKernel(queue, Kill_Pass_Resurrect_kernel, 2, NULL, workLength, NULL, 0, NULL, NULL);
			clFinish(queue);
		}
		else {
			size_t toDo[2] = { (size_t)imgHeight,	(size_t)imgWidth };
			size_t done[2] = { (size_t)0,			(size_t)0 };
			size_t workSize[2] = { globalMaxWorkSize[0], globalMaxWorkSize[1] };
			int iter = 0;
			int xSteps = (int)std::ceil((double)imgWidth / globalMaxWorkSize[0]);
			int ySteps = (int)std::ceil((double)imgHeight / globalMaxWorkSize[1]);
			for (int x = 0; x < xSteps; ++x)
				for (int y = 0; y < ySteps; ++y) {
					done[0] = (size_t)x * globalMaxWorkSize[0];
					done[1] = (size_t)y * globalMaxWorkSize[1];
					workSize[0] = std::min(toDo[0] - done[0], (size_t)globalMaxWorkSize);
					workSize[1] = std::min(toDo[1] - done[1], (size_t)globalMaxWorkSize);
					if(actualArguments.wordy)
						std::cout << "\t\t\t" << iter++ << std::endl
							<< "\t\t\t\tUradjeno:\t\t" << done[0] << ",\t" << done[1] << std::endl
							<< "\t\t\t\tTreba da uradim jos\t" << toDo[0] << ",\t" << toDo[1] << std::endl
							<< "\t\t\t\tRadim posao velicine\t" << workSize[0] << ",\t" << workSize[1] << std::endl;
					clEnqueueNDRangeKernel(queue, Kill_Pass_Resurrect_kernel, 2, done, workSize, NULL, 0, NULL, NULL);
					clFinish(queue);
				}
		}

		clEnqueueReadBuffer(queue, gpu_next, CL_TRUE, 0, imgSize * sizeof(Pixel), next, 0, NULL, NULL);//citaj sto si upisao
		clFinish(queue);
		if (!actualArguments.savingDiskSpace)
			writeImage(out.c_str(), next, imgWidth, imgHeight);
		std::swap(previous, next);//ne moraju se ni oslobadjati nikad
		std::swap(gpu_previous, gpu_next);
	}
	delete[] previous;//dobro ne bas "nikad". Moraju /nekad/
	delete[] next;
	//i opencl cleanup
	clReleaseMemObject(gpu_next);
	clReleaseMemObject(gpu_previous);
	clReleaseKernel(Kill_Pass_Resurrect_kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(queue);
	clReleaseContext(context);

	return 0;
}

void printArgs() {
	std::cout <<
		R"(Ok, vidim da nisi naveo argumente, ali ima nekoliko argumenata koje mozes koristiti:
  -from <M>       pokrece simulaciju kao da je vec uradjeno N-1 koraka (default 0)
  -to <N>         odredjuje broj koraka simulacije (default 100)
  -img            <naziv> koristi img kao ulaznu sliku
  -w <W>          postavlja sirinu polja (uzajamno iskljucivo za -img, -img ima veci prioritet)
  -h <H>          postavlja visinu polja (isto kao -w, ukoliko nije prisutno za -h se koristi <W>)
  -up x,y         oznacava da je celija u x-tom redu u y-toj koloni ziva, moze ih se
	              navesti vise. Radi za sliku i za -w -h fazon
  -nemamprostora  Nece cuvati generisane slike u ./out/ folder. Super kad neces 50 slika po 100MB
                      na disku sa 60GB slobodnog prostora.
  -ogm <N>        (skraceno od overrideGlobalMaximum, dugo je) koristi alternativnu maksimalnu (N)
                      velicinu posla. Default je 1024, dohvata se broj sa graficke, ali moze i 
                      ovako na silu biti nize. Koristi se NxN zbog 2D posla.
  -wordy		  Pored iteracije koju radi takodje ce da spama koje je grupe odradio i koje grupe
					  tek treba da odradi.
)" << std::endl;
}

Arguments parseArguments(int argc, char** argv) {
	Arguments arguments;
	auto niceArguments = charPtrArrayToVector(argv, argc);
	const auto end = niceArguments->end();
#if _DEBUG
	for (auto it = niceArguments->begin(); it != end; ++it)
		std::cout << *it << std::endl;
#endif
	//vrati onaj sljedeci argument. Znaci trazis -img on vrati naziv slike. Trazis -w on vrati sirinu
	auto getArgumentIfExists = [&niceArguments](const std::string& arg) -> auto {
		auto it = std::find(niceArguments->begin(), niceArguments->end(), arg);
		return it != niceArguments->end() ? (++it) : niceArguments->end();
	};
	
	if (std::find(niceArguments->begin(), niceArguments->end(), "-nemamprostora") != end)//sad ne mogu koristiti ovu funkciju gore nazalost...
		arguments.savingDiskSpace = true;

	if (std::find(niceArguments->begin(), niceArguments->end(), "-wordy") != end)
		arguments.wordy = true;

	if (auto ogm = getArgumentIfExists("-ogm"); ogm != end && (*ogm).length() > 0)
		arguments.overrideGlobalMax = std::stoi(*ogm);

	if (auto from = getArgumentIfExists("-from"); from!=end && (*from).length() > 0)
		arguments.from = std::stoi(*from);

	if (auto to = getArgumentIfExists("-to"); to!=end && (*to).length() > 0)
		arguments.to = std::stoi(*to);
	
	if (auto imgTitle = getArgumentIfExists("-img"); imgTitle!=end && (*imgTitle).length() > 4) { //.ppm je 4, tako da duzina mora biti >4
		if ((*imgTitle).find(".ppm") == std::string::npos)
			throw std::exception("Neispravna ekstenzija slike! Ulazna slika mora biti .ppm!");
		arguments.inputImage = *imgTitle;
		arguments.usesInputImage = true;
	}
	
	if (!arguments.usesInputImage) {
		if (auto width = getArgumentIfExists("-w"); width != end && (*width).length() > 0)//postavi sirinu ako je ima, a odmah i visinu na isto to
			arguments.createWidth = arguments.createHeight = std::stoi(*width);
		if (auto height = getArgumentIfExists("-h"); height!=end&& (*height).length() > 0) {//postavi visinu ako je ima
			arguments.createHeight = std::stoi(*height);
			if (arguments.createWidth == 0)//A onda takodje i sirinu ako je ima
				arguments.createHeight = arguments.createHeight;
		}
	}

	auto upIt = getArgumentIfExists("-up");//upIt upIterator
	while (upIt != niceArguments->end()) {//
		auto par = parseIntPair(*upIt);//izvadi ta dva inta, sad treba da ih uklonis iz args, i njih i ono ispred njih
		niceArguments->erase(upIt - 1, upIt+1);//izgleda da je [First, Last), zato +1
		if (arguments.additionalCells == nullptr) {
			arguments.additionalCells = std::make_shared<std::vector<std::pair<int,int>>>();
		}
		arguments.additionalCells->push_back(par);
		upIt = getArgumentIfExists("-up");
	}
	
	return arguments;
}