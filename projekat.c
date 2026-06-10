#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>

typedef struct cvor cvor;

struct cvor {
    int indeks_susjeda;
    int tezina_grane;
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
    char** ime_cvora;
    lista_susjeda* susjedi;
} graf;

cvor* kreiraj_cvor(int indeks_susjeda, int tezina_grane) {
    cvor* novi_cvor = (cvor*)malloc(sizeof(cvor));
    if(novi_cvor == NULL) {
        printf("neuspjesno kreiranje cvora!\n");
        exit(1);
    }
    novi_cvor->indeks_susjeda = indeks_susjeda;
    novi_cvor->tezina_grane = tezina_grane;
    novi_cvor->sledeci = NULL;
    return novi_cvor;
}

void dodaj_na_kraj(cvor** lista, cvor* novi_cvor) {
    if(*lista == NULL) {
        *lista = novi_cvor;
    } else {
        cvor* trenutni = *lista;
        while(trenutni->sledeci != NULL)
            trenutni = trenutni->sledeci;
        trenutni->sledeci = novi_cvor;
    }
}

int sadrzi_element(cvor* lista, int trazeni_indeks) {
    while(lista != NULL) {
        if(lista->indeks_susjeda == trazeni_indeks)
            return 1;
        lista = lista->sledeci;
    }
    return 0;
}

void obrisi_listu(cvor* lista) {
    cvor* pomocni;
    while(lista != NULL) {
        pomocni = lista;
        lista = lista->sledeci;
        free(pomocni);
    }
}

graf* kreiraj_graf(const char* naziv, const char* prefiks, int broj_cvorova, float gustina) {
    graf* novi_graf = (graf*)malloc(sizeof(graf));
    if(novi_graf == NULL) {
        printf("neuspjesno kreiranje grafa!\n");
        exit(1);
    }

    strncpy(novi_graf->naziv, naziv, 59);
    strncpy(novi_graf->prefiks, prefiks, 19);
    novi_graf->broj_cvorova = broj_cvorova;
    novi_graf->gustina = gustina;

    novi_graf->susjedi = (lista_susjeda*)malloc(broj_cvorova * sizeof(lista_susjeda));
    if(novi_graf->susjedi == NULL) {
        printf("neuspjesna alokacija!\n");
        exit(1);
    }
    for(int i = 0; i < broj_cvorova; i++)
        novi_graf->susjedi[i].prvi = NULL;

    novi_graf->ime_cvora = (char**)malloc(broj_cvorova * sizeof(char*));
    if(novi_graf->ime_cvora == NULL) {
        printf("neuspjesna alokacija!\n");
        exit(1);
    }
    for(int i = 0; i < broj_cvorova; i++) {
        novi_graf->ime_cvora[i] = (char*)malloc(30 * sizeof(char));
        if(broj_cvorova <= 26)
            sprintf(novi_graf->ime_cvora[i], "%s %c", prefiks, 'A' + i);
        else
            sprintf(novi_graf->ime_cvora[i], "%s %d", prefiks, i + 1);
    }

    return novi_graf;
}

void dodaj_granu(graf* g, int polazni_cvor, int odredisni_cvor, int tezina_grane) {
    if(polazni_cvor < 0 || odredisni_cvor < 0 || polazni_cvor >= g->broj_cvorova || odredisni_cvor >= g->broj_cvorova)
        return;
    if(polazni_cvor == odredisni_cvor)
        return;
    if(sadrzi_element(g->susjedi[polazni_cvor].prvi, odredisni_cvor))
        return;

    dodaj_na_kraj(&(g->susjedi[polazni_cvor].prvi), kreiraj_cvor(odredisni_cvor, tezina_grane));
    dodaj_na_kraj(&(g->susjedi[odredisni_cvor].prvi), kreiraj_cvor(polazni_cvor, tezina_grane));
}

