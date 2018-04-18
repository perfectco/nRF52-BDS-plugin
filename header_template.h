/*
Copyright (c) 2017, Nordic Semiconductor ASA

All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form, except as embedded into a Nordic
   Semiconductor ASA integrated circuit in a product or a software update for
   such product, must reproduce the above copyright notice, this list of
   conditions and the following disclaimer in the documentation and/or other
   materials provided with the distribution.

3. Neither the name of Nordic Semiconductor ASA nor the names of its
   contributors may be used to endorse or promote products derived from this
   software without specific prior written permission.

4. This software, with or without modification, must only be used with a
   Nordic Semiconductor ASA integrated circuit.

5. Any software provided in binary form under this license must not be reverse
   engineered, decompiled, modified and/or disassembled.

THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/* This file was generated by plugin '<%= PluginInfo.Name %>' (BDS version <%= PluginInfo.AppVersion %>) */

#ifndef BLE_<%= ShortName.toUpperCase() %>_H__
#define BLE_<%= ShortName.toUpperCase() %>_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"
#include "app_util_bds.h"
#include "char_enc_dec.h"

<%
if(ErrorCodes.length > 0) { %>
// Error codes <%
    _.each(ErrorCodes, function(errCode) { %>
#define <%= utils.getNormalizedName(errCode.Name).toUpperCase() %> <%= errCode.Code %>; /* <%= errCode.Description %> */<%
});
}%>

/**@brief <%= Name %> event type. */
typedef enum
{ <%
    _.each(Characteristics, function(characteristic) { %>
    BLE_<%= ShortName.toUpperCase() %>_<%= characteristic.NormalizedName().toUpperCase() %>_EVT_NOTIFICATION_ENABLED,  /**< <%= characteristic.Name %> value notification enabled event. */
    BLE_<%= ShortName.toUpperCase() %>_<%= characteristic.NormalizedName().toUpperCase() %>_EVT_NOTIFICATION_DISABLED, /**< <%= characteristic.Name %> value notification disabled event. */<%
        if (characteristic.isNotifySupported() || characteristic.isIndicateSupported()) { %>
    BLE_<%= ShortName.toUpperCase() %>_<%= characteristic.NormalizedName().toUpperCase() %>_EVT_CCCD_WRITE, /**< <%= characteristic.Name %> CCCD write event. */<%
        }
        if (characteristic.isWriteSupported() || characteristic.isWriteWithoutResponseSupported() || characteristic.isReliableWriteSupported()) { %>
    BLE_<%= ShortName.toUpperCase() %>_<%= characteristic.NormalizedName().toUpperCase() %>_EVT_WRITE, /**< <%= characteristic.Name %> write event. */<%
        }
    }); %>
} ble_<%= ShortName %>_evt_type_t;

// Forward declaration of the ble_<%= ShortName %>_t type.
typedef struct ble_<%= ShortName %>_s ble_<%= ShortName %>_t;

