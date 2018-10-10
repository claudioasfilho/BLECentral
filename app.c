/***********************************************************************************************//**
 * \file   app.c
 * \brief  Event handling and application code for Empty NCP Host application example
 ***************************************************************************************************
 * <b> (C) Copyright 2016 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

/* standard library headers */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

/* BG stack headers */
#include "bg_types.h"
#include "gecko_bglib.h"

/* Own header */
#include "app.h"

/***************************************************************************************************
 Local Macros and Definitions
 **************************************************************************************************/
#define PRINT_ADV_INFO								1

#define NO_CONNECTION									0xFF
#define NO_HANDLE											0xFFFF

#define DISCONNECTED									0
#define SCANNING											1
#define CONNECTED											2
#define SERVICE_FOUND									3
#define CHARACTERISTICS_DISCOVERING 	4
#define CHARACTERISTICS_FOUND					5
#define ENABLING_NOTIFY   						6
#define NOTIFY_ENABLED								7
#define ENABLING_WRITE   						  8
#define WRITE_ENABLED									9

#define NOTIFY_CHAR_ITEM							1
#define RW_CHAR_ITEM									2
#define ALL_CHARS											(NOTIFY_CHAR_ITEM | RW_CHAR_ITEM)

/* UUIDs for the demo service and characteristics */
const uint8_t serviceUUID[] = {
		0xed,
		0x52,
		0x0b,
		0x6b,
		0x1f,
		0x1a,
		0x3a,
		0x94,
		0x6d,
		0x48,
		0xd1,
		0x32,
		0x89,
		0x8b,
		0x6a,
		0xdf };

    //0x0c, 0xed, 0x79, 0x30, 0xb3, 0x1f, 0x45, 0x7d, 0xa6, 0xa2, 0xb3, 0xdb, 0x9b, 0x03, 0xe3, 0x9a
const uint8_t notifyCharUUID[] = {
		0x9a,
		0xe3,
		0x03,
		0x9b,
		0xdb,
		0xb3,
		0xa2,
		0xa6,
		0x7d,
		0x45,
		0x1f,
		0xb3,
		0x30,
		0x79,
		0xed,
		0x0c };
const uint8_t rwCharUUID[] = {
		0x8d,
		0x2d,
		0xfb,
		0xd8,
		0x17,
		0x7e,
		0x7c,
		0x92,
		0xa9,
		0x43,
		0x6e,
		0xf2,
		0x09,
		0x89,
		0x95,
		0xfb };
// App booted flag
static bool appBooted = false;

int8                rssi;
uint8               packet_type;
bd_addr             address;
uint8               address_type;
uint8               bonding;
uint8array          data;

static uint8_t connHandle = NO_CONNECTION;
static uint8_t currentState = DISCONNECTED;
static uint8_t characteristicsState = 0;
static uint32_t serviceHandle = NO_HANDLE;
static uint16_t notifyHandle = NO_HANDLE;
static uint16_t rwHandle = NO_HANDLE;
static uint8_t pb0PressTimes = 0;

  static void Reset_variables() {
	connHandle = NO_CONNECTION;
	currentState = DISCONNECTED;
	characteristicsState = 0;
	serviceHandle = NO_HANDLE;
	notifyHandle = NO_HANDLE;
	rwHandle = NO_HANDLE;
	pb0PressTimes = 0;
}

static uint8_t Process_scan_response(struct gecko_msg_le_gap_scan_response_evt_t *pResp) {
	// decoding advertising packets is done here. The list of AD types can be found
	// at: https://www.bluetooth.com/specifications/assigned-numbers/Generic-Access-Profile

	uint8_t i = 0;
	uint8_t ad_match_found = 0;
	uint8_t ad_len;
	uint8_t ad_type;

	while (i < (pResp->data.len - 1)) {

		ad_len = pResp->data.data[i];
		ad_type = pResp->data.data[i + 1];

		if (ad_type == 0x07 || ad_type == 0x06) {
			// type 0x06 = More 128-bit UUIDs available
			// type 0x07 = Complete list of 128-bit UUIDs available

			// note: this check assumes that the service we are looking for is first
			// in the list. To be fixed so that there is no such limitation...
			if (!memcmp(pResp->data.data + i + 2, serviceUUID, 16)) {
				ad_match_found = 1;
				break;
			}
		}

		//jump to next AD record
		i = i + ad_len + 1;
	}
	return (ad_match_found);
}


