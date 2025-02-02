// según la versión de gcc y opciones de optimización usadas, tal vez haga falta
// usar gcc --fno-omit-frame-pointer si gcc quitara el marco pila (%ebp)

#include <stdio.h>    // para printf()
#include <stdlib.h>   // para exit()
#include <sys/time.h> // para gettimeofday(), struct timeval

#define TEST		0
#define COPY_PASTE_CALC		0
#define WSIZE 8*sizeof(int)

#if ! TEST
	#define NBITS 20
	#define SIZE (1<<NBITS)
	unsigned lista[SIZE];
	#define RESULT (SIZE/2)
#else
/*																*/
	#define SIZE 4
	unsigned lista[SIZE]={0X80000000,0X00100000,0X00000800,0X00000001};
	#define RESULT 4
#endif

    int resultado=0;

int suma1(int* array, int len)
{
	int i, res=0;
	for (i=0; i<len; i++)
		res += array[i];
	return res;
}

int suma2(int* array, int len)
{
	int i, res=0;
	for (i=0; i<len; i++)
	asm("add (%[a],%[i],4),%[r]	\n"
	 : [r] "+r" (res)   // output-input
	 : [i] "r" (i),     // input
	   [a] "r" (array)
	);
	return res;
}

int suma3(int* array, int len)
{
	asm("    mov 8(%%ebp), %%ebx          \n" // array
	    "    mov 12(%%ebp), %%ecx         \n" // len
	    "                                 \n"
	    "    mov $0, %%eax                \n" // retval
	    "    mov $0, %%edx                \n" // index
	    "bucle:                           \n"
	    "    add (%%ebx, %%edx, 4), %%eax \n"
	    "    inc         %%edx            \n"
	    "    cmp %%edx, %%ecx             \n"
	    "    jne bucle                    \n"
	    :       // output
	    :       // input
	    : "ebx" // clobber
	   );
}

void crono(int (*func)(), char* msg)
{
	struct timeval tv1,tv2;  // gettimeofday() secs-usecs
	long           tv_usecs; // y sus cuentas

	gettimeofday(&tv1,NULL);
	resultado = func(lista, SIZE);
	gettimeofday(&tv2,NULL);

	tv_usecs=(tv2.tv_sec - tv1.tv_sec) * 1E6 + (tv2.tv_usec - tv1.tv_usec);
	#if ! COPY_PASTE_CALC
        printf("resultado = %d\t", resultado);
        printf("%s:%9ld us\n", msg, tv_usecs);
    #else
        printf("%6ld\n", tv_usecs);
    #endif
}

int main()
{

	crono(suma1, "suma1 (en lenguaje C    )");
	crono(suma2, "suma2 (1 instrucción asm)");
	crono(suma3, "suma3 (bloque asm entero)");
	printf("Resultado = %d\n",RESULT); /*OF*/

	exit(0);
}
