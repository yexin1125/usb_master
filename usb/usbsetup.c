/**************************************************************
 NAME: usbsetup.c
 DESC: process the USB setup stage operations.
 HISTORY:
 MAR.25.2002:purnnamu: S3C2400X usbsetup.c is ported for S3C2410X.
 AUG.20.2002:purnnamu: rEP0_CSR should be used instead of rOUT_CSR1_REG for EP0 macros.
 **************************************************************/

#include <string.h>
#include "option.h"
#include "2440addr.h"
#include "2440lib.h"
#include "def.h"


#include "2440usb.h"
#include "usbmain.h"
#include "usb.h"
#include "usbin.h"
#include "usblib.h"
#include "usbsetup.h"

// *** End point information ***
//   EP0: control
//   EP1: bulk in end point
//   EP2: not used
//   EP3: bulk out end point
//   EP4: not used

// *** VERY IMPORTANT NOTE ***
// Every descriptor size of EP0 should be 8n+m(m=1~7).
// Otherwise, USB will not operate normally because the program
// doesn't prepare the case that the descriptor size is 8n+0.
// If the size of a descriptor is 8n, the 0 length packit should be sent. 
// Special thanks to E.S.Choi for reminding me of this USB specification.


// ===================================================================
// All following commands will operate only in case 
// - ep0_csr is valid.
// ===================================================================
#define CLR_EP0_OUT_PKT_RDY() 		rEP0_CSR=( ep0_csr & (~EP0_WR_BITS)| EP0_SERVICED_OUT_PKT_RDY )	 
#define CLR_EP0_OUTPKTRDY_DATAEND() 	rEP0_CSR=( ep0_csr & (~EP0_WR_BITS)| (EP0_SERVICED_OUT_PKT_RDY|EP0_DATA_END) )	 
					
#define SET_EP0_IN_PKT_RDY() 		rEP0_CSR=( ep0_csr & (~EP0_WR_BITS)| (EP0_IN_PKT_READY) )	 
#define SET_EP0_INPKTRDY_DATAEND() 	rEP0_CSR=( ep0_csr & (~EP0_WR_BITS)| (EP0_IN_PKT_READY|EP0_DATA_END) )	 
					
#define CLR_EP0_SETUP_END() 		rEP0_CSR=( ep0_csr & (~EP0_WR_BITS)| (EP0_SERVICED_SETUP_END) )

#define CLR_EP0_SENT_STALL() 		rEP0_CSR=( ep0_csr & (~EP0_WR_BITS)& (~EP0_SENT_STALL) )

#define FLUSH_EP0_FIFO() 	{while(rOUT_FIFO_CNT1_REG)rEP0_FIFO;}


U32 ep0State;
U32 ep0SubState;
U8 Buf_Idle[]={0x00};
extern volatile int isUsbdSetConfiguration;
volatile U8 Rwuen;
volatile U8 Configuration=1;
volatile U8 AlterSetting;
volatile U8 Selfpwr=TRUE;
volatile U8 device_status;
volatile U8 interface_status;
volatile U8 endpoint0_status;
volatile U8 endpoint1_status;
volatile U8 endpoint3_status;

struct USB_SETUP_DATA descSetup;
struct USB_DEVICE_DESCRIPTOR descDev;
struct USB_CONFIGURATION_DESCRIPTOR descConf;
struct USB_HID_DESCRIPTOR descHID;
struct USB_INTERFACE_DESCRIPTOR descIf;
struct USB_ENDPOINT_DESCRIPTOR descEndpt1;
struct USB_ENDPOINT_DESCRIPTOR descEndpt3;

struct USB_CONFIGURATION_SET ConfigSet;
struct USB_INTERFACE_GET InterfaceGet;
struct USB_GET_STATUS StatusGet;   //={0,0,0,0,0};


static const U8 ReportDescriptor[]=
{
								0x05,0x01,
								0x09,0x00,
								0xa1,0x01,
								0x15,0x00,
								0x25,0xff,
								0x19,0x01,
								0x29,0x08,
								0x95,0x08,
								0x75,0x08,
								0x81,0x02,
								0x19,0x01,
								0x29,0x08,
								0x91,0x02,
								0xc0
};
static const U8 descStr0[4]={
	0x04,0x03,0x09,0x04  //codes representing languages
    };

