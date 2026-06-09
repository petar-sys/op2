#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>

/* ================================================
   STRUKTURE
   ================================================ */

typedef struct cvor cvor;

struct cvor {
    int ime;        /* indeks susjeda */
    int tezina;     /* tezina grane   */
    cvor* sledeci;
};

typedef struct {
    cvor* prvi;
} lista_susjeda;

typedef struct {
    int broj_cvorova;
    float gustina;
    char naziv[60];
    char prefiks[20];
    char** ime_cvora;       /* dinamicki niz imena cvorova */
    lista_susjeda* susjedi; /* dinamicki niz listi susjeda */
} graf;

/* ================================================
   JPL FUNKCIJE
   ================================================ */

cvor* kreiraj_cvor(int ime, int tezina) {
    cvor* c = (cvor*)malloc(sizeof(cvor));
    if(c == NULL) {
        printf("neuspjesno kreiranje cvora!\n");
        exit(1);
    }
    c->ime = ime;
    c->tezina = tezina;
    c->sledeci = NULL;
    return c;
}

void dodaj_na_kraj(cvor** l, cvor* c) {
    if(*l == NULL) {
        *l = c;
    } else {
        cvor* p = *l;
        while(p->sledeci != NULL)
            p = p->sledeci;
        p->sledeci = c;
    }
}

int sadrzi_element(cvor* l, int e) {
    while(l != NULL) {
        if(l->ime == e)
            return 1;
        l = l->sledeci;
    }
    return 0;
}

void obrisi_listu(cvor* l) {
    cvor* p;
    while(l != NULL) {
        p = l;
        l = l->sledeci;
        free(p);
    }
}

/* ================================================
   KREIRANJE I BRISANJE GRAFA
   ================================================ */

graf* kreiraj_graf(const char* naziv, const char* prefiks, int n, float gustina) {
    graf* g = (graf*)malloc(sizeof(graf));
    if(g == NULL) {
        printf("neuspjesno kreiranje grafa!\n");
        exit(1);
    }

    strncpy(g->naziv, naziv, 59);
    strncpy(g->prefiks, prefiks, 19);
    g->broj_cvorova = n;
    g->gustina = gustina;

    /* dinamicki niz listi susjeda */
    g->susjedi = (lista_susjeda*)malloc(n * sizeof(lista_susjeda));
    if(g->susjedi == NULL) {
        printf("neuspjesna alokacija!\n");
        exit(1);
    }
    for(int i = 0; i < n; i++)
        g->susjedi[i].prvi = NULL;

    /* dinamicki niz imena cvorova */
    g->ime_cvora = (char**)malloc(n * sizeof(char*));
    if(g->ime_cvora == NULL) {
        printf("neuspjesna alokacija!\n");
        exit(1);
    }
    for(int i = 0; i < n; i++) {
        g->ime_cvora[i] = (char*)malloc(30 * sizeof(char));
        if(n <= 26)
            sprintf(g->ime_cvora[i], "%s %c", prefiks, 'A' + i);
        else
            sprintf(g->ime_cvora[i], "%s %d", prefiks, i + 1);
    }

    return g;
}

void dodaj_granu(graf* g, int u, int v, int tezina) {
    if(u < 0 || v < 0 || u >= g->broj_cvorova || v >= g->broj_cvorova)
        return;
    if(u == v)
        return;
    if(sadrzi_element(g->susjedi[u].prvi, v))
        return;

    dodaj_na_kraj(&(g->susjedi[u].prvi), kreiraj_cvor(v, tezina));
    dodaj_na_kraj(&(g->susjedi[v].prvi), kreiraj_cvor(u, tezina));
}

void generisi_grane(graf* g) {
    int n = g->broj_cvorova;

    srand((unsigned)(g->naziv[0] * 31u + n + (unsigned)(g->gustina * 100)));

    /* lanac koji osigurava da je graf bar djelimicno povezan */
    for(int i = 0; i < n - 1; i++)
        dodaj_granu(g, i, i + 1, rand() % 50 + 1);

    /* dodatne nasumicne grane prema gustini */
    long long ciljano = (long long)(g->gustina * n * (n - 1) / 2);
    if(ciljano > (long long)n * 15)
        ciljano = (long long)n * 15;

    long long dodato = n - 1;
    long long pokusaj = 0;
    long long limit = ciljano * 20;

    while(dodato < ciljano && pokusaj < limit) {
        int u = rand() % n;
        int v = rand() % n;
        pokusaj++;
        if(u == v) continue;
        if(!sadrzi_element(g->susjedi[u].prvi, v)) {
            dodaj_granu(g, u, v, rand() % 50 + 1);
            dodato++;
        }
    }
}

