# Mario Rodríguez Ruiz
# Suma de enteros con signo de 32 bits en una plataforma de 32 bits sin perder precisión.
.section .data
		.macro linea
				#.int -1,-1,-1,-1
				#.int 1,-2,1,-2
				.int 1,2,-3,-4
				#.int 0x7FFFFFFF,0x7FFFFFFF,0x7FFFFFFF,0x7FFFFFFF
				#.int 0X80000000,0X80000000,0X80000000,0X80000000
				#.int 0X04000000,0X04000000,0X04000000,0X04000000
				#.int 0X08000000,0X08000000,0X08000000,0X08000000
				#.int 0xFC000000,0xFC000000,0xFC000000,0xFC000000
				#.int 0xF8000000,0xF8000000,0xF8000000,0xF8000000
				#.int 0xF0000000,0xE0000000,0xE0000000,0xD0000000
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
	mov    $lista, %ebx

	# Se guarda en ecx el numero de enteros de la lista
	mov longlista, %ecx

	# Llamada a la función suma
	call suma

	# Se guarda el valor de los 32 bit menos significativos
	# de nuestro número calculado en suma en resultado.
	mov %eax, resultado

	# Se guarda en la posición resultado más 4B el valor de los 32 bit
	# más significativos de nuestro número calculado en suma.
	mov %edx, resultado+4

    # Como el sistema utilizado es little endian metemos en la pila los
	# 32bit mas significativos primero de nuestro número
	push resultado+4

	# Se guardan en la pila los otros 32 bit (los menos significativos)
	push resultado
	push resultado+4
	push resultado
	push $formato      # Parámetros a leer en la pila
	call printf        # Llamada a la función printf
	mov $1, %eax       # Llamada al sistema
	mov $0, %ebx
	int $0x80

suma:
	mov $0, %ebp
	mov $0, %edi
	mov $0, %esi		# Establece el índice a 0
bucle:
    # Se guarda en eax cada número de la lista
	mov (%ebx,%esi,4), %eax

	# Se extiende el valor de eax a un número utilizando también el registro edx para el signo
	cltd

	# Se suma el valor existente en eax con el valor de edi manteniendo la suma en él
	add %eax,%edi

	# Se suman los bits de signo de edx con el valor de ebp más el acarreo si lo hubiera.
	adc %edx,%ebp

    # Incrementa en uno el registro esi (contador) cada vez
	# que se suma un número
	inc %esi

	# Si el registro esi es menor que ecx (longitud de lista)
	# salta al bucle para seguir sumando.
	cmp %esi,%ecx
	jne bucle

	# Se mueven los bits de signo a edx que son los 32 bits más significativos
	mov %ebp,%edx

	# Se mueve la suma total al acumulador eax
	mov %edi,%eax
	ret
