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

#include "pgpCertificate.h"

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

#define SERVAPP_NUM_ATTR_SUPPORTED        11

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

// Certificate Service UUID: 0x8E37                      
CONST uint8 certificateServUUID[ATT_UUID_SIZE] =         
{              
    CERTIFICATE_SERVICE_BASE_UUID_128(CERTIFICATE_SERV_UUID),             
};             
               
// Central to SFIDA UUID: 0x8E38                      
CONST uint8 centralToSfidaCharUUID[ATT_UUID_SIZE] =        
{              
    CERTIFICATE_SERVICE_BASE_UUID_128(CENTRAL_TO_SFIDA_CHAR_UUID),            
};             
// Sfida commands UUID: 0x8E39                      
CONST uint8 sfidaCommandsCharUUID[ATT_UUID_SIZE] =        
{              
    CERTIFICATE_SERVICE_BASE_UUID_128(SFIDA_COMMANDS_CHAR_UUID),            
};     
// SFIDA to Central UUID: 0x8E3A                      
CONST uint8 sfidaToCentralCharUUID[ATT_UUID_SIZE] =        
{              
    CERTIFICATE_SERVICE_BASE_UUID_128(SFIDA_TO_CENTRAL_CHAR_UUID),            
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

static pgpCertificateCBs_t *pgpCertificate_AppCBs = NULL;

/*********************************************************************
 * Profile Attributes - variables
 */

// Certificate Service attribute                      
static CONST gattAttrType_t pgpCertificateService = { ATT_UUID_SIZE, certificateServUUID};     
//-------------------------------------------------------------------
// Certificate Service Central to SFIDA Properties    
static uint8 centralToSfidaCharProps = GATT_PROP_READ | GATT_PROP_WRITE;                       

// Central to SFIDA Value
static uint8 centralToSfidaCharLen = 0;
static uint8 centralToSfidaChar[16] = {0};                    

// Certificate Service Central to SFIDA User Description           
static uint8 centralToSfidaCharUserDesp[] = "Central to Sfida\0";        

//-------------------------------------------------------------------
// Certificate Service Sfida commands Properties    
static uint8 sfidaCommandsCharProps = GATT_PROP_READ | GATT_PROP_WRITE | GATT_PROP_NOTIFY;                       

// Sfida commands Value
static uint8 sfidaCommandsCharLen = 0;
static uint8 sfidaCommandsChar[16] = {0};                   

// Sfida commands Notif Configuration.                                       
static gattCharCfg_t *sfidaCommandsCharConfig;        

// Device Control Service Sfida commands User Description           
static uint8 sfidaCommandsCharUserDesp[] = "Sfida commands\0";  

//-------------------------------------------------------------------
// Certificate Service SFIDA to Central Properties    
static uint8 sfidaToCentralCharProps = GATT_PROP_READ | GATT_PROP_WRITE;                       

// SFIDA to Central Value
static uint8 sfidaToCentralCharLen = 0;
static uint8 sfidaToCentralChar[16] = {0};                   

// Device Control Service SFIDA to Central User Description           
static uint8 sfidaToCentralCharUserDesp[] = "Sfida to Central\0";  

/*********************************************************************
 * Profile Attributes - Table
 */

static gattAttribute_t pgpCertificateAttrTbl[SERVAPP_NUM_ATTR_SUPPORTED] = 
{
  // Certificate Service
  { 
    { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
    GATT_PERMIT_READ,                         /* permissions */
    0,                                        /* handle */
    (uint8 *)&pgpCertificateService           /* pValue */
  },
  
    // Central to SFIDA Declaration                                 
  {                                                               
    { ATT_BT_UUID_SIZE, characterUUID },                          
    GATT_PERMIT_READ,                                             
    0,                                                            
    &centralToSfidaCharProps                                           
  },                                                              
                                                                  
  // Central to SFIDA Value                                    
  {                                                               
    { ATT_UUID_SIZE, centralToSfidaCharUUID },                         
    GATT_PERMIT_READ | GATT_PERMIT_WRITE,                         
    0,                                                            
    centralToSfidaChar                                                
  },                                                              
                                                                  
  // Central to SFIDA User Description                            
  {                                                               
    { ATT_BT_UUID_SIZE, charUserDescUUID },                       
    GATT_PERMIT_READ,                                             
    0,                                                            
    centralToSfidaCharUserDesp                                         
  },                                                              
  
  
    // Sfida Commands Declaration                                 
  {                                                               
    { ATT_BT_UUID_SIZE, characterUUID },                          
    GATT_PERMIT_READ,                                             
    0,                                                            
    &sfidaCommandsCharProps                                           
  },                                                              
                                                                  
  // Sfida Commands Value                                    
  {                                                               
    { ATT_UUID_SIZE, sfidaCommandsCharUUID },                         
    GATT_PERMIT_READ | GATT_PERMIT_WRITE,                         
    0,                                                            
    sfidaCommandsChar                                                
  },                              
  
  // Sfida Commands configuration                               
  {                                                               
    { ATT_BT_UUID_SIZE, clientCharCfgUUID },                      
    GATT_PERMIT_READ | GATT_PERMIT_WRITE,                         
    0,                                                            
    (uint8 *)&sfidaCommandsCharConfig                                 
  },   
                                                                  
  // Sfida Commands User Description                            
  {                                                               
    { ATT_BT_UUID_SIZE, charUserDescUUID },                       
    GATT_PERMIT_READ,                                             
    0,                                                            
    sfidaCommandsCharUserDesp                                         
  },                                                              

    
  // SFIDA to Central,  Declaration                                 
  {                                                               
    { ATT_BT_UUID_SIZE, characterUUID },                          
    GATT_PERMIT_READ,                                             
    0,                                                            
    &sfidaToCentralCharProps                                           
  },                                                              
                                                                  
  // SFIDA to Central,  Value                                    
  {                                                               
    { ATT_UUID_SIZE, sfidaToCentralCharUUID },                         
    GATT_PERMIT_READ | GATT_PERMIT_WRITE,                         
    0,                                                            
    sfidaToCentralChar                                                
  },                                                              
                                                                  
  // SFIDA to Central,  User Description                            
  {                                                               
    { ATT_BT_UUID_SIZE, charUserDescUUID },                       
    GATT_PERMIT_READ,                                             
    0,                                                            
    sfidaToCentralCharUserDesp                                         
  },                                                                   
};

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static bStatus_t pgpCertificate_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr, 
    uint8 *pValue, uint8 *pLen, uint16 offset,
    uint8 maxLen, uint8 method );
static bStatus_t pgpCertificate_WriteAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
     uint8 *pValue, uint8 len, uint16 offset,
     uint8 method );

