/************************************
*	  Mario Rodríguez Ruiz			*
*	  Bomba digital					*
*	  Estructura de Computadores    *
*	  Diciembre 2016				*
*                                   *
*     gcc -m32 bomba.c -o bomba     *
*************************************/

#include <stdio.h>	// para printf()
#include <stdlib.h>	// para exit()
#include <string.h>	// para strncmp()/strlen()
#include <sys/time.h>	// para gettimeofday(), struct timeval

char ebx[]="%edx";
int  pc   = 1892;

void boom(){
	printf("***************\n");
	printf("*** BOOM!!! ***\n");
	printf("***************\n");
	exit(-1);
}

void defused(){
	printf("*************************\n");
	printf("*** bomba desactivada ***\n");
	printf("*************************\n");
	exit(0);
}

int main(){
#define SIZE 100
	char pass[SIZE];
	int  pasv;
    int pdn = pc*strlen(ebx) ;
	ebx[2] = ebx[2] - 2 ;
#define TLIM 5
	struct timeval tv1,tv2;	// gettimeofday() secs-usecs

	gettimeofday(&tv1,NULL);
	printf("Introduce la contraseña: ");
	fgets(pass,SIZE,stdin);
	if (strncmp(pass,ebx,strlen(ebx)))
	    boom();

	gettimeofday(&tv2,NULL);
	if (tv2.tv_sec - tv1.tv_sec > TLIM)
	    boom();

	printf("Introduce el código: ");
	scanf("%i",&pasv);
	if (pasv*strlen(ebx)!=pdn) boom();

	gettimeofday(&tv1,NULL);
	if (tv1.tv_sec - tv2.tv_sec > TLIM)
	    boom();

	defused();
}
