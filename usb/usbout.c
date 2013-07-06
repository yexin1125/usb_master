/****************************************************************
 NAME: usbout.c
 Harlan YE
 JUNE.6.2013
 ****************************************************************/
 
#include <string.h>
#include "option.h"
#include <stdlib.h>
#include "2440addr.h"
#include "2440lib.h"
#include "def.h"

#include "2440usb.h"
#include "usbmain.h"
#include "usb.h"
#include "usblib.h"
#include "usbsetup.h"
#include "usbout.h"

#include "u2440mon.h"

extern U8 Data_Prepared_Ready;
//---------------------------------------------------------------------
struct EP3_IMFORMATION_REPORT *Imfo_Report;
struct EP3_PARAMETER_REPOTR	*Para_Report;

struct EP3_IMFORMATION_REPORT Imfo_Report_Table[50];
struct EP3_IMFORMATION_REPORT Imfo_Report_Table_Part1[50];
struct EP3_IMFORMATION_REPORT Imfo_Report_Table_Part2[50];
  
U8 ep3Buf[16];
U8 Cnt_Imfo_Report_NO_CHANGE;
U8 Cnt_Imfo_Report_Three2Two;
U8 State_Report;
//---------------------------------------------------------------------
static void PrintEpoPkt(U8 *pt,int cnt);
static void RdPktEp3_CheckSum(U8 *buf,int num);

#define IDLE				(0x00)
#define SET_PARAMETER		(0x01)
#define ADD_IMFORMATION		(0x02)
#define CLEAR_ALL			(0x03)
#define SEND_ALL			(0x04)
 

#define CLR_EP3_OUT_PKT_READY() rOUT_CSR1_REG= ( out_csr3 &(~ EPO_WR_BITS)\
					&(~EPO_OUT_PKT_READY) ) 
#define SET_EP3_SEND_STALL()	rOUT_CSR1_REG= ( out_csr3 & (~EPO_WR_BITS)\
					| EPO_SEND_STALL) )
#define CLR_EP3_SENT_STALL()	rOUT_CSR1_REG= ( out_csr3 & (~EPO_WR_BITS)\
					&(~EPO_SENT_STALL) )
#define FLUSH_EP3_FIFO() 	rOUT_CSR1_REG= ( out_csr3 & (~EPO_WR_BITS)\
					|EPO_FIFO_FLUSH) )

//#define DEBUG_RE_NO_CHANGE
//#define DEBUG_SEND_PARA
//#define DEBUG_FPGA_ACHIEVE
//#define DEBUG_RE_THREE2TWO

U8 Imformation_Configure;
static U8 tempBuf[64+1];
U8 Sending_State;

void Send_All_Imformation();

