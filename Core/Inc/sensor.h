#ifndef __SENSOR_H_
#define __SENSOR_H_

#include "main.h"

#define __ENABLE_SENSOR_                1

#define __USE_SENSOR_AM1002_            1
#define __USE_SENSOR_PM2012_            0
#define __USE_SENSOR_HTU31D_            0

#define MAX_PM_01_0                     150
#define MAX_PM_02_5                     150
#define MAX_PM_10_0                     300


#if (__USE_SENSOR_AM1002_ == 1)
#include "sensor_am1002.h"
#endif

#if (__USE_SENSOR_PM2012_ == 1)
#include "sensor_pm2012.h"
#endif

#if (__USE_SENSOR_HTU31D_ == 1)
#include "sensor_htu31.h"
#endif


typedef struct
{
    uint32_t pm_01_0;
    uint32_t pm_02_5;
    uint32_t pm_10_0;
} t_dust;

typedef struct
{
    uint32_t pc_00_3;
    uint32_t pc_00_5;
    uint32_t pc_01_0;
    uint32_t pc_02_5;
    uint32_t pc_05_0;
    uint32_t pc_10_0;
} t_particle;

typedef struct
{
	uint32_t data;
} t_co2, t_tvoc;

typedef struct
{
	int32_t temperature;
	uint32_t humidity;
} t_t_h;

typedef struct
{
    t_dust      dust;
    t_particle  particle;
    t_co2       co2;
    t_tvoc      tvoc;
    t_t_h       t_h;
} t_sensor_data;

// ����
extern t_sensor_data g_sensor_current;
extern t_sensor_data g_sensor_output;

// ���
extern t_sensor_data g_sensor_average_sec_10;               /* 10 �� */
extern t_sensor_data g_sensor_average_min_01;               /*  1 �� */
extern t_sensor_data g_sensor_average_min_05;               /*  5 �� */
extern t_sensor_data g_sensor_average_min_60;               /* 60 �� */

extern uint8_t g_temperature_minus;

void sensor_output_data_gathering (void);

void sensor_parser (uint8_t data);
void sensor_handle (void);
void sensor_init (void);

#endif
