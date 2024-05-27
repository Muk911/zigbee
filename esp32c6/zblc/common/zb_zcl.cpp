#include <Arduino.h>
#include "esp_log.h"
#include "zcl/esp_zigbee_zcl_command.h"
#include "zb_zcl.h"
#include "zb_defs.h"

#define TAG "zb_zcl"

zb_cluster_info_t *clusters = NULL;

void addClusterAttrs(uint16_t clusterId, zb_attr_info_t *attrs, uint16_t count) 
{
  zb_cluster_info_t *cluster = (zb_cluster_info_t *) malloc(sizeof(zb_cluster_info_t));
  cluster->clusterId = clusterId;
  cluster->attrs = attrs;
  cluster->attrCount = count;
  cluster->next = clusters;
  clusters = cluster;
}

uint8_t uint8Unknown = 0x80;
uint16_t uint16Unknown = 0x8000;
uint32_t uintZero = 0;
uint32_t uintFFFF = 0xffffffff;
uint8_t boolFalse = 0;
uint8_t boolTrue = 1;

void fillInfo()
{
  #ifdef USE_BASIC_CLUSTER
    static uint8_t zclVersion = 3;
    static zb_attr_info_t a0[] = {
      {ESP_ZB_ZCL_ATTR_BASIC_ZCL_VERSION_ID,                  ESP_ZB_ZCL_ATTR_TYPE_U8,           ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  true,  &zclVersion}, 
      {ESP_ZB_ZCL_ATTR_BASIC_APPLICATION_VERSION_ID,          ESP_ZB_ZCL_ATTR_TYPE_U8,           ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, &uintZero},
      {ESP_ZB_ZCL_ATTR_BASIC_STACK_VERSION_ID,                ESP_ZB_ZCL_ATTR_TYPE_U16,          ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, &uintZero},
      {ESP_ZB_ZCL_ATTR_BASIC_HW_VERSION_ID,                   ESP_ZB_ZCL_ATTR_TYPE_U16,          ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, &uintZero},
      {ESP_ZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID,            ESP_ZB_ZCL_ATTR_TYPE_CHAR_STRING,  ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, NULL},
      {ESP_ZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID,             ESP_ZB_ZCL_ATTR_TYPE_CHAR_STRING,  ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, NULL},
      {ESP_ZB_ZCL_ATTR_BASIC_DATE_CODE_ID,                    ESP_ZB_ZCL_ATTR_TYPE_CHAR_STRING,  ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, NULL},
      {ESP_ZB_ZCL_ATTR_BASIC_POWER_SOURCE_ID,                 ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM,    ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  true,  &uintZero},
      {ESP_ZB_ZCL_ATTR_BASIC_GENERIC_DEVICE_CLASS_ID,         ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM,    ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, &uintFFFF},
      {ESP_ZB_ZCL_ATTR_BASIC_GENERIC_DEVICE_TYPE_ID,          ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM,    ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, &uintFFFF},
      {ESP_ZB_ZCL_ATTR_BASIC_PRODUCT_CODE_ID,                 ESP_ZB_ZCL_ATTR_TYPE_OCTET_STRING, ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, NULL},
      {ESP_ZB_ZCL_ATTR_BASIC_PRODUCT_URL_ID,                  ESP_ZB_ZCL_ATTR_TYPE_CHAR_STRING,  ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, NULL},
      {ESP_ZB_ZCL_ATTR_BASIC_MANUFACTURER_VERSION_DETAILS_ID, ESP_ZB_ZCL_ATTR_TYPE_CHAR_STRING,  ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, NULL},
      {ESP_ZB_ZCL_ATTR_BASIC_SERIAL_NUMBER_ID,                ESP_ZB_ZCL_ATTR_TYPE_CHAR_STRING,  ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, NULL},
      {ESP_ZB_ZCL_ATTR_BASIC_PRODUCT_LABEL_ID,                ESP_ZB_ZCL_ATTR_TYPE_CHAR_STRING,  ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, NULL},
      {ESP_ZB_ZCL_ATTR_BASIC_LOCATION_DESCRIPTION_ID,         ESP_ZB_ZCL_ATTR_TYPE_CHAR_STRING,  ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, NULL},
      {ESP_ZB_ZCL_ATTR_BASIC_PHYSICAL_ENVIRONMENT_ID,         ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM,    ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &uintZero},
      {ESP_ZB_ZCL_ATTR_BASIC_DEVICE_ENABLED_ID,               ESP_ZB_ZCL_ATTR_TYPE_BOOL,         ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &boolTrue},
      {ESP_ZB_ZCL_ATTR_BASIC_ALARM_MASK_ID,                   ESP_ZB_ZCL_ATTR_TYPE_8BITMAP,      ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &uintZero},
      {ESP_ZB_ZCL_ATTR_BASIC_DISABLE_LOCAL_CONFIG_ID,         ESP_ZB_ZCL_ATTR_TYPE_8BITMAP,      ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &uintZero},
      {ESP_ZB_ZCL_ATTR_BASIC_SW_BUILD_ID,                     ESP_ZB_ZCL_ATTR_TYPE_CHAR_STRING,  ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, NULL}
    };
    addClusterAttrs(ESP_ZB_ZCL_CLUSTER_ID_BASIC, a0, sizeof(a0)/sizeof(zb_attr_info_t));
  #endif // USE_BASIC_CLUSTER

  #ifdef USE_IDENTIFY_CLUSTER
    static zb_attr_info_t a3[] = {
      {ESP_ZB_ZCL_ATTR_IDENTIFY_IDENTIFY_TIME_ID, ESP_ZB_ZCL_ATTR_TYPE_U8, ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, true, &uint8Unknown}
    };
    addClusterAttrs(ESP_ZB_ZCL_CLUSTER_ID_IDENTIFY, a3, sizeof(a3)/sizeof(zb_attr_info_t));
  #endif // USE_IDENTIFY_CLUSTER

  #ifdef USE_ON_OFF_CLUSTER
    static zb_attr_info_t a6[] = {
      {ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID,            ESP_ZB_ZCL_ATTR_TYPE_BOOL,      ESP_ZB_ZCL_ATTR_ACCESS_RPS, true, &boolFalse}, 
      {ESP_ZB_ZCL_ATTR_ON_OFF_GLOBAL_SCENE_CONTROL, ESP_ZB_ZCL_ATTR_TYPE_BOOL,      ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, &boolTrue},
      {ESP_ZB_ZCL_ATTR_ON_OFF_ON_TIME,              ESP_ZB_ZCL_ATTR_TYPE_U16,       ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &uintZero},
      {ESP_ZB_ZCL_ATTR_ON_OFF_OFF_WAIT_TIME,        ESP_ZB_ZCL_ATTR_TYPE_U16,       ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &uintZero},
      {ESP_ZB_ZCL_ATTR_ON_OFF_START_UP_ON_OFF,      ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM, ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &uintZero} 
    };
    addClusterAttrs(ESP_ZB_ZCL_CLUSTER_ID_ON_OFF, a6, sizeof(a6)/sizeof(zb_attr_info_t));
  #endif // USE_ON_OFF_CLUSTER

  #ifdef USE_LEVEL_CLUSTER
    static zb_attr_info_t a8[] = {
      {ESP_ZB_ZCL_ATTR_LEVEL_CONTROL_CURRENT_LEVEL_ID,          ESP_ZB_ZCL_ATTR_TYPE_U8,        ESP_ZB_ZCL_ATTR_ACCESS_RPS,        true,  &uintFFFF},
      {ESP_ZB_ZCL_ATTR_LEVEL_CONTROL_REMAINING_TIME_ID,         ESP_ZB_ZCL_ATTR_TYPE_U16,       ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, &uintZero},
      {ESP_ZB_ZCL_ATTR_LEVEL_CONTROL_MIN_LEVEL_ID,              ESP_ZB_ZCL_ATTR_TYPE_U8,        ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, &uintZero},
      {ESP_ZB_ZCL_ATTR_LEVEL_CONTROL_MAX_LEVEL_ID,              ESP_ZB_ZCL_ATTR_TYPE_U8,        ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, &uintFFFF},
      {ESP_ZB_ZCL_ATTR_LEVEL_CONTROL_CURRENT_FREQUENCY_ID,      ESP_ZB_ZCL_ATTR_TYPE_U16,       ESP_ZB_ZCL_ATTR_ACCESS_RPS,        false, &uintZero},
      {ESP_ZB_ZCL_ATTR_LEVEL_CONTROL_MIN_FREQUENCY_ID,          ESP_ZB_ZCL_ATTR_TYPE_U16,       ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, &uintZero},
      {ESP_ZB_ZCL_ATTR_LEVEL_CONTROL_MAX_FREQUENCY_ID,          ESP_ZB_ZCL_ATTR_TYPE_U16,       ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, &uintZero}, 
      {ESP_ZB_ZCL_ATTR_LEVEL_CONTROL_ON_OFF_TRANSITION_TIME_ID, ESP_ZB_ZCL_ATTR_TYPE_U16,       ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &uintZero},
      {ESP_ZB_ZCL_ATTR_LEVEL_CONTROL_ON_LEVEL_ID,               ESP_ZB_ZCL_ATTR_TYPE_U8,        ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &uintFFFF}, 
      {ESP_ZB_ZCL_ATTR_LEVEL_CONTROL_ON_TRANSITION_TIME_ID,     ESP_ZB_ZCL_ATTR_TYPE_U16,       ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &uintFFFF},
      {ESP_ZB_ZCL_ATTR_LEVEL_CONTROL_OFF_TRANSITION_TIME_ID,    ESP_ZB_ZCL_ATTR_TYPE_U16,       ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &uintFFFF},
      {ESP_ZB_ZCL_ATTR_LEVEL_CONTROL_DEFAULT_MOVE_RATE_ID,      ESP_ZB_ZCL_ATTR_TYPE_U16,       ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &uintZero}, // Def????
      {ESP_ZB_ZCL_ATTR_LEVEL_CONTROL_OPTIONS_ID,                ESP_ZB_ZCL_ATTR_TYPE_8BITMAP,   ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &uintZero},
      {ESP_ZB_ZCL_ATTR_LEVEL_CONTROL_START_UP_CURRENT_LEVEL_ID, ESP_ZB_ZCL_ATTR_TYPE_U8,        ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &uintZero}  // Def????
    };
    addClusterAttrs(ESP_ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL, a8, sizeof(a8)/sizeof(zb_attr_info_t)); 
  #endif // USE_LEVEL_CLUSTER

  #ifdef USE_TEMPERATURE_MEASUREMENT_CLUSTER
    static zb_attr_info_t a402[] = {
      {ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID,      ESP_ZB_ZCL_ATTR_TYPE_16BIT,  ESP_ZB_ZCL_ATTR_ACCESS_RP,        true,  &uint16Unknown}, 
      {ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_MIN_VALUE_ID,  ESP_ZB_ZCL_ATTR_TYPE_16BIT,  ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY, true,  &uint16Unknown},
      {ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_MAX_VALUE_ID,  ESP_ZB_ZCL_ATTR_TYPE_16BIT,  ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY, true,  &uint16Unknown},
      {ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_TOLERANCE_ID,  ESP_ZB_ZCL_ATTR_TYPE_U16,    ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY, false, &uintZero} 
    };
    addClusterAttrs(ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT, a402, sizeof(a402)/sizeof(zb_attr_info_t));
  #endif // USE_TEMPERATURE_MEASUREMENT_CLUSTER

  #ifdef USE_PRESSURE_MEASUREMENT_CLUSTER
    static zb_attr_info_t a403[] = {
      {ESP_ZB_ZCL_ATTR_PRESSURE_MEASUREMENT_VALUE_ID,             ESP_ZB_ZCL_ATTR_TYPE_16BIT,  ESP_ZB_ZCL_ATTR_ACCESS_RP,        true,  &uint16Unknown}, //MinMeasuredValue – MaxMeasuredValue 
      {ESP_ZB_ZCL_ATTR_PRESSURE_MEASUREMENT_MIN_VALUE_ID,         ESP_ZB_ZCL_ATTR_TYPE_16BIT,  ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY, true,  &uint16Unknown}, //0x8001– 0x7ffe
      {ESP_ZB_ZCL_ATTR_PRESSURE_MEASUREMENT_MAX_VALUE_ID,         ESP_ZB_ZCL_ATTR_TYPE_16BIT,  ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY, true,  &uint16Unknown}, //0x8002– 0x7fff
      {0x0003,                                                    ESP_ZB_ZCL_ATTR_TYPE_U16,    ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY, false, &uintZero}, //0x0000 – 0x0800
      {ESP_ZB_ZCL_ATTR_PRESSURE_MEASUREMENT_SCALED_VALUE_ID,      ESP_ZB_ZCL_ATTR_TYPE_16BIT,  ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY, false, &uintZero}, //MinScaledValue – MaxScaledValue
      {ESP_ZB_ZCL_ATTR_PRESSURE_MEASUREMENT_MIN_SCALED_VALUE_ID,  ESP_ZB_ZCL_ATTR_TYPE_16BIT,  ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY, false, &uint16Unknown}, //0x8001-0x7ffe
      {ESP_ZB_ZCL_ATTR_PRESSURE_MEASUREMENT_MAX_SACLED_VALUE_ID,  ESP_ZB_ZCL_ATTR_TYPE_16BIT,  ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY, false, &uint16Unknown}, //0x8002-0x7fff
      {ESP_ZB_ZCL_ATTR_PRESSURE_MEASUREMENT_SCALED_TOLERANCE_ID,  ESP_ZB_ZCL_ATTR_TYPE_U16,    ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY, false, &uintZero}, //0x0000 – 0x0800 
      {ESP_ZB_ZCL_ATTR_PRESSURE_MEASUREMENT_SCALE_ID,             ESP_ZB_ZCL_ATTR_TYPE_8BIT,   ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY, false, &uintZero} //0x81 – 0x7f
    };
    addClusterAttrs(ESP_ZB_ZCL_CLUSTER_ID_PRESSURE_MEASUREMENT, a403, sizeof(a403)/sizeof(zb_attr_info_t));
  #endif // USE_PRESSURE_MEASUREMENT_CLUSTER

  #ifdef USE_HUMIDITY_MEASUREMENT_CLUSTER
    static zb_attr_info_t a405[] = {
      {ESP_ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_ID,      ESP_ZB_ZCL_ATTR_TYPE_U16,  ESP_ZB_ZCL_ATTR_ACCESS_RP,        true,  &uintFFFF}, //MinMeasuredValue – MaxMeasuredValue
      {ESP_ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_MIN_VALUE_ID,  ESP_ZB_ZCL_ATTR_TYPE_U16,  ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY, true,  &uintFFFF}, //0x0000 – 0x270f
      {ESP_ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_MAX_VALUE_ID,  ESP_ZB_ZCL_ATTR_TYPE_U16,  ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY, true,  &uintFFFF}, //0x0001 – 0x2710
      {ESP_ZB_ZCL_ATTR_REL_HUMIDITY_TOLERANCE_ID,              ESP_ZB_ZCL_ATTR_TYPE_U16,  ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY, false, &uintZero}  //0x0000 – 0x0800
    };
    addClusterAttrs(ESP_ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT, a405, sizeof(a405)/sizeof(zb_attr_info_t));
  #endif // USE_HUMIDITY_MEASUREMENT_CLUSTER

  #ifdef USE_TIME_CLUSTER
    static zb_attr_info_t aa[] = {
      {ESP_ZB_ZCL_ATTR_TIME_TIME_ID,                            ESP_ZB_ZCL_ATTR_TYPE_UTC_TIME,  ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, true,  &uintFFFF},
      {ESP_ZB_ZCL_ATTR_TIME_TIME_STATUS_ID,                     ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM, ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, true,  &uintZero},
      {ESP_ZB_ZCL_ATTR_TIME_TIME_ZONE_ID,                       ESP_ZB_ZCL_ATTR_TYPE_32BIT,     ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &uintZero},
      {ESP_ZB_ZCL_ATTR_TIME_DST_START_ID,                       ESP_ZB_ZCL_ATTR_TYPE_32BIT,     ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &uintFFFF},
      {ESP_ZB_ZCL_ATTR_TIME_DST_END_ID,                         ESP_ZB_ZCL_ATTR_TYPE_32BIT,     ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &uintFFFF},
      {ESP_ZB_ZCL_ATTR_TIME_DST_SHIFT_ID,                       ESP_ZB_ZCL_ATTR_TYPE_U32,       ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &uintZero},
      {ESP_ZB_ZCL_ATTR_TIME_STANDARD_TIME_ID,                   ESP_ZB_ZCL_ATTR_TYPE_U32,       ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, &uintFFFF},
      {ESP_ZB_ZCL_ATTR_TIME_LOCAL_TIME_ID,                      ESP_ZB_ZCL_ATTR_TYPE_U32,       ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, &uintFFFF},
      {ESP_ZB_ZCL_ATTR_TIME_LAST_SET_TIME_ID,                   ESP_ZB_ZCL_ATTR_TYPE_UTC_TIME,  ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, &uintFFFF},
      {ESP_ZB_ZCL_ATTR_TIME_VALID_UNTIL_TIME_ID,                ESP_ZB_ZCL_ATTR_TYPE_UTC_TIME,  ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &uintFFFF}
    };
    addClusterAttrs(ESP_ZB_ZCL_CLUSTER_ID_TIME, aa, sizeof(aa)/sizeof(zb_attr_info_t));
  #endif // USE_TIME_CLUSTER

  #ifdef USE_THERMOSTAT_CLUSTER 
    static int16_t absMinHeatSetpointLimit = 0x02bc;
    static int16_t absMaxHeatSetpointLimit = 0x0bb8;
    static int16_t absMinCoolSetpointLimit = 0x0640;
    static int16_t absMaxCoolSetpointLimit = 0x0c80;
    static int16_t occupiedCoolingSetpoint = 0x0a28;
    static int16_t occupiedHeatingSetpoint = 0x07d0;
    static int16_t unoccupiedCoolingSetpoint = 0x0a28;
    static int16_t unoccupiedHeatingSetpoint = 0x07d0;
    static int16_t minHeatSetpointLimit = 0x02bc;
    static int16_t maxHeatSetpointLimit = 0x0bb8;
    static int16_t minCoolSetpointLimit = 0x0640;
    static int16_t maxCoolSetpointLimit = 0x0c80;
    static int8_t minSetpointDeadBand = 0x19;
    static uint8_t controlSequenceOfOperation = 0x04;
    static uint8_t systemMode = 0x01;

    static zb_attr_info_t a201[] = {
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_LOCAL_TEMPERATURE_ID,                     ESP_ZB_ZCL_ATTR_TYPE_16BIT,     ESP_ZB_ZCL_ATTR_ACCESS_RP,         true,  &uintZero},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_OUTDOOR_TEMPERATURE_ID,                   ESP_ZB_ZCL_ATTR_TYPE_16BIT,     ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, &uintZero},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPANCY_ID,                             ESP_ZB_ZCL_ATTR_TYPE_8BITMAP,   ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, &uintZero},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_ABS_MIN_HEAT_SETPOINT_LIMIT_ID,           ESP_ZB_ZCL_ATTR_TYPE_16BIT,     ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, &absMinHeatSetpointLimit},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_ABS_MAX_HEAT_SETPOINT_LIMIT_ID,           ESP_ZB_ZCL_ATTR_TYPE_16BIT,     ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, &absMaxHeatSetpointLimit},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_ABS_MIN_COOL_SETPOINT_LIMIT_ID,           ESP_ZB_ZCL_ATTR_TYPE_16BIT,     ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, &absMinCoolSetpointLimit},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_ABS_MAX_COOL_SETPOINT_LIMIT_ID,           ESP_ZB_ZCL_ATTR_TYPE_16BIT,     ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, &absMaxCoolSetpointLimit},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_PI_COOLING_DEMAND_ID,                     ESP_ZB_ZCL_ATTR_TYPE_U8,        ESP_ZB_ZCL_ATTR_ACCESS_RP,         false, &uintZero},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_PI_HEATING_DEMAND_ID,                     ESP_ZB_ZCL_ATTR_TYPE_U8,        ESP_ZB_ZCL_ATTR_ACCESS_RP,         false, &uintZero},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_HVAC_SYSTEM_TYPE_CONFIGURATION_ID,        ESP_ZB_ZCL_ATTR_TYPE_8BITMAP,   ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &uintZero},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_LOCAL_TEMPERATURE_CALIBRATION_ID,         ESP_ZB_ZCL_ATTR_TYPE_8BIT,      ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &uintZero},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPIED_COOLING_SETPOINT_ID,             ESP_ZB_ZCL_ATTR_TYPE_16BIT,     ESP_ZB_ZCL_ATTR_ACCESS_RWS,        false, &occupiedCoolingSetpoint},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPIED_HEATING_SETPOINT_ID,             ESP_ZB_ZCL_ATTR_TYPE_16BIT,     ESP_ZB_ZCL_ATTR_ACCESS_RWS,        true,  &occupiedHeatingSetpoint},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_UNOCCUPIED_COOLING_SETPOINT_ID,           ESP_ZB_ZCL_ATTR_TYPE_16BIT,     ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &unoccupiedCoolingSetpoint},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_UNOCCUPIED_HEATING_SETPOINT_ID,           ESP_ZB_ZCL_ATTR_TYPE_16BIT,     ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &unoccupiedHeatingSetpoint},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_MIN_HEAT_SETPOINT_LIMIT_ID,               ESP_ZB_ZCL_ATTR_TYPE_16BIT,     ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &minHeatSetpointLimit},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_MAX_HEAT_SETPOINT_LIMIT_ID,               ESP_ZB_ZCL_ATTR_TYPE_16BIT,     ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &maxHeatSetpointLimit},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_MIN_COOL_SETPOINT_LIMIT_ID,               ESP_ZB_ZCL_ATTR_TYPE_16BIT,     ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &minCoolSetpointLimit},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_MAX_COOL_SETPOINT_LIMIT_ID,               ESP_ZB_ZCL_ATTR_TYPE_16BIT,     ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &maxCoolSetpointLimit},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_MIN_SETPOINT_DEAD_BAND_ID,                ESP_ZB_ZCL_ATTR_TYPE_8BIT,      ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &minSetpointDeadBand},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_REMOTE_SENSING_ID,                        ESP_ZB_ZCL_ATTR_TYPE_8BITMAP,   ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &uintZero},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_CONTROL_SEQUENCE_OF_OPERATION_ID,         ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM, ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, true,  &controlSequenceOfOperation},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_SYSTEM_MODE_ID,                           ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM, ESP_ZB_ZCL_ATTR_ACCESS_RWS,        true,  &systemMode},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_ALARM_MASK_ID,                            ESP_ZB_ZCL_ATTR_TYPE_8BITMAP,   ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, &uintZero},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_RUNNING_MODE_ID,                          ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM, ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, &uintZero},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_START_OF_WEEK_ID,                         ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM, ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, &uintZero},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_NUMBER_OF_WEEKLY_TRANSITIONS_ID,          ESP_ZB_ZCL_ATTR_TYPE_U8,        ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, &uintZero},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_NUMBER_OF_DAILY_TRANSITIONS_ID,           ESP_ZB_ZCL_ATTR_TYPE_U8,        ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, &uintZero},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_TEMPERATURE_SETPOINT_HOLD_ID,             ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM, ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &uintZero},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_TEMPERATURE_SETPOINT_HOLD_DURATION_ID,    ESP_ZB_ZCL_ATTR_TYPE_U16,       ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &uintFFFF},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_THERMOSTAT_PROGRAMMING_OPERATION_MODE_ID, ESP_ZB_ZCL_ATTR_TYPE_8BITMAP,   ESP_ZB_ZCL_ATTR_ACCESS_RWP,        false, &uintZero},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_THERMOSTAT_RUNNING_STATE_ID,              ESP_ZB_ZCL_ATTR_TYPE_16BITMAP,  ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, &uintZero},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_SETPOINT_CHANGE_SOURCE_ID,                ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM, ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, &uintZero},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_SETPOINT_CHANGE_AMOUNT_ID,                ESP_ZB_ZCL_ATTR_TYPE_U16,       ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, &uint16Unknown},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_SETPOINT_CHANGE_SOURCE_TIMESTAMP_ID,      ESP_ZB_ZCL_ATTR_TYPE_UTC_TIME,  ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, &uintZero},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPIED_SETBACK_ID,                      ESP_ZB_ZCL_ATTR_TYPE_U8,        ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &uintFFFF},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPIED_SETBACK_MIN_ID,                  ESP_ZB_ZCL_ATTR_TYPE_U8,        ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, &uintFFFF},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPIED_SETBACK_MAX_ID,                  ESP_ZB_ZCL_ATTR_TYPE_U8,        ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, &uintFFFF},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_UNOCCUPIED_SETBACK_ID,                    ESP_ZB_ZCL_ATTR_TYPE_U8,        ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &uintFFFF},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_UNOCCUPIED_SETBACK_MIN_ID,                ESP_ZB_ZCL_ATTR_TYPE_U8,        ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, &uintFFFF},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_UNOCCUPIED_SETBACK_MAX_ID,                ESP_ZB_ZCL_ATTR_TYPE_U8,        ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, &uintFFFF},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_EMERGENCY_HEAT_DELTA_ID,                  ESP_ZB_ZCL_ATTR_TYPE_U8,        ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &uintFFFF},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_AC_TYPE_ID,                               ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM, ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &uintZero},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_AC_CAPACITY_ID,                           ESP_ZB_ZCL_ATTR_TYPE_U16,       ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &uintZero},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_AC_REFRIGERANT_TYPE_ID,                   ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM, ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &uintZero},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_AC_COMPRESSOR_TYPE_ID,                    ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM, ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &uintZero},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_AC_ERROR_CODE_ID,                         ESP_ZB_ZCL_ATTR_TYPE_32BITMAP,  ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &uintZero},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_AC_LOUVER_POSITION_ID,                    ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM, ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &uintZero},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_AC_COIL_TEMPERATURE_ID,                   ESP_ZB_ZCL_ATTR_TYPE_16BIT,     ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,  false, &uintZero},
      {ESP_ZB_ZCL_ATTR_THERMOSTAT_AC_CAPACITY_FORMAT_ID,                    ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM, ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, false, &uintZero}  
    };
    addClusterAttrs(ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, a201, sizeof(a201)/sizeof(zb_attr_info_t));
  #endif // USE_THERMOSTAT_CLUSTER
}

