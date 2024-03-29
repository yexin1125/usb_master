/****************************************************************
 NAME: usbsetup.h
 DESC: usb setup
 HISTORY:
 Mar.25.2002:purnnamu: reuse the source of S3C2400X u24xmon 
 ****************************************************************/
 
#ifndef __USBSETUP_H__
#define __USBSETUP_H__

void Ep0Handler(void);
void InitDescriptorTable(void);
void PrintEp0Pkt(U8 *pt);
void Ep0Handler_test(void);

#define EP0_STATE_INIT 			(0)  

//NOTE: The ep0State value in a same group should be added by 1.
#define EP0_STATE_GD_DEV_0	 	(10)  //10-10=0 
#define EP0_STATE_GD_DEV_1 		(11)  //11-10=1
#define EP0_STATE_GD_DEV_2 		(12)  //12-10=2

#define EP0_STATE_GD_CFG_0	 	(20)
#define EP0_STATE_GD_CFG_1 		(21)
#define EP0_STATE_GD_CFG_2 		(22)
#define EP0_STATE_GD_CFG_3 		(23)
#define EP0_STATE_GD_CFG_4 		(24)
#define EP0_STATE_GD_CFG_5      (25)

#define EP0_STATE_GD_CFG_ONLY_0		(40)
#define EP0_STATE_GD_CFG_ONLY_1		(41)
#define EP0_STATE_GD_IF_ONLY_0 		(42)
#define EP0_STATE_GD_IF_ONLY_1 		(43)
#define EP0_STATE_GD_EP0_ONLY_0		(44)
#define EP0_STATE_GD_EP1_ONLY_0		(45)
#define EP0_STATE_GD_EP3_ONLY_0		(60)
#define EP0_INTERFACE_GET                        (46)
#define EP0_STATE_REPORT_DESCIRPTOR0 (50)
#define EP0_STATE_REPORT_DESCIRPTOR1 (54)
#define EP0_STATE_REPORT_DESCIRPTOR2 (55)
#define EP0_STATE_REPORT_DESCIRPTOR3 (56)
#define EP0_STATE_REPORT_DESCIRPTOR4 (57)
#define EP0_STATE_REPORT_DESCIRPTOR5 (58)

#define EP0_STATE_GD_STR_I0	 	(30)  
#define EP0_STATE_GD_STR_I1	 	(31)  
#define EP0_STATE_GD_STR_I2	 	(32)  
#define EP0_STATE_GD_STR_I3		(51)

#define EP0_CONFIG_SET    (33)
#define EP0_GET_STATUS0  (35)
#define EP0_GET_STATUS1  (36)
#define EP0_GET_STATUS2  (37)
#define EP0_GET_STATUS3  (38)
#define EP0_GET_STATUS4  (39)




extern U32 ep0State;


#endif /*__USBSETUP_H__*/