<%
_.each(Characteristics, function(characteristic) {
_.each(characteristic.Descriptors, function(descriptor) {
    if(descriptor.UUID !== '2902') { // CCCD, this is already handled in SDK code
    _.each(descriptor.Fields, function(field) {
        if(field.isStruct()) {
            if(field.Enumerations && field.getEnumerations().length > 0) { %>
typedef enum
{ <%
            _.each(field.getEnumerations(), function(enumVal) { %>
    <%= characteristic.NormalizedName().toUpperCase() %>_<%= descriptor.NormalizedName().toUpperCase() %>_<%= field.NormalizedName().toUpperCase() %>_<%= enumVal.NormalizedName().toUpperCase() %> = <%= enumVal.Key %>, <%
                                });
            %>
} enum_<%= characteristic.NormalizedName() %>_<%= descriptor.NormalizedName() %>_<%= field.NormalizedName() %>_t;
<%
            }
			if(field.BitField) {
                _.each(field.BitField.Bits, function(bitField) {
                    if(bitField.Enumerations && bitField.getEnumerations().length > 0 && bitField.isBitEnumValid(field.NormalizedName(), bitField.NormalizedName())) { %>
typedef enum
{ <%
                        _.each(bitField.getEnumerations(), function(enumVal) { %>
    <%= bitField.NormalizedName().toUpperCase() %>_<%= enumVal.NormalizedName().toUpperCase() %> = <%= enumVal.Key %>, <%
                        });
%>
} enum_<%= field.NormalizedName() %>_<%= bitField.NormalizedName(field.NormalizedName()) %>_t;
<%
                    }
		        });
		    }%>
typedef struct
{<%
            if (field.isVariableLength()) { %>
    <%= field.FormatTypeName() %> <%= field.NormalizedName() %>;<%
            } else {
                if (field.Enumerations && field.getEnumerations().length > 0) { %>
    enum_<%= characteristic.NormalizedName() %>_<%= descriptor.NormalizedName() %>_<%= field.NormalizedName() %>_t <%= field.NormalizedName() %>; <%
                }
				if(field.BitField) {
                    _.each(field.BitField.Bits, function(bitField) { %>
    enum_<%= field.NormalizedName() %>_<%= bitField.NormalizedName() %>_t <%= bitField.NormalizedName() %>; <%
			        });
			    }
            }%>
} <%= ShortName %>_<%= characteristic.NormalizedName() %>_<%= descriptor.NormalizedName() %>_<%= field.NormalizedName() %>_t; <%
        }
    });
}});
});%>

<%
_.each(Characteristics, function(characteristic) {
_.each(characteristic.Descriptors, function(descriptor) {
    if(descriptor.UUID !== '2902') { // CCCD, this is already handled in SDK code
    if (descriptor.Fields.length > 0) {
%>

/**@brief <%= descriptor.Name %> structure. */
typedef struct
{<%
        _.each(descriptor.Fields, function(field) {
            if(field.isStruct()) { %>
    <%= ShortName %>_<%= characteristic.NormalizedName() %>_<%= descriptor.NormalizedName() %>_<%= field.NormalizedName() %>_t <%= field.NormalizedName() %>;<%
            }
            else { %>
    <%= field.FormatTypeName() %> <%= field.NormalizedName() %>;<%
        }}); %>
} ble_<%= ShortName %>_<%= characteristic.NormalizedName() %>_<%= descriptor.NormalizedName() %>_t;
<%
    }
}});
}); %>

<%

print(prettify(innerTemplates.CHAR_STRUCT_DECL(enriched)));

%>

/**@brief <%= Name %> Service event. */
typedef struct
{
    ble_<%= ShortName %>_evt_type_t evt_type;    /**< Type of event. */
    union {
        uint16_t cccd_value; /**< Holds decoded data in Notify and Indicate event handler. */<%
        _.each(Characteristics, function(characteristic) {
            if (characteristic.isWriteSupported() || characteristic.isWriteWithoutResponseSupported() || characteristic.isReliableWriteSupported()) { %>
        ble_<%= ShortName %>_<%= characteristic.NormalizedName() %>_t <%= characteristic.NormalizedName() %>; /**< Holds decoded data in Write event handler. */<%
            }
        });%>
    } params;
} ble_<%= ShortName %>_evt_t;

/**@brief <%= Name %> Service event handler type. */
typedef void (*ble_<%= ShortName %>_evt_handler_t) (ble_<%= ShortName %>_t * p_<%= ShortName %>, ble_<%= ShortName %>_evt_t * p_evt);