static const U8 descStr1[82]={	
								82,
								0x03,
								
								0x35,0x75,
								0x11,0x81,
								0x08,0x57,
								0x08,0x57,
								
								0x84,0x76,
								0x55,0x00,
								0x53,0x00,
								0x42,0x00,
								
								0x13,0x4e,
								0x3a,0x53,
								0x20,0x00,								
								0x48,0x00,
								
								0x74,0x00,
								0x74,0x00,
								0x70,0x00,
								0x3a,0x00,
								
								0x2f,0x00,
								0x2f,0x00,
								0x67,0x00,			
								0x72,0x00,
								
								0x6f,0x00,
								0x75,0x00,
								0x70,0x00,
								0x2e,0x00,
								
								0x65,0x00,
								0x64,0x00,
								0x6e,0x00,
								0x63,0x00,
								
								0x68,0x00,
								0x69,0x00,
								0x6e,0x00,
								0x61,0x00,
								
								0x2e,0x00,
								0x63,0x00,
								0x6f,0x00,				
								0x6d,0x00,
								
								0x2f,0x00,
								0x39,0x00,
								0x33,0x00,
								0x2f,0x00						
								};
    
static const U8 descStr2[34]={ 
								34,
								0x03,
								0x0a,0x30,
								0x08,0x57,
								0x08,0x57,
								0x59,0x65, 
								0x60,0x4f,
								0xa9,0x73,
								0x55,0x00,
								0x53,0x00,
								0x42,0x00,
								0x0b,0x30,
								0x4b,0x4e,
								0x55,0x00,
								0x53,0x00,
								0x42,0x00,
								0x55,0x00,
								0x55,0x00

    };

static const U8 descStr3[22]={
								22,
								0x03,
								0x32,0x00,
								0x30,0x00,
								0x30,0x00,
								0x38,0x00,
								0x2d,0x00,
								0x30,0x00,
								0x38,0x00,
								0x2d,0x00,
								0x30,0x00,
								0x38,0x00
};