void Ep3Handler(void)
{ 
   	U8 j;
   	U8 *p = ep3Buf;
    U8 out_csr3; 
    U8 State_Report;
    rINDEX_REG=3;
	RdPktEp3(p,16);
	
	
	switch(ep3Buf[6])
	{
		case PARAMETER_REPORT:
		
			Para_Report->L_Circletime = ep3Buf[0];
			Para_Report->H_Circletime = ep3Buf[1];
			Para_Report->L_Accuracy = ep3Buf[2];
			Para_Report->H_Accuracy = ep3Buf[3];
			Para_Report->Time_Unit = ep3Buf[5];
			Para_Report->Voltage_Level = ep3Buf[4];
		
			if(Para_Report->Voltage_Level == 0x02)
				Para_Report->Voltage_Level = 0x00;
			else if(Para_Report->Voltage_Level == 0x03)
				Para_Report->Voltage_Level = 0x01;
				
			CLR_EP3_OUT_PKT_READY();
			State_Report = SET_PARAMETER;	
		break;
		
		case IMFORMATION_REPORT:
			 State_Report = ADD_IMFORMATION;
			 CLR_EP3_OUT_PKT_READY();
		break;
		
		case CLEAR_IMFORMATION:
			 State_Report = CLEAR_ALL;
		 break;
		
		case END_CONFIGURED:
			 State_Report = SEND_ALL;

		break;	
		
		default:
		break;	
	}
	
	switch(State_Report)
	{
		case IDLE:
		break;
		
		case SET_PARAMETER:
			if(Para_Report->Voltage_Level == TWO_STEP)
			{
				Imformation_Configure = NO_CHANGE;
			}
			else if(Para_Report->Voltage_Level == THREE_STEP)
			{
				Imformation_Configure = Three2Two;
			}
		break;
		
		case ADD_IMFORMATION:
			if(Imformation_Configure == NO_CHANGE)
			{
				
				Imfo_Report_Table[Cnt_Imfo_Report_NO_CHANGE].L_Duration = ep3Buf[0];
				Imfo_Report_Table[Cnt_Imfo_Report_NO_CHANGE].H_Duration = ep3Buf[1];
				Imfo_Report_Table[Cnt_Imfo_Report_NO_CHANGE].Voltage_Value = ep3Buf[2];
				
				Cnt_Imfo_Report_NO_CHANGE++;
			}
			else if(Imformation_Configure == Three2Two)
			{
				if(ep3Buf[2]==MID_VOLTAGE)
				{
					Imfo_Report_Table_Part1[Cnt_Imfo_Report_Three2Two].L_Duration = ep3Buf[0];
					Imfo_Report_Table_Part1[Cnt_Imfo_Report_Three2Two].H_Duration = ep3Buf[1];
					Imfo_Report_Table_Part1[Cnt_Imfo_Report_Three2Two].Voltage_Value = LOW_VOLTAGE;
					
					Imfo_Report_Table_Part2[Cnt_Imfo_Report_Three2Two].L_Duration = ep3Buf[0];
					Imfo_Report_Table_Part2[Cnt_Imfo_Report_Three2Two].H_Duration = ep3Buf[1];
					Imfo_Report_Table_Part2[Cnt_Imfo_Report_Three2Two].Voltage_Value = HIGH_VOLTAGE;
				}
				else
				{
					Imfo_Report_Table_Part1[Cnt_Imfo_Report_Three2Two].L_Duration = ep3Buf[0];
					Imfo_Report_Table_Part1[Cnt_Imfo_Report_Three2Two].H_Duration = ep3Buf[1];
					Imfo_Report_Table_Part1[Cnt_Imfo_Report_Three2Two].Voltage_Value = ep3Buf[2];
					
					Imfo_Report_Table_Part2[Cnt_Imfo_Report_Three2Two].L_Duration = ep3Buf[0];
					Imfo_Report_Table_Part2[Cnt_Imfo_Report_Three2Two].H_Duration = ep3Buf[1];
					Imfo_Report_Table_Part2[Cnt_Imfo_Report_Three2Two].Voltage_Value = ep3Buf[2];			
				}
				
		#ifdef DEBUG_RE_THREE2TWO
					rINDEX_REG=1;			
					rEP1_FIFO = Imfo_Report_Table_Part1[Cnt_Imfo_Report_Three2Two].L_Duration;
					rEP1_FIFO = Imfo_Report_Table_Part1[Cnt_Imfo_Report_Three2Two].H_Duration;
					rEP1_FIFO = Imfo_Report_Table_Part1[Cnt_Imfo_Report_Three2Two].Voltage_Value;
					
					rEP1_FIFO = Imfo_Report_Table_Part2[Cnt_Imfo_Report_Three2Two].L_Duration;
					rEP1_FIFO = Imfo_Report_Table_Part2[Cnt_Imfo_Report_Three2Two].H_Duration;
					rEP1_FIFO = Imfo_Report_Table_Part2[Cnt_Imfo_Report_Three2Two].Voltage_Value;
										
					rIN_CSR1_REG= ( rIN_CSR1_REG &(~EPI_WR_BITS) | EPI_IN_PKT_READY);
		#endif
				
				Cnt_Imfo_Report_Three2Two++;
			}
			
			
		break;
		
		case CLEAR_ALL:
			Cnt_Imfo_Report_NO_CHANGE = 0;
			Cnt_Imfo_Report_Three2Two = 0;
			
			for(j=0;j<50;j++)
			{
				Imfo_Report_Table[j].L_Duration = 0x00;
				Imfo_Report_Table[j].H_Duration = 0x00;
				Imfo_Report_Table[j].Voltage_Value = 0x00;
				
				Imfo_Report_Table_Part1[j].Voltage_Value = 0x00;
				Imfo_Report_Table_Part1[j].L_Duration = 0x00;
				Imfo_Report_Table_Part1[j].H_Duration = 0x00;
				
				Imfo_Report_Table_Part2[j].L_Duration = 0x00;
				Imfo_Report_Table_Part2[j].H_Duration = 0x00;
				Imfo_Report_Table_Part2[j].Voltage_Value = 0x00;
			}
			//memset(Imfo_Report_Table,0,50);
			//memset(Imfo_Report_Table_Part1,0,50);
			//memset(Imfo_Report_Table_Part2,0,50);
		break;
		
		case SEND_ALL:
			//Sending_State = SEND_PARA;
			Send_All_Imformation();
			Cnt_Imfo_Report_NO_CHANGE = 0;
			Cnt_Imfo_Report_Three2Two = 0;			
		break;
		
	 	default:
		break;
	}
}

