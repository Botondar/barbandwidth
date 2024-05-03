global Write32x1
global Write32x2
global Write32x4

section .text

Write32x1:
    vxorps ymm0, ymm0
    align 64
.loop:
    vmovups [rdx], ymm0
    add rdx, 32
    sub rcx, 32
    jnz .loop
    ret

Write32x2:
    vxorps ymm0, ymm0
    align 64
.loop:
    vmovups [rdx], ymm0
    vmovups [rdx + 32], ymm0
    add rdx, 64
    sub rcx, 64
    jnz .loop
    ret

Write32x4:
    vxorps ymm0, ymm0
    align 64
.loop:
    vmovups [rdx], ymm0
    vmovups [rdx + 32], ymm0
    vmovups [rdx + 64], ymm0
    vmovups [rdx + 96], ymm0
    add rdx, 128
    sub rcx, 128
    jnz .loop
    ret
