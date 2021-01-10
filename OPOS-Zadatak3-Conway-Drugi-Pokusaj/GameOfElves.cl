/**
ziva celija sa:
    n<2				=> UMIRE
    n==2 || n==3	=> prezivljava
    n>3				=> UMIRE
mrtva celija sa:
    n==3			=> ozivljava
*/
struct Elf {
    unsigned char r, g, b;
};

int shouldCheck(const int height, const int width, int i, int j, int padLeft, int padRight, int padUp, int padDown);
int isDead(struct Elf c);
int isAlive(struct Elf c);
//NE POZIVAJ PRIJE PROVJERE IMA LI MJESTA!!
int countNeighbours(__global struct Elf* elves, const int width, int i, int j);
void paintIt(__global struct Elf* elves, const int width, int i, int j, int r, int g, int b);
void killIt(__global struct Elf* elves, const int width, int i, int j);
void resurrectIt(__global struct Elf* elves, const int width, int i, int j);
//skraceno od prettyGet
struct Elf pg(__global struct Elf* oldElves, int width, int i, int j, int ii, int jj);
//Blinker mora biti u sredini bloka od 5 jer onda ima border mrtvih oko sebe
int isBlinker(__global struct Elf* oldElves, int width, int i, int j);
//znam da se zove u mnozini ali provjerava samo jedan - obicni blinker
void checkOscilators(__global struct Elf* oldElves, __global struct Elf* nextGenElves, const int height, const int width, int i, int j);


__kernel void GoL_Kill_Pass_Resurrect(__global struct Elf* oldElves, __global struct Elf* nextGenElves, const int height, const int width) {
    int i = get_global_id(0);
    int j = get_global_id(1);

    if (shouldCheck(height, width, i, j, 1, 1, 1, 1)) {
        int nbc = countNeighbours(oldElves, width, i, j);
        int alive = isAlive(oldElves[i * width + j]);
        int dead = isDead(oldElves[i * width + j]);//postoji i ovo jer necu da prepisem nista osim crnih/bijelih. Sareni pikseli se postavljaju na 0,0,0
        if (alive) {
            if (nbc < 2 || nbc > 3)//ne voli guzvu, umire
                killIt(nextGenElves, width, i, j);
            else // prezivljava
                resurrectIt(nextGenElves, width, i, j);//ali radim resurrect da se postavi na bijelu, mozda vise ne osciluje ako je nesto doslo do toga
        }
        else if (dead && nbc == 3) //svi zivi osim nje, nije fazon
            resurrectIt(nextGenElves, width, i, j);
        else //znaci nit je mrtva nit je ziva, to je neki sareni piksel (od detekcije u proslom koraku) => postavi 0,0,0 (ubij)
            killIt(nextGenElves, width, i, j);
    }
        barrier(CLK_GLOBAL_MEM_FENCE);

    checkOscilators(oldElves, nextGenElves, height, width, i, j);
}

//Blinker mora biti u sredini bloka od 5 jer onda ima border mrtvih oko sebe
int isBlinker(__global struct Elf* oldElves, int width, int i, int j) {
    int zivoPoKolonama[5] = { 0, 0, 0, 0, 0 };
    int zivoPoRedovima[5] = { 0, 0, 0, 0, 0 };
    for (int ii = 0; ii < 5; ++ii)
        for (int jj = 0; jj < 5; ++jj) {
            zivoPoRedovima[ii] += isAlive(pg(oldElves, width, i, j, ii, jj));
            zivoPoKolonama[jj] += isAlive(pg(oldElves, width, i, j, ii, jj));
        }
    return ((zivoPoKolonama[0] + zivoPoKolonama[1] + zivoPoKolonama[3] + zivoPoKolonama[4]) == 0 && zivoPoKolonama[2] == 3 && zivoPoRedovima[0] == 0 && zivoPoRedovima[4] == 0)
        || ((zivoPoRedovima[0] + zivoPoRedovima[1] + zivoPoRedovima[3] + zivoPoRedovima[4]) == 0 && zivoPoRedovima[2] == 3 && zivoPoKolonama[0] == 0 && zivoPoKolonama[4] == 0);
}

void checkOscilators(__global struct Elf* oldElves, __global struct Elf* nextGenElves, const int height, const int width, int i, int j) {
    if (shouldCheck(height, width, i, j, 0, 5, 0, 5) && isBlinker(nextGenElves, width, i, j)) {//trazi 5x5 blok oko sebe slobodan, inace rizikujes da ga neko dotakne odmah
        for (int ii = 0; ii < 5; ++ii)
            for (int jj = 0; jj < 5; ++jj)
                if (isAlive(pg(nextGenElves, width, i, j, ii, jj)))
                    paintIt(nextGenElves, width, i + ii, j + jj, 3, 136, 252);
    }
}

int shouldCheck(const int height, const int width, int i, int j, int padLeft, int padRight, int padUp, int padDown) {
    return (i >= padUp && i < (height - padDown)) && (j >= padLeft && j < (width - padRight));
}

int isDead(struct Elf c) {//SAMO CRNI
    return c.r == 0 && c.g == 0 && c.b == 0;
}

int isAlive(struct Elf c) {//svi oni sareni koji su bili oscilovali su zivi
    return !isDead(c);
}

int countNeighbours(__global struct Elf* elves, const int width, int i, int j) {
    return isAlive(elves[i * width + j - 1]) + //lijevi
        isAlive(elves[i * width + j + 1]) + //desni
        isAlive(elves[width * (i + 1) + j]) + //ispod
        isAlive(elves[width * (i - 1) + j]) + //iznad
        isAlive(elves[width * (i - 1) + j - 1]) + //dijag lijevo gore
        isAlive(elves[width * (i - 1) + j + 1]) + //dijag desno gore
        isAlive(elves[width * (i + 1) + j - 1]) + //dijag lijevo dolje
        isAlive(elves[width * (i + 1) + j + 1]);  //dijag desno dolje
}

void paintIt(__global struct Elf* elves, const int width, int i, int j, int r, int g, int b) {
    elves[i * width + j].r = r;
    elves[i * width + j].g = g;
    elves[i * width + j].b = b;
}

void killIt(__global struct Elf* elves, const int width, int i, int j) {
    paintIt(elves, width, i, j, 0, 0, 0);
}

void resurrectIt(__global struct Elf* elves, const int width, int i, int j) {
    paintIt(elves, width, i, j, 255, 255, 255);
}

struct Elf pg(__global struct Elf* elves, int width, int i, int j, int ii, int jj) {
    return elves[(i + ii) * width + j + jj];
}