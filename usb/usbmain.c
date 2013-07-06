/****************************************************************
 NAME: usbmain.c
 DESC: endpoint interrupt handler
       USB init jobs
 HISTORY:
 Mar.25.2002:purnnamu: ported for S3C2410X.
 Mar.27.2002:purnnamu: DMA is enabled.
 ****************************************************************/
#include <string.h>
#include <stdarg.h>
#include "option.h"
#include "2440addr.h"
#include "2440lib.h"
#include "def.h"

#include "2440usb.h"
#include "usbmain.h"
#include "usblib.h"
#include "usbsetup.h"
#include "usbout.h"
#include "usbin.h"

//#define DEBUG_FIFO_RECEIVE
/**************************
    Some PrepareEp1Fifo() should be deleted
 **************************/   

void UsbdMain(void)
{
    int i;
    U8 tmp1;
    U8 oldTmp1=0xff;
    
    
    InitDescriptorTable();
    //ResetUsbd();
    
    rGPGUP  |= 1<<12;		//disable pull-up
	rGPGCON &= ~(3<<24);
	rGPGCON |= 1<<24;		//output	
	Delay(2000);
	rGPGDAT |= 1<<12;		//high    
		
    
    ConfigUsbd();
   //PrepareEp1Fifo(); 

}
 
//----------------------------------------------------------------------------------
void ep_select(U8 endp)
{
	rINDEX_REG = 0x00 + endp;
}

U8 read_ep0_buffer()
{
	U8 read_byte;
	read_byte = rEP0_FIFO;
	
	return read_byte;
	
}
//----------------------------------------------------------------------------------

void __irq IsrUsbd(void)
{
    U8 usbdIntpnd,epIntpnd;
    U8 saveIndexReg=rINDEX_REG;
    usbdIntpnd=rUSB_INT_REG;
    epIntpnd=rEP_INT_REG;
 
    if(usbdIntpnd&SUSPEND_INT)
    {
    	rUSB_INT_REG=SUSPEND_INT;
    	//Uart_Printf("SUSPEND_INT\n");
    	//DbgPrintf( "<SUS]");
    }
    
    if(usbdIntpnd&RESUME_INT)
    {
    	rUSB_INT_REG=RESUME_INT;
    	//Uart_Printf("RESUME_INT\n");
    	//DbgPrintf("<RSM]");
    }
    
    if(usbdIntpnd&RESET_INT)
    {
    	//Uart_Printf("RESET_INT\n");
    	//ResetUsbd();
    	ReconfigUsbd();
    
    	rUSB_INT_REG=RESET_INT;  //RESET_INT should be cleared after ResetUsbd().   	
      // PrepareEp1Fifo(); 
    }

    if(epIntpnd&EP0_INT)
    {
		U8 i, L_fifo_byte;
		rEP_INT_REG=EP0_INT;
		//Uart_Printf("EP0_INT\n"); 
		 
	   	Ep0Handler();
	   	//Ep0Handler_test();
    }
    
    if(epIntpnd&EP1_INT)
    {
   		
     	rEP_INT_REG=EP1_INT;
  // Uart_Printf("EP1_INT\n");  
    	Ep1Handler();
    }

    if(epIntpnd&EP2_INT)
    { 
    	rEP_INT_REG=EP2_INT;  
    	//Uart_Printf("EP2_INT\n");
    	//DbgPrintf("<2:TBD]");   //not implemented yet	
    	//Ep2Handler();
    }

    if(epIntpnd&EP3_INT)
    {
    	   
    	//Uart_Printf("EP3_INT\n");
       	rEP_INT_REG=EP3_INT;
       	
    	Ep3Handler();

    }

    if(epIntpnd&EP4_INT)
   {
    	rEP_INT_REG=EP4_INT;
    	//Uart_Printf("EP4_INT\n");
    	//DbgPrintf("<4:TBD]");   //not implemented yet	
    	//Ep4Handler();
    }

    ClearPending(BIT_USBD);	 
    
    rINDEX_REG=saveIndexReg;
}




/******************* Consol printf for debug *********************/

#define DBGSTR_LENGTH (0x1000)
U8 dbgStrFifo[DBGSTR_LENGTH];
volatile U32 dbgStrRdPt=0;
volatile U32 dbgStrWrPt=0;



void _WrDbgStrFifo(U8 c)
{
    dbgStrFifo[dbgStrWrPt++]=c;
    if(dbgStrWrPt==DBGSTR_LENGTH)dbgStrWrPt=0;

}


int DbgPrintfLoop(void)
{
    if(dbgStrRdPt==dbgStrWrPt)return 0;
    Uart_SendByte(dbgStrFifo[dbgStrRdPt++]);

    if(dbgStrRdPt==DBGSTR_LENGTH)dbgStrRdPt=0;
    return 1;
}


#if 0
void DbgPrintf(char *fmt,...)
{
    int i,slen;
    va_list ap;
    char string[256];

    va_start(ap,fmt);
    vsprintf(string,fmt,ap);
    
    slen=strlen(string);
    
    for(i=0;i<slen;i++)
    	_WrDbgStrFifo(string[i]);
    
    va_end(ap);
}
#else
void DbgPrintf(char *fmt,...)
{
}
#endif


