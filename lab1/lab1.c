#include <time.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> 


#define NAJVECI_BROJ 0xffffffffffffffffULL
#define A 8531648002905263869ULL
#define B 18116652324302926351ULL


typedef unsigned long long int ulli;


bool first_time = true, first_check = true; 
/*jer se static varijabla u c-u ne može inicijalizirati s nekom ne-konstantnom vrijednošću
pa nam treba info generiramo li prvi puta neki random broj ( rnd = time(NULL) ) ili ne 
( rnd = ( rnd*A )%B )*/
ulli rnd; //static varijabla

ulli MS[5]; //međuspremnik

int I, U; //izlaz index, ulaz index inicijalizirani na 0

double t; //za provjeru zahtjeva

double start_time; //da znamo koliko je vremena prošlo


//funkcija za sekunde
double dsecnd (void) {
    return (double)( clock( ) ) / CLOCKS_PER_SEC;
}

ulli random_64bit();

bool test_bitovi( ulli broj );

bool test_pseudo_prost( ulli broj );

const char* provjera_zahtjeva();



int main() {	
	t = dsecnd();
	start_time = dsecnd();

	do {
		ulli x = random_64bit() | 1ULL; //da bude neparan

		int br = 0, br2 = 0; //brojači za zaustavljanje petlje i op. x+=2
		int limit = 100; //koliko puta ćemo dodavati 2

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

		//dodaj x u MS
		MS[U] = x; 
		U = ( U + 1 ) % 5;

	} while ( strcmp( "KRAJ_RADA", provjera_zahtjeva() ) != 0 );

	return 0;
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


const char* provjera_zahtjeva() {
	double t2 = t;

	t = dsecnd();
	if( ( t - t2 ) < 1 || first_check ) {
		//nije prošla bar 1s od zadnje provjere zahtjeva
		first_check = false;
		return "NIJE_KRAJ";
	}
	else {
		int r = rand() % 100 + 1; //r u rasponu od 1 do 100
		if( r >= 50 ){ //postoji novi zahtjev (vjerojatnost 50%)
			//vrijeme proteklo od početka programa
			int time_passed = dsecnd() - start_time;
			printf("%03d - ", time_passed ); //ispisuje vrijeme

			printf("MS[] = { ");
			for( int i = 0; i < 5; i++ ) printf("%2llx ", (MS[i] & 255) );
			printf("} ");

			printf("I=%d", I);
			printf(", U=%d", U);
			printf(", ");

			printf("MS[I]=%#016llx ", MS[I]);
			I = ( I + 1 ) % 5;
			printf("\n");
		}

		r = rand() % 100 + 1;
		if( r <= 10 ) return "KRAJ_RADA"; //postoji zahtjev za kraj rada ( vjerojatnost 10% )
		else return "NIJE_KRAJ";
	}
}