void obrisi_graf(graf* g) {
    for(int i = 0; i < g->broj_cvorova; i++) {
        obrisi_listu(g->susjedi[i].prvi);
        free(g->ime_cvora[i]);
    }
    free(g->susjedi);
    free(g->ime_cvora);
    free(g);
}

/* ================================================
   DIJKSTRA
   ================================================ */

void dijkstra(graf* g, int start, int* dist, int* preth) {
    int n = g->broj_cvorova;
    int* posjecen = (int*)calloc(n, sizeof(int));

    for(int i = 0; i < n; i++) {
        dist[i] = INT_MAX;
        preth[i] = -1;
    }
    dist[start] = 0;

    for(int korak = 0; korak < n; korak++) {
        /* trazimo neposjeceni cvor sa najmanjom distancom */
        int u = -1;
        for(int i = 0; i < n; i++) {
            if(!posjecen[i] && dist[i] != INT_MAX)
                if(u == -1 || dist[i] < dist[u])
                    u = i;
        }
        if(u == -1) break;
        posjecen[u] = 1;

        /* prolazimo kroz listu susjeda cvora u */
        cvor* p = g->susjedi[u].prvi;
        while(p != NULL) {
            int v = p->ime;
            int w = p->tezina;
            if(!posjecen[v] && dist[u] != INT_MAX && dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                preth[v] = u;
            }
            p = p->sledeci;
        }
    }

    free(posjecen);
}

/* ================================================
   REKONSTRUKCIJA PUTA
   ================================================ */

int rekonstruisi_put(int* preth, int cilj, int* put) {
    int d = 0;
    for(int c = cilj; c != -1; c = preth[c])
        put[d++] = c;

    /* obrnemo niz */
    for(int l = 0, r = d - 1; l < r; l++, r--) {
        int tmp = put[l];
        put[l] = put[r];
        put[r] = tmp;
    }
    return d;
}

/* ================================================
   DIO 2: NAJKRACI PUT A -> B
   ================================================ */

void najkraci_put(graf* g) {
    int n = g->broj_cvorova;

    int prikazi = n < 40 ? n : 40;
    printf("\nlokacije (prvih %d od %d):\n", prikazi, n);
    for(int i = 0; i < prikazi; i++)
        printf("  [%3d] %s\n", i + 1, g->ime_cvora[i]);
    if(n > 40)
        printf("  ... unesite broj direktno (1-%d)\n", n);

    int start, cilj;
    printf("\npocetna lokacija (1-%d): ", n); scanf("%d", &start); start--;
    printf("krajnja lokacija  (1-%d): ", n); scanf("%d", &cilj);  cilj--;

    if(start < 0 || start >= n || cilj < 0 || cilj >= n) {
        printf("nevazeci unos!\n");
        return;
    }
    if(start == cilj) {
        printf("vec ste na odredistu: %s\n", g->ime_cvora[start]);
        return;
    }

    int* dist = (int*)malloc(n * sizeof(int));
    int* preth = (int*)malloc(n * sizeof(int));
    int* put   = (int*)malloc(n * sizeof(int));

    clock_t t1 = clock();
    dijkstra(g, start, dist, preth);
    clock_t t2 = clock();

    printf("\n--- rezultat ---\n");
    if(dist[cilj] == INT_MAX) {
        printf("ne postoji put od %s do %s!\n", g->ime_cvora[start], g->ime_cvora[cilj]);
    } else {
        int d = rekonstruisi_put(preth, cilj, put);
        printf("najkraci put: ");
        for(int i = 0; i < d; i++) {
            printf("%s", g->ime_cvora[put[i]]);
            if(i < d - 1) printf(" -> ");
        }
        printf("\nukupna udaljenost : %d\n", dist[cilj]);
        printf("broj koraka       : %d\n", d - 1);
    }
    printf("vrijeme (dijkstra) : %.4f ms\n", 1000.0 * (t2 - t1) / CLOCKS_PER_SEC);

    free(dist);
    free(preth);
    free(put);
}

