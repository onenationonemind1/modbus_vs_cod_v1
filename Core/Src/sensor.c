#include "main.h"

// ���� Data
t_sensor_data g_sensor_current;
t_sensor_data g_sensor_output;

// ���
t_sensor_data g_sensor_average_sec_10; /* 10 �� */
t_sensor_data g_sensor_average_min_01; /*  1 �� */
t_sensor_data g_sensor_average_min_05; /*  5 �� */
t_sensor_data g_sensor_average_min_60; /* 60 �� */

uint8_t g_sensor_enable = ENABLE;
uint8_t g_temperature_minus = DISABLE;

#define T_H_MAX 30

static t_t_h g_temp_average[T_H_MAX];
static uint8_t g_temp_average_index;

static t_t_h t_h_average(void)
{
    t_t_h average;

    int sum_temp = 0;
    int sum_humi = 0;

    for (int i = 0; i < T_H_MAX; i++)
    {
        sum_temp += g_temp_average[i].temperature;
        sum_humi += g_temp_average[i].humidity;
    }

    average.temperature = sum_temp / T_H_MAX;
    average.humidity = sum_humi / T_H_MAX;

    return average;
}

static t_t_h temp_average(t_t_h *data, uint8_t length)
{
    t_t_h average;
    uint8_t count = 1;

    for (int i = 0; i < length; i++, count++)
    {
        average.temperature = ((average.temperature * (count - 1)) + data[i].temperature) / count;
        average.humidity = ((average.humidity * (count - 1)) + data[i].humidity) / count;
    }

    return average;
}

t_t_h g_offset;
uint16_t g_running_time = 0;

// 1 Sec
void sensor_output_data_gathering(void)
{
    static uint32_t pm_02_5_old = 999999;
    static uint32_t pm_10_0_old = 999999;

    memset(&g_sensor_output, 0, sizeof(g_sensor_output));
    g_temperature_minus = DISABLE;

    // 4 Dust
    g_sensor_output.dust.pm_01_0 = g_sensor_average_sec_10.dust.pm_01_0;
    g_sensor_output.dust.pm_02_5 = _offset_dust(g_sensor_average_sec_10.dust.pm_02_5);

    if (g_sensor_average_sec_10.dust.pm_10_0 < 30)
    {
        // pm 02.5 ���� ����
        if (pm_02_5_old != g_sensor_output.dust.pm_02_5)
        {
            pm_02_5_old = g_sensor_output.dust.pm_10_0 = g_sensor_output.dust.pm_02_5;

            if ((rand() % 2) == 0)
                g_sensor_output.dust.pm_10_0 += 1;
            else
                g_sensor_output.dust.pm_10_0 += 2;

            pm_10_0_old = g_sensor_output.dust.pm_10_0;
        }
        else
        {
            g_sensor_output.dust.pm_10_0 = pm_10_0_old;
        }
    }
    else
    {
        // pm 10.0 ���� ��ġ ����
        g_sensor_output.dust.pm_10_0 = _offset_dust(g_sensor_average_sec_10.dust.pm_10_0);
    }

    // pm 01.0 ���� ��ġ ����
    if (g_sensor_output.dust.pm_01_0 > g_sensor_output.dust.pm_02_5)
    {
        if (g_sensor_output.dust.pm_02_5 < 3)
        {
            g_sensor_output.dust.pm_01_0 = 0;
        }
        else
        {
            if ((rand() % 3) == 0)
                g_sensor_output.dust.pm_01_0 = g_sensor_output.dust.pm_02_5 - 1;
            else if ((rand() % 3) == 1)
                g_sensor_output.dust.pm_01_0 = g_sensor_output.dust.pm_02_5 - 2;
            else
                g_sensor_output.dust.pm_01_0 = g_sensor_output.dust.pm_02_5 - 3;
        }
    }

    // 4 TVOC
    if (g_setup.tvoc_baseline > g_sensor_average_sec_10.tvoc.data)
        g_sensor_output.tvoc.data = 0;
    else
        g_sensor_output.tvoc.data = g_sensor_average_sec_10.tvoc.data - g_setup.tvoc_baseline;

#if 1
    // �½��� 30�� ���
    g_temp_average[g_temp_average_index++] = g_sensor_current.t_h;
    if (g_temp_average_index == T_H_MAX)
        g_temp_average_index = 0;

    // t_h = temp_average(g_temp_average, T_H_MAX);
    g_sensor_output.t_h = t_h_average();

    // 10��
    if (g_running_time < (60 * 10))
        g_running_time++;

    // Temperature : 6��, Humidity : 10��
    if (g_running_time <= (60 * 6))
        g_offset.temperature = (100 * (g_running_time / 60)) + (100 * (g_running_time % 60) / 60);
    g_offset.humidity = (100 * (g_running_time / 60)) + (100 * (g_running_time % 60) / 60);

    if (((-10) < g_sensor_output.t_h.temperature) && (g_sensor_output.t_h.temperature < 0))
    {
        g_sensor_output.t_h.temperature = 0;
    }

    if (g_sensor_output.t_h.temperature < 0)
    {
        g_temperature_minus = ENABLE;
        g_sensor_output.t_h.temperature *= (-1);
    }
#endif

    if (g_sensor_output.t_h.temperature > 9999)
        g_sensor_output.t_h.temperature = 9999;
    if (g_sensor_output.t_h.humidity > 9999)
        g_sensor_output.t_h.humidity = 9999;

    // 4 Particle
    g_sensor_output.particle.pc_00_3 = g_sensor_average_sec_10.particle.pc_00_3;
    g_sensor_output.particle.pc_00_5 = g_sensor_average_sec_10.particle.pc_00_5;
    g_sensor_output.particle.pc_01_0 = g_sensor_average_sec_10.particle.pc_01_0;
    g_sensor_output.particle.pc_02_5 = g_sensor_average_sec_10.particle.pc_02_5;
    g_sensor_output.particle.pc_05_0 = g_sensor_average_sec_10.particle.pc_05_0;
    g_sensor_output.particle.pc_10_0 = g_sensor_average_sec_10.particle.pc_10_0;

#if 0
    if (g_sensor_output.dust.pm_01_0 > MAX_PM_01_0)     g_sensor_output.dust.pm_01_0 = MAX_PM_01_0;
    if (g_sensor_output.dust.pm_02_5 > MAX_PM_02_5)     g_sensor_output.dust.pm_02_5 = MAX_PM_02_5;
    if (g_sensor_output.dust.pm_10_0 > MAX_PM_10_0)     g_sensor_output.dust.pm_10_0 = MAX_PM_10_0;
#endif
}

void sensor_parser(uint8_t data)
{
    am1002_parser(data);
    // printf("data = 0x%2X\r\n",data);
}

// 1 ms
void sensor_handle(void)
{
#if (__USE_SENSOR_AM1002_ == 1)
    am1002_handle();
#endif

#if (__USE_SENSOR_PM2012_ == 1)
    pm2012_handle();
#endif

#if (__USE_SENSOR_HTU31D_ == 1)
    htu31_handle();
#endif
}

void sensor_init(void)
{
    printf("-- sensor_init()\n");

#if (__USE_SENSOR_AM1002_ == 1)
    am1002_init();
#endif

#if (__USE_SENSOR_PM2012_ == 1)
    pm2012_init();
#endif

#if (__USE_SENSOR_HTU31D_ == 1)
    htu31_init();
#endif
}
