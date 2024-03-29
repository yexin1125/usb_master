;===========================================
; NAME: OPTION.A
; DESC: Configuration options for .S files
; HISTORY:
; 02.28.2002: ver 0.0
; 03.11.2003: ver 0.0	attached for 2440.
; jan E, 2004: ver0.03  modified for 2440A01.
;===========================================

;Start address of each stacks,
_STACK_BASEADDRESS	EQU 0x33ff8000
_MMUTT_STARTADDRESS	EQU 0x33ff8000
_ISR_STARTADDRESS	EQU 0x33ffff00

		GBLL 	PLL_ON_START
PLL_ON_START	SETL 	{TRUE}


		GBLL	ENDIAN_CHANGE
ENDIAN_CHANGE	SETL	{FALSE}

		GBLA	ENTRY_BUS_WIDTH
ENTRY_BUS_WIDTH	SETA	16


;BUSWIDTH = 16,32
		GBLA    BUSWIDTH	;max. bus width for the GPIO configuration
BUSWIDTH	SETA    32

		GBLA	UCLK
UCLK	SETA	96000000;48000000

		GBLA	XTAL_SEL
		GBLA	FCLK
		GBLA	CPU_SEL

;(1) Select CPU		
;CPU_SEL	SETA	32440000	; 32440000:2440X.
CPU_SEL	SETA	32440001	; 32440001:2440A

;(2) Select XTaL
XTAL_SEL	SETA	12000000	
;XTAL_SEL	SETA	16934400

;(3) Select FCLK
;FCLK		SETA	296352000
;FCLK		SETA	271500000
;FCLK		SETA	100000000	
FCLK		SETA	240000000	
FCLK		SETA	280000000	
FCLK		SETA	320000000	
FCLK		SETA	360000000	
FCLK		SETA	400000000	

;(4) Select Clock Division (Fclk:Hclk:Pclk)
;CLKDIV_VAL	EQU	5	; 0=1:1:1, 1=1:1:2, 2=1:2:2, 3=1:2:4, 4=1:4:4, 5=1:4:8, 6=1:3:3, 7=1:3:6.

 [ XTAL_SEL = 12000000
 
	[ FCLK = 271500000
CLKDIV_VAL	EQU	7		;1:3:6
M_MDIV		EQU	173		;Fin=12.0MHz Fout=271.5MHz
M_PDIV		EQU	2
		[ CPU_SEL = 32440001
M_SDIV		EQU	2		; 2440A
   		|
M_SDIV		EQU	1		; 2440X
   		]
	]
	
	[ FCLK = 100000000
CLKDIV_VAL	EQU	0		;1:1:1
M_MDIV		EQU	42		;Fin=12.0MHz Fout=100MHz
M_PDIV		EQU	4
		[ CPU_SEL = 32440001
M_SDIV		EQU	1		; 2440A
		|
M_SDIV		EQU	0		; 2440X
		]
	]

	[ FCLK = 240000000
CLKDIV_VAL	EQU	4		;1:4:4
M_MDIV		EQU	112		;Fin=12.0MHz Fout=240MHz
M_PDIV		EQU	4
		[ CPU_SEL = 32440001
M_SDIV		EQU	1		; 2440A
		|
M_SDIV		EQU	0		; 2440X
		]
	]
	
	[ FCLK = 280000000
CLKDIV_VAL	EQU	4		;1:4:4
M_MDIV		EQU	132		;Fin=12.0MHz Fout=280MHz
M_PDIV		EQU	4
		[ CPU_SEL = 32440001
M_SDIV		EQU	1		; 2440A
		|
M_SDIV		EQU	0		; 2440X
		]
	]
	
	[ FCLK = 320000000
CLKDIV_VAL	EQU	5		;1:4:8
M_MDIV		EQU	72		;Fin=12.0MHz Fout=320MHz
M_PDIV		EQU	1
		[ CPU_SEL = 32440001
M_SDIV		EQU	1		; 2440A
		|
M_SDIV		EQU	0		; 2440X
		]
	]
	
	[ FCLK = 360000000
CLKDIV_VAL	EQU	5		;1:4:8
M_MDIV		EQU	82		;Fin=12.0MHz Fout=360MHz
M_PDIV		EQU	1
		[ CPU_SEL = 32440001
M_SDIV		EQU	1		; 2440A
		|
M_SDIV		EQU	0		; 2440X
		]
	]

	[ FCLK = 400000000
CLKDIV_VAL	EQU	5		;1:4:8
M_MDIV		EQU	127		;127
M_PDIV		EQU	2		;2
		[ CPU_SEL = 32440001
M_SDIV		EQU	1		; 2440A
		|
M_SDIV		EQU	0		; 2440X
		]
	]
	
	[ UCLK = 48000000
U_MDIV		EQU	56		;Fin=12.0MHz Fout=48MHz
U_PDIV		EQU	2
		[ CPU_SEL = 32440001
U_SDIV		EQU	2		; 2440A
   		|
U_SDIV		EQU	1		; 2440X
   		]
	]
	[ UCLK = 96000000
U_MDIV		EQU	56		;Fin=12.0MHz Fout=96MHz
U_PDIV		EQU	2
		[ CPU_SEL = 32440001
U_SDIV		EQU	1		; 2440A
   		|
U_SDIV		EQU	0		; 2440X
   		]

	]

  |	; else if XTAL_SEL = 16.9344Mhz

	[ FCLK = 266716800
M_MDIV		EQU	118	;Fin=16.9344MHz
M_PDIV		EQU	2
		[ CPU_SEL = 32440001
M_SDIV		EQU	2		; 2440A
		|
M_SDIV		EQU	1		; 2440X
		]
	]
	
	[ FCLK = 296352000
M_MDIV		EQU	97	;Fin=16.9344MHz
M_PDIV		EQU	1
		[ CPU_SEL = 32440001
M_SDIV		EQU	2		; 2440A
		|
M_SDIV		EQU	1		; 2440X
		]
	]
	[ FCLK = 541900800
M_MDIV		EQU	120	;Fin=16.9344MHz
M_PDIV		EQU	2
		[ CPU_SEL = 32440001
M_SDIV		EQU	1		; 2440A
		|
M_SDIV		EQU	0		; 2440X
		]
	]
	
	[ UCLK = 48000000
U_MDIV		EQU	60	;Fin=16.9344MHz Fout=48MHz
U_PDIV		EQU	4
		[ CPU_SEL = 32440001
U_SDIV		EQU	2		; 2440A
   		|
U_SDIV		EQU	1		; 2440X
   		]
   	]
	[ UCLK = 96000000
U_MDIV		EQU	60	;Fin=16.9344MHz Fout=96MHz
U_PDIV		EQU	4
		[ CPU_SEL = 32440001
U_SDIV		EQU	1		; 2440A
   		|
U_SDIV		EQU	0		; 2440X
   		]
	]
	
   ]	; end of if XTAL_SEL = 12000000.
  



	END