void initZclData(void)
{
  if(clusters) return;
  fillInfo();
}

void dumpClusterInfos(void)
{
  zb_cluster_info_t *cluster = clusters;

  ESP_LOGI(TAG, "dumpClusterInfos()");
  while(cluster) {
    ESP_LOGI(TAG, "scan cluster %04X attrCount=%d", cluster->clusterId, cluster->attrCount);
    for(int i = 0; i < cluster->attrCount; i++) {
      zb_attr_info_t *attr = cluster->attrs + i;
      ESP_LOGI(TAG, "  scan attr %04X", attr->attrId);
    }
    cluster = cluster->next;
  }
}

zb_cluster_info_t *findClusterInfo(uint16_t clusterId)
{
  initZclData();
  zb_cluster_info_t *cluster = clusters;

  //ESP_LOGI(TAG, "find cluster %04X", clusterId);
  while(cluster) {
    //ESP_LOGI(TAG, "check cluster %04X", cluster->clusterId);
    if(cluster->clusterId == clusterId) {
      //ESP_LOGI(TAG, "cluster found");
      return cluster;
    }
    cluster = cluster->next;
  }
  return NULL;
}

zb_attr_info_t *findClusterAttrInfo(uint16_t clusterId, uint16_t attrId)
{
  zb_cluster_info_t *cluster = findClusterInfo(clusterId);
  if(!cluster) return NULL;

  for(int i = 0; i < cluster->attrCount; i++ ) {
    if(cluster->attrs[i].attrId == attrId)
      return &cluster->attrs[i];
  } 
  return NULL;
}

