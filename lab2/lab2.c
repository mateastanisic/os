#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>


#define NAJVECI_BROJ 0xffffffffffffffffULL
#define A 8531648002905263869ULL
#define B 18116652324302926351ULL

typedef unsigned long long int ulli;

bool first_time = true;
/*jer se static varijabla u c-u ne može inicijalizirati s nekom ne-konstantnom vrijednošću
pa nam treba info generiramo li prvi puta neki random broj ( rnd = time(NULL) ) ili ne
( rnd = ( rnd*A )%B )*/
ulli rnd; //static varijabla
ulli MS[5]; //međuspremnik
int U = 0; //izlaz index, ulaz index inicijalizirani na 0
char* kraj = "NIJE_KRAJ";


//za lampartov algoritam
//5 radnih dretvi i 5 dretvi za provjeru
int ULAZ_r[5] = {0};
int ULAZ_p[5] = {0};
int BROJ_r[5] = {0};
int BROJ_p[5] = {0};


int max( int* array );
void udi_u_KO( int I );
void izadi_iz_KO( int I );

void *radna_dretva( void *id );
void *dretva_provjera( void *id );

//funkcije iz lab1.c
ulli random_64bit();
bool test_bitovi( ulli broj );
bool test_pseudo_prost( ulli broj );
ulli generiraj_broj();


int main( void ) {
    srand( time( NULL ) );
    pthread_t thr_id[10]; //dretve
    int BR[10];

    //radne dretve
    for( int i=0; i<5; i++ ) {
        BR[i] = i;
        if (pthread_create(&thr_id[i], NULL, radna_dretva, &BR[i]) != 0) {
            printf ("Greska pri stvaranju dretve!\n");
            exit (1);
        }
    }
    sleep(2);

    //dretve provjere
    for( int i=5; i<10; i++ ) {
        BR[i] = i-5;
        if (pthread_create (&thr_id[i], NULL, dretva_provjera, &BR[i]) != 0) {
            printf ("Greska pri stvaranju dretve!\n");
            exit (1);
        }
    }
    sleep(30);
    kraj = "KRAJ_RADA";

    for( int i=0; i<10; i++ ) {
        pthread_join(thr_id[i], NULL );
    }

    return 0;
}



//lamportov algoritam
int max( int* array ) {
    int maxvalue;
    if( array != NULL ) maxvalue = array[0];
    size_t n = sizeof(array)/sizeof(array[0]);

    for( int i=1; i<n; i++ ) {
        if( array[i]>maxvalue ) maxvalue = array[i];
    }
    return maxvalue;
}

void udi_u_KO( int I ) {
    //uzimamo broj
    if( I<5 ) { //radne dretve
        ULAZ_r[I] = 1;
        BROJ_r[I] = max( BROJ_r ) + 1 ;
        ULAZ_r[I] = 0;

        //provjera i čekanje na dretve s manjim brojem
        for( int J=0; J<5; J++ ) {
            while( ULAZ_r[J] == 1 ) ; //cekaj dok J dobije broj
            //cekaj dok J ima prednost
            while( BROJ_r[J]!=0 && ( BROJ_r[J]<BROJ_r[I] || ( BROJ_r[J]==BROJ_r[I] && J<I ) ));

        }
    }
    else { //dretva provjere
        ULAZ_p[I] = 1;
        BROJ_p[I] = max( BROJ_p ) + 1 ;
        ULAZ_p[I] = 0;

        //provjera i čekanje na dretve s manjim brojem
        for( int J=0; J<5; J++ ) {
            while( ULAZ_p[J] == 1 ) ; //cekaj dok J dobije broj
            //cekaj dok J ima prednost
            while( BROJ_p[J]!=0 && ( BROJ_p[J]<BROJ_p[I] || ( BROJ_p[J]==BROJ_p[I] && J<I ) ));

        }
    }

}

void izadi_iz_KO( int I ) {
    if( I < 5 ) BROJ_r[I] = 0; //radna dretva
    else BROJ_p[I] = 0; //dretva provjere
}

void *radna_dretva( void *id ) {
    int *n = id;

    do {
        ulli x = generiraj_broj();
        udi_u_KO( *n );
        MS[U] = x;
        U = ( U + 1 ) % 5;
        izadi_iz_KO( *n );

    } while( strcmp(kraj,"KRAJ_RADA") );

    return NULL;
}

void *dretva_provjera( void *id ) {
    int *n = id;

    do {
        udi_u_KO( *n );
        ulli y = MS[0];
        printf("ID dretve: %d, dohvatio broj: %llu \n", *n, y );
        izadi_iz_KO( *n );
        sleep( y%5 );
        printf("ID dretve: %d, potrosio broj: %llu \n", *n, y );

    } while( strcmp(kraj,"KRAJ_RADA") );

    return NULL;
}


ulli generiraj_broj() {
    ulli x = random_64bit() | 1ULL; //da bude neparan

    int br = 0, br2 = 0; //brojači za zaustavljanje petlje i op. x+=2
    int limit = 50; //koliko puta ćemo dodavati 2

    while ( test_bitovi(x) == false || test_pseudo_prost(x) == false ) {
        if( x <= (NAJVECI_BROJ - 2ULL) && br < limit ) {
            x = x + 2ULL;
            br++;
        }
        else {
            x = random_64bit() | 1ULL; //da bude neparan
            br = 0;
        }
        br2++;
        if( br2 >= 3000000000 ) {
            break; //zaustavlja petlju - predugo tražimo
        }
    }
    if( br2 >= 3000000000 ) {
        br = 0;
        //opet ista stvar, samo ovdje ne gledamo uvjet test_bitovi(x)
        while ( test_pseudo_prost(x) == false ) {
            if ( x <= (NAJVECI_BROJ - 2ULL) && br < limit ) {
                x = x + 2ULL;
                br++;
            }
            else {
                x = random_64bit() | 1ULL; //da bude neparan
                br = 0;
            }
        }
    }
    return x;
}


ulli random_64bit() {
    if( first_time ) {
        first_time = false;
        rnd = time(NULL);
    }
    else {
        rnd = ( rnd * A ) % B;
    }
    return rnd;
}


bool test_bitovi( ulli broj ) {
    for( int i = 0; i < 62; i++ ) {
        //do 62 jer gledamo 3 bita u nizu pa ne trebamo ići do 63-eg bita
        bool i1 = ( (broj & ( 1ULL << i ) ) != 0 );
        bool i2 = ( (broj & ( 1ULL << ( i + 1 ) ) ) != 0 );
        bool i3 = ( (broj & ( 1ULL << ( i + 2 ) ) ) != 0 );

        if( i1 == i2 && i1 == i3 ) {
            return false; //tranzitivnost povlači i2 == i3
        }
    }
    return true;
}


bool test_pseudo_prost( ulli broj ) {
    ulli n;

    //ako je 'sqrt(broj)' prevelik, gledamo je li prost do 50 000
    if( sqrt(broj) > 50000 ) {
        n = 50000;
    }
    else {
        n = sqrt(broj);
    }

    for( unsigned int i = 3; i <= n; i += 2 ) {
        if( broj % i == 0 ) return false;
    }
    return true;
}