uint8_t writeCounter = 0;
/***********************************************************************************************//**
 *  \brief  Event handler function.
 *  \param[in] evt Event pointer.
 **************************************************************************************************/
void appHandleEvents(struct gecko_cmd_packet *evt)
{

  if (NULL == evt) {
    return;
  }

  // Do not handle any events until system is booted up properly.
  if ((BGLIB_MSG_ID(evt->header) != gecko_evt_system_boot_id)
      && !appBooted) {
#if defined(DEBUG)
    printf("Event: 0x%04x\n", BGLIB_MSG_ID(evt->header));
#endif
    usleep(50000);
    return;
  }




  /* Handle events */
  switch (BGLIB_MSG_ID(evt->header)) {
    case gecko_evt_system_boot_id:

        appBooted = true;
        Reset_variables();
  			currentState = SCANNING;
  			/* Start discovery after system booted */
  			struct gecko_msg_le_gap_discover_rsp_t *retDiscover = gecko_cmd_le_gap_discover(le_gap_discover_generic);
  			if (retDiscover->result == 0) {
  				printf("OK --- >Scanning Started.\r\n");
  			} else {
  				printf("Error!!! Start Scanning error, error code = %d\r\n", retDiscover->result);
  			}


      break;


		        /* Check for scan response results */
        case gecko_evt_le_gap_scan_response_id:

        #if (PRINT_ADV_INFO == 1)
        				printf("address ---> ");
        				for(uint8_t i=0; i< 6; i++) {
        					printf("0x%02x ", evt->data.evt_le_gap_scan_response.address.addr[i]);
        				}
        				printf(" ---- ");
        				printf("advertisement packet --> ");
        				for(uint8_t i=0; i< evt->data.evt_le_gap_scan_response.data.len; i++) {
        					printf("0x%02x ", evt->data.evt_le_gap_scan_response.data.data[i]);
        				}
        				printf("\r\n");
        #endif
        				// process scan responses: this function returns 1 if we found the service we are looking for
        				if (Process_scan_response(&(evt->data.evt_le_gap_scan_response)) > 0) {
        					struct gecko_msg_le_gap_open_rsp_t *pResp;
        					// match found -> stop discovery and try to connect
        					gecko_cmd_le_gap_end_procedure();
        					printf("OK --- >Device found, connecting.\r\n");
        					pResp = gecko_cmd_le_gap_open(evt->data.evt_le_gap_scan_response.address, evt->data.evt_le_gap_scan_response.address_type);
        					// make copy of connection handle for later use (for example, to cancel the connection attempt)
        					connHandle = pResp->connection;
        				}
            break;

            /* Connection opened event */
    			case gecko_evt_le_connection_opened_id:
      				printf("OK --- >Connected.\r\n");
      				currentState = CONNECTED;
      				connHandle = evt->data.evt_le_connection_opened.connection;
      				/* Search for specified Demo service after connection opened */
      				struct gecko_msg_gatt_discover_primary_services_by_uuid_rsp_t *retDisService = gecko_cmd_gatt_discover_primary_services_by_uuid(connHandle, 16, serviceUUID);
      				if (retDisService->result == 0) {
      					printf("OK --- >Start discovering demo service.\r\n");
      				} else {
      					printf("Error!!! Start Discovery error, error code = %d\r\n", retDisService->result);
      				}
    			break;

          case gecko_evt_gatt_service_id:
      				if (!memcmp(evt->data.evt_gatt_service.uuid.data, serviceUUID, 16)) {
      					printf("OK --- >Service Found\r\n");
      					currentState = SERVICE_FOUND;
      					serviceHandle = evt->data.evt_gatt_service.service;
      					/* Specified Demo service found, will start discovering characteristics in gap complete event since there might be more services causing current GATT operation not done yet */
      				}
      	 break;

         case gecko_evt_gatt_characteristic_id:
               if (!memcmp(evt->data.evt_gatt_characteristic.uuid.data, notifyCharUUID, 16)) {
                 printf("OK --- >Notify Char Found\r\n");
                 characteristicsState |= NOTIFY_CHAR_ITEM;
                 notifyHandle = evt->data.evt_gatt_characteristic.characteristic;
               }
               if (!memcmp(evt->data.evt_gatt_characteristic.uuid.data, rwCharUUID, 16)) {
                 printf("OK --- >RW Char Found\r\n");
                 characteristicsState |= RW_CHAR_ITEM;
                 rwHandle = evt->data.evt_gatt_characteristic.characteristic;
               }
               if (characteristicsState == ALL_CHARS) {
                 printf("OK --- >All characteristics found.\r\n");
                 currentState = CHARACTERISTICS_FOUND;
                 /* Specified characteristics in Demo service found, will enable notification in gap complete event */
               }
         break;

         /* Received data from notification or read operations */
        case gecko_evt_gatt_characteristic_value_id:
               if (evt->data.evt_gatt_characteristic_value.characteristic == notifyHandle) {
                 printf("OK --- >Received notification data --> ");
                 for (uint8_t i = 0; i < evt->data.evt_gatt_characteristic_value.value.len; i++) {
                   printf("0x%02x ", evt->data.evt_gatt_characteristic_value.value.data[i]);
                 }
                 printf("\r\n");
               } else if (evt->data.evt_gatt_characteristic_value.characteristic == rwHandle) {
                 printf("OK --- >Received read data --> ");
                 for (uint8_t i = 0; i < evt->data.evt_gatt_characteristic_value.value.len; i++) {
                   printf("0x%02x ", evt->data.evt_gatt_characteristic_value.value.data[i]);
                 }
                 printf("\r\n");
               }
        break;

        case gecko_evt_gatt_procedure_completed_id:
      				if (currentState == SERVICE_FOUND) {
      					currentState = CHARACTERISTICS_DISCOVERING;
      					struct gecko_msg_gatt_discover_characteristics_rsp_t *ret = gecko_cmd_gatt_discover_characteristics(connHandle, serviceHandle);
      					if (ret->result == 0) {
      						printf("OK --- >Start discovering demo characteristics.\r\n");
      					} else {
      						printf("Error!!! Start Discovery characteristics error, error code = %d\r\n", ret->result);
      					}
      				} else if (currentState == CHARACTERISTICS_FOUND) {
      					currentState = ENABLING_NOTIFY;
      					struct gecko_msg_gatt_set_characteristic_notification_rsp_t *ret1 = gecko_cmd_gatt_set_characteristic_notification(connHandle, notifyHandle, gatt_notification);
      					if (ret1->result == 0) {
      						printf("OK --- >Set notification CCC to 0x0001.\r\n");
      					} else {
      						printf("Error!!! Enable notification error, error code = %d\r\n", ret1->result);
      					}
      				} else if (currentState == ENABLING_NOTIFY) {
      //		printf("gatt complete result --> %d\r\n", evt->data.evt_gatt_procedure_completed.result);
      					if (evt->data.evt_gatt_procedure_completed.result == 0) {
      						printf("OK --- >Notification enabled.\r\n");
      						currentState = NOTIFY_ENABLED;
									//Start SoftTimer for Write operations every 100ms
									gecko_cmd_hardware_set_soft_timer(3277, 0, 0);
									printf("OK --- > Central will Write to Server every 100ms \r\n");
									currentState = ENABLING_WRITE;

      					} else {
      						printf("Enable notification failed, error code = %d, try the handle next to characteristic.\r\n", evt->data.evt_gatt_procedure_completed.result);
      						uint8_t buf[2] = {
      								0x01,
      								0x00 };
      						gecko_cmd_gatt_write_characteristic_value(connHandle, notifyHandle + 1, 2, buf);

      					}
      				}
			break;

      case gecko_evt_gatt_mtu_exchanged_id:
	           printf("Exchanged MTU = %d\r\n", evt->data.evt_gatt_mtu_exchanged.mtu);
			break;

    	case gecko_evt_le_connection_closed_id:
    					Reset_variables();
    					printf("Disconnected. Scanning Restarted.\r\n");
    					/* Restart advertising after client has disconnected */
    					currentState = SCANNING;
    					gecko_cmd_le_gap_discover(le_gap_discover_generic);
			break;

			case gecko_evt_hardware_soft_timer_id:

     	  writeCounter++;

     	  //It sends the notifications
				gecko_cmd_gatt_write_characteristic_value(connHandle, rwHandle, 1, &writeCounter);

     	  printf("OK --- > Writing to Server %d \r\n", writeCounter);

         break;

    default:
      break;
  }
}
