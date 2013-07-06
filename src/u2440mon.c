/****************************************************************
 NAME: u2440mon.c
 Harlan YE
 2013, JUNE, 7
 ****************************************************************/
#define	GLOBAL_CLK		1

#include <stdlib.h>
#include <string.h>
#include "def.h"
#include "option.h"
#include "2440addr.h"
#include "2440lib.h"
#include "2440slib.h"
#include "mmu.h"
#include "profile.h"
#include "memtest.h"

#include "usb.h"
#include "usbmain.h"
#include "usbout.h"
#include "usblib.h"
#include "2440usb.h"

#include "norflash.h"

void Isr_Init(void);
void HaltUndef(void);
void HaltSwi(void);
void HaltPabort(void);
void HaltDabort(void);
void Lcd_Off(void);

void Menu(void);
void ClearMemory(void);
static int BoardUsbDownload(U32 addr, U32 run);
static int WaitDownload(void);

void Clk0_Enable(int clock_sel);	
void Clk1_Enable(int clock_sel);
void Clk0_Disable(void);
void Clk1_Disable(void);

volatile U32 downloadAddress;

void (*restart)(void)=(void (*)(void))0x0;

void __irq EintHandler(void);
volatile unsigned char *downPt;
volatile U32 downloadFileSize;
volatile U16 checkSum;
volatile unsigned int err=0;
volatile U32 totalDmaCount;

volatile int isUsbdSetConfiguration;

int download_run=0;
U32 tempDownloadAddress;
int menuUsed=0;

extern char Image$$RW$$Limit[];
U32 *pMagicNum=(U32 *)Image$$RW$$Limit;
int consoleNum;
/*************************************************************/
U8 eint5;

#include "bootpara.h"

void LcdDisplay(void);
int find_camera(void);
void Led_Test(void);
void comdownload(void);
U32 GetFlashID(void);
int SectorProg(U32 begin, U16 *data, U32 size);
int RelocateNKBIN(U32 img_src, U32 *pStart, U32 *pLength, U32 *pLaunch);

void NandErase(void);
void NandWrite(void);
void NandLoadRun(void);
void NandLoadRun_wince(void);

unsigned int uiKey;
#define	printf	Uart_Printf

#define LED1ON    0xFE 
#define LED2ON    0xFD
#define LED3ON    0xFB
#define LED4ON    0xF7
#define LEDOFF    0xFF 

U8 buf[8];

void __irq EintHandler(void)
{
    
	if(rINTPND==BIT_EINT4_7)
	{
		
		ClearPending(BIT_EINT4_7);
		
		if(rEINTPEND&(1<<4))
		{
			rEINTPEND |= 1<< 4;
			Uart_Printf("eint 4\n");
			rGPFDAT = LED1ON;
			Delay(500);
			rGPFDAT = LEDOFF;
			
		}
		if(rEINTPEND&(1<<5))
		{
			rEINTPEND |= 1<< 5;
			//Uart_Printf("eint 5\n");
			eint5 = 1;
/*		rGPFDAT = LED2ON;
			Delay(500);
			rGPFDAT = LEDOFF;*/
						
		}
		if(rEINTPEND&(1<<6))
		{
			Uart_Printf("eint 6\n");
			rGPFDAT = LED3ON;
			Delay(500);
			rGPFDAT = LEDOFF;
			rEINTPEND |= 1<< 6;
		}
		if(rEINTPEND&(1<<7))
		{
			Uart_Printf("eint 7\n");
			rGPFDAT = LED4ON;
			Delay(500);
			rGPFDAT = LEDOFF;
			rEINTPEND |= 1<< 7;
		}
	}

}


static U32 cpu_freq;
static U32 UPLL;
static void cal_cpu_bus_clk(void)
{
	U32 val;
	U8 m, p, s;
	
	val = rMPLLCON;
	m = (val>>12)&0xff;
	p = (val>>4)&0x3f;
	s = val&3;

	//(m+8)*FIN*2 不要超出32位数!
	FCLK = ((m+8)*(FIN/100)*2)/((p+2)*(1<<s))*100;
	
	val = rCLKDIVN;
	m = (val>>1)&3;
	p = val&1;	
	val = rCAMDIVN;
	s = val>>8;
	
	switch (m) {
	case 0:
		HCLK = FCLK;
		break;
	case 1:
		HCLK = FCLK>>1;
		break;
	case 2:
		if(s&2)
			HCLK = FCLK>>3;
		else
			HCLK = FCLK>>2;
		break;
	case 3:
		if(s&1)
			HCLK = FCLK/6;
		else
			HCLK = FCLK/3;
		break;
	}
	
	if(p)
		PCLK = HCLK>>1;
	else
		PCLK = HCLK;
	
	if(s&0x10)
		cpu_freq = HCLK;
	else
		cpu_freq = FCLK;
		
	val = rUPLLCON;
	m = (val>>12)&0xff;
	p = (val>>4)&0x3f;
	s = val&3;
	UPLL = ((m+8)*FIN)/((p+2)*(1<<s));
	if(UPLL==96*MEGA)
		rCLKDIVN |= 8;	//UCLK=UPLL/2
	UCLK = (rCLKDIVN&8)?(UPLL>>1):UPLL;
}