/*********************************************************************
 * PROFILE CALLBACKS
 */
// Simple Profile Service Callbacks
CONST gattServiceCBs_t pgpCertificateCBs =
{
  pgpCertificate_ReadAttrCB,  // Read callback function pointer
  pgpCertificate_WriteAttrCB, // Write callback function pointer
  NULL                       // Authorization callback function pointer
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

bStatus_t utilExtractUuid16Certificate(gattAttribute_t *pAttr, uint16 *pUuid)
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
 * @fn      PgpCertificate_AddService
 *
 * @brief   Initializes the Simple Profile service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 *
 * @return  Success or Failure
 */
bStatus_t PgpCertificate_AddService( uint32 services )
{
  uint8 status;
  
  // Allocate Client Characteristic Configuration table
  sfidaCommandsCharConfig = (gattCharCfg_t *)osal_mem_alloc( sizeof(gattCharCfg_t) *
                                                              linkDBNumConns );
  if ( sfidaCommandsCharConfig == NULL )
  {     
    return ( bleMemAllocError );
  }
  
  // Initialize Client Characteristic Configuration attributes
  GATTServApp_InitCharCfg( INVALID_CONNHANDLE, sfidaCommandsCharConfig );
  
  // Register GATT attribute list and CBs with GATT Server App
  status = GATTServApp_RegisterService( pgpCertificateAttrTbl, 
                                          GATT_NUM_ATTRS( pgpCertificateAttrTbl ),
                                          GATT_MAX_ENCRYPT_KEY_SIZE,
                                          &pgpCertificateCBs );

  return ( status );
}

/*********************************************************************
 * @fn      PgpCertificate_RegisterAppCBs
 *
 * @brief   Registers the application callback function. Only call 
 *          this function once.
 *
 * @param   callbacks - pointer to application callbacks.
 *
 * @return  SUCCESS or bleAlreadyInRequestedMode
 */
bStatus_t PgpCertificate_RegisterAppCBs( pgpCertificateCBs_t *appCallbacks )
{
  if ( appCallbacks )
  {
    pgpCertificate_AppCBs = appCallbacks;
    
    return ( SUCCESS );
  }
  else
  {
    return ( bleAlreadyInRequestedMode );
  }
}

/*********************************************************************
 * @fn      PgpCertificate_SetParameter
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
bStatus_t PgpCertificate_SetParameter( uint8 param, uint8 len, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case CENTRAL_TO_SFIDA_CHAR:
      if ( len <= (sizeof(centralToSfidaChar))) {
        (void)memcpy(centralToSfidaChar, value, len);
        centralToSfidaCharLen=len;
      }else{
        ret = bleInvalidRange;
      }
      break;
    case SFIDA_COMMANDS_CHAR:
      if ( len <= (sizeof(sfidaCommandsChar))) {
        (void)memcpy(sfidaCommandsChar, value, len);
        sfidaCommandsCharLen=len;
        // See if Notification has been enabled                                               
        GATTServApp_ProcessCharCfg( sfidaCommandsCharConfig, sfidaCommandsChar, FALSE,               
                                    pgpCertificateAttrTbl, GATT_NUM_ATTRS( pgpCertificateAttrTbl ),       
                                    INVALID_TASK_ID, pgpCertificate_ReadAttrCB );           
      }else{
        ret = bleInvalidRange;
      }
      break;
    case SFIDA_TO_CENTRAL_CHAR:
      if ( len <= (sizeof(sfidaToCentralChar))) {
        (void)memcpy(sfidaToCentralChar, value, len);
        sfidaToCentralCharLen=len;
      }else{
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
 * @fn      PgpCertificate_GetParameter
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
bStatus_t PgpCertificate_GetParameter( uint8 param, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case CENTRAL_TO_SFIDA_CHAR:
      (void)memcpy(value, centralToSfidaChar, centralToSfidaCharLen);
      break;

    case SFIDA_COMMANDS_CHAR:
      (void)memcpy(value, sfidaCommandsChar, sfidaCommandsCharLen);
      break;      

    case SFIDA_TO_CENTRAL_CHAR:
      (void)memcpy(value, sfidaToCentralChar, sfidaToCentralCharLen);
      break;  
      
    default:
      ret = INVALIDPARAMETER;
      break;
  }
  
  return ( ret );
}

/*********************************************************************
 * @fn          pgpCertificate_ReadAttrCB
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
static bStatus_t pgpCertificate_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr, 
    uint8 *pValue, uint8 *pLen, uint16 offset,
    uint8 maxLen, uint8 method )
{
  uint16 uuid;
  bStatus_t status = SUCCESS;
  
  #if (defined HAL_UART) && (HAL_UART == TRUE)
    HalUARTWrite ( HAL_UART_PORT_1, "pgpCertificate_ReadAttrCB\n", 26 );
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
 
  
  if (utilExtractUuid16Certificate(pAttr,&uuid) == FAILURE) {                                      
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
  case CENTRAL_TO_SFIDA_CHAR_UUID:
    *pLen = centralToSfidaCharLen;
    (void) memcpy(pValue, pAttr->pValue, *pLen);  
    break;
  case SFIDA_COMMANDS_CHAR_UUID:
    *pLen = sfidaCommandsCharLen;
    (void) memcpy(pValue, pAttr->pValue, *pLen);  
    break;
  case SFIDA_TO_CENTRAL_CHAR_UUID:
    *pLen = sfidaToCentralCharLen;
    (void) memcpy(pValue, pAttr->pValue, *pLen);  
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
 * @fn      pgpCertificate_WriteAttrCB
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
static bStatus_t pgpCertificate_WriteAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
     uint8 *pValue, uint8 len, uint16 offset,
     uint8 method )
{
  bStatus_t status = SUCCESS;
  uint8 notifyApp = 0xFF;
  uint16 uuid; 
  
  #if (defined HAL_UART) && (HAL_UART == TRUE)
    HalUARTWrite ( HAL_UART_PORT_1, "pgpCertificate_WriteAttrCB\n", 26 );
  #endif
  
  // If attribute permissions require authorization to write, return error
  if ( gattPermitAuthorWrite( pAttr->permissions ) )
  {
    // Insufficient authorization
    return ( ATT_ERR_INSUFFICIENT_AUTHOR );
  }
  
  if (utilExtractUuid16Certificate(pAttr,&uuid) == FAILURE) {                                       
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
    case CENTRAL_TO_SFIDA_CHAR_UUID:                                                             
    case SFIDA_COMMANDS_CHAR_UUID:
    case SFIDA_TO_CENTRAL_CHAR_UUID:

      //Validate the value
      // Make sure it's not a blob oper
      if ( offset == 0 )
      {
        if ( len > sizeof(centralToSfidaChar) )
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
        (void)memcpy(pAttr->pValue, pValue, len);      

        if( pAttr->pValue == centralToSfidaChar )
        {
          notifyApp = CENTRAL_TO_SFIDA_CHAR;
          centralToSfidaCharLen=len;
        }
        else if( pAttr->pValue == sfidaCommandsChar )
        {
          notifyApp = SFIDA_COMMANDS_CHAR;
          sfidaCommandsCharLen = len;
        }
        else if( pAttr->pValue == sfidaToCentralChar )
        {
          notifyApp = SFIDA_TO_CENTRAL_CHAR;
          sfidaToCentralCharLen=len;
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
  if ( (notifyApp != 0xFF ) && pgpCertificate_AppCBs && pgpCertificate_AppCBs->pfnPgpCertificateChange )
  {
    pgpCertificate_AppCBs->pfnPgpCertificateChange( notifyApp );  
  }
  
  return ( status );
}

/*********************************************************************
*********************************************************************/
