/******************************************************************************

 @file  simpleGATTprofile.c

 @brief This file contains the Simple GATT profile sample GATT service profile
        for use with the BLE sample application.

 Group: WCS, BTS
 Target Device: CC2540, CC2541

 ******************************************************************************
 
 Copyright (c) 2010-2016, Texas Instruments Incorporated
 All rights reserved.

 IMPORTANT: Your use of this Software is limited to those specific rights
 granted under the terms of a software license agreement between the user
 who downloaded the software, his/her employer (which must be your employer)
 and Texas Instruments Incorporated (the "License"). You may not use this
 Software unless you agree to abide by the terms of the License. The License
 limits your use, and you acknowledge, that the Software may not be modified,
 copied or distributed unless embedded on a Texas Instruments microcontroller
 or used solely and exclusively in conjunction with a Texas Instruments radio
 frequency transceiver, which is integrated into your product. Other than for
 the foregoing purpose, you may not use, reproduce, copy, prepare derivative
 works of, modify, distribute, perform, display or sell this Software and/or
 its documentation for any purpose.

 YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
 PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
 NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
 TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
 NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
 LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
 INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
 OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
 OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
 (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

 Should you have any questions regarding your right to use this Software,
 contact Texas Instruments Incorporated at www.TI.com.

 ******************************************************************************
 Release Name: ble_sdk_1.4.2.2
 Release Date: 2016-06-09 06:57:10
 *****************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include <string.h>

#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "gapbondmgr.h"

#include "pgpDeviceControl.h"

#if (defined HAL_UART) && (HAL_UART == TRUE)
#include <stdio.h>
#include <stdlib.h>
#include "hal_uart.h"
#endif

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

//need to update if RW is updated or more char
#define SERVAPP_NUM_ATTR_SUPPORTED        14    

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
   
// Device Control Service UUID: 0x9AEB                      
CONST uint8 deviceControlServUUID[ATT_UUID_SIZE] =         
{                                                     
    DEVICE_CONTROL_SERVICE_BASE_UUID_128(DEVICE_CONTROL_SERV_UUID),             
};                                                    
                                                      
// Led Vibrate Ctrl UUID: 0x9AEC                      
CONST uint8 ledVibrateCtrlCharUUID[ATT_UUID_SIZE] =        
{                                                     
    DEVICE_CONTROL_SERVICE_BASE_UUID_128(LED_VIBRATE_CTRL_CHAR_UUID),            
};                                                    
// Button Notif UUID: 0x9AED                      
CONST uint8 buttonNotifCharUUID[ATT_UUID_SIZE] =        
{                                                     
    DEVICE_CONTROL_SERVICE_BASE_UUID_128(BUTTON_NOTIF_CHAR_UUID),            
};     
// Fw Update Request UUID: 0x9AEF                      
CONST uint8 fwUpdateRequestCharUUID[ATT_UUID_SIZE] =        
{                                                     
    DEVICE_CONTROL_SERVICE_BASE_UUID_128(FW_UPDATE_REQUEST_CHAR_UUID),            
};     
// Fw Version UUID: 0x9AF0                      
CONST uint8 fwVersionCharUUID[ATT_UUID_SIZE] =        
{                                                     
    DEVICE_CONTROL_SERVICE_BASE_UUID_128(FW_VERSION_CHAR_UUID),            
};        


/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

static pgpDeviceControlCBs_t *pgpDeviceControl_AppCBs = NULL;

/*********************************************************************                    
 * Service Attributes - variables                                                         
 */                                                                                       

// Device Control Service attribute                                                             
static CONST gattAttrType_t pgpDeviceControlService = { ATT_UUID_SIZE, deviceControlServUUID};     
//-------------------------------------------------------------------
// Device Control Service Led Vibrate Ctrl Properties                                           
static uint8 ledVibrateCtrlCharProps = GATT_PROP_READ | GATT_PROP_WRITE;                       

// Led Vibrate Ctrl Value                                                                 
static uint8 ledVibrateCtrlChar = 0;                                                           

// Device Control Service Led Vibrate Ctrl User Description                                     
static uint8 ledVibrateCtrlCharUserDesp[] = "Led Vibrate Ctrl\0";        

//-------------------------------------------------------------------
// Device Control Service Button Notif Properties                                           
static uint8 buttonNotifCharProps = GATT_PROP_NOTIFY;                                       

// Button Notif Value                                                                 
static uint8 buttonNotifChar = 0;                                                           

