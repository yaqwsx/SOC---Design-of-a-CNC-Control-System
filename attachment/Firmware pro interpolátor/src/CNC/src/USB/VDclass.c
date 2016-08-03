/*
 * Autor: yaqwsx
 Tøída pro vendor-defined zaøízení. Odvozeno od usbd_hid_core.c
 seriálu Zaèínáme s STM32F4Discovery 7 od mard (mcu.cz)
 */

//Pùvodní popis souboru
/**
  ******************************************************************************

  * @file    usbd_hid_core.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    22-July-2011
  * @brief   This file provides the HID core functions.
  *
  * @verbatim
  *      
  *          ===================================================================      
  *                                HID Class  Description
  *          =================================================================== 
  *           This module manages the HID class V1.11 following the "Device Class Definition
  *           for Human Interface Devices (HID) Version 1.11 Jun 27, 2001".
  *           This driver implements the following aspects of the specification:
  *             - The Boot Interface Subclass
  *             - The Custom Generic protocol
  *             - Usage Page : Generic Desktop
  *             - Collection : Application 
  *      
  * @note     In HS mode and when the DMA is used, all variables and data structures
  *           dealing with the DMA during the transaction process should be 32-bit aligned.
  *           
  *      
  *  @endverbatim
  *
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "VDclass.h"
#include "usbd_desc.h"
#include "usbd_req.h"
#include "usbd_conf.h"
#include "receive.h"
#include <string.h>



static uint8_t  USBD_VD_Init (void  *pdev,
                               uint8_t cfgidx);

static uint8_t  USBD_VD_DeInit (void  *pdev,
                                 uint8_t cfgidx);

static uint8_t  USBD_VD_Setup (void  *pdev,
                                USB_SETUP_REQ *req);

static uint8_t  *USBD_VD_GetCfgDesc (uint8_t speed, uint16_t *length);

static uint8_t  USBD_VD_DataIn (void  *pdev, uint8_t epnum);

static uint8_t  USBD_VD_DataOut (void  *pdev, uint8_t epnum);


//Callback struktura
USBD_Class_cb_TypeDef  USBD_VD_cb =
{
  USBD_VD_Init,
  USBD_VD_DeInit,
  USBD_VD_Setup,
  NULL, //EP0_TxSent
  NULL, //EP0_RxReady
  USBD_VD_DataIn, //DataIn
  //  NULL,
  USBD_VD_DataOut, //DataOut
  NULL, //SOF
  NULL,
  NULL,      
  USBD_VD_GetCfgDesc,
#ifdef USB_OTG_HS_CORE  
  USBD_VD_GetCfgDesc, // use same config as per FS
#endif  
};

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) //!< IAR Compiler
    #pragma data_alignment=4   
  #endif
#endif // USB_OTG_HS_INTERNAL_DMA_ENABLED
__ALIGN_BEGIN static uint32_t  USBD_HID_AltSet  __ALIGN_END = 0;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) //!< IAR Compiler
    #pragma data_alignment=4   
  #endif
#endif // USB_OTG_HS_INTERNAL_DMA_ENABLED


#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */  


#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */ 

//USB VD Configuration descriptor
__ALIGN_BEGIN static uint8_t USBD_VD_CfgDesc[32] __ALIGN_END =
{
	0x09, // bLength: Configuration Descriptor size
    USB_CONFIGURATION_DESCRIPTOR_TYPE, // bDescriptorType: Configuration
    32,//Size of cfgDesc - including the others descriptors
    0x00,
    0x01,         //bNumInterfaces: 1 interface
    0x01,         //bConfigurationValue: Configuration value
    0x00,         //iConfiguration: Index of string descriptor describing the configuration
    // 0xC0,         //bmAttributes: bus powered
    0xE0,         //bmAttributes: bus powered and Support Remote Wake-up
    0x32,         //MaxPower 100 mA: this current is used for detecting Vbus

    //------Descriptor of Custom (vendor-defined) interface----------
    0x09,         // bLength: Interface Descriptor size
    USB_INTERFACE_DESCRIPTOR_TYPE, // bDescriptorType: Interface descriptor type
    0x00,         // bInterfaceNumber: Number of Interface
    0x00,         // bAlternateSetting: Alternate setting
    0x02,         // bNumEndpoints
    0xFF,         // bInterfaceClass: Vendor-specific
    0x00,         // bInterfaceSubClass
    0x00,         // nInterfaceProtocol
    0,            // iInterface: Index of string descriptor
    //Endpoint desciptors
    //Out endpoint
    7,				//bLenght
    USB_ENDPOINT_DESCRIPTOR_TYPE,	//bDescriptorType - endpoint
    VD_OUT_EP,		//bEndpointAdress, OUT
    0x02,			//bmAttributes - bulk, data, no synchronisation
    VD_OUT_PACKET,	//wMaxPacketSize
    00,				//wMaxPacketSize
    0,				//bInterval - ignored for bulk
    //In endpoint
    7,				//bLenght
    USB_ENDPOINT_DESCRIPTOR_TYPE,	//bDescriptorType - endpoint
    VD_IN_EP,		//bEndpointAdress, IN
    0x02,			//bmAttributes - bulk, data, no synchronisation
    VD_IN_PACKET,
    00,				//wMaxPacketSize
    0				//bInterval - ignored for bulk
}
;


