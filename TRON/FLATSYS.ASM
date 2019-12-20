;
;  Flat-realmode system, based on code by by Ed Beroset
;

.model tiny
.code
.386P

SegReg dw ?

DESC386 STRUC
	limlo   dw      ?
	baselo  dw      ?
	basemid db      ?
	dpltype db      ?       ; p(1) dpl(2) s(1) cd(1) ec(1) rw(1) a(1)
	limhi   db      ?       ; g(1) d/b(1) 0(1) avl(1) lim(4)
	basehi  db      ?
DESC386 ENDS

;
; DPL flags:
; P = Present, DPL = PL 0-3, S = 1
; CD = Code/Data (1=Code), EC = Expand Up/Down (1=down, must be 0 for Code)
; RW = RO/RW access (0=RO, 1=RW .. code must be 1 (?))
; A = whether program has accessed this selector

;;;;;;

;;
;; This little function will turn the world inside-out
;;

_FRM_Init proc
PUBLIC _FRM_Init

pushad

jmp skipsys

;;;; Declare the mini-GDT for our brief stay in protected mode ;;;;;
GDT     DESC386 <GDT_END - GDT - 1, GDT, 0, 0, 0, 0>  ; the GDT itself
	DESC386 <0ffffh, 0, 0, 093h, 0cfh, 0>          ; 4G data segment
GDT_END:

skipsys:

	; first, calculate the linear address of GDT
	xor     edx,edx         ; clear edx
	xor     eax,eax         ; clear eax
	mov     dx,ds           ; get the data segment
	shl     edx,4           ; shift it over a bit
	add     dword ptr [gdt+2],edx   ; store as GDT linear base addr

	; now load the GDT into the GDTR
	lgdt    fword ptr gdt   ; load GDT base (286-style 24-bit load)
	mov     bx,1 * size DESC386 ; point to first descriptor
	mov     eax,cr0         ; get the control register
	or      al,1            ; flip the PE bit in our copy
	cli                     ; turn off interrupts (or we'll die)
	mov     cr0,eax         ; ..and this is what the Devil does
	mov     fs,bx
	and     al,0FEh         ; clear the PE bit again
	mov     cr0,eax         ; back to real mode

	mov	SegReg,fs

	sti                     ; now it's safe to reenable the interrupts


	popad
	ret                     ;
_FRM_Init endp


;;
;;  Copy block to FRMspace
;;

PROC _FRM_CopyChunk
PUBLIC _FRM_CopyChunk

push bp
mov bp,sp
ARG dest:dword,source:dword,len:word
pushad

; Set up the copy loop
mov cx,len
shr cx,1	; divide by 2

;xor ax,ax
;mov fs,ax
mov fs,SegReg

lds si,source
mov edi,dest
;mov edi,0b8000h ; screen - for testing

copyloop:
lodsw
mov fs:[edi],ax
inc edi
inc edi
loop copyloop

popad
pop bp
ret
ENDP _FRM_CopyChunk

;;
;;  Retrieve a single sample from FRMspace
;;

PROC _FRM_GetSample
PUBLIC _FRM_GetSample
push bp
mov bp,sp
ARG source:dword
push fs
push edi

;xor ax,ax
;mov fs,ax
mov fs,SegReg

mov edi,source
mov ax,fs:[edi]
pop edi
pop fs
pop bp
ret
ENDP _FRM_GetSample


end