void Ep0Handler(void)
{
    static int ep0SubState;
    int i;
    U8 ep0_csr;
	//U8 in_csr1;
    rINDEX_REG=0;
    ep0_csr=rEP0_CSR;
    
    //DbgPrintf("<0:%x]",ep0_csr);

    //DATAEND interrupt(ep0_csr==0x0) will be ignored 
    //because ep0State==EP0_STATE_INIT when the DATAEND interrupt is issued.

    
    if(ep0_csr & EP0_SETUP_END)
    {   
    	 //Uart_Printf("EP0_SETUP_END\n");
    	 // Host may end GET_DESCRIPTOR operation without completing the IN data stage.
    	 // If host does that, SETUP_END bit will be set.
    	 // OUT_PKT_RDY has to be also cleared because status stage sets OUT_PKT_RDY to 1.
   		//DbgPrintf("[SETUPEND]");
		CLR_EP0_SETUP_END();
	if(ep0_csr & EP0_OUT_PKT_READY) 
	{
		//Uart_Printf("EP0_OUT_PKT_READY\n");
	    FLUSH_EP0_FIFO(); //(???)
	    	//I think this isn't needed because EP0 flush is done automatically.   
	    CLR_EP0_OUT_PKT_RDY();
	}
	
	ep0State=EP0_STATE_INIT;
	return;
    }	

    //I think that EP0_SENT_STALL will not be set to 1.
    if(ep0_csr & EP0_SENT_STALL)
    {   
   	//Uart_Printf("EP0_SENT_STALL\n");
   	//DbgPrintf("[STALL]");
   	CLR_EP0_SENT_STALL();
	if(ep0_csr & EP0_OUT_PKT_READY) 
	{
	    CLR_EP0_OUT_PKT_RDY();
	}
	
	ep0State=EP0_STATE_INIT;
	return;
    }	



    if((ep0_csr & EP0_OUT_PKT_READY) && (ep0State==EP0_STATE_INIT))
    {	
		RdPktEp0((U8 *)&descSetup,EP0_PKT_SIZE);

		//if((descSetup.bmRequestType != 0xa1) && (descSetup.bmRequestType != 0x21))
		if(descSetup.bmRequestType != 0x21)
		{
			switch(descSetup.bRequest)
			{
				case GET_DESCRIPTOR:  
					switch(descSetup.bValueH)        
					{
						case DEVICE_TYPE:
							Uart_Printf("DEVICE_DESCRIPTOR\n");
							CLR_EP0_OUT_PKT_RDY();
							ep0State=EP0_STATE_GD_DEV_0;	        
						break;	
						
						case CONFIGURATION_TYPE:  	
							Uart_Printf("CONFIGURATION_DESCRIPTOR\n");
							CLR_EP0_OUT_PKT_RDY();
								if((descSetup.bLengthL+(descSetup.bLengthH<<8))>0x9)
									//bLengthH should be used for bLength=0x209 at WIN2K.    	
									ep0State=EP0_STATE_GD_CFG_0; //for WIN98,WIN2K
								else	    	    
									ep0State=EP0_STATE_GD_CFG_ONLY_0; //for WIN2K
						break;
   	    
						case STRING_TYPE:	    	
							Uart_Printf("STRING_TYPE_DESCRIPTOR\n");
							CLR_EP0_OUT_PKT_RDY();
							switch(descSetup.bValueL)
							{
								case 0:
									ep0State=EP0_STATE_GD_STR_I0;
								break;
								case 1:
									ep0State=EP0_STATE_GD_STR_I1;
								break;
								case 2:	
									ep0State=EP0_STATE_GD_STR_I2;
								break;
								case 3:
									ep0State=EP0_STATE_GD_STR_I3;
								break;
								
								default:
								break;
							}
							ep0SubState=0;
						break;	    
						
						case INTERFACE_TYPE:
							Uart_Printf("INTERFACE_DESCRIPTOR\n");
							CLR_EP0_OUT_PKT_RDY();
							ep0State=EP0_STATE_GD_IF_ONLY_0; //for WIN98
						break;
	    
						case ENDPOINT_TYPE:	    	
							Uart_Printf("ENDPOINT_DESCRIPTOR\n");
							CLR_EP0_OUT_PKT_RDY();
							switch(descSetup.bValueL&0xf)
							{
								case 0:
									ep0State=EP0_STATE_GD_EP1_ONLY_0;
								break;
								case 1:
									ep0State=EP0_STATE_GD_EP3_ONLY_0;
								break;
								default:									
								break;
							}
						break;						
						
						case REPORT_TYPE:
							Uart_Printf("GET_REPORT_DESCRIPTOR\n");
							CLR_EP0_OUT_PKT_RDY();
							ep0State=EP0_STATE_REPORT_DESCIRPTOR0;         
						break;
						
						default:
						break;	    
					}	
				break;

				case SET_ADDRESS:
					Uart_Printf("SET_ADDRESS\n");					
					rFUNC_ADDR_REG=descSetup.bValueL | 0x80;
					CLR_EP0_OUTPKTRDY_DATAEND();
					ep0State=EP0_STATE_INIT;
				break;
    	
				case SET_CONFIGURATION:
					Uart_Printf("SET_CONFIGURATION\n");
					ConfigSet.ConfigurationValue=descSetup.bValueL;            
					Uart_Printf("%d\n",ConfigSet.ConfigurationValue);
					CLR_EP0_OUTPKTRDY_DATAEND(); //Because of no data control transfers.										
					ep0State=EP0_STATE_INIT;	
					//isUsbdSetConfiguration=1; 
				break;
/*
    	    //////////////////////// For chapter 9 test ////////////////////

				case CLEAR_FEATURE:
					Uart_Printf("CLEAR_FEATURE\n");
					switch (descSetup.bmRequestType)
					{
						case DEVICE_RECIPIENT:
						if (descSetup.bValueL == 1)
							Rwuen = FALSE;  	  	  	
						break;

						case ENDPOINT_RECIPIENT:
							if (descSetup.bValueL == 0)
							{
							if((descSetup.bIndexL & 0x7f) == 0x00){
								StatusGet.Endpoint0= 0;    
							}
							if((descSetup.bIndexL & 0x8f) == 0x81){           // IN  Endpoint 1
								StatusGet.Endpoint1= 0;           
								}
							if((descSetup.bIndexL & 0x8f) == 0x03){          // OUT Endpoint 3
								StatusGet.Endpoint3= 0;      
								}
						}	  	  	 
						break;

						default:
						break;
					}
				CLR_EP0_OUTPKTRDY_DATAEND();
				ep0State=EP0_STATE_INIT;
			break;

			case GET_CONFIGURATION:
				Uart_Printf("GET_CONFIGURATION\n");
                CLR_EP0_OUT_PKT_RDY();
	    	 	ep0State=EP0_CONFIG_SET;	   
    	    break;

			case GET_INTERFACE:
				Uart_Printf("GET_INTERACE\n");
				CLR_EP0_OUT_PKT_RDY();
				ep0State=EP0_INTERFACE_GET;  	  	  
    	  	break;

			case GET_STATUS:
				Uart_Printf("GET_STATUS\n");
				switch(descSetup.bmRequestType)
				{
					case  (0x80):
						CLR_EP0_OUT_PKT_RDY();
						StatusGet.Device=((U8)Rwuen<<1)|(U8)Selfpwr;
						ep0State=EP0_GET_STATUS0;   	  	 		    	  	 		
                    break;

                    case  (0x81):
                        CLR_EP0_OUT_PKT_RDY();
						StatusGet.Interface=0;
						ep0State=EP0_GET_STATUS1;
                    break;

                    case  (0x82):
                     	CLR_EP0_OUT_PKT_RDY();
						if((descSetup.bIndexL & 0x7f) == 0x00){
	                          ep0State=EP0_GET_STATUS2;
						} 	  	 		
	                    if((descSetup.bIndexL & 0x8f) == 0x81){
	                        ep0State=EP0_GET_STATUS3;
	                    }	                       
	                    if((descSetup.bIndexL & 0x8f) == 0x03){
                            ep0State=EP0_GET_STATUS4;
	                    }
                    break;

                    default:
                    break;
				}    	  	      
    	  	break;


			case SET_DESCRIPTOR:
				Uart_Printf("SET_DESCRIPTOR\n");
				CLR_EP0_OUTPKTRDY_DATAEND();
				ep0State=EP0_STATE_INIT;
    	  	break;

			case SET_FEATURE:
				Uart_Printf("SET_FEARURE\n");
				switch (descSetup.bmRequestType)
				{
					case DEVICE_RECIPIENT:
						if (descSetup.bValueL == 1)
						Rwuen = TRUE;   	  	  	
					break;

					case ENDPOINT_RECIPIENT:
						if (descSetup.bValueL == 0)
						{
	                       if((descSetup.bIndexL & 0x7f) == 0x00){
	                        StatusGet.Endpoint0= 1;
	                       }
	                       if((descSetup.bIndexL & 0x8f) == 0x81){
	                         StatusGet.Endpoint1= 1;
	                       }
	                       if((descSetup.bIndexL & 0x8f) == 0x03){
	                         StatusGet.Endpoint3= 1;
	                       }
                       }
					break;

					default:
					break;
				}
				CLR_EP0_OUTPKTRDY_DATAEND();
				ep0State=EP0_STATE_INIT; 	  
    	  	break;

			case SET_INTERFACE:
				InterfaceGet.AlternateSetting= descSetup.bValueL;
				CLR_EP0_OUTPKTRDY_DATAEND(); 
                ep0State=EP0_STATE_INIT;
    	  	break;

			case SYNCH_FRAME:
				ep0State=EP0_STATE_INIT;
    	  	break;
    	  /////////////////////////////////////////////////////////////
*/
			default:
				CLR_EP0_OUTPKTRDY_DATAEND(); //Because of no data control transfers.
				ep0State=EP0_STATE_INIT;
			break;
			}	
		}
		else
		{
			CLR_EP0_OUT_PKT_RDY();
							//CLR_EP0_SENT_STALL();
							//CLR_EP0_SETUP_END();
			Uart_Printf("HDI_REQUEST\n");
			//if(descSetup.bRequest == 0x0a)
			//{
				//case SET_IDLE:
					//Uart_Printf("ÉèÖÃ¿ÕÏÐ\n");
					//WrPktEp0(Buf_Idle,0);
				//break;
			//}
		}
    }

	
	
    switch(ep0State)
    {	
	case EP0_STATE_INIT:
	    break; 

	//------------------------------------GET_DESCRIPTOR:DEVICE-----------------------------
	
    	case EP0_STATE_GD_DEV_0:
            WrPktEp0((U8 *)&descDev+0,8); //EP0_PKT_SIZE
            SET_EP0_IN_PKT_RDY();
            ep0State=EP0_STATE_GD_DEV_1;
            break;
            
    	case EP0_STATE_GD_DEV_1:
            WrPktEp0((U8 *)&descDev+0x8,8); 
            SET_EP0_IN_PKT_RDY();
            
            ep0State=EP0_STATE_GD_DEV_2;
            break;

    	case EP0_STATE_GD_DEV_2:
            WrPktEp0((U8 *)&descDev+0x10,2);   //8+8+2=0x12
            SET_EP0_INPKTRDY_DATAEND();
            ep0State=EP0_STATE_INIT;
            break;   

        //=== GET_DESCRIPTOR:CONFIGURATION+INTERFACE+ENDPOINT0+ENDPOINT1 ===
        //Windows98 gets these 4 descriptors all together by issuing only a request.
        //Windows2000 gets each descriptor seperately.

 
   	case EP0_STATE_GD_CFG_0:
            WrPktEp0((U8 *)&descConf+0,8); //EP0_PKT_SIZE
            SET_EP0_IN_PKT_RDY();
            ep0State=EP0_STATE_GD_CFG_1;
            break;
    
    	case EP0_STATE_GD_CFG_1:
            WrPktEp0((U8 *)&descConf+8,1); 
            WrPktEp0((U8 *)&descIf+0,7); 
            SET_EP0_IN_PKT_RDY();
            ep0State=EP0_STATE_GD_CFG_2;
            break;

    	case EP0_STATE_GD_CFG_2:
            WrPktEp0((U8 *)&descIf+7,2); 
            WrPktEp0((U8 *)&descHID+0,6); 
            SET_EP0_IN_PKT_RDY();
            ep0State=EP0_STATE_GD_CFG_3;
            break;

    	case EP0_STATE_GD_CFG_3:
            WrPktEp0((U8 *)&descHID+6,3); 
            WrPktEp0((U8 *)&descEndpt1+0,5); 
            SET_EP0_IN_PKT_RDY();
            ep0State=EP0_STATE_GD_CFG_4;            
            break;
            
        case EP0_STATE_GD_CFG_4:
            WrPktEp0((U8 *)&descEndpt1+5,2);
            WrPktEp0((U8 *)&descEndpt3,6); 
            SET_EP0_IN_PKT_RDY();
            ep0State=EP0_STATE_GD_CFG_5;                   
           // SET_EP0_INPKTRDY_DATAEND();
           // ep0State=EP0_STATE_INIT;            
            break;
            
       case EP0_STATE_GD_CFG_5:
       		WrPktEp0((U8 *)&descEndpt3+5,1);
       		SET_EP0_INPKTRDY_DATAEND();
           	ep0State=EP0_STATE_INIT;            

       break;
/*
        case EP0_STATE_GD_CFG_5:
            //DbgPrintf("[GDC4]");
            WrPktEp0((U8 *)&descEndpt3+6,1);                
            SET_EP0_INPKTRDY_DATAEND();
            ep0State=EP0_STATE_INIT;            
            break;
*/
/*
    	case EP0_STATE_GD_CFG_2:
            DbgPrintf("[GDC2]");
            WrPktEp0((U8 *)&descIf+7,2); 
            WrPktEp0((U8 *)&descEndpt0+0,6); 
            SET_EP0_IN_PKT_RDY();
            ep0State=EP0_STATE_GD_CFG_3;
            break;

    	case EP0_STATE_GD_CFG_3:
            DbgPrintf("[GDC3]");
            WrPktEp0((U8 *)&descEndpt0+6,1); 
            WrPktEp0((U8 *)&descEndpt1+0,7); 
            SET_EP0_IN_PKT_RDY();
            ep0State=EP0_STATE_GD_CFG_4;            
            break;

    	case EP0_STATE_GD_CFG_4:
            DbgPrintf("[GDC4]");
             //zero length data packit 
            SET_EP0_INPKTRDY_DATAEND();
            ep0State=EP0_STATE_INIT;            
            break;
*/
        //=== GET_DESCRIPTOR:CONFIGURATION ONLY===
    	case EP0_STATE_GD_CFG_ONLY_0:
            WrPktEp0((U8 *)&descConf+0,8); //EP0_PKT_SIZE
            SET_EP0_IN_PKT_RDY();
            ep0State=EP0_STATE_GD_CFG_ONLY_1;
            break;
    
    	case EP0_STATE_GD_CFG_ONLY_1:
            WrPktEp0((U8 *)&descConf+8,1); 
            SET_EP0_INPKTRDY_DATAEND();
            ep0State=EP0_STATE_INIT;            
            break;

        //=== GET_DESCRIPTOR:INTERFACE ONLY===
    	case EP0_STATE_GD_IF_ONLY_0:
            WrPktEp0((U8 *)&descIf+0,8); 
            SET_EP0_IN_PKT_RDY();
            ep0State=EP0_STATE_GD_IF_ONLY_1;
            break;
    	case EP0_STATE_GD_IF_ONLY_1:
            WrPktEp0((U8 *)&descIf+8,1); 
            SET_EP0_INPKTRDY_DATAEND();
            ep0State=EP0_STATE_INIT;            
            break;

        //=== GET_DESCRIPTOR:ENDPOINT 0 ONLY===
    	case EP0_STATE_GD_EP1_ONLY_0:
            WrPktEp0((U8 *)&descEndpt1+0,7); 
            SET_EP0_INPKTRDY_DATAEND();
            ep0State=EP0_STATE_INIT;            
            break;
            
        //=== GET_DESCRIPTOR:ENDPOINT 1 ONLY===
    	case EP0_STATE_GD_EP3_ONLY_0:
            DbgPrintf("[GDE10]");
            WrPktEp0((U8 *)&descEndpt3+0,7); 
            SET_EP0_INPKTRDY_DATAEND();
            ep0State=EP0_STATE_INIT;            
            break;
            
////////////////////////////////////////////

         case EP0_INTERFACE_GET:
            WrPktEp0((U8 *)&InterfaceGet+0,1);
            SET_EP0_INPKTRDY_DATAEND();
            ep0State=EP0_STATE_INIT;      
         break;

 
        //=== GET_DESCRIPTOR:STRING ===

    	case EP0_STATE_GD_STR_I0:
           WrPktEp0((U8 *)descStr0, 4 );  
	    	SET_EP0_INPKTRDY_DATAEND();
	    	ep0State=EP0_STATE_INIT;     
	   		ep0SubState=0;
	    break;

	case EP0_STATE_GD_STR_I1:
            if( (ep0SubState*EP0_PKT_SIZE+EP0_PKT_SIZE)<sizeof(descStr1) )
            {
            	WrPktEp0((U8 *)descStr1+(ep0SubState*EP0_PKT_SIZE),EP0_PKT_SIZE); 
            	SET_EP0_IN_PKT_RDY();
            	ep0State=EP0_STATE_GD_STR_I1;
            	ep0SubState++;
            }
	    else
	    {
	    	WrPktEp0((U8 *)descStr1+(ep0SubState*EP0_PKT_SIZE),
	    	sizeof(descStr1)-(ep0SubState*EP0_PKT_SIZE)); 
			SET_EP0_INPKTRDY_DATAEND();
			ep0State=EP0_STATE_INIT;     
			ep0SubState=0;
	    }
	    break;

	case EP0_STATE_GD_STR_I2:
         if( (ep0SubState*EP0_PKT_SIZE+EP0_PKT_SIZE)<sizeof(descStr2) )
         {
            WrPktEp0((U8 *)descStr2+(ep0SubState*EP0_PKT_SIZE),EP0_PKT_SIZE); 
            SET_EP0_IN_PKT_RDY();
            ep0State=EP0_STATE_GD_STR_I2;
            ep0SubState++;
         }
	    else
	    {
	    	WrPktEp0((U8 *)descStr2+(ep0SubState*EP0_PKT_SIZE),
	    	sizeof(descStr2)-(ep0SubState*EP0_PKT_SIZE)); 
			SET_EP0_INPKTRDY_DATAEND();
			ep0State=EP0_STATE_INIT;     
			ep0SubState=0;
	    }
	    break;

	case EP0_STATE_GD_STR_I3:
            if( (ep0SubState*EP0_PKT_SIZE+EP0_PKT_SIZE)<sizeof(descStr3) )
            {
            	WrPktEp0((U8 *)descStr3+(ep0SubState*EP0_PKT_SIZE),EP0_PKT_SIZE); 
            	SET_EP0_IN_PKT_RDY();
            	ep0State=EP0_STATE_GD_STR_I3;
            	ep0SubState++;
            }
	    else
	    {
	    	WrPktEp0((U8 *)descStr3+(ep0SubState*EP0_PKT_SIZE),
	    	sizeof(descStr3)-(ep0SubState*EP0_PKT_SIZE)); 
			SET_EP0_INPKTRDY_DATAEND();
			ep0State=EP0_STATE_INIT;     
			ep0SubState=0;
	    }
	    break;

	 case EP0_CONFIG_SET:
	 	WrPktEp0((U8 *)&ConfigSet+0,1); 
            SET_EP0_INPKTRDY_DATAEND();
            ep0State=EP0_STATE_INIT;      
            break;

        case EP0_GET_STATUS0:
	     WrPktEp0((U8 *)&StatusGet+0,1);
            SET_EP0_INPKTRDY_DATAEND();
            ep0State=EP0_STATE_INIT;      
         break;

         case EP0_GET_STATUS1:
	     WrPktEp0((U8 *)&StatusGet+1,1);
            SET_EP0_INPKTRDY_DATAEND();
            ep0State=EP0_STATE_INIT;      
         break;

         case EP0_GET_STATUS2:
	     WrPktEp0((U8 *)&StatusGet+2,1);
            SET_EP0_INPKTRDY_DATAEND();
            ep0State=EP0_STATE_INIT;      
         break;

         case EP0_GET_STATUS3:
	     WrPktEp0((U8 *)&StatusGet+3,1);
            SET_EP0_INPKTRDY_DATAEND();
            ep0State=EP0_STATE_INIT;      
         break;

         case EP0_GET_STATUS4:
	     WrPktEp0((U8 *)&StatusGet+4,1);
            SET_EP0_INPKTRDY_DATAEND();
            ep0State=EP0_STATE_INIT;      
         break;
        
         case EP0_STATE_REPORT_DESCIRPTOR0:  
         	if((ep0SubState*8+8) < sizeof(ReportDescriptor))
         	{      	
         		WrPktEp0((U8*)ReportDescriptor+8*ep0SubState,8);
         		SET_EP0_IN_PKT_RDY();
				ep0State=EP0_STATE_REPORT_DESCIRPTOR0;
				ep0SubState++;
			}
			else
			{
				WrPktEp0((U8 *)ReportDescriptor+(ep0SubState*8),sizeof(ReportDescriptor)-(ep0SubState*8)); 
				SET_EP0_INPKTRDY_DATAEND();
				ep0State=EP0_STATE_INIT;
				ep0SubState=0;
			}         
         break;	 	
	 	
  /*       case EP0_STATE_REPORT_DESCIRPTOR1:        	
         	WrPktEp0((U8*)ReportDescriptor+7,8);
         	SET_EP0_IN_PKT_RDY();
         	SET_EP0_INPKTRDY_DATAEND();
			ep0State=EP0_STATE_REPORT_DESCIRPTOR2;         
         break;		 	
*/	 	
     	default:
     	break;  
    }
   
}


 
    
