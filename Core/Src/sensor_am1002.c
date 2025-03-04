#include "main.h"

#if (__USE_SENSOR_AM1002_ == 1)


extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart4;

extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart3_rx;
extern DMA_HandleTypeDef hdma_usart4_rx;


static uint8_t g_am1002_state = AM1002_STATE_SOM;
static t_packet_am1002 g_packet_am1002;

static uint8_t g_am1002_data[10];

static uint8_t g_am1002_crc_data[20];
static uint8_t g_am1002_crc_index = 0;
static uint8_t g_am1002_crc = 0;

#if 1
static t_dust g_dust_sec_10[LIST_COUNT_10];   /* 10 �� : 10�� */
static t_dust g_dust_min_01[LIST_COUNT_06];   /*  6 �� : 10�� *  6 = 1 �� */
static t_dust g_dust_min_05[LIST_COUNT_05];   /*  5 �� :  1�� *  5 = 5 �� */
static t_dust g_dust_min_60[LIST_COUNT_60];   /* 60 �� :  1�� * 60 = 1�ð� */

static t_tvoc g_tvoc_sec_10[LIST_COUNT_10];
static t_tvoc g_tvoc_min_01[LIST_COUNT_06];
static t_tvoc g_tvoc_min_05[LIST_COUNT_05];
static t_tvoc g_tvoc_min_60[LIST_COUNT_60];

static t_t_h g_t_h_sec_10[LIST_COUNT_10];
static t_t_h g_t_h_min_01[LIST_COUNT_06];
static t_t_h g_t_h_min_05[LIST_COUNT_05];
static t_t_h g_t_h_min_60[LIST_COUNT_60];

static t_dust sensor_average_dust (t_dust *p_dust, uint8_t length)
{
    t_dust average;
    uint8_t count = 1;

    for (int i=0; i<length; i++, count++)
    {
        average.pm_01_0 = ((average.pm_01_0 * (count - 1)) + p_dust[i].pm_01_0) / count;
        average.pm_02_5 = ((average.pm_02_5 * (count - 1)) + p_dust[i].pm_02_5) / count;
        average.pm_10_0 = ((average.pm_10_0 * (count - 1)) + p_dust[i].pm_10_0) / count;
    }

    return average;
}

static t_tvoc sensor_average_tvoc (t_tvoc *p_tvoc, uint8_t length)
{
    t_tvoc average;
    uint8_t count = 1;

    for (int i=0; i<length; i++, count++)
    {
        average.data = ((average.data  * (count - 1)) + p_tvoc[i].data)  / count;
    }

    return average;
}

static t_t_h sensor_average_t_h (t_t_h *p_t_h, uint8_t length)
{
    t_t_h average;
    uint8_t count = 1;

    for (int i=0; i<length; i++, count++)
    {
        average.temperature = ((average.temperature  * (count - 1)) + p_t_h[i].temperature)  / count;
        average.humidity    = ((average.humidity     * (count - 1)) + p_t_h[i].humidity)     / count;
    }

    return average;
}

