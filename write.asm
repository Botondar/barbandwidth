global Write16x1

section .text

Write16x1:
    vxorps ymm0, ymm0
    align 64
.loop:
    vmovups [rdx], ymm0
    add rdx, 32
    sub rcx, 32
    jnz .loop
    ret