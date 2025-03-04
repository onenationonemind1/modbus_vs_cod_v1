#ifndef __MODBUS_H_
#define __MODBUS_H_

/*
    
*/

enum
{
    MODBUS_PM_01_0 = 0,         /* 00 : 40001 */
    MODBUS_PM_02_5,             /* 01 : 40002 */
    MODBUS_PM_10_0,             /* 02 : 40003 */
    // ---------------------------------
    MODBUS_CO2,                 /* 03 : 40004 */
    MODBUS_TVOC,                /* 04 : 40005 */
    // ---------------------------------
    MODBUS_TEMPERATURE,         /* 05 : 40006 */
    MODBUS_HUMIDITY,            /* 06 : 40007 */
    // ---------------------------------
    MODBUS_PC_00_3_MSB,         /* 07 : 40008 */
    MODBUS_PC_00_3_LSB,         /* 08 : 40009 */
    MODBUS_PC_00_5_MSB,         /* 09 : 40010 */
    MODBUS_PC_00_5_LSB,         /* 10 : 40011 */
    MODBUS_PC_01_0_MSB,         /* 11 : 40012 */
    MODBUS_PC_01_0_LSB,         /* 12 : 40013 */
    MODBUS_PC_02_5_MSB,         /* 13 : 40014 */
    MODBUS_PC_02_5_LSB,         /* 14 : 40015 */
    MODBUS_PC_05_0_MSB,         /* 15 : 40016 */
    MODBUS_PC_05_0_LSB,         /* 16 : 40017 */
    MODBUS_PC_10_0_MSB,         /* 17 : 40018 */
    MODBUS_PC_10_0_LSB,         /* 18 : 40019 */
    // ---------------------------------
    MODBUS_CO,                  /* 19 : 40020 */
    MODBUS_NO2,                 /* 20 : 40021 */
    MODBUS_HCHO,                /* 21 : 40022 */
    MODBUS_SO2,                 /* 22 : 40023 */
    MODBUS_H2S,                 /* 23 : 40024 */
    MODBUS_O3                   /* 24 : 40025 */
};

void modbus_handle (void);
void modbus_init (void);

#endif