static void sensor_data_am1002 (void)
{
    static uint8_t index_sec_10 = 0;
    static uint8_t index_min_01 = 0;
    static uint8_t index_min_05 = 0;
    static uint8_t index_min_60 = 0;

    // ���� ������
    g_dust_sec_10[index_sec_10] = g_sensor_current.dust;
    g_tvoc_sec_10[index_sec_10] = g_sensor_current.tvoc;
    g_t_h_sec_10[index_sec_10] = g_sensor_current.t_h;

    // 10�� ��� 
    g_sensor_average_sec_10.dust = sensor_average_dust(g_dust_sec_10, LIST_COUNT_10);
    g_sensor_average_sec_10.tvoc = sensor_average_tvoc(g_tvoc_sec_10, LIST_COUNT_10);
    g_sensor_average_sec_10.t_h = sensor_average_t_h(g_t_h_sec_10, LIST_COUNT_10);

    #if 0
    printf("1 - %02d-%5d,%5d,%5d-%5d-%5d,%5d\n", index_sec_10,
                                                                     g_sensor_current.dust.pm_01_0,
                                                                     g_sensor_current.dust.pm_02_5,
                                                                     g_sensor_current.dust.pm_10_0,
                                                                     g_sensor_current.tvoc.data,
                                                                     g_sensor_current.t_h.temperature,
                                                                     g_sensor_current.t_h.humidity);
    #endif

    #if 0
    printf("2 - %02d-%5d,%5d,%5d-%5d-%5d,%5d\n", index_sec_10,
                                                                     g_sensor_average_sec_10.dust.pm_01_0,
                                                                     g_sensor_average_sec_10.dust.pm_02_5,
                                                                     g_sensor_average_sec_10.dust.pm_10_0,
                                                                     g_sensor_average_sec_10.tvoc.data,
                                                                     g_sensor_average_sec_10.t_h.temperature,
                                                                     g_sensor_average_sec_10.t_h.humidity);
    #endif


    index_sec_10++;

    // 10��
    if (index_sec_10 == LIST_COUNT_10)
    {
        index_sec_10 = 0;
        g_dust_min_01[index_min_01] = g_sensor_average_sec_10.dust;
        g_tvoc_min_01[index_min_01] = g_sensor_average_sec_10.tvoc;
        g_t_h_min_01[index_min_01]  = g_sensor_average_sec_10.t_h;

        index_min_01++;
    }

    // 1��
    if (index_min_01 == LIST_COUNT_06)
    {
        // 1�� ��� (10�� * 6)
        g_sensor_average_min_01.dust = sensor_average_dust(g_dust_min_01, LIST_COUNT_06);
        g_sensor_average_min_01.tvoc = sensor_average_tvoc(g_tvoc_min_01, LIST_COUNT_06);
        g_sensor_average_min_01.t_h  = sensor_average_t_h(g_t_h_min_01, LIST_COUNT_06);

        g_dust_min_05[index_min_05] = g_sensor_average_min_01.dust;
        g_dust_min_60[index_min_60] = g_sensor_average_min_01.dust;

        g_tvoc_min_05[index_min_05] = g_sensor_average_min_01.tvoc;
        g_tvoc_min_60[index_min_60] = g_sensor_average_min_01.tvoc;

        g_t_h_min_05[index_min_05] = g_sensor_average_min_01.t_h;
        g_t_h_min_60[index_min_60] = g_sensor_average_min_01.t_h;

        // 5�� ��� (1�� * 5)
        g_sensor_average_min_05.dust = sensor_average_dust(g_dust_min_05, LIST_COUNT_05);
        g_sensor_average_min_05.tvoc = sensor_average_tvoc(g_tvoc_min_05, LIST_COUNT_05);
        g_sensor_average_min_05.t_h  = sensor_average_t_h(g_t_h_min_05, LIST_COUNT_05);

        // 60�� ��� (1�� * 60)
        g_sensor_average_min_60.dust = sensor_average_dust(g_dust_min_60, LIST_COUNT_60);
        g_sensor_average_min_60.tvoc = sensor_average_tvoc(g_tvoc_min_60, LIST_COUNT_60);
        g_sensor_average_min_60.t_h  = sensor_average_t_h(g_t_h_min_60, LIST_COUNT_60);

        index_min_01 = 0;
        index_min_05++;
        index_min_60++;

        if (index_min_05 == LIST_COUNT_05)
        {
            index_min_05 = 0;
        }

        if (index_min_60 == LIST_COUNT_60)
        {
            index_min_60 = 0;
        }
    }
}
#endif

uint8_t am1002_command (uint8_t command)
{
    switch (command)
    {
        case AM1002_READ_MEASUREMENT_RESULT:
        case AM1002_READ_SOFTWARE_VERSION_NUMBER:
        case AM1002_READ_SERIAL_NUMBER:
        {
            return ENABLE;
        }
    }

    return DISABLE;
}

// Cumulative sum of data = 256 - (HEAD + LEN + CMD + DATA)
uint8_t am1002_checksum (uint8_t *pdata, uint8_t length)
{
    uint16_t crc = 256;

    for (int i=0; i<length; i++)
    {
        crc -= pdata[i];
        // printf("crc .. [%d] = 0x%02X\r\n",i,crc);
    }

    return (uint8_t)(crc); 
}

