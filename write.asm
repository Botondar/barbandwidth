global Write32x1
global Write32x2
global Write32x4
global WriteNonTemporal32x4
global Copy32x4
global CopyNonTemporal32x4

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

WriteNonTemporal32x4:
    vxorps ymm0, ymm0,
    align 64
.loop:
    vmovntdq [rdx], ymm0
    vmovntdq [rdx + 32], ymm0
    vmovntdq [rdx + 64], ymm0
    vmovntdq [rdx + 96], ymm0
    add rdx, 128
    sub rcx, 128
    jnz .loop
    ret

Copy32x4:
    align 64
.loop:
    vmovdqu ymm0, [r8]
    vmovdqu [rdx], ymm0
    vmovdqu ymm0, [r8 + 32]
    vmovdqu [rdx + 32], ymm0
    vmovdqu ymm0, [r8 + 64]
    vmovdqu [rdx + 64], ymm0
    vmovdqu ymm0, [r8 + 96]
    vmovdqu [rdx + 96], ymm0
    add rdx, 128
    add r8, 128
    sub rcx, 128
    jnz .loop
    ret

CopyNonTemporal32x4:
    align 64
.loop:
    vmovdqu ymm0, [r8]
    vmovntdq [rdx], ymm0
    vmovdqu ymm0, [r8 + 32]
    vmovntdq [rdx + 32], ymm0
    vmovdqu ymm0, [r8 + 64]
    vmovntdq [rdx + 64], ymm0
    vmovdqu ymm0, [r8 + 96]
    vmovntdq [rdx + 96], ymm0
    add rdx, 128
    add r8, 128
    sub rcx, 128
    jnz .loop
    ret