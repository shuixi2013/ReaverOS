;
; ReaverOS
; loader/stage2/vbe.asm
; VESA BIOS Extensions video modes support.
;

;
; Reaver Project OS, Rose License
;
; Copyright (C) 2011-2012 Reaver Project Team:
; 1. Michał "Griwes" Dominiak
; 
; This software is provided 'as-is', without any express or implied
; warranty. In no event will the authors be held liable for any damages
; arising from the use of this software.
; 
; Permission is granted to anyone to use this software for any purpose,
; including commercial applications, adn to alter it and redistribute it
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

bits    16

highestx:           dw 0
highesty:           dw 0
depth:              db 0
modenumber:         dw 0

vbe_controller_info:
    .signature:     dd 0
    .version:       dw 0
    .oemstringoff:  dw 0        ; far ptr 
    .oemstringseg:  dw 0
    .capabilities:  dd 0
    .videomodesoff: dw 0
    .videomodesseg: dw 0
    .totalmemory:   dw 0
    times 492       db 0

video_mode_description:
    .modeatrrib:    dw 0
    .winaatrrib:    db 0
    .winbatrrib:    db 0
    .wingran:       dw 0
    .winsize:       dw 0
    .winasegment:   dw 0
    .winbsegment:   dw 0
    .winfuncoff:    dw 0
    .winfuncseg:    dw 0
    .bytesperscanl: dw 0
    .xres:          dw 0
    .yres:          dw 0
    .xcharsize:     db 0
    .ycharsize:     db 0
    .planes:        db 0
    .bpp:           db 0
    .banks:         db 0
    .memorymodel:   db 0
    .banksize:      db 0
    .imagepages:    db 0
    .reserved1:     db 1
    .redmasksize:   db 0
    .redmaskpos:    db 0
    .greenmasksize: db 0
    .greenmaskpos:  db 0
    .bluemasksize:  db 0
    .bluemaskpos:   db 0
    .rsvdmasksize:  db 0
    .rsvdmaskpos:   db 0
    .colormodeinfo: db 0
    .physbaseoff:   dw 0
    .physbaseseg:   dw 0
    .reserved2:     dd 0
    .reserved3:     dw 0
    .linbytes:      dw 0
    .banknumber:    db 0
    .linnumber:     db 0
    .linredmasks:   db 0
    .linredmaskp:   db 0
    .lingreenmasks: db 0
    .lingreenmaskp: db 0
    .linbluemasks:  db 0
    .linbluemaskp:  db 0
    .linrsvdmasks:  db 0
    .linrsvdmaskp:  db 0
    .maxpixelclock: db 0
    .reserved4:     times 189 db 0

;
; get_controller_info()
; es:di - buffer for VBE Controller Info structure
;

get_controller_info:
    mov     ax, 0x4f00
    mov     di, vbe_controller_info
    int     0x10

    cmp     al, 0x4f
    jne     .not_supported

    cmp     ah, 0
    jne     .not_supported

    cmp     byte [vbe_controller_info.signature], 'V'
    jne     .not_supported
    cmp     byte [vbe_controller_info.signature + 1], 'E'
    jne     .not_supported
    cmp     byte [vbe_controller_info.signature + 2], 'S'
    jne     .not_supported
    cmp     byte [vbe_controller_info.signature + 3], 'A'
    jne     .not_supported

    ret
    
    .not_supported:
        xor     eax, eax
    
        mov     byte [enablevbe], 0
        ret

;
; get_mode_info()
; Loads given mode's info into mode info structure.
; ax - mode number
;

get_mode_info:
    mov     cx, ax
    mov     di, video_mode_description

    mov     ax, 0x4f01

    int     0x10

    cmp     al, 0x4f
    jne     .fail

    cmp     ah, 0
    jne     .fail

    ret

    .fail:
        xor     eax, eax

        ret

;
; setup_video_mode()
; Setups video mode, using mode with highest resolution available.
;

setup_video_mode:
    mov     di, vbe_controller_info
    call    get_controller_info

    cmp     eax, 0
    je      .fail

    xor     edx, edx
    
    .loop:
        xor     eax, eax
        xor     ebx, ebx

        push    es
        mov     ax, [vbe_controller_info.videomodesseg]
        mov     es, ax
        mov     ax, [es:vbe_controller_info.videomodesoff + 2 * edx]
        pop     es

        cmp     ax, 0xffff
        je      .selected

        mov     word [modenumber], ax

        call    get_mode_info

        cmp     eax, 0
        je      .advance

        mov     eax, [video_mode_description.modeatrrib]

        and     eax, 0x90
        cmp     eax, 0x90
        jne      .advance

        cmp     byte [video_mode_description.memorymodel], 4
        jne     .advance

        cmp     byte [video_mode_description.planes], 1
        jne     .advance

        mov     al, byte [depth]
        cmp     byte [video_mode_description.bpp], al
        jl      .advance

        mov     ax, word [highestx]
        cmp     word [video_mode_description.xres], ax
        jle     .advance

        mov     ax, word [highesty]
        cmp     word [video_mode_description.yres], ax
        jle     .advance

    .advance:
        inc     edx

        jmp     .loop

    .selected:
        cmp     word [modenumber], 0
        je      .failset

        mov     bx, word [modenumber]

        xor     eax, eax
        
        ; set only bit D14 and D0-D8 (use linar frame buffer mode)
        and     bx, 0100000111111111b
        or      bx, 0100000000000000b

        mov     ax, 0x4f02
        int     0x10

        cmp     ah, 0
        jne     .failset

        push    es
        xor     ax, ax
        mov     es, ax
        mov     di, 0x6c00
        call    get_bios_vga_font
        pop     es

        ret
    
    .failset:
        mov     byte [enablevbe], 0

    .fail:
        xor     eax, eax

        ret

;
; get_bios_vga_font()
; es:di - buffer to load the font to
;

get_bios_vga_font:
    push    ds
    
    mov     ax, 0x1130
    mov     bh, 6
        
    int     0x10

    push    es
    pop     ds

    mov     si, bp
    mov     cx, 256 * 16 / 4

    rep     movsd
    pop     ds

    ret