// Device Control Service Button Notif Configuration.                                       
static gattCharCfg_t *buttonNotifCharConfig;                                                

// Device Control Service Button Notif User Description                                     
static uint8 buttonNotifCharUserDesp[] = "Button Notif\0"; 

//-------------------------------------------------------------------
// Device Control Service Fw Update Request Properties                                           
static uint8 fwUpdateRequestCharProps = GATT_PROP_READ | GATT_PROP_WRITE;                       

// Fw Update Request Value                                                                 
static uint8 fwUpdateRequestChar = 0;                                                           

// Device Control Service Fw Update Request User Description                                     
static uint8 fwUpdateRequestCharUserDesp[] = "Fw Update Request\0";  

//-------------------------------------------------------------------
// Device Control Service Fw Version Properties                                           
static uint8 fwVersionCharProps = GATT_PROP_READ;                       

// Fw Version Value                                                                 
static uint8 fwVersionChar = 0;                                                           

// Device Control Service Fw Version User Description                                     
static uint8 fwVersionCharUserDesp[] = "Fw Version\0";                              
                            

/*********************************************************************
 * Profile Attributes - Table
 */

static gattAttribute_t pgpDeviceControlAttrTbl[SERVAPP_NUM_ATTR_SUPPORTED] = 
{
  // Device Control Service
  { 
    { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
    GATT_PERMIT_READ,                         /* permissions */
    0,                                        /* handle */
    (uint8 *)&pgpDeviceControlService         /* pValue */
  },
  
    // Led Vibrate Ctrl Declaration                                 
  {                                                               
    { ATT_BT_UUID_SIZE, characterUUID },                          
    GATT_PERMIT_READ,                                             
    0,                                                            
    &ledVibrateCtrlCharProps                                           
  },                                                              
                                                                  
  // Led Vibrate Ctrl Value                                    
  {                                                               
    { ATT_UUID_SIZE, ledVibrateCtrlCharUUID },                         
    GATT_PERMIT_READ | GATT_PERMIT_WRITE,                         
    0,                                                            
    &ledVibrateCtrlChar                                                
  },                                                              
                                                                  
  // Led Vibrate Ctrl User Description                            
  {                                                               
    { ATT_BT_UUID_SIZE, charUserDescUUID },                       
    GATT_PERMIT_READ,                                             
    0,                                                            
    ledVibrateCtrlCharUserDesp                                         
  },                                                              
                                                                  
   
  // Button Notif Declaration                                 
  {                                                               
    { ATT_BT_UUID_SIZE, characterUUID },                          
    GATT_PERMIT_READ,                                             
    0,                                                            
    &buttonNotifCharProps                                           
  },                                                              
                                                                  
  // Button Notif Value                                      
  {                                                               
    { ATT_UUID_SIZE, buttonNotifCharUUID },                         
    0,                                                            
    0,                                                            
    &buttonNotifChar                                                
  },                                                              
                                                                  
  // Button Notif configuration                               
  {                                                               
    { ATT_BT_UUID_SIZE, clientCharCfgUUID },                      
    GATT_PERMIT_READ | GATT_PERMIT_WRITE,                         
    0,                                                            
    (uint8 *)&buttonNotifCharConfig                                 
  },                                                              
                                                                  
  // Button Notif User Description                            
  {                                                               
    { ATT_BT_UUID_SIZE, charUserDescUUID },                       
    GATT_PERMIT_READ,                                             
    0,                                                            
    buttonNotifCharUserDesp                                         
  },  
  
  
    // Fw Update Request Declaration                                 
  {                                                               
    { ATT_BT_UUID_SIZE, characterUUID },                          
    GATT_PERMIT_READ,                                             
    0,                                                            
    &fwUpdateRequestCharProps                                           
  },                                                              
                                                                  
  // Fw Update Request Value                                    
  {                                                               
    { ATT_UUID_SIZE, fwUpdateRequestCharUUID },                         
    GATT_PERMIT_READ | GATT_PERMIT_WRITE,                         
    0,                                                            
    &fwUpdateRequestChar                                                
  },                                                              
                                                                  
  // Fw Update Request User Description                            
  {                                                               
    { ATT_BT_UUID_SIZE, charUserDescUUID },                       
    GATT_PERMIT_READ,                                             
    0,                                                            
    fwUpdateRequestCharUserDesp                                         
  },                                                              

    
    // Fw Version Declaration                                 
  {                                                               
    { ATT_BT_UUID_SIZE, characterUUID },                          
    GATT_PERMIT_READ,                                             
    0,                                                            
    &fwVersionCharProps                                           
  },                                                              
                                                                  
  // Fw Version Value                                    
  {                                                               
    { ATT_UUID_SIZE, fwVersionCharUUID },                         
    GATT_PERMIT_READ,                         
    0,                                                            
    &fwVersionChar                                                
  },                                                              
                                                                  
  // Fw Version User Description                            
  {                                                               
    { ATT_BT_UUID_SIZE, charUserDescUUID },                       
    GATT_PERMIT_READ,                                             
    0,                                                            
    fwVersionCharUserDesp                                         
  },                                                              

};

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static bStatus_t pgpDeviceControl_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr, 
                                           uint8 *pValue, uint8 *pLen, uint16 offset,
                                           uint8 maxLen, uint8 method );
