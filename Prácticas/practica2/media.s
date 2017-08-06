# Mario Rodríguez Ruiz
# Media de enteros con signo de 32 bits en una plataforma de 32 bits sin perder precisión.
.section .data
		.macro linea
				#.int 0,-2,-1,-1
				#.int 1,-2,1,-2
				.int 1,2,-3,-4
				#.int 0x7FFFFFFF,0x7FFFFFFF,0x7FFFFFFF,0x7FFFFFFF
				#.int 0X80000000,0X80000000,0X80000000,0X80000000
				#.int 0xF0000000,0xE0000000,0xE0000000,0xD0000000
		.endm

lista:
		linea
		.irpc i,1234567
				linea
		.endr

# Calcula la longitud de la lista
longlista:	.int (.-lista)/4

media: .int 0
resto: .int 0
# Formato para mostrar resultados
formato: .ascii "Media decimal = %d \nMedia hexadecimal = 0x%x \n"
		 .ascii "Resto decimal = %d \nResto hexadecimal = 0x%x \n\0"
.section .text
main:	.global main

	# Se guarda en ebx la posición donde comienza la
    # lista de enteros en memoria
	mov    $lista, %ebx

	# Se guarda en ecx el numero de enteros de la lista
	mov longlista, %ecx

	# Llamada a la función suma
	call suma

	# Se guarda el valor de la media de todos los elementos en eax.
	mov %eax,media

    # Se guarda el valor del resto del resultado de hacer la media en la variable edx
	mov %edx,resto
	# Se incluye dos veces el resto y la media
    # porque se mostrará en decimal y hexadecimal.
	push resto
	push resto
	push media
	push media
	push $formato      # Parámetros a leer en la pila
	call printf        # Llamada a la función printf
	mov $1, %eax       # Llamada al sistema
	mov $0, %ebx
	int $0x80

suma:
	mov $0, %ebp
	mov $0, %edi
	mov $0, %esi       # Establece el índice a 0
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

	# Se divide el número alojado en entre el numero de elementos de la lista(ecx),
    # El cociente se guarda en eax y el resto en edx.
	idiv %ecx
	ret