/* ================================================
   DIO 2: NAJKRACI PUT S MEDUSTANICAMA
   ================================================ */

void najkraci_put_medustanice(graf* g) {
    int n = g->broj_cvorova;

    int prikazi = n < 40 ? n : 40;
    printf("\nlokacije (prvih %d od %d):\n", prikazi, n);
    for(int i = 0; i < prikazi; i++)
        printf("  [%3d] %s\n", i + 1, g->ime_cvora[i]);
    if(n > 40)
        printf("  ... unesite broj direktno (1-%d)\n", n);

    int start, cilj;
    printf("\npocetna lokacija (1-%d): ", n); scanf("%d", &start); start--;
    printf("krajnja lokacija  (1-%d): ", n); scanf("%d", &cilj);  cilj--;

    if(start < 0 || start >= n || cilj < 0 || cilj >= n) {
        printf("nevazeci unos!\n");
        return;
    }

    int bm;
    printf("koliko obaveznih medustanica? (0-10): ");
    scanf("%d", &bm);
    if(bm < 0 || bm > 10) {
        printf("nevazeci unos! broj medustanica mora biti izmedju 0 i 10.\n");
        return;
    }

    int medustanice[10];
    for(int i = 0; i < bm; i++) {
        printf("  medustanica %d (1-%d): ", i + 1, n);
        scanf("%d", &medustanice[i]);
        medustanice[i]--;
        if(medustanice[i] < 0 || medustanice[i] >= n) {
            printf("nevazeci unos!\n");
            return;
        }
    }

    /* niz svih tacaka: start -> ms0 -> ms1 -> ... -> cilj */
    int tocke[12], bt = 0;
    tocke[bt++] = start;
    for(int i = 0; i < bm; i++) tocke[bt++] = medustanice[i];
    tocke[bt++] = cilj;

    int* dist = (int*)malloc(n * sizeof(int));
    int* preth = (int*)malloc(n * sizeof(int));
    int* put   = (int*)malloc(n * sizeof(int));

    /* svaki segment cuvamo u posebnom nizu kako bismo
       prvo provjerili postoji li cijeli put, pa tek onda ispisali */
    int* svi_putevi  = (int*)malloc(bt * n * sizeof(int));
    int* duzine      = (int*)malloc(bt * sizeof(int));
    int* udaljenosti = (int*)malloc(bt * sizeof(int));

    int ukupno = 0;
    int postoji = 1;

    clock_t t1 = clock();

    for(int seg = 0; seg < bt - 1; seg++) {
        int od  = tocke[seg];
        int do_ = tocke[seg + 1];

        dijkstra(g, od, dist, preth);

        if(dist[do_] == INT_MAX) {
            postoji = 0;
            break;
        }

        udaljenosti[seg] = dist[do_];
        ukupno += dist[do_];
        duzine[seg] = rekonstruisi_put(preth, do_, svi_putevi + seg * n);
    }

    clock_t t2 = clock();

    printf("\n--- rezultat ---\n");

    if(!postoji) {
        printf("ne postoji put koji prolazi kroz sve odabrane lokacije!\n");
    } else {
        printf("  [S] = pocetna    [M] = medustanica    [C] = cilj\n\n");
        printf("put:\n  ");

        for(int seg = 0; seg < bt - 1; seg++) {
            int* seg_put = svi_putevi + seg * n;
            int  d       = duzine[seg];
            int  poc     = (seg == 0) ? 0 : 1;

            for(int i = poc; i < d; i++) {
                int je_zadnji_u_segmentu = (i == d - 1);
                int je_zadnji_segment    = (seg == bt - 2);
                int je_medustanica       = je_zadnji_u_segmentu && !je_zadnji_segment;
                int je_cilj              = je_zadnji_u_segmentu && je_zadnji_segment;
                int je_start             = (seg == 0 && i == 0);

                if(je_start)
                    printf("[S] %s", g->ime_cvora[seg_put[i]]);
                else if(je_medustanica)
                    printf("[M] %s [M]", g->ime_cvora[seg_put[i]]);
                else if(je_cilj)
                    printf("[C] %s", g->ime_cvora[seg_put[i]]);
                else
                    printf("%s", g->ime_cvora[seg_put[i]]);

                if(!je_cilj) printf(" -> ");
            }
        }
        printf("\nukupna udaljenost : %d\n", ukupno);
    }

    printf("vrijeme (dijkstra) : %.4f ms\n", 1000.0 * (t2 - t1) / CLOCKS_PER_SEC);

    free(svi_putevi);
    free(duzine);
    free(udaljenosti);
    free(dist);
    free(preth);
    free(put);
}

