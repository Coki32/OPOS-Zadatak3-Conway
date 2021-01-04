/**
ziva celija sa:
	n<2				=> UMIRE
	n==2 || n==3	=> prezivljava
	n>3				=> UMIRE
mrtva celija sa:
	n==3			=> ozivljava
*/
struct Pixel {
    unsigned char r, g, b;
};

int shouldCheck(const int height, const int width, int i, int j, int padLeft, int padRight, int padUp, int padDown){
    return (i >= padUp && i < (height - padDown)) && (j >= padLeft && j < (width - padRight));
}

int isAlive(struct Pixel p){
    return p.r == p.g && p.g==p.b && p.b == 0; 
}

//NE POZIVAJ PRIJE PROVJERE IMA LI MJESTA!!
int countNeighbours(__global struct Pixel* bytes, const int width, int i, int j){
    return isAlive( bytes[i*width + j - 1]) + //lijevi
           isAlive( bytes[i*width + j + 1]) + //desni
           isAlive( bytes[i*(width+1) + j]) + //ispod
           isAlive( bytes[i*(width-1) + j]) + //iznad
           isAlive( bytes[i*(width-1) + j - 1]) + //dijag lijevo gore
           isAlive( bytes[i*(width-1) + j + 1]) + //dijag desno gore
           isAlive( bytes[i*(width+1) + j - 1]) + //dijag lijevo dolje
           isAlive( bytes[i*(width+1) + j + 1]); //dijag desno dolje
}

void paintIt(__global struct Pixel* bytes, const int width, int i, int j, int r, int g, int b){
    bytes[i*width+j].r = r;
    bytes[i*width+j].g = g;
    bytes[i*width+j].b = b;
}

void killIt(__global struct Pixel* bytes, const int width, int i, int j){
    paintIt(bytes, width, i, j, 0, 0, 0);
}

void resurrectIt(__global struct Pixel* bytes, const int width, int i, int j){
    paintIt(bytes, width, i, j, 255, 255, 255);
}

__kernel void GoL_Kill_Pass_Resurrect(__global struct Pixel* bytes, const int height, const int width){
    int i = get_global_id(0);
    int j = get_global_id(1);
    bytes[i*width+j].r = i;
    bytes[i*width+j].g = j;

    if(shouldCheck(height, width, i, j, 1, 1, 1, 1)){
        int nbc = countNeighbours(bytes, width, i, j);
        int alive = isAlive(bytes[i*width+j]);
        if(alive && (nbc<2 || nbc>3))
            killIt(bytes, width, i, j);
        else if( !alive && nbc==3)
            resurrectIt(bytes, width, i, j);
    }
	barrier(CLK_GLOBAL_MEM_FENCE);
}
//NE POZIVAJ PRIJE PROVJERE IMA LI MJESTA!!
int isBlinker(__global struct Pixel* bytes, int width, int i, int j){
    return ( isAlive(bytes[i*width + j + 1]) && isAlive(bytes[i*(width+1) + j + 1]) && isAlive(bytes[i*(width+2) + j + 2]) ) ||
        (isAlive(bytes[i*(width+1) + j]) && isAlive(bytes[i*(width+1) + j + 1]) && isAlive(bytes[i*(width+1) + j + 2]));
}

__kernel void GoL_Check_Oscilators(__global struct Pixel* bytes, const int height, const int width){
    int i = get_global_id(0);
    int j = get_global_id(1);
}