static bStatus_t pgpDeviceControl_WriteAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                            uint8 *pValue, uint8 len, uint16 offset,
                                            uint8 method );

/*********************************************************************
 * PROFILE CALLBACKS
 */
// Simple Profile Service Callbacks
CONST gattServiceCBs_t pgpDeviceControlCBs =
{
  pgpDeviceControl_ReadAttrCB,  // Read callback function pointer
  pgpDeviceControl_WriteAttrCB, // Write callback function pointer
  NULL                       // Authorization callback function pointer
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

bStatus_t utilExtractUuid16(gattAttribute_t *pAttr, uint16 *pUuid)
{
  bStatus_t status = SUCCESS;

  if (pAttr->type.len == ATT_BT_UUID_SIZE )
  {
    // 16-bit UUID direct
    *pUuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);
  }
  else if (pAttr->type.len == ATT_UUID_SIZE)
  {
    // 16-bit UUID extracted bytes 0 and 1 for this service
    *pUuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);
  } else {
    *pUuid = 0xFFFF;
    status = FAILURE;
  }

  return status;
}

/*********************************************************************
 * @fn      PgpDeviceControl_AddService
 *
 * @brief   Initializes the Simple Profile service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 *
 * @return  Success or Failure
 */
bStatus_t PgpDeviceControl_AddService( uint32 services )
{
  uint8 status;
  
  // Allocate Client Characteristic Configuration table
  buttonNotifCharConfig = (gattCharCfg_t *)osal_mem_alloc( sizeof(gattCharCfg_t) *
                                                              linkDBNumConns );
  if ( buttonNotifCharConfig == NULL )
  {     
    return ( bleMemAllocError );
  }
  
  // Initialize Client Characteristic Configuration attributes
  GATTServApp_InitCharCfg( INVALID_CONNHANDLE, buttonNotifCharConfig );
  

  // Register GATT attribute list and CBs with GATT Server App
  status = GATTServApp_RegisterService( pgpDeviceControlAttrTbl, 
                                          GATT_NUM_ATTRS( pgpDeviceControlAttrTbl ),
                                          GATT_MAX_ENCRYPT_KEY_SIZE,
                                          &pgpDeviceControlCBs );

  
  return ( status );
}

/*********************************************************************
 * @fn      PgpDeviceControl_RegisterAppCBs
 *
 * @brief   Registers the application callback function. Only call 
 *          this function once.
 *
 * @param   callbacks - pointer to application callbacks.
 *
 * @return  SUCCESS or bleAlreadyInRequestedMode
 */
bStatus_t PgpDeviceControl_RegisterAppCBs( pgpDeviceControlCBs_t *appCallbacks )
{
  if ( appCallbacks )
  {
    pgpDeviceControl_AppCBs = appCallbacks;
    
    return ( SUCCESS );
  }
  else
  {
    return ( bleAlreadyInRequestedMode );
  }
}

/*********************************************************************
 * @fn      PgpDeviceControl_SetParameter
 *
 * @brief   Set a Simple Profile parameter.
 *
 * @param   param - Profile parameter ID
 * @param   len - length of data to write
 * @param   value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 *
 * @return  bStatus_t
 */