/* ================================================
   DIO 3: MJERENJE VREMENA
   - 5 mjerenja po grafu
   - odvojeno: bez medustanica i sa medustanicama
   ================================================ */

void mjeri_vremena(graf* mape[9]) {
    printf("\n================================================\n");
    printf("  MJERENJE VREMENA IZVRSAVANJA\n");
    printf("================================================\n\n");

    srand(42);

    /* --- bez medustanica --- */
    printf("BEZ MEDUSTANICA (prosjek 5 mjerenja):\n");
    printf("%-35s | cvorovi | gust. | prosjek (ms)\n", "graf");
    printf("----------------------------------------------------------\n");

    for(int g = 0; g < 9; g++) {
        graf* gr = mape[g];
        int* dist  = (int*)malloc(gr->broj_cvorova * sizeof(int));
        int* preth = (int*)malloc(gr->broj_cvorova * sizeof(int));
        double ukupno_ms = 0.0;

        for(int m = 0; m < 5; m++) {
            int src = rand() % gr->broj_cvorova;
            clock_t t1 = clock();
            dijkstra(gr, src, dist, preth);
            clock_t t2 = clock();
            ukupno_ms += 1000.0 * (t2 - t1) / CLOCKS_PER_SEC;
        }

        printf("%-35s | %7d | %5.1f | %.4f ms\n",
               gr->naziv, gr->broj_cvorova, gr->gustina, ukupno_ms / 5.0);

        free(dist);
        free(preth);
    }

    /* --- sa medustanicama (1 medustanica) --- */
    printf("\nSA MEDUSTANICAMA (1 medustanica, prosjek 5 mjerenja):\n");
    printf("%-35s | cvorovi | gust. | prosjek (ms)\n", "graf");
    printf("----------------------------------------------------------\n");

    for(int g = 0; g < 9; g++) {
        graf* gr = mape[g];
        int* dist  = (int*)malloc(gr->broj_cvorova * sizeof(int));
        int* preth = (int*)malloc(gr->broj_cvorova * sizeof(int));
        int* put   = (int*)malloc(gr->broj_cvorova * sizeof(int));
        double ukupno_ms = 0.0;

        for(int m = 0; m < 5; m++) {
            int src  = rand() % gr->broj_cvorova;
            int med  = rand() % gr->broj_cvorova;
            int dst  = rand() % gr->broj_cvorova;

            clock_t t1 = clock();

            /* src -> med */
            dijkstra(gr, src, dist, preth);
            /* med -> dst */
            dijkstra(gr, med, dist, preth);

            clock_t t2 = clock();
            ukupno_ms += 1000.0 * (t2 - t1) / CLOCKS_PER_SEC;
        }

        printf("%-35s | %7d | %5.1f | %.4f ms\n",
               gr->naziv, gr->broj_cvorova, gr->gustina, ukupno_ms / 5.0);

        free(dist);
        free(preth);
        free(put);
    }

    printf("----------------------------------------------------------\n");
}

/* ================================================
   INICIJALIZACIJA 9 MAPA
   Grupa 1: do 1000 cvorova,      gustine 0.3 / 0.6 / 0.8
   Grupa 2: 1000-5000 cvorova,    gustine 0.3 / 0.6 / 0.8
   Grupa 3: 5000-10000 cvorova,   gustine 0.3 / 0.6 / 0.8
   Napomena: za gustinu 0.8 u grupi 3, broj cvorova je
   smanjen na 2000 jer bi 8000+ cvorova s gustinom 0.8
   generisalo desetine miliona grana sto nije izvodljivo.
   ================================================ */

