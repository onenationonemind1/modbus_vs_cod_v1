#include "main.h"

static uint32_t g_flags = flag_NONE;

static uint32_t g_tick_start = 0;
static uint32_t g_tick_elapsed = 0;

t_setup g_setup;
u_factor g_factor[K_FACTOR_MAX][4];

void flag_set (uint32_t flag)
{
    g_flags = flag;
}

void flag_add (uint32_t flag)
{
    g_flags |= flag;
}

void flag_del (uint32_t flag)
{
    g_flags &= ~flag;
}

uint8_t is_flag (uint32_t flag)
{
     if (g_flags & flag)
        return ENABLE;

    return DISABLE;
}

void tick_start (void)
{
    g_tick_start = HAL_GetTick();
}

void tick_stop (void)
{
    g_tick_elapsed = HAL_GetTick() - g_tick_start;
}

uint32_t tick_elapsed (void)
{
    g_tick_elapsed = HAL_GetTick() - g_tick_start;
    return g_tick_elapsed;
}

#if 1
void setup_data_print (void)
{
    printf("--------------------------------------\n");
    printf("-- Setup\n");
    printf("   init_data     : [0x%02X][0x%02X]\n", g_setup.init_data_1, g_setup.init_data_2);
    printf("   rs485 id      : [0x%02X]\n", g_setup.device_id);
    printf("   tvoc_baseline : [0x%02X]\n", g_setup.tvoc_baseline);
    printf("   eco2_baseline : [0x%02X]\n", g_setup.eco2_baseline);

    printf("--------------------------------------\n");
    printf("-- K Factor [%d]\n", g_setup.k_factor_index);
    for (int i=0; i<K_FACTOR_MAX; i++)
    {
        printf("   %f, %f, %f, %f\n", g_factor[i][0].f, g_factor[i][1].f, g_factor[i][2].f, g_factor[i][3].f);
    }
}

void setup_data_read (void)
{
    uint32_t data = 0;

    // setup
    data = flash_read_user_page(ADDR_USER_FLASH_PAGE, 0);
    g_setup.init_data_1     = (uint8_t)((data & 0x000000FF) >> 0);
    g_setup.init_data_2     = (uint8_t)((data & 0x0000FF00) >> 8);
    g_setup.device_id       = (uint8_t)((data & 0x00FF0000) >> 16);
    g_setup.k_factor_index  = (uint8_t)((data & 0xFF000000) >> 24);

    // Sensor Data
    data = flash_read_user_page(ADDR_USER_FLASH_PAGE, 1);
    g_setup.tvoc_baseline   = (uint16_t)((data & 0x0000FFFF) >> 0);
    g_setup.eco2_baseline   = (uint16_t)((data & 0xFFFF0000) >> 16);

    for (int i=0; i<K_FACTOR_MAX; i++)
    {
        g_factor[i][0].u = flash_read_user_page(ADDR_USER_FLASH_PAGE, 5 + i + (K_FACTOR_MAX * 0));
        g_factor[i][1].u = flash_read_user_page(ADDR_USER_FLASH_PAGE, 5 + i + (K_FACTOR_MAX * 1));
        g_factor[i][2].u = flash_read_user_page(ADDR_USER_FLASH_PAGE, 5 + i + (K_FACTOR_MAX * 2));
        g_factor[i][3].u = flash_read_user_page(ADDR_USER_FLASH_PAGE, 5 + i + (K_FACTOR_MAX * 3));
    }
}    

void setup_data_write (void)
{
    uint32_t data = 0;

    // erase
    flash_erase_user_page(ADDR_USER_FLASH_PAGE);

    data  = g_setup.init_data_1    << 0;
    data += g_setup.init_data_2    << 8;
    data += g_setup.device_id      << 16;
    data += g_setup.k_factor_index << 24;
    flash_write_user_page(ADDR_USER_FLASH_PAGE, 0, data);

    data  = g_setup.tvoc_baseline << 0;
    data += g_setup.eco2_baseline << 16;
    flash_write_user_page(ADDR_USER_FLASH_PAGE, 1, data);

    // K Factor
    for (int i=0; i<K_FACTOR_MAX; i++)
    {
        data = g_factor[i][0].u;
        flash_write_user_page(ADDR_USER_FLASH_PAGE, 5 + i + (K_FACTOR_MAX * 0), data);

        data = g_factor[i][1].u;
        flash_write_user_page(ADDR_USER_FLASH_PAGE, 5 + i + (K_FACTOR_MAX * 1), data);

        data = g_factor[i][2].u;
        flash_write_user_page(ADDR_USER_FLASH_PAGE, 5 + i + (K_FACTOR_MAX * 2), data);

        data = g_factor[i][3].u;
        flash_write_user_page(ADDR_USER_FLASH_PAGE, 5 + i + (K_FACTOR_MAX * 3), data);
    }
}

