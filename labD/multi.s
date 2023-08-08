section .data
    HEX: db "%02hhx", 0
    STR: db "%s", 10, 0
    DEC: db "%d", 10, 0
    LINE: db 10, 0
    SIZE EQU 600
    STATE: dd 0xACE1 
    MASK: dd 0x002d
    FORMAT: db "%x", 0

    a_struct: db 0
    a_num: times 600 db 0

    b_struct: db 0
    b_num: times 600 db 0

    concatenatedByte db 0
    size dd 0

    x_struct: db 5
    x_num: db 0xaa, 1,2,0x44,0x4f
    
    y_struct: db 6
    y_num: db 0xaa, 1,2,3,0x44,0x4f

    x_size: dd 0
    y_size: dd 0
    data_ptr: db 0

    case_1: db "-I" ,0
    case_2: db "-R",0

section .bss
    buffer: resb SIZE

section .text
    global main

    extern printf
    extern puts
    extern fgets
    extern stdin
    extern strlen
    extern malloc

main:
    push ebp
    mov ebp, esp
    mov ecx, [ebp+8] ; argc

    cmp ecx, 1
    je default1

    mov edx, [ebp+12] ; argv
    mov eax, [edx+4] ; argv[1]
    jmp test_case_1 

    mov esp, ebp
    pop ebp
    
    call exit

default1:    
    push x_struct
    call print_multi

    push y_struct
    call print_multi

    push x_struct
    push y_struct
    call add_multi

    push data_ptr
    call print_multi

    call exit

test_case_1:  
    cmp byte [eax], '-'
    jnz done_case_1

    cmp byte [eax+1], 'I'
    jnz test_case_2

    jmp CASE_1

done_case_1:
    ret

CASE_1:
    call getmulti_a
    call getmulti_b

    push a_struct
    call print_multi

    push b_struct
    call print_multi

    push a_struct
    push b_struct
    call add_multi
    push data_ptr
    call print_multi
    mov esp, ebp
    pop ebp
    call exit

test_case_2:
    cmp byte [eax], '-'
    jnz done_case_2

    cmp byte [eax+1], 'R'
    jnz done_case_2

    call pre_loop_rand_num
    mov esp, ebp
    pop ebp
    call exit

done_case_2:
    ret

pre_loop_rand_num:
    push dword [STATE]
    push FORMAT
    call printf 
    add esp, 8

    mov esi, 10   

loop_rand_num:
    push esi 
    call rand_num

    push eax
    push FORMAT
    call printf
    add esp, 8

    pop esi
    sub esi, 1

    jz done_loop_rand_num
    jmp loop_rand_num

done_loop_rand_num:
    mov esp, ebp
    pop ebp
    ret 

rand_num:
    push ebp
    mov ebp, esp

    mov ax, [STATE]
    mov bx, [MASK]

    xor bx, ax
    jp parity 
    
    STC
    RCR ax,1
    jmp done_rand_num  

parity:
    shr ax,1

done_rand_num:   
    mov [STATE] ,ax
    mov eax, [STATE]
    
    mov esp, ebp
    pop ebp
    ret

MaxMin:
    movzx ecx, byte [eax]   
    mov dword [x_size], ecx  

    movzx edx, byte [ebx]  
    mov dword [y_size], edx   

    mov esi, [x_size]
    mov edi, [y_size]

    cmp esi, edi
    jl min

    ret

min:
    mov ecx, eax
    mov eax, ebx
    mov ebx, ecx

    ret

add_multi:
    push ebp
    mov ebp, esp

    mov eax, [ebp+8] 
    mov ebx, [ebp+12]
    call MaxMin

    mov esi, eax
    mov edi, ebx

    movzx ecx, byte [esi]
    add ecx, 2

    push ecx
    call malloc
    add esp, 4

    mov [data_ptr], eax
    inc byte [esi]
    movzx ecx, byte [esi]
    dec byte [esi]
    mov byte [data_ptr], cl

pre_clean:
    movzx ebx, byte [esi]
    inc ebx