void PrintEp0Pkt(U8 *pt)
{
    int i;
    DbgPrintf("[RCV:");
    for(i=0;i<EP0_PKT_SIZE;i++)
        DbgPrintf("%x,",pt[i]);
    DbgPrintf("]");
}



void InitDescriptorTable(void)
{	
    //Standard device descriptor
    descDev.bLength=0x12;	//EP0_DEV_DESC_SIZE=0x12 bytes    
    descDev.bDescriptorType=0x01;         
    descDev.bcdUSBL=0x10;
    descDev.bcdUSBH=0x01; 	//Ver 1.10
    descDev.bDeviceClass=0x00; //0x0          
    descDev.bDeviceSubClass=0x00;          
    descDev.bDeviceProtocol=0x00;          
    descDev.bMaxPacketSize0=0x08;         
    descDev.idVendorL=0x80;
    descDev.idVendorH=0x80;
    descDev.idProductL=0x06;
    descDev.idProductH=0x00;
    descDev.bcdDeviceL=0x00;
    descDev.bcdDeviceH=0x01;
    descDev.iManufacturer=0x01;  //index of string descriptor
    descDev.iProduct=0x02;	//index of string descriptor 
    descDev.iSerialNumber=0x03;
    descDev.bNumConfigurations=0x01;

    //Standard configuration descriptor
    descConf.bLength=0x09;    
    descConf.bDescriptorType=0x02;         
    descConf.wTotalLengthL=0x29; //<cfg desc>+<if desc>+<endp0 desc>+<endp1 desc>
    descConf.wTotalLengthH=0x00;
    descConf.bNumInterfaces=0x01;
//dbg    descConf.bConfigurationValue=2;  //why 2? There's no reason.
    descConf.bConfigurationValue=0x01;  
    descConf.iConfiguration=0x00;
    descConf.bmAttributes=0x80;  //bus powered only.
    descConf.maxPower=0x32; //draws 50mA current from the USB bus.          

    //Standard interface descriptor
    descIf.bLength=0x9;    
    descIf.bDescriptorType=0x04;         
    descIf.bInterfaceNumber=0x00;
    descIf.bAlternateSetting=0x00; //?
    descIf.bNumEndpoints=0x02;	//# of endpoints except EP0
    descIf.bInterfaceClass=0x03; //0x0 ?
    descIf.bInterfaceSubClass=0x00;  
    descIf.bInterfaceProtocol=0x00;
    descIf.iInterface=0x00;

//--------------------HID descriptor------------------------
	descHID.bLength = 0x09;
	descHID.bDescriptorType = 0x21;
	descHID.bcdHIDL = 0x10;
	descHID.bcdHIDH = 0x01;
	descHID.bCountyCode = 0x21;    //
	descHID.bNumDescriptors = 0x01;
	descHID.bSubDescriptorType = 0x22;
	descHID.bDescriptorLengthL = 0x1B;
	descHID.bDescriptorLengthH = 0x00;
	
//--------------------EP descriptor--------------------------

    descEndpt1.bLength=0x07;    
    descEndpt1.bDescriptorType=0x05;         
    descEndpt1.bEndpointAddress=0x81;   // 2400X endpoint 3 is OUT endpoint.
    descEndpt1.bmAttributes=0x03;
    descEndpt1.wMaxPacketSizeL= 0x10; //64
    descEndpt1.wMaxPacketSizeH=0x00;
    descEndpt1.bInterval=0x0A; //not used 


    //Standard endpoint0 descriptor
    descEndpt3.bLength=0x7;    
    descEndpt3.bDescriptorType=0x05;         
    descEndpt3.bEndpointAddress=0x03;   // 2400Xendpoint 1 is IN endpoint.
    descEndpt3.bmAttributes=0x03;
    descEndpt3.wMaxPacketSizeL=0x10; //64
    descEndpt3.wMaxPacketSizeH=0x00;
    descEndpt3.bInterval=0x0A; //not used
/*
    //Standard endpoint1 descriptor
    descEndpt1.bLength=0x7;    
    descEndpt1.bDescriptorType=ENDPOINT_TYPE;         
    descEndpt1.bEndpointAddress=3|EP_ADDR_OUT;   // 2400X endpoint 3 is OUT endpoint.
    descEndpt1.bmAttributes=EP_ATTR_BULK;
    descEndpt1.wMaxPacketSizeL=EP3_PKT_SIZE; //64
    descEndpt1.wMaxPacketSizeH=0x0;
    descEndpt1.bInterval=0x0; //not used 
*/
//InterfaceGet.AlternateSetting = 0x01;
}