bStatus_t PgpDeviceControl_SetParameter( uint8 param, uint8 len, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case LED_VIBRATE_CTRL_CHAR:                                                                      
      if ( len == sizeof ( uint8 ) )                                                          
      {                                                                                       
        ledVibrateCtrlChar = *((uint8*)value);                                                     
      }                                                                                       
      else                                                                                    
      {                                                                                       
        ret = bleInvalidRange;                                                                
      }                                                                                       
      break;                                                                                  
                                                                                              
    case BUTTON_NOTIF_CHAR:                                                                      
      if ( len == sizeof ( uint8 ) )                                                          
      {                                                                                       
        buttonNotifChar = *((uint8*)value);                                                     
                                                                                              
        // See if Notification has been enabled                                               
        GATTServApp_ProcessCharCfg( buttonNotifCharConfig, &buttonNotifChar, FALSE,               
                                    pgpDeviceControlAttrTbl, GATT_NUM_ATTRS( pgpDeviceControlAttrTbl ),       
                                    INVALID_TASK_ID, pgpDeviceControl_ReadAttrCB );                   
      }                                                                                       
      else                                                                                    
      {                                                                                       
        ret = bleInvalidRange;                                                                
      }                                                                                       
      break;
      
    case FW_UPDATE_REQUEST_CHAR:                                                                      
      if ( len == sizeof ( uint8 ) )                                                          
      {                                                                                       
        fwUpdateRequestChar = *((uint8*)value);                                                     
      }                                                                                       
      else                                                                                    
      {                                                                                       
        ret = bleInvalidRange;                                                                
      }                                                                                       
      break;                                                                                  

    case FW_VERSION_CHAR:                                                                      
      if ( len == sizeof ( uint8 ) )                                                          
      {                                                                                       
        fwVersionChar = *((uint8*)value);                                                     
      }                                                                                       
      else                                                                                    
      {                                                                                       
        ret = bleInvalidRange;                                                                
      }                                                                                       
      break;                                                                                  

      
    default:
      ret = INVALIDPARAMETER;
      break;
  }
  
  return ( ret );
}

/*********************************************************************
 * @fn      PgpDeviceControl_GetParameter
 *
 * @brief   Get a Simple Profile parameter.
 *
 * @param   param - Profile parameter ID
 * @param   value - pointer to data to put.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 *
 * @return  bStatus_t
 */
bStatus_t PgpDeviceControl_GetParameter( uint8 param, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case LED_VIBRATE_CTRL_CHAR:
      *((uint8*)value) = ledVibrateCtrlChar;
      break;

    case BUTTON_NOTIF_CHAR:
      *((uint8*)value) = buttonNotifChar;
      break;      

    case FW_UPDATE_REQUEST_CHAR:
      *((uint8*)value) = fwUpdateRequestChar;
      break;  

    case FW_VERSION_CHAR:
      *((uint8*)value) = fwVersionChar;
      break;   
      
    default:
      ret = INVALIDPARAMETER;
      break;
  }
  
  return ( ret );
}

/*********************************************************************
 * @fn          pgpDeviceControl_ReadAttrCB
 *
 * @brief       Read an attribute.
 *
 * @param       connHandle - connection message was received on
 * @param       pAttr - pointer to attribute
 * @param       pValue - pointer to data to be read
 * @param       pLen - length of data to be read
 * @param       offset - offset of the first octet to be read
 * @param       maxLen - maximum length of data to be read
 * @param       method - type of read message
 *
 * @return      SUCCESS, blePending or Failure
 */
static bStatus_t pgpDeviceControl_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr, 
                                           uint8 *pValue, uint8 *pLen, uint16 offset,
                                           uint8 maxLen, uint8 method )
{
  uint16 uuid;
  bStatus_t status = SUCCESS;
  
  #if (defined HAL_UART) && (HAL_UART == TRUE)
    HalUARTWrite ( HAL_UART_PORT_1, "pgpDeviceControl_ReadAttrCB\n", 28 );
  #endif

  // If attribute permissions require authorization to read, return error
  if ( gattPermitAuthorRead( pAttr->permissions ) )
  {
    // Insufficient authorization
    return ( ATT_ERR_INSUFFICIENT_AUTHOR );
  }
  
  // Make sure it's not a blob operation (no attributes in the profile are long)
  if ( offset > 0 )
  {
    return ( ATT_ERR_ATTR_NOT_LONG );
  }
 
  
  if (utilExtractUuid16(pAttr,&uuid) == FAILURE) {                                      
    // Invalid handle                                                                   
    *pLen = 0;                                                                          
    return ATT_ERR_INVALID_HANDLE;                                                      
  }
  
  #if (defined HAL_UART) && (HAL_UART == TRUE)
  {
    char buf[32];
    sprintf(buf,"UUID: %04X\n",uuid);
    uint8 strLength=strlen(buf);
    HalUARTWrite ( HAL_UART_PORT_1, (uint8 *)buf, strLength );
  }
  #endif
                                                                                        
  switch ( uuid )                                                                       
  {                                                                                     
    // No need for "GATT_SERVICE_UUID" or "GATT_CLIENT_CHAR_CFG_UUID" cases;            
    // gattserverapp handles those reads                                                
                                                                                        
    // characteristics 1 has read permissions                                           
    // characteristic 2 does not have read permissions, but because it                  
    //   can be sent as a notification, it is included here                             
  case LED_VIBRATE_CTRL_CHAR_UUID:                                                             
  case BUTTON_NOTIF_CHAR_UUID:
  case FW_UPDATE_REQUEST_CHAR_UUID:
  case FW_VERSION_CHAR_UUID:
    *pLen = 1;                                                                          
    pValue[0] = *pAttr->pValue;                                                         
    break;                                                                              
                                                                                        
  default:                                                                              
    // Should never get here! (characteristics 3 and 4 do not have read permissions)    
    *pLen = 0;                                                                          
    status = ATT_ERR_ATTR_NOT_FOUND;                                                    
    break;                                                                              
  }                                                                                     
  return ( status ); 
}

