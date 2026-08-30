#include "opc_handlers.h"
#include "hardware.h"

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"

#include "pico/cyw43_arch.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"

static UA_StatusCode
read_adc_datasource(UA_Server *server,
                    const UA_NodeId *sessionId, void *sessionContext,
                    const UA_NodeId *nodeId, void *nodeContext,
                    UA_Boolean sourceTimeStamp,
                    const UA_NumericRange *range,
                    UA_DataValue *dataValue)
{
    uint8_t adc_channel = (uint8_t)(uintptr_t)nodeContext;

    adc_select_input(adc_channel);
    uint16_t raw = adc_read();
    float voltage = (raw * 3.3f) / 4095.0f; /* 12-bit ADC conversion */

    UA_Variant_setScalarCopy(&dataValue->value, &voltage, &UA_TYPES[UA_TYPES_FLOAT]);
    dataValue->hasValue = true;
    dataValue->status = UA_STATUSCODE_GOOD;
    dataValue->hasStatus = true;

    if (sourceTimeStamp) {
        dataValue->sourceTimestamp = UA_DateTime_now();
        dataValue->hasSourceTimestamp = true;
    }
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode
read_gpio_datasource(UA_Server *server,
                     const UA_NodeId *sessionId, void *sessionContext,
                     const UA_NodeId *nodeId, void *nodeContext,
                     UA_Boolean sourceTimeStamp,
                     const UA_NumericRange *range,
                     UA_DataValue *dataValue)
{
    uint8_t gpio_pin = (uint8_t)(uintptr_t)nodeContext;
    UA_Boolean state = gpio_get(gpio_pin);

    UA_Variant_setScalarCopy(&dataValue->value, &state, &UA_TYPES[UA_TYPES_BOOLEAN]);
    dataValue->hasValue = true;
    dataValue->status = UA_STATUSCODE_GOOD;
    dataValue->hasStatus = true;

    if (sourceTimeStamp) {
        dataValue->sourceTimestamp = UA_DateTime_now();
        dataValue->hasSourceTimestamp = true;
    }
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode
write_gpio_datasource(UA_Server *server,
                      const UA_NodeId *sessionId, void *sessionContext,
                      const UA_NodeId *nodeId, void *nodeContext,
                      const UA_NumericRange *range,
                      const UA_DataValue *dataValue)
{
    uint8_t gpio_pin = (uint8_t)(uintptr_t)nodeContext;

    if (dataValue->hasValue && dataValue->value.type == &UA_TYPES[UA_TYPES_BOOLEAN]) {
        UA_Boolean new_state = *(UA_Boolean *)dataValue->value.data;
        gpio_set_dir(gpio_pin, GPIO_OUT);
        gpio_put(gpio_pin, new_state);
        return UA_STATUSCODE_GOOD;
    }
    
    /* Toggle onboard LED to indicate type mismatch error */
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    return UA_STATUSCODE_BADTYPEMISMATCH;
}

static UA_StatusCode
read_rssi_datasource(UA_Server *server,
                     const UA_NodeId *sessionId, void *sessionContext,
                     const UA_NodeId *nodeId, void *nodeContext,
                     UA_Boolean sourceTimeStamp,
                     const UA_NumericRange *range,
                     UA_DataValue *dataValue)
{
    int32_t rssi = read_rssi();

    UA_Variant_setScalarCopy(&dataValue->value, &rssi, &UA_TYPES[UA_TYPES_INT32]);
    dataValue->hasValue = true;
    dataValue->status = UA_STATUSCODE_GOOD;
    dataValue->hasStatus = true;

    if (sourceTimeStamp) {
        dataValue->sourceTimestamp = UA_DateTime_now();
        dataValue->hasSourceTimestamp = true;
    }
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode
read_cpu_temp_datasource(UA_Server *server,
                         const UA_NodeId *sessionId, void *sessionContext,
                         const UA_NodeId *nodeId, void *nodeContext,
                         UA_Boolean sourceTimeStamp,
                         const UA_NumericRange *range,
                         UA_DataValue *dataValue)
{
    adc_select_input(4); 
    uint16_t raw = adc_read();
    
    float voltage = (raw * 3.3f) / 4095.0f;
    float temp_c = 27.0f - ((voltage - 0.706f) / 0.001721f);

    UA_Variant_setScalarCopy(&dataValue->value, &temp_c, &UA_TYPES[UA_TYPES_FLOAT]);
    dataValue->hasValue = true;
    dataValue->status = UA_STATUSCODE_GOOD;
    dataValue->hasStatus = true;

    if (sourceTimeStamp) {
        dataValue->sourceTimestamp = UA_DateTime_now();
        dataValue->hasSourceTimestamp = true;
    }
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode
read_memory_datasource(UA_Server *server,
                       const UA_NodeId *sessionId, void *sessionContext,
                       const UA_NodeId *nodeId, void *nodeContext,
                       UA_Boolean sourceTimeStamp,
                       const UA_NumericRange *range,
                       UA_DataValue *dataValue)
{
    uint32_t free_heap = xPortGetFreeHeapSize();
    uint32_t allocated_heap = configTOTAL_HEAP_SIZE - free_heap;

    UA_Variant_setScalarCopy(&dataValue->value, &allocated_heap, &UA_TYPES[UA_TYPES_UINT32]);
    dataValue->hasValue = true;
    dataValue->status = UA_STATUSCODE_GOOD;
    dataValue->hasStatus = true;

    if (sourceTimeStamp) {
        dataValue->sourceTimestamp = UA_DateTime_now();
        dataValue->hasSourceTimestamp = true;
    }
    return UA_STATUSCODE_GOOD;
}

void add_variables(UA_Server *server)
{
    char node_id_str[32];
    char display_str[32];

    /* --- 1. ADC Folder & Variables --- */
    UA_ObjectAttributes adcFolderAttr = UA_ObjectAttributes_default;
    adcFolderAttr.displayName = UA_LOCALIZEDTEXT("en-US", "ADC");
    UA_NodeId adcFolderId = UA_NODEID_STRING(1, "ADC");

    UA_Server_addObjectNode(server, adcFolderId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
        UA_QUALIFIEDNAME(1, "ADC"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE),
        adcFolderAttr, NULL, NULL);

    for (uint8_t ch = 0; ch < 4; ch++) {
        uint8_t gpio_pin = 26 + ch;
        snprintf(node_id_str, sizeof(node_id_str), "ADC.Channel%d", ch);
        snprintf(display_str, sizeof(display_str), "ADC%d (GPIO%d)", ch, gpio_pin);

        UA_VariableAttributes attr = UA_VariableAttributes_default;
        attr.displayName = UA_LOCALIZEDTEXT("en-US", display_str);
        attr.dataType = UA_TYPES[UA_TYPES_FLOAT].typeId;
        attr.accessLevel = UA_ACCESSLEVELMASK_READ;

        UA_DataSource adcSource = (UA_DataSource){.read = read_adc_datasource, .write = NULL};

        UA_Server_addDataSourceVariableNode(server, UA_NODEID_STRING(1, node_id_str), adcFolderId,
            UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES), UA_QUALIFIEDNAME(1, display_str),
            UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), attr, adcSource, (void *)(uintptr_t)ch, NULL);
    }

    /* --- 2. GPIO Folder & Variables --- */
    UA_ObjectAttributes gpioFolderAttr = UA_ObjectAttributes_default;
    gpioFolderAttr.displayName = UA_LOCALIZEDTEXT("en-US", "GPIO");
    UA_NodeId gpioFolderId = UA_NODEID_STRING(1, "GPIO");

    UA_Server_addObjectNode(server, gpioFolderId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
        UA_QUALIFIEDNAME(1, "GPIO"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE),
        gpioFolderAttr, NULL, NULL);

    for (uint8_t pin = 0; pin <= 22; pin++) {
        snprintf(node_id_str, sizeof(node_id_str), "GPIO.%d", pin);
        snprintf(display_str, sizeof(display_str), "GPIO %d", pin);

        UA_VariableAttributes attr = UA_VariableAttributes_default;
        attr.displayName = UA_LOCALIZEDTEXT("en-US", display_str);
        attr.dataType = UA_TYPES[UA_TYPES_BOOLEAN].typeId;
        attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

        UA_DataSource gpioSource = (UA_DataSource){.read = read_gpio_datasource, .write = write_gpio_datasource};

        UA_Server_addDataSourceVariableNode(server, UA_NODEID_STRING(1, node_id_str), gpioFolderId,
            UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES), UA_QUALIFIEDNAME(1, display_str),
            UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), attr, gpioSource, (void *)(uintptr_t)pin, NULL);
    }

    /* --- 3. System Metrics (Attached to ServerStatus) --- */
    UA_NodeId serverStatusId = UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS);

    /* WiFi RSSI */
    UA_VariableAttributes rssiAttr = UA_VariableAttributes_default;
    rssiAttr.displayName = UA_LOCALIZEDTEXT("en-US", "WiFi_RSSI");
    rssiAttr.dataType = UA_TYPES[UA_TYPES_INT32].typeId;
    rssiAttr.accessLevel = UA_ACCESSLEVELMASK_READ;
    
    UA_DataSource rssiSource = (UA_DataSource){.read = read_rssi_datasource, .write = NULL};
    UA_Server_addDataSourceVariableNode(server, UA_NODEID_STRING(1, "Server.WiFi_RSSI"), serverStatusId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY), UA_QUALIFIEDNAME(1, "WiFi_RSSI"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE), rssiAttr, rssiSource, NULL, NULL);

    /* CPU Temperature */
    UA_VariableAttributes tempAttr = UA_VariableAttributes_default;
    tempAttr.displayName = UA_LOCALIZEDTEXT("en-US", "CPU_Temperature_C");
    tempAttr.dataType = UA_TYPES[UA_TYPES_FLOAT].typeId;
    tempAttr.accessLevel = UA_ACCESSLEVELMASK_READ;

    UA_DataSource tempSource = (UA_DataSource){.read = read_cpu_temp_datasource, .write = NULL};
    UA_Server_addDataSourceVariableNode(server, UA_NODEID_STRING(1, "Server.CPU_Temperature"), serverStatusId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY), UA_QUALIFIEDNAME(1, "CPU_Temperature_C"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE), tempAttr, tempSource, NULL, NULL);

    /* Memory Allocated */
    UA_VariableAttributes memAttr = UA_VariableAttributes_default;
    memAttr.displayName = UA_LOCALIZEDTEXT("en-US", "Memory_Allocated_Bytes");
    memAttr.dataType = UA_TYPES[UA_TYPES_UINT32].typeId;
    memAttr.accessLevel = UA_ACCESSLEVELMASK_READ;

    UA_DataSource memSource = (UA_DataSource){.read = read_memory_datasource, .write = NULL};
    UA_Server_addDataSourceVariableNode(server, UA_NODEID_STRING(1, "Server.Memory_Allocated"), serverStatusId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY), UA_QUALIFIEDNAME(1, "Memory_Allocated_Bytes"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE), memAttr, memSource, NULL, NULL);
}