void Port_Initial(void)
{
	rGPBCON = (0x01<<6)|(0x01<<8)|(0x01<<10)|(0x01<<12)|(0x01<<14)|(0x01<<16)|(0x01<<18)|(0x01<<20);
	//           Time_Unit[0]|Time_Unit[1]|Report_Type  
	//			 L_Duration[0]\....\L_Duration[7]	
	//			 part1_L_Duration[0]\....\part1_L_Duration[7]
	
	rGPCCON = (0x01)|(0x01<<10)|(0x01<<12)|(0x01<<14);
	//           L_Circletime[7]|H_Circletime[0]|H_Circletime[1]|Voltage_Level
	//			 Voltage_Value[0]|Voltage_Value[1] | 
	//			 part1_Voltage_Value[0]|part1_Voltage_Value[1] |part2L_Duration[5]|part2L_Duration[6]
	
	rGPECON = (0x01<<2)|(0x01<<14)|(0x01<<16)|(0x01<<18)|(0x01<<20);
	//			 L_Accuracy[0]\....\L_Accuracy[4]
	//           part2L_Duration[7] | part2H_Duration[0] | part2H_Duration[1]|part2_Voltage_Value[0]|part2_Voltage_Value[1]
	
	rGPGCON = rGPGCON | (0x01<<2)|(0x01<<6)|(0x01<<12)|(0x01<<16)|(0x01<<22);
	//			  L_Accuracy[5]\....\L_Accuracy[7]\H_Accuracy[0]\H_Accuracy[1]
	//			  cnt_infor_report_no_change[4]/..../cnt_infor_report_no_change[0]
	//            Cnt_Imfo_Report_Three2Two[4]/..../Cnt_Imfo_Report_Three2Two[0]
	rGPHCON = (0x01<<10)|(0x01<<18)|(0x01);
	//            CLK     PARA_EN   IMFO_EN 
	
	rGPJCON = (0x01)|(0x01<<2)|(0x01<<4)|(0x01<<6)|(0x01<<8)|(0x01<<14)|(0x01<<16);
	//  		  L_Circletime[0]\[1]\...\[6]
	//			  H_Duration[0]|H_Duration[1]
	//			  part1_H_Duration[0]|H_Duration[1] | part2L_Duration[0]\....\part2L_Duration[4]
}
/*************************************************************/

void Main(void)
{
	char *mode;
	int i,j;
	U8 key;
	U32 mpll_val;	
	key = 14;
	mpll_val = (92<<12)|(1<<4)|(1);
	
	ChangeMPllValue((mpll_val>>12)&0xff, (mpll_val>>4)&0x3f, mpll_val&3);
	ChangeClockDivider(key, 12);//FCLK:HCLK:PCLK=1:4:8
	cal_cpu_bus_clk();			//FCLK=400MHz
	if(PCLK<(40*MEGA))
	 {
	 	ChangeClockDivider(key, 11); //FCLK:HCLK:PCLK=1:4:4
		cal_cpu_bus_clk();
	 }	
	Uart_Init(0,115200);
	Uart_Select(0);//Uart_Select(consoleNum) 默认用串口0，如果用户要用别的串口的话请修改这里
	rMISCCR=rMISCCR&~(1<<3); // USBD is selected instead of USBH1 
	rMISCCR=rMISCCR&~(1<<13); // USB port 1 is enabled.
	//isUsbdSetConfiguration=0;
	MMU_Init();	
	Port_Initial();
	while(1)
	{

		Uart_Printf("yexin_2440_usb_test!\n");		
		UsbdMain();
		MMU_Init();
		pISR_USBD =(unsigned)IsrUsbd;
	 	ClearPending(BIT_USBD); 
		EnableIrq(BIT_USBD);
		
		rGPFCON = (rGPFCON|0xFFFF)&0xFFFFaa55; 	//GPF4-7设置为EINT4-7,GPF0-3为输出
   		rGPFUP &= 0xFF00;                     	//打开上拉功能
   		rGPFDAT |= 0xF; 
   	
		rEXTINT0 &= ~(7<<16 | 7<<20 | 7<<24 | 7<<28);	
		rEXTINT0 |= (2<<16 | 2<<20 | 2<<24 | 2<<28) ;  //设置外部中断4_7下降沿触发	
		rEINTPEND |= (1<<4|1<<5|1<<6|1<<7);							//clear eint 4
		rEINTMASK &= ~(1<<4|1<<5|1<<6|1<<7);						//enable eint 4
		ClearPending(BIT_EINT4_7);
  		pISR_EINT4_7=(unsigned)EintHandler; 	//外部中断4_7中断服务子程序入口地址
		EnableIrq(BIT_EINT4_7);	
				
		while(1)
		{
			if(eint5)
			{
			eint5=0;
			buf[1]= 1;
			rINDEX_REG = 1;
			rIN_CSR1_REG =EPI_WR_BITS;			
			WrPktEp1((U8*)buf,8);
			rIN_CSR1_REG = EPI_IN_PKT_READY;
			}		
		}
	}

}