clean:
    cmp ebx, 0
    jz pre_first_loop

    mov byte [data_ptr+ebx], 0

    dec ebx
    jmp clean

pre_first_loop:
    movzx ebx, byte [edi]

first_loop:
    cmp ebx, 0
    jz pre_second_loop

    mov dl, byte [edi+ebx] 
    add byte [data_ptr+ebx], dl

    dec ebx
    jmp first_loop

pre_second_loop:
    movzx ebx, byte [esi]
    clc

second_loop:
    cmp ebx, 0
    jz done

    mov dl, byte [esi+ebx] 
    add byte [data_ptr+ebx], dl
    adc byte [data_ptr+ebx+1], 0

    dec ebx
    jmp second_loop

done:
    mov dword esi, 0
    mov dword edi, 0
    mov dword ebx, 0
    mov dword ecx, 0
    mov dword edx, 0

    mov esp, ebp
    pop ebp
    ret

getmulti_a:
    push ebp
    mov ebp, esp

    push dword [stdin]
    push dword SIZE
    push dword buffer
    call fgets
    add esp, 12

    push buffer
    call strlen

    mov ebx, eax
    mov ecx, 1

    mov [size], eax
    shr byte [size], 1

    mov eax, [size]
    mov dword [a_struct], eax

    mov esi, buffer ; buffer
    mov edi, 0 ; counter

looper_a:
    cmp edi, [size]
    je end

    mov al, [esi]
    call convert_letter_al
    inc esi
    inc ecx

    cmp ebx,ecx
    je null_a

    mov ah, [esi]
    call convert_letter_ah
    inc esi
    inc ecx

    shl al, 4
    or al, ah

    mov [concatenatedByte], al
    mov edx, [concatenatedByte]
    mov dword [a_num+edi], edx

    inc edi
    jmp looper_a

null_a:
    mov [concatenatedByte], al
    mov edx, [concatenatedByte]
    mov dword [a_num+edi], edx

    jmp end

getmulti_b:
    push ebp
    mov ebp, esp

    push dword [stdin]
    push dword SIZE
    push dword buffer
    call fgets
    add esp, 12

    push buffer
    call strlen

    mov ebx, eax
    mov ecx, 1

    mov [size], eax
    shr byte [size], 1

    mov eax, [size]
    mov dword [b_struct], eax

    mov esi, buffer ; buffer
    mov edi, 0 ; counter

looper_b:
    cmp edi, [size]
    je end

    mov al, [esi]
    call convert_letter_al
    inc esi
    inc ecx

    cmp ebx,ecx
    je null_b

    mov ah, [esi]
    call convert_letter_ah
    inc esi
    inc ecx

    shl al, 4
    or al, ah

    mov [concatenatedByte], al
    mov edx, [concatenatedByte]
    mov dword [b_num+edi], edx

    inc edi
    jmp looper_b

null_b:
    mov [concatenatedByte], al
    mov edx, [concatenatedByte]
    mov dword [b_num+edi], edx

    jmp end

convert_letter_al:
    cmp al, 'a'
    jl convert_digit_al
    sub al, 87 
    ret 

convert_digit_al:
    sub al, 48
    ret

convert_letter_ah:
    cmp ah, 'a'
    jl convert_digit_ah
    sub ah, 87 
    ret 

convert_digit_ah:
    sub ah, 48 
    ret

print_multi: 
    push ebp
    mov ebp, esp

    mov esi, [ebp+8] ; pointer
    mov bl, byte [esi] ; size
    add esi, ebx ; little endian

loop:
    cmp ebx, 0
    jz end

    mov ecx, esi
    mov cl, [ecx]

    push dword ecx
    push dword HEX
    call printf
    add esp, 8

    dec esi
    dec ebx

    jmp loop

end:
    push LINE
    push dword STR
    call printf
    add esp, 8

    mov esp, ebp
    pop ebp
    ret

exit:
    mov eax, 1
    mov ebx, 0
    int 0x80