void setup_data_init (void)
{
    printf("-- setup_data_init()\n");

    setup_data_read();

    if ((g_setup.init_data_1 == INIT_DATA_1) || (g_setup.init_data_2 == INIT_DATA_2))
    {
    }
    else
    {
        printf("-- Init ~~\n");
    
        // Spi Flash ���� Ȯ�ο� ������
        g_setup.init_data_1     = INIT_DATA_1;
        g_setup.init_data_2     = INIT_DATA_2;

        // RS485
        g_setup.device_id       = RS485_ID;

        // Sensor Data
        g_setup.tvoc_baseline   = 0;
        g_setup.eco2_baseline   = 0;

        g_setup.k_factor_index  = 0;

        g_factor[0][FACTOR_LOW].f =  0.0;  g_factor[0][FACTOR_HIGH].f =  9.0;  g_factor[0][FACTOR_MULTIPLE].f = 0.5;   g_factor[0][FACTOR_PLUS].f =  7.0;
        g_factor[1][FACTOR_LOW].f =  9.0;  g_factor[1][FACTOR_HIGH].f = 11.0;  g_factor[1][FACTOR_MULTIPLE].f = 0.5;   g_factor[1][FACTOR_PLUS].f =  9.0;
        g_factor[2][FACTOR_LOW].f = 11.0;  g_factor[2][FACTOR_HIGH].f = 15.0;  g_factor[2][FACTOR_MULTIPLE].f = 0.4;   g_factor[2][FACTOR_PLUS].f = 11.0;
        g_factor[3][FACTOR_LOW].f = 15.0;  g_factor[3][FACTOR_HIGH].f = 21.0;  g_factor[3][FACTOR_MULTIPLE].f = 0.4;   g_factor[3][FACTOR_PLUS].f = 14.0;
        g_factor[4][FACTOR_LOW].f = 21.0;  g_factor[4][FACTOR_HIGH].f = 29.0;  g_factor[4][FACTOR_MULTIPLE].f = 0.5;   g_factor[4][FACTOR_PLUS].f = 17.0;
        g_factor[5][FACTOR_LOW].f = 29.0;  g_factor[5][FACTOR_HIGH].f = 36.0;  g_factor[5][FACTOR_MULTIPLE].f = 0.4;   g_factor[5][FACTOR_PLUS].f = 24.0;
        g_factor[6][FACTOR_LOW].f = 36.0;  g_factor[6][FACTOR_HIGH].f = 58.0;  g_factor[6][FACTOR_MULTIPLE].f = 0.45;  g_factor[6][FACTOR_PLUS].f = 35.0;
        g_factor[7][FACTOR_LOW].f = 58.0;  g_factor[7][FACTOR_HIGH].f =  0.0;  g_factor[7][FACTOR_MULTIPLE].f = 0.45;  g_factor[7][FACTOR_PLUS].f = 40.0;
        g_factor[8][FACTOR_LOW].f =  0.0;  g_factor[8][FACTOR_HIGH].f =  0.0;  g_factor[8][FACTOR_MULTIPLE].f = 0.0;   g_factor[8][FACTOR_PLUS].f =  0.0;
        g_factor[9][FACTOR_LOW].f =  0.0;  g_factor[9][FACTOR_HIGH].f =  0.0;  g_factor[9][FACTOR_MULTIPLE].f = 0.0;   g_factor[9][FACTOR_PLUS].f =  0.0;

        setup_data_write();
        setup_data_read();
    }

    setup_data_print();
}
#endif
