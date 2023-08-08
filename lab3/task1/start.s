section .rodata
    STDIN EQU 0
    STDOUT EQU 1
    STERR EQU 2
    READ EQU 3
    WRITE EQU 4
    OPEN EQU 5
    CLOSE EQU 6
    SPACE: db "", 10, 0

section .bss
    buffer resb 1

section .data
    infile: dd STDIN
    outfile: dd STDOUT

section .text
global _start
global system_call   
;;extern main

_start:
    pop    dword ecx    ; ecx = argc
    mov    esi,esp      ; esi = argv
    ;; lea eax, [esi+4*ecx+4] ; eax = envp = (4*ecx)+esi+4
    mov     eax,ecx     ; put the number of arguments into eax
    shl     eax,2       ; compute the size of argv in bytes
    add     eax,esi     ; add the size to the address of argv 
    add     eax,4       ; skip NULL at the end of argv
    push    dword eax   ; char *envp[]
    push    dword esi   ; char* argv[]
    push    dword ecx   ; int argc
    
    call    main        ; int main( int argc, char *argv[], char *envp[] )
    call    encode

    mov     ebx,eax 
    mov     eax,1
    int     0x80
    nop
        
system_call:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

    mov     eax, [ebp+8]    ; Copy function args to registers: leftmost...        
    mov     ebx, [ebp+12]   ; Next argument...
    mov     ecx, [ebp+16]   ; Next argument...
    mov     edx, [ebp+20]   ; Next argument...
    int     0x80            ; Transfer control to operating system
    mov     [ebp-4], eax    ; Save returned value...
    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller

main:
    push ebp 
    mov ebp, esp
    push ebx
    mov ecx, [ebp+8] 
    mov edx, [ebp+12]

    mov ebx, 0
    call direct
    ret

direct:
    mov eax, [edx+ebx*4]

    call inputDirect
    call outputDirect
  
    inc ebx
    cmp ebx, ecx
    jnz direct 

    mov ebx, 0
    call next
    ret

next: 
    mov eax, [edx+ebx*4]

    push eax
    call print
    inc ebx
    cmp ebx, ecx
    jnz next 
    pop ebx
    mov esp, ebp
    pop ebp
    ret

print:
    push ebp
    mov ebp, esp
    pushad
    mov ecx, [ebp+8]
    call my_strlen

    mov ecx, [ebp+8]
    mov edx, eax
    mov ebx, [outfile]
    mov eax, WRITE
    int 0x80 

    mov eax, WRITE
    mov ebx, [outfile]
    mov ecx, SPACE
    mov edx, 1
    int 0x80 

    popad
    mov esp, ebp
    pop ebp
    ret

my_strlen: 
    mov eax,1

cont: 
    cmp byte [ecx], 0
    jz done
    inc ecx
    inc eax
    jmp cont

encode:
    push ebp 
    mov ebp, esp
    pushad

    mov eax, READ
    mov ebx, [infile]
    mov ecx, buffer
    mov edx, 1
    int 0x80 

    cmp eax, 0
    jz exit

    mov esi, buffer

increase:
    cmp byte [esi], 'A' 
    jb loop        

    cmp byte [esi], 'z'
    ja loop

    inc byte [esi]

loop:
    mov eax, WRITE
    mov ebx, [outfile]
    mov ecx, esi
    mov edx, 1
    int 0x80   

    jmp encode

inputDirect:    
    cmp byte [eax], '-'
    jnz done

    cmp byte [eax+1], 'i'
    jnz done

    call input
    ret

outputDirect:
    cmp byte [eax], '-'
    jnz done

    cmp byte [eax+1], 'o'
    jnz done

    call output
    ret

input:
    pushad

    add eax, 2

    mov ebx, eax
    mov eax, OPEN
    mov ecx, 0 ; open for reading only
    mov edx, 0777
    int 0x80

    mov [infile], eax

    popad
    ret

output:
    pushad

    add eax, 2
    
    mov ebx, eax
    mov eax, OPEN
    mov ecx, 1 ; open for writing only
    or ecx, 64 ; create the file if its doesnt exits
    mov edx, 0777
    int 0x80

    mov [outfile], eax

    popad
    ret
    
exit:
    popad 
    mov esp, ebp 
    pop ebp 

    mov eax, CLOSE
    mov ebx, [infile]
    int 0x80

    mov eax, CLOSE
    mov ebx, [outfile]
    int 0x80

    mov eax, 1
    mov ebx, 0
    int 0x80

done: 
    ret
