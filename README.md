# OPOS2020 - Projekt 3 - Conway's game of life
Game of Life implementirano na GPU-u koristeci OpenCL.
## Predgovor
Intelov OpenCL SDK nije lijepo saradjivao tako da sam koristio header-e i x64 biblioteku sa [KhronosGroup-ovog GitHub repozitorijuma](https://github.com/KhronosGroup/OpenCL-SDK). Biblioteka je, cini mi se, 3.0, ali je koristen OpenCL 1.2 zbog toga sto su primjeri bili 1.2 i dosta dokumentacije je za 1.2. Zbog toga je prva linija u (main.cpp)[../OPOS-Zadatak3-Conway-Drugi-Pokusaj/main.cpp] fajlu `#define CL_USE_DEPRECATED_OPENCL_1_2_APIS` da bi mi dozvolilo da koristim 1.2 API.