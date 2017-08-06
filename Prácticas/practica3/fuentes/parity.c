/*
 *  Mario Rodríguez Ruiz
 *  parity.c
 *
 */

#define TEST		0
#define COPY_PASTE_CALC		1
#define WSIZE 8*sizeof(int)

#if ! TEST
	#define NBITS 20
	#define SIZE (1<<NBITS)
	unsigned lista[SIZE];
	#define RESULT (SIZE/2)
#else
/*
	#define SIZE 4
	unsigned lista[SIZE]={0X80000000,0X00100000,0X00000800,0X00000001};
	#define RESULT 4

	#define SIZE 8
	unsigned lista [SIZE]={0x7fffffff,0xffefffff,0xfffff7ff,0xfffffffe,
	0x01000024,0x00356700,0x8900ac00,0x00bd00ef};
	#define RESULT 8
*/
	#define SIZE 8
	unsigned lista[SIZE]={0X0,0x10204080,0x3590ac06,0x70b0d0e0,0xffffffff,
	0x12345678,0x9abcdef0,0xcafebeef};
	#define RESULT 2

#endif

#include <stdio.h>	// para printf()
#include <stdlib.h>	// para exit()
#include <sys/time.h>	// para gettimeofday(), struct timeval

int resultado=0;

// Primera versión
int parity1(unsigned* array, int len)
{
    int  i,j,  res=0,val;
    for (i=0; i<len; i++){          // Recorre el array
        val=0;
        unsigned x =array[i];
        for (j=0; j<WSIZE;j++){     // Recorre los bits
            val ^= x & 0x1;         // Aplica la máscara y acumula lateralmente los bits
            x >>=1;                 // Desplaza a la derecha para extraer y acumular bits
        }
        res+=val;
    }
    return res;
}

// Segunda versión
int parity2(unsigned* array, int len)
{
    int  i,res=0,val;
    unsigned x;
    for (i=0; i<len; i++){
        val=0;
        x=array[i];
        do{
            val ^= x & 0x1;     // Aplica la máscara y acumula lateralmente los bits
            x >>=1;             // Desplaza a la derecha para extraer y acumular bits
        }while(x);              // Recorre el array
        res+=val;
    }
    return res;
}

// Tercera versión: Adapta la segunda versión para el array completo
int parity3(unsigned* array, int len) {
	int i;
	unsigned x;
	int result = 0;
	for (i = 0; i < len; i++) {
		x = array[i];
		int res=0;
		while (x) {
			res ^= x;            // Acumula lateralmente los bits
			x >>= 1;             // Desplaza a la derecha para extraer y acumular bits
		}
		result += res & 0x1;     // Aplicar la máscara al acumular con result.
	}
	return result;
}

// Cuarta versión: Traduce el bucle intero por ASM
int parity4(unsigned* array, int len) {
    int i,res;
    unsigned x;
    int result = 0;
    for (i = 0; i < len; i++) {
        x = array[i];
        res = 0;
        asm(
            "ini3:				\n\t"   // seguir mientras que x!=0
            "xor %[x], %[v]		\n\t"
            "shr $1, %[x]		\n\t"   // LSB en ZF
            "test %[x], %[x]	\n\t"
            "jnz ini3			\n\t"   // salto, modificando CF y ZF
            : [v]"+r"(res)              // e/s: entrada 0, salida paridad elemento

            : [x]"r"(x)                 // entrada: valor elemento
        );
        result += res & 0x1;            // Aplicar la máscara al acumular con result.
    }
    return result;

}

// Quinta versión: Suma en árbol
int parity5(unsigned* array, int len) {
    int i, j;
    int result = 0;
    unsigned x;
    for (i = 0; i < len; i++) {
        x = array[i];
        // Somete a cada elemento a XOR y desplazamientos
        // sucesivos a mitdad de la distancia cada vez
        for (j = 16; j > 0; j /= 2)
            x ^= x >> j;
        result += x & 0x01;     // Aplica la máscara al valor final.
    }
    return result;
}

// Sexta versión: Traduce el bucle for por ASM
int parity6(unsigned* array, int len) {
    int j,result = 0;;
    unsigned x = 0;

    for (j = 0; j < len; j++) {
        x = array[j];
        asm(
            "mov	%[x], %%edx	     \n\t" // Sacar copia para XOR
            "shr	$16,	%%edx	 \n\t"
            "xor	%[x],	%%edx	 \n\t" // PF, señalando la paridad par de los 8 bits inferiores
            "xor	%%dh,	%%dl	 \n\t"
            "setpo  %%dl	         \n\t"
            "movzx	%%dl,	%[x]	 \n\t" // Devuelve en 32 bits
            : [x] "+r" (x)                 // e/s entrada valor elemento, salida paridad
            :
            : "edx"                        // clobber
        );
        result += x;
    }
    return result;
}

void crono(int (*func)(), char* msg){
    struct timeval tv1,tv2;	    // gettimeofday() secs-usecs
    long           tv_usecs;	// y sus cuentas

    gettimeofday(&tv1,NULL);
    resultado = func(lista, SIZE);
    gettimeofday(&tv2,NULL);

    tv_usecs=(tv2.tv_sec -tv1.tv_sec )*1E6+
    (tv2.tv_usec-tv1.tv_usec);

    #if ! COPY_PASTE_CALC
        printf("resultado = %d\t", resultado);
        printf("%s:%9ld us\n", msg, tv_usecs);
    #else
        printf("%6ld\n", tv_usecs);
    #endif
}

int main()
{
	#if !TEST
        int i;			// inicializar array
        for (i=0; i<SIZE; i++)	// se queda en cache
            lista[i]=i;
    #endif
    crono(parity1, "parity1 (Lenguaje C)");
    crono(parity2, "parity2 (Instrucción ASM)");
    crono(parity3, "parity3 (While en C)");
    crono(parity4, "parity4 (While en ASM)");
    crono(parity5, "parity5 (Suma en árbol)");
    crono(parity6, "parity6 (for en ASM)");
    #if ! COPY_PASTE_CALC
        printf("Resultado = %d\n",RESULT); /*OF*/
    #endif

    exit(0);
}
