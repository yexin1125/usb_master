/****************************************************************
 NAME: usb.h
 DESC: definitions(USB data structure) for USB setup operation.
 By: Harlan YE
 2013.6
 ****************************************************************/
#ifndef __USB_H__
#define __USB_H__

//************************
//       Endpoint 0      
//************************

// Standard bmRequestTyje (Direction) 
#define HOST_TO_DEVICE              (0x00)
#define DEVICE_TO_HOST              (0x80)    

// Standard bmRequestType (Type) 
#define STANDARD_TYPE               (0x00)
#define CLASS_TYPE                  (0x20)
#define VENDOR_TYPE                 (0x40)
#define RESERVED_TYPE               (0x60)

// Standard bmRequestType (Recipient) 
#define DEVICE_RECIPIENT            (0)
#define INTERFACE_RECIPIENT         (1)
#define ENDPOINT_RECIPIENT          (2)
#define OTHER_RECIPIENT             (3)

// Feature Selectors 
#define DEVICE_REMOTE_WAKEUP        (1)
#define EP_STALL                    (0)

// Standard Request Codes 
#define GET_STATUS                  (0)
#define CLEAR_FEATURE               (1)
#define SET_FEATURE                 (3)
#define SET_ADDRESS                 (5)
#define GET_DESCRIPTOR              (6)
#define SET_DESCRIPTOR              (7)
#define GET_CONFIGURATION           (8)
#define SET_CONFIGURATION           (9)
#define GET_INTERFACE               (10)
#define SET_INTERFACE               (11)
#define SYNCH_FRAME                 (12)

//HID request codes
#define GET_REPORT					0x01
#define GET_IDLE					0x02
#define GET_PROTOCOL				0x03
#define SET_REPORT					0x09
#define SET_IDLE					0x0a
#define SET_PROTOCOL				0x0b

// Class-specific Request Codes 
#define GET_DEVICE_ID               (0)
#define GET_PORT_STATUS             (1)
#define SOFT_RESET                  (2)

// Descriptor Types
#define DEVICE_TYPE                 (1)
#define CONFIGURATION_TYPE          (2)
#define STRING_TYPE                 (3)
#define INTERFACE_TYPE              (4)
#define ENDPOINT_TYPE               (5)

#define REPORT_TYPE					(0x22)

//configuration descriptor: bmAttributes 
#define CONF_ATTR_DEFAULT	    (0x80) //Spec 1.0 it was BUSPOWERED bit.
#define CONF_ATTR_REMOTE_WAKEUP     (0x20)
#define CONF_ATTR_SELFPOWERED       (0x40)

//endpoint descriptor
#define EP_ADDR_IN		    (0x80)	
#define EP_ADDR_OUT		    (0x00)

#define EP_ATTR_CONTROL		    (0x0)	
#define EP_ATTR_ISOCHRONOUS	    (0x1)
#define EP_ATTR_BULK		    (0x2)
#define EP_ATTR_INTERRUPT	    (0x3)	


//string descriptor
#define LANGID_US_L 		    (0x09)  
#define LANGID_US_H 		    (0x04)


//report type define 
#define PARAMETER_REPORT		(0x01)
#define IMFORMATION_REPORT		(0x02)
#define CLEAR_IMFORMATION		(0x03)
#define END_CONFIGURED			(0x04)

#define TWO_STEP				(0x00)
#define THREE_STEP				(0x01)

#define NO_CHANGE				(0x00)
#define Three2Two				(0x01)

#define LOW_VOLTAGE				(0x01)
#define MID_VOLTAGE				(0x02)
#define HIGH_VOLTAGE			(0x03)

#define SEND_PARA				(0x01)
#define SEND_IMFO				(0x02)

struct USB_SETUP_DATA{
    U8 bmRequestType;    
    U8 bRequest;         
    U8 bValueL;          
    U8 bValueH;          
    U8 bIndexL;          
    U8 bIndexH;          
    U8 bLengthL;         
    U8 bLengthH;         
};


struct USB_DEVICE_DESCRIPTOR{
    U8 bLength;    
    U8 bDescriptorType;         
    U8 bcdUSBL;
    U8 bcdUSBH;
    U8 bDeviceClass;          
    U8 bDeviceSubClass;          
    U8 bDeviceProtocol;          
    U8 bMaxPacketSize0;         
    U8 idVendorL;
    U8 idVendorH;
    U8 idProductL;
    U8 idProductH;
    U8 bcdDeviceL;
    U8 bcdDeviceH;
    U8 iManufacturer;
    U8 iProduct;
    U8 iSerialNumber;
    U8 bNumConfigurations;
};


struct USB_CONFIGURATION_DESCRIPTOR{
    U8 bLength;    
    U8 bDescriptorType;         
    U8 wTotalLengthL;
    U8 wTotalLengthH;
    U8 bNumInterfaces;
    U8 bConfigurationValue;
    U8 iConfiguration;
    U8 bmAttributes;
    U8 maxPower;          
};
    

struct USB_INTERFACE_DESCRIPTOR{
    U8 bLength;    
    U8 bDescriptorType;         
    U8 bInterfaceNumber;
    U8 bAlternateSetting;
    U8 bNumEndpoints;
    U8 bInterfaceClass;
    U8 bInterfaceSubClass;
    U8 bInterfaceProtocol;
    U8 iInterface;
};


struct USB_HID_DESCRIPTOR{
	U8 bLength;
	U8 bDescriptorType;
	U8 bcdHIDL;
	U8 bcdHIDH;
	U8 bCountyCode;
	U8 bNumDescriptors;
	U8 bSubDescriptorType;
	U8 bDescriptorLengthL;
	U8 bDescriptorLengthH;
};

struct USB_ENDPOINT_DESCRIPTOR{
    U8 bLength;    
    U8 bDescriptorType;         
    U8 bEndpointAddress;
    U8 bmAttributes;
    U8 wMaxPacketSizeL;
    U8 wMaxPacketSizeH;
    U8 bInterval;
};

 struct USB_CONFIGURATION_SET{
     U8 ConfigurationValue;
 };

 struct USB_GET_STATUS{
     U8 Device;
     U8 Interface;
     U8 Endpoint0;
     U8 Endpoint1;
     U8 Endpoint3;
 };

 struct USB_INTERFACE_GET{
     U8 AlternateSetting;
 };
 
 
 struct EP3_PARAMETER_REPOTR{
 	//U8 Report_ID;       //0x00
 	U8 L_Circletime;
 	U8 H_Circletime;
 	U8 L_Accuracy;
 	U8 H_Accuracy;
 	U8 Voltage_Level;	//0x00:two level voltage 0x01:three level voltage
 	U8 Time_Unit;		//0x01:us  0x02:ms  0x03:s
 	U8 Report_Type;		//0x01;
 	U8 Reserve;
 };
 
 struct EP3_IMFORMATION_REPORT{
 	//U8 Report_ID;
 	U8 L_Duration;
 	U8 H_Duration;
 	U8 Voltage_Value;	//0x01:Low_Voltage, 0x02:Mid_Voltage, 0x03:High_Voltage
 	U8 Reserve1;
 	U8 Reserve2;
 	U8 Reserve3;
 	U8 Report_Type;		//0x02;
 	U8 Reserve4;
 };
 
 struct IMFORMATION_REPOTR_2LEVRL{
  	U8 L_Duration;
 	U8 H_Duration;
 	U8 Voltage_Value;	//0x01:Low_Voltage, 0x03:High_Voltage
 };


 
#endif /*__USB_H__*/                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            