/*********************************************************************
 * @fn      pgpDeviceControl_WriteAttrCB
 *
 * @brief   Validate attribute data prior to a write operation
 *
 * @param   connHandle - connection message was received on
 * @param   pAttr - pointer to attribute
 * @param   pValue - pointer to data to be written
 * @param   len - length of data
 * @param   offset - offset of the first octet to be written
 * @param   method - type of write message
 *
 * @return  SUCCESS, blePending or Failure
 */
static bStatus_t pgpDeviceControl_WriteAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                            uint8 *pValue, uint8 len, uint16 offset,
                                            uint8 method )
{
  bStatus_t status = SUCCESS;
  uint8 notifyApp = 0xFF;
  uint16 uuid; 
  
  #if (defined HAL_UART) && (HAL_UART == TRUE)
    HalUARTWrite ( HAL_UART_PORT_1, "pgpDeviceControl_ReadAttrCB\n", 29 );
  #endif
  
  // If attribute permissions require authorization to write, return error
  if ( gattPermitAuthorWrite( pAttr->permissions ) )
  {
    // Insufficient authorization
    return ( ATT_ERR_INSUFFICIENT_AUTHOR );
  }
  
  if (utilExtractUuid16(pAttr,&uuid) == FAILURE) {                                       
    // Invalid handle                                                                    
    return ATT_ERR_INVALID_HANDLE;                                                       
  }   
  
  #if (defined HAL_UART) && (HAL_UART == TRUE)
  {
    char buf[32];
    sprintf(buf,"UUID: %04X\n",uuid);
    uint8 strLength=strlen(buf);
    HalUARTWrite ( HAL_UART_PORT_1, (uint8 *)buf, strLength );
  }
  #endif
 
  switch ( uuid )
  {
    case LED_VIBRATE_CTRL_CHAR_UUID:
    case FW_UPDATE_REQUEST_CHAR_UUID:

      //Validate the value
      // Make sure it's not a blob oper
      if ( offset == 0 )
      {
        if ( len != 1 )
        {
          status = ATT_ERR_INVALID_VALUE_SIZE;
        }
      }
      else
      {
        status = ATT_ERR_ATTR_NOT_LONG;
      }
        
      //Write the value
      if ( status == SUCCESS )
      {
        uint8 *pCurValue = (uint8 *)pAttr->pValue;        
        *pCurValue = pValue[0];

        if( pAttr->pValue == &ledVibrateCtrlChar )
        {
          notifyApp = LED_VIBRATE_CTRL_CHAR;        
        }
        else if( pAttr->pValue == &fwUpdateRequestChar )
        {
          notifyApp = FW_UPDATE_REQUEST_CHAR;           
        }
      }
             
      break;

    case GATT_CLIENT_CHAR_CFG_UUID:
      status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
                                                 offset, GATT_CLIENT_CFG_NOTIFY );
      break;
        
    default:
      // Should never get here! (characteristics 2 and 4 do not have write permissions)
      status = ATT_ERR_ATTR_NOT_FOUND;
      break;
  }

  // If a charactersitic value changed then callback function to notify application of change
  if ( (notifyApp != 0xFF ) && pgpDeviceControl_AppCBs && pgpDeviceControl_AppCBs->pfnPgpDeviceControlChange )
  {
    pgpDeviceControl_AppCBs->pfnPgpDeviceControlChange( notifyApp );  
  }
  
  return ( status );
}

/*********************************************************************
*********************************************************************/
