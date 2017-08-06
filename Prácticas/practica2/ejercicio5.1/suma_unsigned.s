# Mario Rodríguez Ruiz
# Suma de enteros sin signo de 32 bits en una plataforma de 32 bits sin perder precisión.
.section .data
		.macro linea
				#.int 1,1,1,1
				.int 2,2,2,2
				#.int 1,2,3,4
				#.int -1,-1,-1,-1
				#.int 0xffffffff,0xffffffff,0xffffffff,0xffffffff
				#.int 0x08000000,0x08000000,0x08000000,0x08000000
				#.int 0x10000000,0x20000000,0x40000000,0x80000000
		.endm
lista:  .irpc i,12345678
				linea
		.endr

# Calcula la longitud de la lista
longlista:	.int (.-lista)/4

resultado:	.quad 0x0123456789ABCDF    # Valor que ocupa 8B

# Formato para mostrar resultados
formato:	.ascii "Decimal = %lld \nHexadecimal = 0x%llx \n\0"

.section .text
main:	.global main
    # Se guarda en ebx la posición donde comienza la
    # lista de enteros en memoria
	mov $lista, %ebx

	# Se guarda en ecx el numero de enteros de la lista
	mov longlista, %ecx

	# Llamada a la función suma
	call suma

	# Se guarda el valor de los 32 bit menos significativos
	# del número calculado en suma en resultado.
	mov %eax, resultado

	# Se guarda en la posición resultado más 4B el valor de los 32 bit
	# más significativos del número calculado en suma.
	mov %edx, resultado+4

	# Como el sistema utilizado es little endian metemos en la pila los
	# 32bit mas significativos primero de nuestro número
	push resultado+4

	# Se guardan en la pila los otros 32 bit (los menos significativos)
	push resultado
	push resultado+4
	push resultado
	push $formato          # Parámetros a leer en la pila
	call printf            # Llamada a la función printf
	mov $1, %eax           # Llamada al sistema
	mov $0, %ebx
	int $0x80

suma:
	mov $0, %eax
	mov $0, %edx
	mov $0, %esi           # Establece el índice a 0
bucle:
    # Se suma al acumulador eax los números de la lista y,
    # como son enteros de 4B, se realiza un desplazamiento
    # de 4 en 4 Bytes
	add (%ebx,%esi,4), %eax

	# Se suma el acarreo en el registro edx (registro para
	# los 32bit más signficativos)
	adc $0,%edx

	# Incrementa en uno el registro esi (contador) cada vez
	# que se suma un número
	inc %esi

	# Si el registro esi es menor que ecx (longitud de lista)
	# salta al bucle para seguir sumando.
	cmp  %esi,%ecx
	jne bucle

	ret