/**@brief <%= Name %> Service init structure. This contains all options and data needed for initialization of the service. */
typedef struct
{
    ble_<%= ShortName %>_evt_handler_t     evt_handler; /**< Event handler to be called for handling events in the <%= Name %> Service. */<%
_.each(Characteristics, function(characteristic) {
    if (characteristic.isReadSupported() && !characteristic.isReadMandatory()) { %>
    bool is_<%= characteristic.NormalizedName() %>_read_supported; /**< TRUE if read of <%= characteristic.Name %> is supported. */<%
    }
    if (characteristic.isNotifySupported() && !characteristic.isNotifyMandatory()) { %>
    bool is_<%= characteristic.NormalizedName() %>_notify_supported;    /**< TRUE if notification of <%= characteristic.Name %> is supported. */<%
    }
    if (characteristic.isIndicateSupported() && !characteristic.isIndicateMandatory()) { %>
    bool is_<%= characteristic.NormalizedName() %>_indicate_supported;    /**< TRUE if indication of <%= characteristic.Name %> is supported. */ <%
    }
    if (characteristic.isWriteSupported() && !characteristic.isWriteMandatory()) { %>
    bool is_<%= characteristic.NormalizedName() %>_write_supported;    /**< TRUE if write to <%= characteristic.Name %> is supported. */ <%
    }
    if (characteristic.isWriteWithoutResponseSupported() && !characteristic.isWriteWithoutResponseMandatory()) { %>
    bool is_<%= characteristic.NormalizedName() %>_write_wo_resp_supported;    /**< TRUE if write without response to <%= characteristic.Name %> is supported. */ <%
    }
    if (characteristic.isReliableWriteSupported() && !characteristic.isReliableWriteMandatory()) { %>
    bool is_<%= characteristic.NormalizedName() %>_reliable_write_supported;    /**< TRUE if reliable write to <%= characteristic.Name %> is supported. */ <%
    } %>
    <%= characteristic.StructName() %> ble_<%= ShortName %>_<%= characteristic.NormalizedName() %>_initial_value; /**< If not NULL, initial value of the <%= characteristic.Name %> characteristic. */ <%
}); %>
} ble_<%= ShortName %>_init_t;

/**@brief <%= Name %> Service structure. This contains various status information for the service.*/
struct ble_<%= ShortName %>_s
{
    ble_<%= ShortName %>_evt_handler_t evt_handler; /**< Event handler to be called for handling events in the <%= Name %> Service. */
    uint16_t service_handle; /**< Handle of <%= Name %> Service (as provided by the BLE stack). */<%

    addedDescriptorUuids = [];
_.each(Characteristics, function(characteristic) {
    if (characteristic.isReadSupported() && !characteristic.isReadMandatory()) { %>
    bool is_<%= characteristic.NormalizedName() %>_read_supported; /**< TRUE if read of <%= characteristic.Name %> is supported. */<%
    }
    if (characteristic.isNotifySupported() && !characteristic.isNotifyMandatory()) { %>
    bool is_<%= characteristic.NormalizedName()%>_notify_supported;    /**< TRUE if notification of <%= characteristic.Name %> is supported. */<%
    }
    if (characteristic.isIndicateSupported() && !characteristic.isIndicateMandatory()) { %>
    bool is_<%= characteristic.NormalizedName()%>_indicate_supported;    /**< TRUE if indication of <%= characteristic.Name %> is supported. */<%
    }
    if (characteristic.isWriteSupported() && !characteristic.isWriteMandatory()) { %>
    bool is_<%= characteristic.NormalizedName() %>_write_supported;    /**< TRUE if write to <%= characteristic.Name %> is supported. */ <%
    }
    if (characteristic.isWriteWithoutResponseSupported() && !characteristic.isWriteWithoutResponseMandatory()) { %>
    bool is_<%= characteristic.NormalizedName() %>_write_wo_resp_supported;    /**< TRUE if write without response to <%= characteristic.Name %> is supported. */ <%
    }
    if (characteristic.isReliableWriteSupported() && !characteristic.isReliableWriteMandatory()) { %>
    bool is_<%= characteristic.NormalizedName() %>_reliable_write_supported;    /**< TRUE if reliable write to <%= characteristic.Name %> is supported. */ <%
    } %>
    ble_gatts_char_handles_t <%= characteristic.NormalizedName() %>_handles; /**< Handles related to the <%= characteristic.Name %> characteristic. */<%
    _.each(characteristic.Descriptors, function(descriptor) {
        if(descriptor.UUID !== '2902') { // CCCD, this is already handled in SDK code
    %>
    ble_gatts_char_handles_t <%= characteristic.NormalizedName() %>_<%= descriptor.NormalizedName() %>_handles; /**< Handles related to the <%= descriptor.Name %> descriptor. */<%
    }});
}); %>
    uint16_t conn_handle; /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */
};