#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */  


/**
  * @brief  USBD_VD_Init
  *         Initialize the VD interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_VD_Init (void  *pdev,
                               uint8_t cfgidx)
{
  
  /* Open EP IN */
  DCD_EP_Open(pdev,
              VD_IN_EP,
              VD_IN_PACKET,
              USB_OTG_EP_INT);
  
  /* Open EP OUT */
  DCD_EP_Open(pdev,
              VD_OUT_EP,
              VD_OUT_PACKET,
              USB_OTG_EP_INT);

  /* Prepare Out endpoint to receive next packet */
  DCD_EP_PrepareRx(pdev,
                   VD_OUT_EP,
                   (uint8_t*)(Buffer),
                   VD_OUT_PACKET);
  
  return USBD_OK;
}

/**
  * @brief  USBD_VD_DeInit
  *         DeInitialize the VD layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_VD_DeInit (void  *pdev,
                                 uint8_t cfgidx)
{
  /* Close HID EPs */
  DCD_EP_Close (pdev , VD_IN_EP);
  DCD_EP_Close (pdev , VD_OUT_EP);
  
  
  return USBD_OK;
}

/**
  * @brief  USBD_VD_Setup
  *         Handle the HID specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t  USBD_VD_Setup (void  *pdev,
                                USB_SETUP_REQ *req)
{
  
  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
  case USB_REQ_TYPE_CLASS :  
    switch (req->bRequest)
    {
		default:
		  USBD_CtlError (pdev, req);
		  return USBD_FAIL;
    }
    break;
    
  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest)
    {
		case USB_REQ_GET_DESCRIPTOR:
		  //Získat deskriptor - nepoužito pro VD
		  break;
      
		case USB_REQ_GET_INTERFACE :
		  USBD_CtlSendData (pdev,
							(uint8_t *)&USBD_HID_AltSet,
							1);
		  break;
      
		case USB_REQ_SET_INTERFACE :
		  USBD_HID_AltSet = (uint8_t)(req->wValue);
		  break;
    }
  }
  return USBD_OK;
}

/**
  * @brief  USBD_VD_SendData
  *         Send Data from device to host
  * @param  pdev: device instance
  * @param  buff: pointer to data
  * @param	len:	lengt of data
  * @retval status
  */
uint8_t USBD_VD_SendData     (USB_OTG_CORE_HANDLE  *pdev,
                                 uint8_t *data,
                                 uint16_t len)
{
  if (pdev->dev.device_status == USB_OTG_CONFIGURED )
  {
    DCD_EP_Tx (pdev, VD_IN_EP, data, len);
    return USBD_OK;
  }
  return USBD_FAIL;
}

/**
  * @brief  USBD_VD_GetCfgDesc
  *         return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_VD_GetCfgDesc (uint8_t speed, uint16_t *length)
{
  *length = sizeof (USBD_VD_CfgDesc);
  return USBD_VD_CfgDesc;
}

/**
  * @brief  USBD_VD_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_VD_DataIn (void  *pdev, uint8_t epnum)
{


  /* Ensure that the FIFO is empty before a new transfer, this condition could 
  be caused by  a new transfer before the end of the previous transfer */

  DCD_EP_Flush(pdev, VD_IN_EP);
  return USBD_OK;
}


/**
  * @brief  USBD_VD_DataOut
  *         handle data OUT Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */

static uint8_t  USBD_VD_DataOut (void *pdev, uint8_t epnum)
{
	uint16_t USB_RecData_Cnt;
	/*static uint8_t buffer[VD_OUT_PACKET];
	memset(buffer, 0, 64);*/
	if (epnum == VD_OUT_EP)
	{
		/* Get the received data buffer and update the counter */
		USB_RecData_Cnt = ((USB_OTG_CORE_HANDLE*)pdev)->dev.out_ep[epnum].xfer_count;
		/* USB data will be immediately processed, this allow next USB traffic being
		   NAKed till the end of the application Xfer */
		if (((USB_OTG_CORE_HANDLE*)pdev)->dev.device_status == USB_OTG_CONFIGURED )
		{
			USB_OTG_ReadPacket((USB_OTG_CORE_HANDLE*)pdev, *Buffer, VD_OUT_PACKET);
			/* process the report setting */
			//STM32F4_Discovery_LEDToggle(buffer[63]);
			/*if(Buffer[0] == 1)
			{
				STM32F4_Discovery_LEDToggle(LED3);
				STM32F4_Discovery_LEDToggle(LED5);
				STM32F4_Discovery_LEDToggle(LED6);
				STM32F4_Discovery_LEDToggle(LED4);
			}*/
			ProcessPacket(Buffer);
			/* Prepare Out endpoint to receive next packet */
			DCD_EP_PrepareRx(pdev,
			                   VD_OUT_EP,
			                   (uint8_t*)(Buffer),
			                   VD_OUT_PACKET);
		}
	}
	return USBD_OK;
}


/**
  * @}
  */ 


/**
  * @}
  */ 


/**
  * @}
  */ 

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