void generisi_grane(graf* g) {
    int broj_cvorova = g->broj_cvorova;

    srand((unsigned)(g->naziv[0] * 31u + broj_cvorova + (unsigned)(g->gustina * 100)));

    for(int i = 0; i < broj_cvorova - 1; i++)
        dodaj_granu(g, i, i + 1, rand() % 50 + 1);

    long long ciljani_broj_grana = (long long)(g->gustina * broj_cvorova * (broj_cvorova - 1) / 2);
    if(ciljani_broj_grana > (long long)broj_cvorova * 15)
        ciljani_broj_grana = (long long)broj_cvorova * 15;

    long long broj_dodatih_grana = broj_cvorova - 1;
    long long broj_pokusaja = 0;
    long long maksimalni_pokusaji = ciljani_broj_grana * 20;

    while(broj_dodatih_grana < ciljani_broj_grana && broj_pokusaja < maksimalni_pokusaji) {
        int polazni_cvor = rand() % broj_cvorova;
        int odredisni_cvor = rand() % broj_cvorova;
        broj_pokusaja++;
        if(polazni_cvor == odredisni_cvor) continue;
        if(!sadrzi_element(g->susjedi[polazni_cvor].prvi, odredisni_cvor)) {
            dodaj_granu(g, polazni_cvor, odredisni_cvor, rand() % 50 + 1);
            broj_dodatih_grana++;
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

void dijkstra(graf* g, int polazni_cvor, int* udaljenosti, int* prethodni) {
    int broj_cvorova = g->broj_cvorova;
    int* posjecen = (int*)calloc(broj_cvorova, sizeof(int));

    for(int i = 0; i < broj_cvorova; i++) {
        udaljenosti[i] = INT_MAX;
        prethodni[i] = -1;
    }
    udaljenosti[polazni_cvor] = 0;

    for(int i = 0; i < broj_cvorova; i++) {
        int trenutni_cvor = -1;
        for(int j = 0; j < broj_cvorova; j++) {
            if(!posjecen[j] && udaljenosti[j] != INT_MAX)
                if(trenutni_cvor == -1 || udaljenosti[j] < udaljenosti[trenutni_cvor])
                    trenutni_cvor = j;
        }
        if(trenutni_cvor == -1) break;
        posjecen[trenutni_cvor] = 1;

        cvor* susjedni_cvor = g->susjedi[trenutni_cvor].prvi;
        while(susjedni_cvor != NULL) {
            int indeks_susjeda = susjedni_cvor->indeks_susjeda;
            int tezina_grane = susjedni_cvor->tezina_grane;
            if(!posjecen[indeks_susjeda] && udaljenosti[trenutni_cvor] != INT_MAX && udaljenosti[trenutni_cvor] + tezina_grane < udaljenosti[indeks_susjeda]) {
                udaljenosti[indeks_susjeda] = udaljenosti[trenutni_cvor] + tezina_grane;
                prethodni[indeks_susjeda] = trenutni_cvor;
            }
            susjedni_cvor = susjedni_cvor->sledeci;
        }
    }

    free(posjecen);
}

int rekonstruisi_put(int* prethodni, int odredisni_cvor, int* put) {
    int duzina_puta = 0;
    for(int trenutni = odredisni_cvor; trenutni != -1; trenutni = prethodni[trenutni])
        put[duzina_puta++] = trenutni;

    for(int i = 0, j = duzina_puta - 1; i < j; i++, j--) {
        int pomocna_vrijednost = put[i];
        put[i] = put[j];
        put[j] = pomocna_vrijednost;
    }
    return duzina_puta;
}

void najkraci_put(graf* g) {
    int broj_cvorova = g->broj_cvorova;

    int broj_za_prikaz = broj_cvorova < 40 ? broj_cvorova : 40;
    printf("\nlokacije (prvih %d od %d):\n", broj_za_prikaz, broj_cvorova);
    for(int i = 0; i < broj_za_prikaz; i++)
        printf("  [%3d] %s\n", i + 1, g->ime_cvora[i]);
    if(broj_cvorova > 40)
        printf("  ... unesite broj direktno (1-%d)\n", broj_cvorova);

    int polazna_lokacija, odredisna_lokacija;
    printf("\npocetna lokacija (1-%d): ", broj_cvorova); scanf("%d", &polazna_lokacija); polazna_lokacija--;
    printf("krajnja lokacija  (1-%d): ", broj_cvorova); scanf("%d", &odredisna_lokacija);  odredisna_lokacija--;

    if(polazna_lokacija < 0 || polazna_lokacija >= broj_cvorova || odredisna_lokacija < 0 || odredisna_lokacija >= broj_cvorova) {
        printf("nevazeci unos!\n");
        return;
    }
    if(polazna_lokacija == odredisna_lokacija) {
        printf("vec ste na odredistu: %s\n", g->ime_cvora[polazna_lokacija]);
        return;
    }

    int* udaljenosti = (int*)malloc(broj_cvorova * sizeof(int));
    int* prethodni  = (int*)malloc(broj_cvorova * sizeof(int));
    int* put = (int*)malloc(broj_cvorova * sizeof(int));

    clock_t pocetak_mjerenja = clock();
    dijkstra(g, polazna_lokacija, udaljenosti, prethodni);
    clock_t kraj_mjerenja = clock();

    printf("\n--- rezultat ---\n");
    if(udaljenosti[odredisna_lokacija] == INT_MAX) {
        printf("ne postoji put od %s do %s!\n", g->ime_cvora[polazna_lokacija], g->ime_cvora[odredisna_lokacija]);
    } else {
        int duzina_puta = rekonstruisi_put(prethodni, odredisna_lokacija, put);
        printf("najkraci put: ");
        for(int i = 0; i < duzina_puta; i++) {
            printf("%s", g->ime_cvora[put[i]]);
            if(i < duzina_puta - 1) printf(" -> ");
        }
        printf("\nukupna udaljenost : %d\n", udaljenosti[odredisna_lokacija]);
        printf("broj koraka : %d\n", duzina_puta - 1);
    }
    printf("vrijeme (dijkstra) : %.4f ms\n", 1000.0 * (kraj_mjerenja - pocetak_mjerenja) / CLOCKS_PER_SEC);

    free(udaljenosti);
    free(prethodni);
    free(put);
}

void najkraci_put_medjustanice(graf* g) {
    int broj_cvorova = g->broj_cvorova;

    int broj_za_prikaz = broj_cvorova < 40 ? broj_cvorova : 40;
    printf("\nlokacije (prvih %d od %d):\n", broj_za_prikaz, broj_cvorova);
    for(int i = 0; i < broj_za_prikaz; i++)
        printf("  [%3d] %s\n", i + 1, g->ime_cvora[i]);
    if(broj_cvorova > 40)
        printf("  ... unesite broj direktno (1-%d)\n", broj_cvorova);

    int polazna_lokacija, odredisna_lokacija;
    printf("\npocetna lokacija (1-%d): ", broj_cvorova); scanf("%d", &polazna_lokacija); polazna_lokacija--;
    printf("krajnja lokacija  (1-%d): ", broj_cvorova); scanf("%d", &odredisna_lokacija);  odredisna_lokacija--;

    if(polazna_lokacija < 0 || polazna_lokacija >= broj_cvorova || odredisna_lokacija < 0 || odredisna_lokacija >= broj_cvorova) {
        printf("nevazeci unos!\n");
        return;
    }

    int broj_medjustanica;
    printf("koliko obaveznih medjustanica? (0-10): ");
    scanf("%d", &broj_medjustanica);
    if(broj_medjustanica < 0 || broj_medjustanica > 10) {
        printf("nevazeci unos! broj medjustanica mora biti izmedju 0 i 10.\n");
        return;
    }

    int medjustanice[10];
    for(int i = 0; i < broj_medjustanica; i++) {
        printf("  medjustanica %d (1-%d): ", i + 1, broj_cvorova);
        scanf("%d", &medjustanice[i]);
        medjustanice[i]--;
        if(medjustanice[i] < 0 || medjustanice[i] >= broj_cvorova) {
            printf("nevazeci unos!\n");
            return;
        }
    }

    int sve_tacke[12], broj_tacaka = 0;
    sve_tacke[broj_tacaka++] = polazna_lokacija;
    for(int i = 0; i < broj_medjustanica; i++) sve_tacke[broj_tacaka++] = medjustanice[i];
    sve_tacke[broj_tacaka++] = odredisna_lokacija;

    int* udaljenosti = (int*)malloc(broj_cvorova * sizeof(int));
    int* prethodni = (int*)malloc(broj_cvorova * sizeof(int));
    int* put = (int*)malloc(broj_cvorova * sizeof(int));

    int* svi_putevi = (int*)malloc(broj_tacaka * broj_cvorova * sizeof(int));
    int* duzine_segmenata = (int*)malloc(broj_tacaka * sizeof(int));
    int* udaljenosti_segmenata = (int*)malloc(broj_tacaka * sizeof(int));

    int ukupna_udaljenost = 0;
    int put_postoji = 1;

    clock_t pocetak_mjerenja = clock();

    for(int i = 0; i < broj_tacaka - 1; i++) {
        int od = sve_tacke[i];
        int do_ = sve_tacke[i + 1];

        dijkstra(g, od, udaljenosti, prethodni);

        if(udaljenosti[do_] == INT_MAX) {
            put_postoji = 0;
            break;
        }

        udaljenosti_segmenata[i] = udaljenosti[do_];
        ukupna_udaljenost += udaljenosti[do_];
        duzine_segmenata[i] = rekonstruisi_put(prethodni, do_, svi_putevi + i * broj_cvorova);
    }

    clock_t kraj_mjerenja = clock();

    printf("\n--- rezultat ---\n");

    if(!put_postoji) {
        printf("ne postoji put koji prolazi kroz sve odabrane lokacije!\n");
    } else {
        printf("  [S] = pocetna    [M] = medjustanica    [C] = cilj\n\n");
        printf("put:\n  ");

        for(int i = 0; i < broj_tacaka - 1; i++) {
            int* trenutni_segment = svi_putevi + i * broj_cvorova;
            int  duzina_segmenta = duzine_segmenata[i];
            int  pocetak_ispisa = (i == 0) ? 0 : 1;

            for(int j = pocetak_ispisa; j < duzina_segmenta; j++) {
                int je_zadnji_u_segmentu = (j == duzina_segmenta - 1);
                int je_zadnji_segment = (i == broj_tacaka - 2);
                int je_medjustanica = je_zadnji_u_segmentu && !je_zadnji_segment;
                int je_cilj = je_zadnji_u_segmentu && je_zadnji_segment;
                int je_start = (i == 0 && j == 0);

                if(je_start)
                    printf("[S] %s", g->ime_cvora[trenutni_segment[j]]);
                else if(je_medjustanica)
                    printf("[M] %s [M]", g->ime_cvora[trenutni_segment[j]]);
                else if(je_cilj)
                    printf("[C] %s", g->ime_cvora[trenutni_segment[j]]);
                else
                    printf("%s", g->ime_cvora[trenutni_segment[j]]);

                if(!je_cilj) printf(" -> ");
            }
        }
        printf("\nukupna udaljenost : %d\n", ukupna_udaljenost);
    }

    printf("vrijeme (dijkstra) : %.4f ms\n", 1000.0 * (kraj_mjerenja - pocetak_mjerenja) / CLOCKS_PER_SEC);

    free(svi_putevi);
    free(duzine_segmenata);
    free(udaljenosti_segmenata);
    free(udaljenosti);
    free(prethodni);
    free(put);
}

void mjeri_vremena(graf* mape[9]) {
    printf("\n================================================\n");
    printf("  MJERENJE VREMENA IZVRSAVANJA\n");
    printf("================================================\n\n");

    srand(42);

    printf("BEZ MEDJUSTANICA (prosjek 5 mjerenja):\n");
    printf("%-35s | cvorovi | gust. | prosjek (ms)\n", "graf");
    printf("----------------------------------------------------------\n");

    for(int i = 0; i < 9; i++) {
        graf* trenutni_graf = mape[i];
        int* udaljenosti = (int*)malloc(trenutni_graf->broj_cvorova * sizeof(int));
        int* prethodni   = (int*)malloc(trenutni_graf->broj_cvorova * sizeof(int));
        double ukupno_milisekundi = 0.0;

        for(int j = 0; j < 5; j++) {
            int nasumicni_polazni = rand() % trenutni_graf->broj_cvorova;
            clock_t pocetak_mjerenja = clock();
            dijkstra(trenutni_graf, nasumicni_polazni, udaljenosti, prethodni);
            clock_t kraj_mjerenja = clock();
            ukupno_milisekundi += 1000.0 * (kraj_mjerenja - pocetak_mjerenja) / CLOCKS_PER_SEC;
        }

        printf("%-35s | %7d | %5.1f | %.4f ms\n",
               trenutni_graf->naziv, trenutni_graf->broj_cvorova, trenutni_graf->gustina, ukupno_milisekundi / 5.0);

        free(udaljenosti);
        free(prethodni);
    }

    printf("\nSA MEDJUSTANICAMA (1 medjustanica, prosjek 5 mjerenja):\n");
    printf("%-35s | cvorovi | gust. | prosjek (ms)\n", "graf");
    printf("----------------------------------------------------------\n");

    for(int i = 0; i < 9; i++) {
        graf* trenutni_graf = mape[i];
        int* udaljenosti = (int*)malloc(trenutni_graf->broj_cvorova * sizeof(int));
        int* prethodni = (int*)malloc(trenutni_graf->broj_cvorova * sizeof(int));
        int* put = (int*)malloc(trenutni_graf->broj_cvorova * sizeof(int));
        double ukupno_milisekundi = 0.0;

        for(int j = 0; j < 5; j++) {
            int nasumicni_polazni = rand() % trenutni_graf->broj_cvorova;
            int nasumicna_medjustanica = rand() % trenutni_graf->broj_cvorova;
            int nasumicno_odrediste  = rand() % trenutni_graf->broj_cvorova;

            clock_t pocetak_mjerenja = clock();
            dijkstra(trenutni_graf, nasumicni_polazni, udaljenosti, prethodni);
            dijkstra(trenutni_graf, nasumicna_medjustanica, udaljenosti, prethodni);
            clock_t kraj_mjerenja = clock();
            ukupno_milisekundi += 1000.0 * (kraj_mjerenja - pocetak_mjerenja) / CLOCKS_PER_SEC;
        }

        printf("%-35s | %7d | %5.1f | %.4f ms\n",
               trenutni_graf->naziv, trenutni_graf->broj_cvorova, trenutni_graf->gustina, ukupno_milisekundi / 5.0);

        free(udaljenosti);
        free(prethodni);
        free(put);
    }

    printf("----------------------------------------------------------\n");
}

void inicijalizuj_mape(graf* mape[9]) {
    printf("  [1/9] pabovi u beogradu...\n");
    mape[0] = kreiraj_graf("Pabovi u Beogradu","Pub", 800, 0.3f);
    generisi_grane(mape[0]);

    printf("  [2/9] kafici u banjoj luci...\n");
    mape[1] = kreiraj_graf("Kafici u Banjoj Luci", "Kafic",500, 0.6f);
    generisi_grane(mape[1]);

    printf("  [3/9] restorani u sarajevu...\n");
    mape[2] = kreiraj_graf("Restorani u Sarajevu", "Restoran", 120, 0.8f);
    generisi_grane(mape[2]);

    printf("  [4/9] pabovi u zagrebu...\n");
    mape[3] = kreiraj_graf("Pabovi u Zagrebu","Pub", 2800, 0.3f);
    generisi_grane(mape[3]);

    printf("  [5/9] restorani u splitu...\n");
    mape[4] = kreiraj_graf("Restorani u Splitu","Restoran",3800, 0.6f);
    generisi_grane(mape[4]);

    printf("  [6/9] kafici u splitu...\n");
    mape[5] = kreiraj_graf("Kafici u Splitu", "Kafic",  4500, 0.8f);
    generisi_grane(mape[5]);

    printf("  [7/9] kafici u skoplju...\n");
    mape[6] = kreiraj_graf("Kafici u Skoplju", "Kafic",  9400, 0.3f);
    generisi_grane(mape[6]);

    printf("  [8/9] restorani u becu...\n");
    mape[7] = kreiraj_graf("Restorani u Becu", "Restoran",6500, 0.6f);
    generisi_grane(mape[7]);

    printf("  [9/9] pabovi u podgorici...\n");
    mape[8] = kreiraj_graf("Pabovi u Podgorici", "Pub", 8300, 0.8f);
    generisi_grane(mape[8]);
}

int main() {
    printf("=== osnove programiranja 2 - projektni zadatak ===\n");
    printf("ucitavanje mapa...\n\n");

    graf* mape[9];
    inicijalizuj_mape(mape);

    printf("\nsve mape ucitane!\n");

    char korisnicki_unos[10];
    while(1) {
        printf("\n================================\n");
        printf("      sistem za navigaciju\n");
        printf("================================\n");
        printf("odaberite mapu:\n");
        for(int i = 0; i < 9; i++)
            printf("  [%d] %-35s ( %5d lokacija | gustina : %.1f) \n",
                   i + 1, mape[i]->naziv, mape[i]->broj_cvorova, mape[i]->gustina);
        printf("  [m] mjerenje vremena izvrsavanja\n");
        printf("  [0] izlaz\n");
        printf("vas izbor: ");
        scanf("%s", korisnicki_unos);

        if(korisnicki_unos[0] == '0') break;

        if(korisnicki_unos[0] == 'm' || korisnicki_unos[0] == 'M') {
            mjeri_vremena(mape);
            continue;
        }

        int odabrana_mapa = atoi(korisnicki_unos);
        if(odabrana_mapa < 1 || odabrana_mapa > 9) {
            printf("nevazeci izbor!\n");
            continue;
        }

        graf* odabrani_graf = mape[odabrana_mapa - 1];

        printf("\n--- %s ---\n", odabrani_graf->naziv);
        printf("  [1] najkraci put od a do b\n");
        printf("  [2] najkraci put s medjustanicama\n");
        printf("  [3] povratak\n");
        printf("vas izbor: ");
        int odabrana_opcija;
        scanf("%d", &odabrana_opcija);

        switch(odabrana_opcija) {
            case 1: najkraci_put(odabrani_graf);            break;
            case 2: najkraci_put_medjustanice(odabrani_graf); break;
            case 3: break;
            default: printf("nevazeci izbor!\n");
        }
    }

    for(int i = 0; i < 9; i++)
        obrisi_graf(mape[i]);

    printf("dovidjenja!\n");
    return 0;
}
