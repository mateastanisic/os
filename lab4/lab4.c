#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
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
int spremnik[40]; //spremnik (na početku inicijaliziramo sve pozicije slobodne)

//funkcije iz lab1.c
ulli random_64bit();
bool test_bitovi( ulli broj );
bool test_pseudo_prost( ulli broj );
ulli generiraj_broj();

void *radna_dretva( void *id );
void *dretva_provjera( void *id );

bool zauzmi( int x, int id );
void oslobodi( int x, int id );

int nadi_blok( int size ); //pronalazi best-fit blok
void print();

sem_t sem1, sem2, sem3; //semafori


int main( void ) {
    srand( time( NULL ) );
    pthread_t thr_id[10]; //dretve

    struct dretva_id *ids; //id-ovi dretvi

    //postavi semafore
    sem_init( &sem1, 0, 1 ); //za ulazak u KO
    sem_init( &sem2, 0, 0 ); //za određivanje ima li novih brojeva u MS
    sem_init( &sem2, 0, 1 ); //lab4 - ima li mjesta za alociranje

    for( int i=0; i<40; i++ ){ spremnik[i] = -1; } 

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

    sem_destroy(&sem1);
    sem_destroy(&sem2);
    sem_destroy(&sem3);


    return 0;
}





void *radna_dretva( void *id ) {
    //struct dretva_id *n = (struct dretva_id*)id; //ne treba nam id ovdje

    do {
        ulli x = generiraj_broj();
        sleep( x%5 ); // poanta: promijeniti parametre računanja prostih brojeva i provjere bitova da ti poslovi traju duže

        sem_wait( &sem1 ); //ulaz u KO


        MS[U] = x;
        //postavljanje semafora da dretva provjere čeka da je barem jedna nova vrijednost u spremniku
        int sval2;
        sem_getvalue(&sem2, &sval2); //dohvati vrijednost semafora2
        if( sval2==0 ) sem_post( &sem2 ); //postavi sem2 ako već nije postavljen i ako je dodan novi broj 
 
        U = ( U + 1 ) % 5;


        sem_post( &sem1 ); //izlaz iz KO

    } while( strcmp(kraj,"KRAJ_RADA") );

    free( id );
    return NULL;
}

void *dretva_provjera( void *id ) {
    struct dretva_id *n = (struct dretva_id*)id;

    do {

        sem_wait( &sem2 ); //čekanje da je barem jedna nova vrijednost u međuspremniku 
        //(mora ići prije sem1 jer je sem1 zajednički za obje vrste dretvi pa da ne bi došlo do potpunog zastoja)
        sem_wait( &sem1 ); //ulaz u KO

        ulli y = MS[0];
        printf("ID dretve: %d, dohvatio broj: %llu \n", (*n).id, y );

        bool x = false;
        do {
            x = zauzmi( y%20, (*n).id );
            print();
            if( !x ){
                sem_post( &sem1 );
                sem_wait( &sem3 );
                sem_wait( &sem1 );
            }
        } while( !x );

        sem_post( &sem1 ); //izlaz iz KO
        sleep( y%5 );
        sem_wait( &sem1 );
        printf("ID dretve: %d, potrosio broj: %llu \n", (*n).id, y );
        oslobodi( x, (*n).id );
        print();
        sem_post( &sem1 );
        sem_post( &sem3 );

    } while( strcmp(kraj,"KRAJ_RADA") );

    free( id );
    return NULL;
}


bool zauzmi( int y, int id ){
    int i = nadi_blok(y);
    if( i != -1 ){
        for( int j = i; j<(i+y); j++ ){
            spremnik[j] = id;
        }
        return true;
    }
    return false;
}


void oslobodi( int x, int id ){
    for( int j=0; j<40; j++ ){
        if( spremnik[j]==id ) spremnik[j] = -1;
    }
    return;
}


int nadi_blok( int size ){
    int velicina, index, velicina_best = -1, index_best = -1;
    //o i -1 su zastavice

    for( int i=0; i<40; i++ ){
        velicina = 0;
        if( spremnik[i] == -1 ){
            velicina++;
            index = i;
            i++;
            while( i<40 && spremnik[i] == -1 ){
                velicina++;
                i++;
            }
            if( velicina_best==-1 && velicina >= size ){
                velicina_best = velicina;
                index_best = index;
            }
            else if( velicina_best != -1 && velicina >= size && velicina < velicina_best ){
                velicina_best = velicina;
                index_best = index;
            }
        }

    }

    return index_best;
}


void print(){
    printf("Spremnik: ");
    for( int i=0; i<40; i++ ){
        if( spremnik[i] == -1 ) printf("-");
        else printf("%d", spremnik[i]);
    }
    printf("\n");
    return;
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