void inicijalizuj_mape(graf* mape[9]) {
    /* Grupa 1: do 1000 cvorova, gustine 0.3 / 0.6 / 0.8 */
    printf("  [1/9] pabovi u beogradu...\n");
    mape[0] = kreiraj_graf("Pabovi u Beogradu",    "Pab",   800, 0.3f);
    generisi_grane(mape[0]);

    printf("  [2/9] kafici u banjoj luci...\n");
    mape[1] = kreiraj_graf("Kafici u Banjoj Luci", "Kafic", 500, 0.6f);
    generisi_grane(mape[1]);

    printf("  [3/9] restorani u sarajevu...\n");
    mape[2] = kreiraj_graf("Restorani u Sarajevu", "Rest",  120, 0.8f);
    generisi_grane(mape[2]);

    /* Grupa 2: 1000-5000 cvorova, gustine 0.3 / 0.6 / 0.8 */
    printf("  [4/9] pabovi u zagrebu...\n");
    mape[3] = kreiraj_graf("Pabovi u Zagrebu",      "Pab",  2800, 0.3f);
    generisi_grane(mape[3]);

    printf("  [5/9] restorani u splitu...\n");
    mape[4] = kreiraj_graf("Restorani u Splitu",   "Rest", 3800, 0.6f);
    generisi_grane(mape[4]);

    printf("  [6/9] kafici u splitu...\n");
    mape[5] = kreiraj_graf("Kafici u Splitu",      "Kafic",4500, 0.8f);
    generisi_grane(mape[5]);

    /* Grupa 3: 5000-10000 cvorova, gustine 0.3 / 0.6 / 0.8
       Napomena: broj grana je ogranicen na n*15 radi
       razumnog vremena generisanja i memorije.            */
    printf("  [7/9] kafici u skoplju...\n");
    mape[6] = kreiraj_graf("Kafici u Skoplju",     "Kafic",9400, 0.3f);
    generisi_grane(mape[6]);

    printf("  [8/9] restorani u becu...\n");
    mape[7] = kreiraj_graf("Restorani u Becu",     "Rest", 6500, 0.6f);
    generisi_grane(mape[7]);

    printf("  [9/9] pabovi u podgorici...\n");
    mape[8] = kreiraj_graf("Pabovi u Podgorici",   "Pab",  8300, 0.8f);
    generisi_grane(mape[8]);
}

/* ================================================
   MAIN
   ================================================ */

int main() {
    printf("=== osnove programiranja 2 - projektni zadatak ===\n");
    printf("ucitavanje mapa...\n\n");

    /* niz pokazivaca - sve na heapu, nema stack overflow */
    graf* mape[9];
    inicijalizuj_mape(mape);

    printf("\nsve mape ucitane!\n");

    char unos[10];
    while(1) {
        printf("\n================================\n");
        printf("      sistem za navigaciju\n");
        printf("================================\n");
        printf("odaberite mapu:\n");
        for(int i = 0; i < 9; i++)
            printf("  [%d] %-35s (%5d lok., gustina %.1f)\n",
                   i + 1, mape[i]->naziv, mape[i]->broj_cvorova, mape[i]->gustina);
        printf("  [m] mjerenje vremena izvrsavanja\n");
        printf("  [0] izlaz\n");
        printf("vas izbor: ");
        scanf("%s", unos);

        if(unos[0] == '0') break;

        if(unos[0] == 'm' || unos[0] == 'M') {
            mjeri_vremena(mape);
            continue;
        }

        int izbor = atoi(unos);
        if(izbor < 1 || izbor > 9) {
            printf("nevazeci izbor!\n");
            continue;
        }

        graf* g = mape[izbor - 1];

        printf("\n--- %s ---\n", g->naziv);
        printf("  [1] najkraci put od a do b\n");
        printf("  [2] najkraci put s medustanicama\n");
        printf("  [3] povratak\n");
        printf("vas izbor: ");
        int op;
        scanf("%d", &op);

        switch(op) {
            case 1: najkraci_put(g);            break;
            case 2: najkraci_put_medustanice(g); break;
            case 3: break;
            default: printf("nevazeci izbor!\n");
        }
    }

    for(int i = 0; i < 9; i++)
        obrisi_graf(mape[i]);

    printf("dovidjenja!\n");
    return 0;
}