/**@brief Function for initializing the <%= Name %>.
 *
 * @param[out]  p_<%= ShortName %>       <%= Name %> Service structure. This structure will have to be supplied by
 *                          the application. It will be initialized by this function, and will later
 *                          be used to identify this particular service instance.
 * @param[in]   p_<%= ShortName %>_init  Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
 */
uint32_t ble_<%= ShortName %>_init(ble_<%= ShortName %>_t * p_<%= ShortName %>, const ble_<%= ShortName %>_init_t * p_<%= ShortName %>_init);

/**@brief Function for handling the Application's BLE Stack events.*/
void ble_<%= ShortName %>_on_ble_evt(ble_<%= ShortName %>_t * p_<%= ShortName %>, ble_evt_t const * p_ble_evt);
<%
addedDescriptorUuids = [];
_.each(Characteristics, function(characteristic) {
    if (characteristic.isReadSupported()) {
%>
/**@brief Function for setting the <%= characteristic.Name %>.
 *
 * @details Sets a new value of the <%= characteristic.Name %> characteristic. The new value will be sent
 *          to the client the next time the client reads the <%= characteristic.Name %> characteristic.
 *          This function is only generated if the characteristic's Read property is not 'Excluded'.
 *
 * @param[in]   p_<%= ShortName %>                 <%= Name %> Service structure.
 * @param[in]   p_<%= characteristic.NormalizedName() %>  New <%= characteristic.Name %>.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_<%= ShortName %>_<%= characteristic.NormalizedName()%>_set(ble_<%= ShortName %>_t * p_<%= ShortName %>, ble_<%= ShortName %>_<%= characteristic.NormalizedName()%>_t * p_<%= characteristic.NormalizedName() %>);
<%
    }
    if (characteristic.isNotifySupported() || characteristic.isIndicateSupported()) {
%>
/**@brief Function for sending the <%= characteristic.Name %>.
 *
 * @details The application calls this function after having performed a <%= characteristic.Name.toLowerCase() %>.
 *          The <%= characteristic.Name.toLowerCase() %> data is encoded and sent to the client.
 *          This function is only generated if the characteristic's Notify or Indicate property is not 'Excluded'.
 *
 * @param[in]   p_<%= ShortName %>                    <%= Name %> Service structure.
 * @param[in]   p_<%= characteristic.NormalizedName() %>               New <%= characteristic.Name.toLowerCase() %>.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_<%= ShortName %>_<%= characteristic.NormalizedName()%>_send(ble_<%= ShortName %>_t * p_<%= ShortName %>, ble_<%= ShortName %>_<%= characteristic.NormalizedName()%>_t * p_<%= characteristic.NormalizedName() %>);
<%
    }
    _.each(characteristic.Descriptors, function(descriptor) {
        if(descriptor.UUID !== '2902') { // CCCD, this is already handled in SDK code
            if (descriptor.isReadSupported()) { %>
/**@brief Function for setting the <%= descriptor.Name %>.
 *
 * @details Sets a new value of the <%= descriptor.Name %> descriptor. The new value will be sent
 *          to the client the next time the client reads the <%= descriptor.Name %> descriptor.
 *          This function is only generated if the descriptor's Read property is not 'Excluded'.
 *
 * @param[in]   p_<%= ShortName %>                 <%= Name %> Service structure.
 * @param[in]   p_<%= characteristic.NormalizedName() %>_<%= characteristic.NormalizedName() %>_<%= descriptor.NormalizedName() %>  New <%= descriptor.Name %>.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_<%= ShortName %>_<%= characteristic.NormalizedName()%>_<%= descriptor.NormalizedName()%>_set(ble_<%= ShortName %>_t * p_<%= ShortName %>, ble_<%= ShortName %>_<%= characteristic.NormalizedName() %>_<%= descriptor.NormalizedName()%>_t * p_<%= characteristic.NormalizedName() %>_<%= descriptor.NormalizedName() %>);
<%          }
            if(descriptor.isWriteSupported()) {
// TODO: What about descriptor.Write?
            }
    }});
});
%>
#endif //_BLE_<%= ShortName.toUpperCase() %>_H__
