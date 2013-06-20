;
; Reaver Project OS, Rose License
;
; Copyright (C) 2011-2013 Reaver Project Team:
; 1. Michał "Griwes" Dominiak
;
; This software is provided 'as-is', without any express or implied
; warranty. In no event will the authors be held liable for any damages
; arising from the use of this software.
;
; Permission is granted to anyone to use this software for any purpose,
; including commercial applications, and to alter it and redistribute it
; freely, subject to the following restrictions:
;
; 1. The origin of this software must not be misrepresented; you must not
;    claim that you wrote the original software. If you use this software
;    in a product, an acknowledgment in the product documentation is required.
; 2. Altered source versions must be plainly marked as such, and must not be
;    misrepresented as being the original software.
; 3. This notice may not be removed or altered from any source distribution.
;
; Michał "Griwes" Dominiak
;

bits    64

global  res
global  de
global  nmi
global  rp
global  of
global  br
global  ud
global  nm
global  df
global  ts
global  np
global  sf
global  gp
global  pf
global  mf
global  ac
global  mc
global  xm

extern  reserved
extern  divide_error
extern  non_maskable
extern  breakpoint
extern  overflow
extern  bound_range
extern  invalid_opcode
extern  no_coprocessor
extern  double_fault
extern  invalid_tss
extern  segment_not_present
extern  stack_fault
extern  protection_fault
extern  page_fault
extern  fpu_error
extern  alignment_check
extern  machine_check
extern  simd_exception

extern  common_interrupt_handler

%macro  setup   0
    push    rax
    push    rbx
    push    rcx
    push    rdx
    push    rsi
    push    rdi
    push    r8
    push    r9
    push    r10
    push    r11
    push    r12
    push    r13
    push    r14
    push    r15
%endmacro

%macro  exit    0
    pop     r15
    pop     r14
    pop     r13
    pop     r12
    pop     r11
    pop     r10
    pop     r9
    pop     r8
    pop     rdi
    pop     rsi
    pop     rdx
    pop     rcx
    pop     rbx
    pop     rax

    add     rsp, 8

    iretq
%endmacro

%macro  exitnoerror 0
    pop     r15
    pop     r14
    pop     r13
    pop     r12
    pop     r11
    pop     r10
    pop     r9
    pop     r8
    pop     rdi
    pop     rsi
    pop     rdx
    pop     rcx
    pop     rbx
    pop     rax

    iretq
%endmacro

res:
    setup
    call    reserved
    exitnoerror

de:
    setup
    call    divide_error
    exitnoerror

nmi:
    setup
    call    non_maskable
    exitnoerror

rp:
    setup
    call    breakpoint
    exitnoerror

of:
    setup
    call    overflow
    exitnoerror

br:
    setup
    call    bound_range
    exitnoerror

ud:
    setup
    call    invalid_opcode
    exitnoerror

nm:
    setup
    call    no_coprocessor
    exitnoerror

df:
    setup
    call    double_fault
    exitnoerror

ts:
    setup
    call    invalid_tss
    exitnoerror

np:
    setup
    call    segment_not_present
    exit

sf:
    setup
    call    stack_fault
    exit

gp:
    setup
    call    protection_fault
    exit

pf:
    setup
    call    page_fault
    exit

mf:
    setup
    call    fpu_error
    exitnoerror

ac:
    setup
    call    alignment_check
    exitnoerror

mc:
    setup
    call    machine_check
    exitnoerror

xm:
    setup
    call    simd_exception
    exitnoerror

common_interrupt_stub:
    setup

    call    common_interrupt_handler

    exit

%macro  irq_macro   1
global      irq%1

irq%1:
    push    %1

    jmp     common_interrupt_stub
%endmacro

%assign     i   32
%rep        224
    irq_macro   i
%assign     i   i+1
%endrep