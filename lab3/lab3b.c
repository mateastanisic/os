#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <malloc.h>


#define NAJVECI_BROJ 0xffffffffffffffffULL
#define A 8531648002905263869ULL
#define B 18116652324302926351ULL

typedef unsigned long long int ulli;

struct dretva_id{
    unsigned int id;
};

bool first_time = true; //za random64bit()
ulli rnd;//static varijabla koja predstvalja random ulli broj
ulli MS[] = {0, 0, 0, 0, 0}; //međuspremnik za pohranjivanje random ulli brojeva
int U = 0; //izlaz index, ulaz index inicijalizirani na 0
char* kraj = "NIJE_KRAJ";

//funkcije iz lab1.c
ulli random_64bit();
bool test_bitovi( ulli broj );
bool test_pseudo_prost( ulli broj );
ulli generiraj_broj();
void *radna_dretva( void *id );
void *dretva_provjera( void *id );

//monitori
pthread_mutex_t monitor = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t red_uvjeta = PTHREAD_COND_INITIALIZER;


int main( void ) {
    srand( time( NULL ) );
    pthread_t thr_id[10]; //dretve
    
    struct dretva_id *ids; //id-ovi dretvi

    //radne dretve
    for( int i=0; i<5; i++ ) {
        ids = malloc( sizeof(struct dretva_id) );
        (*ids).id = i;
        if (pthread_create(&thr_id[i], NULL, radna_dretva, (void*)ids) != 0) {
            printf ("Greska pri stvaranju dretve!\n");
            exit (1);
        }
    }
    sleep(2);

    //dretve provjere
    for( int i=5; i<10; i++ ) {
        ids = malloc( sizeof(struct dretva_id) );
        (*ids).id = i-5;
        if (pthread_create (&thr_id[i], NULL, dretva_provjera, (void*)ids) != 0) {
            printf ("Greska pri stvaranju dretve!\n");
            exit (1);
        }
    }
    sleep(30);
    kraj = "KRAJ_RADA";

    for( int i=0; i<10; i++ ) {
        pthread_join(thr_id[i], NULL );
    }

    pthread_mutex_destroy(&monitor);
    return 0;
}





void *radna_dretva( void *id ) {
    //struct dretva_id *n = (struct dretva_id*)id; ne treba nam id ovdje

    do {
        ulli x = generiraj_broj();
        sleep( x%5 );

        pthread_mutex_lock(&monitor); //ulaz u KO


        MS[U] = x; 
        U = ( U + 1 ) % 5;
        pthread_cond_signal( &red_uvjeta ); //našao novi broj


        pthread_mutex_unlock(&monitor); //izlaz iz KO

    } while( strcmp(kraj,"KRAJ_RADA") );

    free( id );
    return NULL;
}

void *dretva_provjera( void *id ) {
    struct dretva_id *n = (struct dretva_id*)id;

    do {

        pthread_mutex_lock(&monitor); //ulaz u KO
        

        pthread_cond_wait( &red_uvjeta, &monitor );
        ulli y = MS[0];
        printf("ID dretve: %d, dohvatio broj: %llu \n", (*n).id, y );


        pthread_mutex_unlock(&monitor);  //izlaz iz KO
        
        printf("ID dretve: %d, potrosio broj: %llu \n", (*n).id, y );

    } while( strcmp(kraj,"KRAJ_RADA") );

    free( id );
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