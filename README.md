# OPOS2020 - Projekt 3 - Conway's game of life
Game of Life implementirano na GPU-u koristeci OpenCL.
## Predgovor
Intelov OpenCL SDK nije lijepo saradjivao tako da sam koristio header-e i x64 biblioteku sa [KhronosGroup-ovog GitHub repozitorijuma](https://github.com/KhronosGroup/OpenCL-SDK). Biblioteka je, cini mi se, 3.0, ali je koristen OpenCL 1.2 zbog toga sto su primjeri bili 1.2 i dosta dokumentacije je za 1.2. Zbog toga je prva linija u [main.cpp](OPOS-Zadatak3-Conway-Drugi-Pokusaj/main.cpp) fajlu `#define CL_USE_DEPRECATED_OPENCL_1_2_APIS` da bi mi dozvolilo da koristim 1.2 API.

## Zahtjevi
Redom:
* na GPU? - Jeste. Svo odlucivanje ko zivi a ko umire se radi na grafickoj u `GoL_Kill_Pass_Resurrect` kernelu.
* Crno bijela slika proizvoljnih dimenzija? - Jeste. Testirao sam do 16384x16384 i bilo je ok, samo sto je jedna takva oko 730MB.
* Rucno podesavanje inicijalno zivih celija? - Jeste, pogledaj [Argumente](#argumenti), ukratko koristeci `-up x,y` parametar se moze postaviti ziva celija.
* Prelazak na proizvoljnu iteraciju od trenutne? - Mozda. Ne znam sta ovo znaci. Moze se reci "E, ovo nije prvi korak, ovo je 153. korak" koristeci `-from` flag (khm [argumenti](#argumenti))
* Dodatna funkcionalnost? - Da, najlaksi oscilator: onaj 3x1 ili 1x3 stapic, blinker. Bude oznacen nekom plavom bojom ako se detektuje.
* Svi kao posebni kerneli? - E ovaj dio je sumnjiv. Htio sam generisanje i detekciju uraditi kao dva, ali pise ipak kao jedan i tako ja ostado sa jednim kernelom...
* Velik prostor u vise pokretanja? - Da, pokrene ga koristeci ispravan offset, cini mi se da cak i radi ispravno. Za potrebe testiranja mogu se manje granice forsirati koristeci `-ogm` flag iz [argumenata](#argumenti)
* Prostor za igru alociran na uredjaju? - Jeste, postoje dva buffera za trenutno i sljedece stanje, smjenjuju se bez oslobadjanja izmedju iteracija. Isto vazi i za memoriju na hostu koja se upisuje u fajl.

## Argumenti
Ima ih nekoliko i svi su donekle korisni. Obavezno je navesti ILI (sirinu ILI visinu) ILI ulaznu sliku u `.ppm` formatu iz koje se moze procitati sirina i visina.
* `-from <M>` : pokrece simulaciju kao da je vec uradjeno `M` koraka (default 0)
* `-to <N>` : odredjuje broj koraka simulacije (default 100)
* `-img <naziv>` : koristi img kao ulaznu sliku
* `-w <W>` : postavlja sirinu polja (uzajamno iskljucivo za `-img`, `-img` ima veci prioritet)
* `-h <H>` : postavlja visinu polja (isto kao `-w`, ukoliko nije prisutno za `-h` se koristi `<W>`)
* `-up x,y` : oznacava da je celija u x-tom redu u y-toj koloni ziva, moze ih se navesti vise. Radi za sliku i za `-w <W>` `-h <H>` fazon. Ako je van opsega i ukljucen je `-wordy` flag spamace malo u konzolu kad naidje na lose koordinate. Ako je ijedna neispravna zatvara se
* `-nemamprostora` : Nece cuvati generisane slike u ./out/ folder. Super kad neces 50 slika po 100MB na disku sa 60GB slobodnog prostora.
* `-ogm <N>` : (skraceno od overrideGlobalMaximum, dugo je) koristi alternativnu maksimalnu (N) velicinu posla. Default je 1024, dohvata se broj sa graficke, ali moze i ovako na silu biti nize. Koristi se NxN zbog 2D posla.
* `-wordy` : Pored iteracije koju radi takodje ce da spama koje je grupe odradio i koje grupe tek treba da odradi.