void am1002_transmit (uint8_t command, uint8_t enable, uint8_t data)
{
    uint8_t index = 0;
    uint8_t check_sum = 0;

    // Start Symbol
    g_am1002_data[index++] = AM1002_HEAD_SEND;

    switch (command)
    {
        case AM1002_READ_MEASUREMENT_RESULT:
        {
            // Length
            g_am1002_data[index++] = 0x01;

            // Command (0x1F)
            g_am1002_data[index++] = AM1002_READ_MEASUREMENT_RESULT;

        } break;

        case AM1002_READ_SOFTWARE_VERSION_NUMBER:
        {
            // Length
            g_am1002_data[index++] = 0x01;

            // Command (0x1F)
            g_am1002_data[index++] = AM1002_READ_SOFTWARE_VERSION_NUMBER;
        } break;

        case AM1002_READ_SERIAL_NUMBER:
        {
            // Length
            g_am1002_data[index++] = 0x01;

            // Command (0x1F)
            g_am1002_data[index++] = AM1002_READ_SERIAL_NUMBER;
        } break;
    }

    // Crc (256 - HEAD + LEN + CMD + DATA)
    check_sum = am1002_checksum(g_am1002_data, index);
    g_am1002_data[index++] = check_sum;

    send_packet_to_sensor(g_am1002_data, index);
}

void am1002_receive (uint8_t command, uint8_t *p_data)
{
    uint8_t index = 0;

    switch (command)
    {
        case AM1002_READ_MEASUREMENT_RESULT:
        {
            // [DF1-DF2] VOC concentration = DF01*256^1 + DF02, unit: ppb
            g_sensor_current.tvoc.data  = p_data[index++] << 8;
            g_sensor_current.tvoc.data += p_data[index++];

            // [DF3-DF4] reserved
            p_data[index++];
            p_data[index++];

            // [DF5-DF6] PM 01.0 = DF05*256^1 + DF06, unit: ��g/m3
            g_sensor_current.dust.pm_01_0  = p_data[index++] << 8;
            g_sensor_current.dust.pm_01_0 += p_data[index++];

            // [DF7-DF8] PM 02.5 = DF07*256^1 + DF08, unit: ��g/m3
            g_sensor_current.dust.pm_02_5  = p_data[index++] << 8;
            g_sensor_current.dust.pm_02_5 += p_data[index++];

            // [DF9-DF10] PM 10.0 = DF09*256^1 + DF10, unit: ��g/m3
            g_sensor_current.dust.pm_10_0  = p_data[index++] << 8;
            g_sensor_current.dust.pm_10_0 += p_data[index++];

            // [DF11-DF12 Temperate = DF11*256^1 + DF12, unit:�� (real temperature((DF11*256^1 + DF12)-500��/ 10)
            g_sensor_current.t_h.temperature  = p_data[index++] << 8;
            g_sensor_current.t_h.temperature += p_data[index++];
            g_sensor_current.t_h.temperature = (g_sensor_current.t_h.temperature - 500);

            // [DF13-DF14] humidity = DF13*256^1 + DF14, unit:1% (real relative humidity =((DF13*256^1 + DF14) / 10)
            g_sensor_current.t_h.humidity  = p_data[index++] << 8;
            g_sensor_current.t_h.humidity += p_data[index++];

            g_sensor_current.t_h.temperature  *= 10;
            g_sensor_current.t_h.humidity     *= 10;

            // [DF15-DF16] reserved
            p_data[index++];
            p_data[index++];

            // [DF17-DF18] reserved
            p_data[index++];
            p_data[index++];

            #if 1
            sensor_data_am1002();
            #else
            for (int i=0; i<10000; i++)
                sensor_data_am1002();


            printf("1 - %5d,%5d,%5d-%5d-%5d,%5d\n",                     g_sensor_current.dust.pm_01_0,
                                                                             g_sensor_current.dust.pm_02_5,
                                                                             g_sensor_current.dust.pm_10_0,
                                                                             g_sensor_current.tvoc.data,
                                                                             g_sensor_current.t_h.temperature,
                                                                             g_sensor_current.t_h.humidity);

            printf("2 - %5d,%5d,%5d-%5d-%5d,%5d\n",                     g_sensor_average_sec_10.dust.pm_01_0,
                                                                             g_sensor_average_sec_10.dust.pm_02_5,
                                                                             g_sensor_average_sec_10.dust.pm_10_0,
                                                                             g_sensor_average_sec_10.tvoc.data,
                                                                             g_sensor_average_sec_10.t_h.temperature,
                                                                             g_sensor_average_sec_10.t_h.humidity);
            #endif
        } break;

        case AM1002_READ_SOFTWARE_VERSION_NUMBER:
        {
        } break;

        case AM1002_READ_SERIAL_NUMBER:
        {
            printf("-- AM1002_READ_SERIAL_NUMBER : ");
            uint8_t g_pm2012_software_version[14];

            for (int i=0; i<13; i++)
            {
                g_pm2012_software_version[i] = p_data[index++];
                printf("0x%02X ", g_pm2012_software_version[i]);
            }

            printf("\n");

        } break;
    }
}