bool isVarAttrType(uint8_t attrType)
{
	switch (attrType) {
    case ESP_ZB_ZCL_ATTR_TYPE_OCTET_STRING:
    case ESP_ZB_ZCL_ATTR_TYPE_CHAR_STRING:
    case ESP_ZB_ZCL_ATTR_TYPE_LONG_OCTET_STRING:
    case ESP_ZB_ZCL_ATTR_TYPE_LONG_CHAR_STRING:
    case ESP_ZB_ZCL_ATTR_TYPE_ARRAY:
    case ESP_ZB_ZCL_ATTR_TYPE_STRUCTURE:
    case ESP_ZB_ZCL_ATTR_TYPE_SET:
    case ESP_ZB_ZCL_ATTR_TYPE_BAG:
    case ESP_ZB_ZCL_ATTR_TYPE_INVALID:
      return true;

    default:
      return false;
  }
}

uint8_t getAttrTypeLength(uint8_t attrType)
{
	uint8_t len;

	switch (attrType) {
    case ESP_ZB_ZCL_ATTR_TYPE_8BIT:
    case ESP_ZB_ZCL_ATTR_TYPE_BOOL:
    case ESP_ZB_ZCL_ATTR_TYPE_8BITMAP:
    case ESP_ZB_ZCL_ATTR_TYPE_U8:
    case ESP_ZB_ZCL_ATTR_TYPE_S8:
    case ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM:
      len = 1;
      break;

    case ESP_ZB_ZCL_ATTR_TYPE_16BIT:
    case ESP_ZB_ZCL_ATTR_TYPE_16BITMAP:
    case ESP_ZB_ZCL_ATTR_TYPE_U16:
    case ESP_ZB_ZCL_ATTR_TYPE_S16:
    case ESP_ZB_ZCL_ATTR_TYPE_16BIT_ENUM:
    case ESP_ZB_ZCL_ATTR_TYPE_SEMI:
    case ESP_ZB_ZCL_ATTR_TYPE_CLUSTER_ID:
    case ESP_ZB_ZCL_ATTR_TYPE_ATTRIBUTE_ID:
      len = 2;
      break;

    case ESP_ZB_ZCL_ATTR_TYPE_24BIT:
    case ESP_ZB_ZCL_ATTR_TYPE_24BITMAP:
    case ESP_ZB_ZCL_ATTR_TYPE_U24:
    case ESP_ZB_ZCL_ATTR_TYPE_S24:
      len = 3;
      break;

    case ESP_ZB_ZCL_ATTR_TYPE_32BIT:
    case ESP_ZB_ZCL_ATTR_TYPE_32BITMAP:
    case ESP_ZB_ZCL_ATTR_TYPE_U32:
    case ESP_ZB_ZCL_ATTR_TYPE_S32:
    case ESP_ZB_ZCL_ATTR_TYPE_SINGLE:
    case ESP_ZB_ZCL_ATTR_TYPE_TIME_OF_DAY:
    case ESP_ZB_ZCL_ATTR_TYPE_DATE:
    case ESP_ZB_ZCL_ATTR_TYPE_UTC_TIME:
    case ESP_ZB_ZCL_ATTR_TYPE_BACNET_OID:
      len = 4;
      break;

    case ESP_ZB_ZCL_ATTR_TYPE_40BIT:
    case ESP_ZB_ZCL_ATTR_TYPE_40BITMAP:
    case ESP_ZB_ZCL_ATTR_TYPE_U40:
    case ESP_ZB_ZCL_ATTR_TYPE_S40:
      len = 5;
      break;

    case ESP_ZB_ZCL_ATTR_TYPE_48BIT:
    case ESP_ZB_ZCL_ATTR_TYPE_48BITMAP:
    case ESP_ZB_ZCL_ATTR_TYPE_U48:
    case ESP_ZB_ZCL_ATTR_TYPE_S48:
      len = 6;
      break;

    case ESP_ZB_ZCL_ATTR_TYPE_56BIT:
    case ESP_ZB_ZCL_ATTR_TYPE_56BITMAP:
    case ESP_ZB_ZCL_ATTR_TYPE_U56:
    case ESP_ZB_ZCL_ATTR_TYPE_S56:
      len = 7;
      break;

    case ESP_ZB_ZCL_ATTR_TYPE_64BIT:
    case ESP_ZB_ZCL_ATTR_TYPE_64BITMAP:
    case ESP_ZB_ZCL_ATTR_TYPE_U64:
    case ESP_ZB_ZCL_ATTR_TYPE_S64:
    case ESP_ZB_ZCL_ATTR_TYPE_DOUBLE:
    case ESP_ZB_ZCL_ATTR_TYPE_IEEE_ADDR:
      len = 8;
      break;

    case ESP_ZB_ZCL_ATTR_TYPE_128_BIT_KEY:
      len = 16; //SEC_KEY_LEN;
      break;

    case ESP_ZB_ZCL_ATTR_TYPE_OCTET_STRING:
    case ESP_ZB_ZCL_ATTR_TYPE_CHAR_STRING:
    case ESP_ZB_ZCL_ATTR_TYPE_LONG_OCTET_STRING:
    case ESP_ZB_ZCL_ATTR_TYPE_LONG_CHAR_STRING:
    case ESP_ZB_ZCL_ATTR_TYPE_ARRAY:
    case ESP_ZB_ZCL_ATTR_TYPE_STRUCTURE:
    case ESP_ZB_ZCL_ATTR_TYPE_SET:
    case ESP_ZB_ZCL_ATTR_TYPE_BAG:
    case ESP_ZB_ZCL_ATTR_TYPE_INVALID:
      len = 255;
      break;

    default:
      len = 0;
      break;
	}

	return (len);
}
