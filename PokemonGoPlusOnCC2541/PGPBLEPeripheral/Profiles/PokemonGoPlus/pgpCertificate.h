/******************************************************************************

 @file  simpleGATTprofile.h

 @brief This file contains the Simple GATT profile definitions and prototypes.

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

#ifndef PGPCERTIFICATEPROFILE_H
#define PGPCERTIFICATEPROFILE_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * CONSTANTS
 */

//public static final UUID UUID_CERTIFICATE_SERVICE = UUID.fromString("bbe87709-5b89-4433-ab7f-8b8eef0d8e37");
//public static final UUID UUID_CENTRAL_TO_SFIDA_CHAR = UUID.fromString("bbe87709-5b89-4433-ab7f-8b8eef0d8e38");
//public static final UUID UUID_SFIDA_COMMANDS_CHAR = UUID.fromString("bbe87709-5b89-4433-ab7f-8b8eef0d8e39");
//public static final UUID UUID_SFIDA_TO_CENTRAL_CHAR = UUID.fromString("bbe87709-5b89-4433-ab7f-8b8eef0d8e3a");

// Profile Parameters
#define CENTRAL_TO_SFIDA_CHAR                 0  //RW
#define SFIDA_COMMANDS_CHAR                   1  //RW
#define SFIDA_TO_CENTRAL_CHAR                 2  //RW
    
// UUID for CERTIFICATE_SERVICE service                          
#define CERTIFICATE_SERV_UUID                  0x8E37    
/* The 16 bit UUID listen above is only a part of the 
 * full DEVICE CONTROL 128 bit UUID:                       
 * BBE87709-5B89-4433-AB7F-8B8EEF0D???? */   
   
                                                      
//  Certificate service characteristic UUID              
#define CENTRAL_TO_SFIDA_CHAR_UUID          0x8E38    
#define SFIDA_COMMANDS_CHAR_UUID            0x8E39 
#define SFIDA_TO_CENTRAL_CHAR_UUID          0x8E3A 

/*********************************************************************
 * TYPEDEFS
 */

  
/*********************************************************************
 * MACROS
 */

// CERTIFICATE_SERVICE Base 128-bit UUID:  BBE87709-5B89-4433-AB7F-8B8EEF0DXXXX
#define CERTIFICATE_SERVICE_BASE_UUID_128( uuid )  LO_UINT16( uuid ), HI_UINT16( uuid ), 0x0D, 0xEF, 0x8E, 0x8B, 0x7F, 0xAB, \
                                  0x33, 0x44, 0x89, 0x5B, 0x09, 0x77, 0xE8, 0xBB

/*********************************************************************
 * Profile Callbacks
 */

// Callback when a characteristic value has changed
typedef void (*pgpCertificateChange_t)( uint8 paramID );

typedef struct
{
  pgpCertificateChange_t        pfnPgpCertificateChange;  // Called when characteristic value changes
} pgpCertificateCBs_t;

    

/*********************************************************************
 * API FUNCTIONS 
 */


/*
 * PgpCertificate_AddService- Initializes the Simple GATT Profile service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 */

extern bStatus_t PgpCertificate_AddService( uint32 services );

/*
 * PgpCertificate_RegisterAppCBs - Registers the application callback function.
 *                    Only call this function once.
 *
 *    appCallbacks - pointer to application callbacks.
 */
extern bStatus_t PgpCertificate_RegisterAppCBs( pgpCertificateCBs_t *appCallbacks );

/*
 * PgpCertificate_SetParameter - Set a Simple GATT Profile parameter.
 *
 *    param - Profile parameter ID
 *    len - length of data to right
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 */
extern bStatus_t PgpCertificate_SetParameter( uint8 param, uint8 len, void *value );
  
/*
 * PgpCertificate_GetParameter - Get a Simple GATT Profile parameter.
 *
 *    param - Profile parameter ID
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 */
extern bStatus_t PgpCertificate_GetParameter( uint8 param, void *value );


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* PGPCERTIFICATEPROFILE_H */