void am1002_parser (uint8_t data)
{
    static uint8_t index = 0;

    switch (g_am1002_state)
    {
        case AM1002_STATE_SOM:
        {
            if (data == AM1002_HEAD_RECEIVE)
            {
                g_packet_am1002.header = data;
                g_am1002_state = AM1002_STATE_LENGTH;
  
            }
            
        } break;

        //4 Length
        case AM1002_STATE_LENGTH:
        {
            g_packet_am1002.length = data;
            g_am1002_state = AM1002_STATE_COMMAND;
            // printf("length = 0x%02X\r\n", data);
            
        } break;

        //4 Command
        case AM1002_STATE_COMMAND:
        {
            g_packet_am1002.command = data;
            
            if (am1002_command(g_packet_am1002.command) == ENABLE)
            {
                index = 0;
                g_am1002_state = AM1002_STATE_DATA;
            }
            else
            {
                g_am1002_state = AM1002_STATE_SOM;
            }
        } break;

        //4 Data
        case AM1002_STATE_DATA:
        {
            // printf("data[%d] = 0x%02X\r\n", index, data);
            g_packet_am1002.data[index++] = data;

            if (index == (g_packet_am1002.length - 1))
            {
                g_am1002_state = AM1002_STATE_CHECK_SUM;
            }
        } break;

        //4 CheckSum
        case AM1002_STATE_CHECK_SUM:
        {
            g_packet_am1002.crc = data;

            g_am1002_crc_index = 0;
            g_am1002_crc_data[g_am1002_crc_index++] = g_packet_am1002.header;
            g_am1002_crc_data[g_am1002_crc_index++] = g_packet_am1002.length;
            g_am1002_crc_data[g_am1002_crc_index++] = g_packet_am1002.command;
            // printf("crc_check = 0x%02X\r\n", data);
            // printf("index = %d\r\n", index);
 
            for (int i=0; i<index-1; i++)
            {
                // printf("crc_index[%d] = 0x%02X\r\n", i, g_am1002_crc_index);
                g_am1002_crc_data[g_am1002_crc_index++] = g_packet_am1002.data[i];
                // printf("data [%d] = 0x%02X \r\n" ,i , g_am1002_crc_data[i]);
            }
            
            // printf("crc_index1 = %d\r\n", g_am1002_crc_index);
            g_am1002_crc = am1002_checksum(g_am1002_crc_data, g_am1002_crc_index);
            // printf("crc_index2 = %d\r\n", g_am1002_crc_index);
            
            if (g_packet_am1002.crc == g_am1002_crc)
            {
                am1002_receive(g_packet_am1002.command, g_packet_am1002.data);
                // printf("ok\r\n");
            }
            else
            {
                printf("----------------------------------\n");
                printf("-- [[ AM1002 ]]\n");
                printf("-- crc fail : [0x%02X]-[0x%02X]\n", g_packet_am1002.crc, g_am1002_crc);
                printf("-- header   : [0x%02X]\n", g_packet_am1002.header);
                printf("-- length   : [0x%02X]\n", g_packet_am1002.length);
                printf("-- command  : [0x%02X]\n", g_packet_am1002.command);
                printf("-- data     : ");
                for(int i=0; i<(g_packet_am1002.length-1); i++)
                {
                    if ((i % 4) == 0)
                        printf("\n");
                    printf("[%02d]0x%02X ", i, g_packet_am1002.data[i]);
                }
                printf("\n");
            }

            g_am1002_state = AM1002_STATE_SOM;
        } break;
    
    }
}

void am1002_handle (void)
{
   

        am1002_transmit(AM1002_READ_MEASUREMENT_RESULT, 0, 0);
        // am1002_transmit(AM1002_READ_SOFTWARE_VERSION_NUMBER, 0, 0);
    
}

void am1002_init (void)
{
}

#endif
