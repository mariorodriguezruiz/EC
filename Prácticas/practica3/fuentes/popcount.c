/*
 *  Mario Rodríguez Ruiz
 *  popcount.c
 *
 */

#define TEST		0
#define COPY_PASTE_CALC		0
#define WSIZE 8*sizeof(int)

#if ! TEST
	#define NBITS 20
	#define SIZE (1<<NBITS)
	unsigned lista[SIZE];
	#define RESULT NBITS*(SIZE/2)
#else
/*																* /
	#define SIZE 4
	unsigned lista[SIZE]={0X80000000,0X00100000,0X00000800,0X000000001};
	#define RESULT 4
																* /
	#define SIZE 8
	unsigned lista [SIZE]={0x7fffffff,0xffefffff,0xfffff7ff,0xfffffffe,
	0x01000024,0x00356700,0x8900ac00,0x00bd00ef};
	#define RESULT 156
*/
	#define SIZE 8
	unsigned lista[SIZE]={0X0,0x10204080,0x3590ac06,0x70b0d0e0,0xffffffff,
	0x12345678,0x9abcdef0,0xcafebeef};
	#define RESULT 116
/*																*/
#endif

#include <stdio.h>	// para printf()
#include <stdlib.h>	// para exit()
#include <sys/time.h>	// para gettimeofday(), struct timeval

int resultado=0;

// Primera versión, previsiblemente con peores prestaciones.
int popcount1(unsigned* array, int len)
{
    int  i,j,  res=0;
    for (i=0; i<len; i++){
        unsigned x =array[i];
        for (j=0; j<WSIZE;j++){     // Recorre los bits
            res += x & 0x1;         // Aplica la máscara 0x1
            x >>=1;                 // Desplaza a la derecha para
        }
    }
    return res;
}

int popcount2(unsigned* array, int len)
{
    int  i, res=0;
    unsigned x;
    for (i=0; i<len; i++){
        x=array[i];
        do{
            res += x & 0x1;         // Aplica la máscara 0x1
            x >>=1;                 // Desplaza a la derecha
        }while(x);                  // Recorre los bits
    }
    return res;
}

// Traduce el bucle interno while por código ASM.
int popcount3(unsigned* array, int len){
    int res=0,i;
    unsigned x;

    for(i=0; i<len; i++){
        x=array[i];
        asm("\n"
        "ini3:		\n\t"        // seguir mientras que x!=0
        "shr %[x]	\n\t"        // LSB en CF
        "adc $0, %[r] \n\t"      // suma con acarreo
        "cmp $0,%[x] \n\t"       // compara
        "jne	ini3 \n\t"       // salto, modificando CF y ZF
        : [r]"+r" (res)          // e/s: añadir a lo acumulado por el momento
        : [x]"r" (x)             // entrada: valor elemento
        );
    }

    return res;
}

int popcount4(unsigned* array, int len){
	int i,j;
	int result = 0;
	unsigned n;

	for(i = 0; i < len; i++){
		int res = 0;
		n = array[i];
		for(j = 0; j < 8; j++){       // Aplica la máscara a cada elemento.
			res += n & 0x01010101;    // Acumula los bits de cada byte.
			n >>= 1;
		}
		// Suma en árbol los 4B
        res += (res >> 16);
        res += (res >> 8);
        result += (res & 0xff);
	}
	return result;
}

// Versión SSE3 (pshufb)
int popcount5(unsigned* array, int len) {
    int i;
    int res, result = 0;
    int SSE_mask[] = { 0x0f0f0f0f, 0x0f0f0f0f, 0x0f0f0f0f, 0x0f0f0f0f };
    int SSE_LUTb[] = { 0x02010100, 0x03020201, 0x03020201, 0x04030302 };

    if (len & 0x3)
        printf("leyendo 128b pero len no múltiplo de 4?n");
    for (i = 0; i < len; i += 4) {
        asm("movdqu        %[x], %%xmm0 \n\t"
        "movdqa  %%xmm0, %%xmm1 \n\t"       // dos copias de x
        "movdqu    %[m], %%xmm6 \n\t"       // máscara
        "psrlw           $4, %%xmm1 \n\t"
        "pand    %%xmm6, %%xmm0 \n\t"       // xmm0 - nibbles inferiores
        "pand    %%xmm6, %%xmm1 \n\t"       // xmm1 - nibbles superiores

        "movdqu    %[l], %%xmm2 \n\t"       // como pshufb sobrescribe LUT
        "movdqa  %%xmm2, %%xmm3 \n\t"       // queremos 2 copias
        "pshufb  %%xmm0, %%xmm2 \n\t"       // xmm2 = vector popcount inferiores
        "pshufb  %%xmm1, %%xmm3 \n\t"       // xmm3 = vector popcount superiores

        "paddb   %%xmm2, %%xmm3 \n\t"       // xmm3 - vector popcount bytes
        "pxor    %%xmm0, %%xmm0 \n\t"       // xmm0 = 0,0,0,0
        "psadbw  %%xmm0, %%xmm3 \n\t"       // xmm3 = [pcnt bytes0..7|pcnt bytes8..15]
        "movhlps %%xmm3, %%xmm0 \n\t"       // xmm3 = [       0      |pcnt bytes0..7 ]
        "paddd   %%xmm3, %%xmm0 \n\t"       // xmm0 = [   no usado   |pcnt bytes0..15]
        "movd    %%xmm0, %[res] \n\t"
        : [res]"=r" (res)
        : [x]  "m"  (array[i]),
          [m]  "m"  (SSE_mask[0]),
          [l]  "m"  (SSE_LUTb[0])
        );
        result += res;
    }
    return result;
}

// Versión SS4.2
int popcount6(unsigned* array, int len) {
 	int i;
	unsigned x;
	 int val, result=0;
	for(i=0; i<len; i++){
		x=array[i];
		asm("popcnt %[x], %[val]"
		 : [val] "=r" (val)
		 :   [x]  "r" (x)
		);
		result += val;
	}
	return result;
}

void crono(int (*func)(), char* msg){
    struct timeval tv1,tv2;	// gettimeofday() secs-usecs
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
    crono(popcount1, "popcount1 (Lenguaje C)");
    crono(popcount2, "popcount2 (Instrucción ASM)");
    crono(popcount3, "popcount3 (While en ASM)");
    crono(popcount4, "popcount4 (Lenguaje C 32bit)");
    crono(popcount5, "popcount5 (SSE3)");
	crono(popcount6, "popcount6 (SSE4.2)");


    #if ! COPY_PASTE_CALC
        printf("Resultado = %d\n",RESULT); /*OF*/
    #endif

    exit(0);
}