void Send_All_Imformation()
{
	U8 i;	

#ifdef DEBUG_SEND_PARA	
	rINDEX_REG=1;
	rEP1_FIFO = (Para_Report->L_Circletime);
	rEP1_FIFO = (Para_Report->H_Circletime);
	rEP1_FIFO = (Para_Report->L_Accuracy);
	rEP1_FIFO = (Para_Report->H_Accuracy);
	rEP1_FIFO = (Para_Report->Time_Unit);
	rEP1_FIFO = (Para_Report->Voltage_Level);
	rIN_CSR1_REG= ( rIN_CSR1_REG &(~ EPI_WR_BITS) | EPI_IN_PKT_READY );													
#endif
#ifdef DEBUG_RE_NO_CHANGE
	rINDEX_REG=1;			
	rEP1_FIFO = Imfo_Report_Table[0].L_Duration;
	rEP1_FIFO = Imfo_Report_Table[0].H_Duration;
	rEP1_FIFO = Imfo_Report_Table[0].Voltage_Value;	
	
	rEP1_FIFO = Imfo_Report_Table[1].L_Duration;
	rEP1_FIFO = Imfo_Report_Table[1].H_Duration;
	rEP1_FIFO = Imfo_Report_Table[1].Voltage_Value;		
					
	rIN_CSR1_REG= ( rIN_CSR1_REG &(~EPI_WR_BITS) | EPI_IN_PKT_READY);	
#endif


#ifdef DEBUG_FPGA_ACHIEVE

	rGPHDAT = (0x01<<9) | (0x01<<5);	//para_en signal enable ,clk posedge
			
	rGPJDAT = (0x01) | (0x02) |
			  (0x04) | (0x08) |
			  (0x10) | ((0x20)<<2) |
			  ((0x40)<<2);
			//L_Circletime[0]\...\[6]
			
	rGPCDAT = ((0x80)>>7) | ((0x01)<<5) |
			  ((0x02)<<5) | (0x01<<8); 
			//L_Circletime[7]|H_Circletime[0]|H_Circletime[1]|Voltage_Level
			
	rGPEDAT = ((0x02)<<6) |
			  ((0x08)<<6);
			//L_Accuracy[0]\....\L_Accuracy[4]
		
	rGPGDAT = ((0x20)>>4) |
			  ((0x80)>>1) |
			  ((0x02)<<10);
			//L_Accuracy[5]\....\L_Accuracy[7]\H_Accuracy[0]\H_Accuracy[1]
		
	rGPBDAT = ((0x01)<<3) | ((0x02)<<3) |
			  ((0x01)<<5);
			//Time_Unit[0]|Time_Unit[1]|Report_Type
#else


	rGPHDAT = (1<<9) | (1<<5);	//para_en signal enable ,clk posedge
			
	rGPJDAT = ((Para_Report->L_Circletime)&0x01) | ((Para_Report->L_Circletime)&0x02) |
			  ((Para_Report->L_Circletime)&0x04) | ((Para_Report->L_Circletime)&0x08) |
			  ((Para_Report->L_Circletime)&0x10) | (((Para_Report->L_Circletime)&0x20)<<2) |
			  (((Para_Report->L_Circletime)&0x40)<<2);
			//L_Circletime[0]\...\[6]
			
	rGPCDAT = (((Para_Report->L_Circletime)&0x80)>>7) | (((Para_Report->H_Circletime)&0x01)<<5) |
					  (((Para_Report->H_Circletime)&0x02)<<5) | (((Para_Report->Voltage_Level)&0x01)<<7); 
			//L_Circletime[7]|H_Circletime[0]|H_Circletime[1]|Voltage_Level
			
	rGPEDAT = (((Para_Report->L_Accuracy)&0x01)<<1) | (((Para_Report->L_Accuracy)&0x02)<<6) |
			(((Para_Report->L_Accuracy)&0x04)<<6) | (((Para_Report->L_Accuracy)&0x08)<<6) |
			(((Para_Report->L_Accuracy)&0x10)<<6);
			//L_Accuracy[0]\....\L_Accuracy[4]
		
	rGPGDAT = (((Para_Report->L_Accuracy)&0x20)>>4) | (((Para_Report->L_Accuracy)&0x40)>>3) |
			(((Para_Report->L_Accuracy)&0x80)>>1) | (((Para_Report->H_Accuracy)&0x01)<<8) |
			(((Para_Report->H_Accuracy)&0x02)<<10);
			//L_Accuracy[5]\....\L_Accuracy[7]\H_Accuracy[0]\H_Accuracy[1]
		
	rGPBDAT = (((Para_Report->Time_Unit)&0x01)<<3) | (((Para_Report->Time_Unit)&0x02)<<3) |
			(((Para_Report->Report_Type)&0x01)<<5);
			//Time_Unit[0]|Time_Unit[1]|Report_Type
#endif
			

			
	Delay(500);
	rGPHDAT = rGPHDAT & ~(1<<5);	
	Delay(500);
	rGPHDAT = rGPHDAT & ~(1<<9);
	
	if(Imformation_Configure == NO_CHANGE)
	{
		rGPFDAT = 0xFE;
		for(i=0;i<Cnt_Imfo_Report_NO_CHANGE;i++)
		{
			rGPHDAT = (1<<5) | 1;   //clk posedge,info_en signal enable
			
			rGPBDAT = (((Imfo_Report_Table[i].L_Duration)&0x01)<<3) | (((Imfo_Report_Table[i].L_Duration)&0x02)<<3) |
					  (((Imfo_Report_Table[i].L_Duration)&0x04)<<3) | (((Imfo_Report_Table[i].L_Duration)&0x08)<<3) |
					  (((Imfo_Report_Table[i].L_Duration)&0x10)<<3) | (((Imfo_Report_Table[i].L_Duration)&0x20)<<3) |
					  (((Imfo_Report_Table[i].L_Duration)&0x40)<<3) | (((Imfo_Report_Table[i].L_Duration)&0x80)<<3);
			//L_Duration[0]\....\L_Duration[7]
			
			rGPJDAT = ((Imfo_Report_Table[i].H_Duration)&0x01) | ((Imfo_Report_Table[i].H_Duration)&0x02);
			//H_Duration[0]\....\H_Duration[1]
			
			rGPCDAT = ((Imfo_Report_Table[i].Voltage_Value)&0x01) | (((Imfo_Report_Table[i].Voltage_Value)&0x02)<<4);
			//Voltage_Value[0]|Voltage_Value[1]
			
			rGPGDAT = ((Cnt_Imfo_Report_NO_CHANGE&0x10)>>3) | (Cnt_Imfo_Report_NO_CHANGE&0x08) |
					  ((Cnt_Imfo_Report_NO_CHANGE&0x04)<<4) | ((Cnt_Imfo_Report_NO_CHANGE&0x02)<<7) |
					  ((Cnt_Imfo_Report_NO_CHANGE&0x01)<<11);
			
			Delay(500);
			rGPHDAT = rGPHDAT & (~(1<<5));
			Delay(500);
		}
		rGPHDAT = rGPHDAT & ~1;
		
	}
	else if(Imformation_Configure = Three2Two)
	{
		rGPFDAT = 0xFD;
		for(i=0;i<Cnt_Imfo_Report_Three2Two;i++)
		{
			rGPHDAT = (1<<5) | 1;   //clk posedge,info_en signal enable
			
			rGPBDAT = (((Imfo_Report_Table_Part1[i].L_Duration)&0x01)<<3) | (((Imfo_Report_Table_Part1[i].L_Duration)&0x02)<<3) |
					  (((Imfo_Report_Table_Part1[i].L_Duration)&0x04)<<3) | (((Imfo_Report_Table_Part1[i].L_Duration)&0x08)<<3) |
					  (((Imfo_Report_Table_Part1[i].L_Duration)&0x10)<<3) | (((Imfo_Report_Table_Part1[i].L_Duration)&0x20)<<3) |
					  (((Imfo_Report_Table_Part1[i].L_Duration)&0x40)<<3) | (((Imfo_Report_Table_Part1[i].L_Duration)&0x80)<<3);
			//part1_L_Duration[0]\....\part1_L_Duration[7]
			
			rGPJDAT = ((Imfo_Report_Table_Part1[i].H_Duration)&0x01) | ((Imfo_Report_Table_Part1[i].H_Duration)&0x02) |
					  (((Imfo_Report_Table_Part2[i].L_Duration)&0x01)<<2) | (((Imfo_Report_Table_Part2[i].L_Duration)&0x02)<<2) |
					  (((Imfo_Report_Table_Part2[i].L_Duration)&0x04)<<2) | (((Imfo_Report_Table_Part2[i].L_Duration)&0x08)<<4) |
					  (((Imfo_Report_Table_Part2[i].L_Duration)&0x10)<<4);
			//part1_H_Duration[0]|H_Duration[1] | part2L_Duration[0]\....\part2L_Duration[4]		  
					  
			rGPCDAT = ((Imfo_Report_Table_Part1[i].Voltage_Value)&0x01)	| (((Imfo_Report_Table_Part1[i].Voltage_Value)&0x02)<<4) |
					  (((Imfo_Report_Table_Part2[i].L_Duration)&0x20)<<1) | (((Imfo_Report_Table_Part2[i].L_Duration)&0x40)<<1);
			//part1_Voltage_Value[0]|part1_Voltage_Value[1] |part2L_Duration[5]|part2L_Duration[6]	  
	
			rGPEDAT = (((Imfo_Report_Table_Part2[i].L_Duration)&0x80)>>6) | (((Imfo_Report_Table_Part2[i].H_Duration)&0x01)<<7) |
					  (((Imfo_Report_Table_Part2[i].H_Duration)&0x02)<<7) | (((Imfo_Report_Table_Part2[i].Voltage_Value)&0x01)<<9) | 
					  (((Imfo_Report_Table_Part2[i].Voltage_Value)&0x02)<<9);
			//part2L_Duration[7] | part2H_Duration[0] | part2H_Duration[1]|part2_Voltage_Value[0]|part2_Voltage_Value[1]		  
			
			 rGPGDAT = ((Cnt_Imfo_Report_Three2Two&0x10)>>3) | (Cnt_Imfo_Report_Three2Two&0x08) |
					   ((Cnt_Imfo_Report_Three2Two&0x04)<<4) | ((Cnt_Imfo_Report_Three2Two&0x02)<<7) |
					   ((Cnt_Imfo_Report_Three2Two&0x01)<<11);
			 
			Delay(500);
			rGPHDAT = rGPHDAT & (~(1<<5));
			Delay(500);			
		}
		rGPHDAT = rGPHDAT & ~1;
	}
}

//------------------------------------------------------------------------
void ClearEp3OutPktReady(void)
{
    U8 out_csr3;
    rINDEX_REG=3;
    out_csr3=rOUT_CSR1_REG;
    CLR_EP3_OUT_PKT_READY();
}
