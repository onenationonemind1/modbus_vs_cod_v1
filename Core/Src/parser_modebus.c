#include "main.h"

#define HOLDING_REGISTER_MAX_INDEX 100 // 적절한 값으로 조정
#define MODBUS_READ_COILS 1
#define MODBUS_READ_DISCRETE_INPUT 2
#define MODBUS_READ_HOLDING_REGISTERS 3
#define MODBUS_READ_INPUT_REGISTERS 4

#define MODBUS_WRITE_SINGLE_COIL 5
#define MODBUS_WRITE_SINGLE_REGISTER 6
#define MODBUS_WRITE_MULTIPLE_COILS 15
#define MODBUS_WRITE_MULTIPLE_REGISTERS 16

// �� ���ڸ� �ٲٸ�, ������ ������ ���� ������ �ٲ��. 5 = 5 ms,
#define _MODBUS_RETURN_INTERVAL 5
#define _MODBUS_BLANK_INTERVAL 10

#define _RING_BUFFER_MAX 254

// HMI���� 00001 �̶�� �Ǿ� ������, ��Ʈ ������ 0������ ���Ѵ�. �� 00001�� modbus_coil[0]�� 0����Ʈ�̴�.
// 00002�� modbus_coil[0]�� 1����Ʈ �̴�. �̷������� 00008�� modbus_coil[1]�� 0�� ��Ʈ�� �ȴ�.

// HMI���� 40001 �̶�� �Ǿ� ������ ���念���� 0������ ���Ѵ�. �� 40001�� mnodbus_register[0] �̴�.
// �տ� 4�� <���念��>�� ���ϸ� �ڿ� 4�ڸ��� ���� �ּ��̴�.

// static uint8_t modbus_coil[100];

#define MODBUS_REGISTER_MAX 32

#define HOLDING_REGISTER_SENSOR_QUANTITY MODBUS_REGISTER_MAX
#define HOLDING_REGISTER_DEVICE_QUANTITY 12
#define HOLDING_REGISTER_INFORMATION_QUANTITY 12

#define HOLDING_REGISTER_SENSOR_START 1
#define HOLDING_REGISTER_SENSOR_END (HOLDING_REGISTER_SENSOR_START + HOLDING_REGISTER_SENSOR_QUANTITY)

#define HOLDING_REGISTER_DEVICE_START 201
#define HOLDING_REGISTER_DEVICE_END (HOLDING_REGISTER_DEVICE_START + HOLDING_REGISTER_DEVICE_QUANTITY)

#define HOLDING_REGISTER_INFORMATION_START 501
#define HOLDING_REGISTER_INFORMATION_END (HOLDING_REGISTER_INFORMATION_START + HOLDING_REGISTER_INFORMATION_QUANTITY)

enum
{
    MODBUS_SOM = 0,              /* 00 : Search for start of message */
    MODBUS_FUNCTION_CODE,        /* 01 */
    MODBUS_ADDRESS,              /* 02 */
    MODBUS_QUANTITY_OF_REGISTER, /* 03 */
    MODBUS_LENGTH_OF_DATA,       /* 04 */
    MODBUS_DATA,                 /* 05 */
    MODBUS_CRC,                  /* 06 */
};

enum
{
    MODBUS_READ = 0, /* 00 */
    MODBUS_WRITE,    /* 00 */
    MODBUS_NONE,     /* 00 */
};

enum
{
    EXCEPTION_ILLEGAL_FUNCTION = 0x1, /* 0x01 */
    EXCEPTION_ILLEGAL_DATA_ADDRESS,   /* 0x02 */
    EXCEPTION_ILLEGAL_DATA_VALUE,     /* 0x03 */
    EXCEPTION_SLAVE_DEVICE_FAILURE,   /* 0x04 */
    EXCEPTION_ACKNOWLEDGE,            /* 0x05 */
    EXCEPTION_SLAVE_DEVICE_BUSY,      /* 0x06 */

    EXCEPTION_MEMORY_PARITY_ERROR = 0x8,                     /* 0x08 */
    EXCEPTION_GATEWAY_PATH_UNAVALIABLE = 0xA,                /* 0x0A */
    EXCEPTION_GATEWAY_TARGET_DEVICE_FAILED_TO_RESPOND = 0xB, /* 0x0B */
};

typedef struct
{
    uint8_t slave_id;
    uint8_t function_code;
    uint8_t length_of_data;
    uint16_t start_address;
    uint16_t quantity_of_register;
    uint16_t crc16;
    uint8_t data[MODBUS_REGISTER_MAX * 2];
} t_packet_modbus;

enum
{
    MODBUS_REGISTER_SENSOR = 0,  /* 00 */
    MODBUS_REGISTER_DEVICE,      /* 01 */
    MODBUS_REGISTER_INFORMATION, /* 02 */
    MODBUS_REGISTER_NONE
};

static uint16_t g_holding_register_sensor[HOLDING_REGISTER_SENSOR_QUANTITY];           /* 32 */
static uint16_t g_holding_register_device[HOLDING_REGISTER_DEVICE_QUANTITY];           /* 12 */
static uint16_t g_holding_register_information[HOLDING_REGISTER_INFORMATION_QUANTITY]; /* 12 */

static uint16_t *g_holding_register[] =
    {
        g_holding_register_sensor,
        g_holding_register_device,
        g_holding_register_information};

static uint16_t g_holding_register_address[] =
    {
        HOLDING_REGISTER_SENSOR_START,
        HOLDING_REGISTER_DEVICE_START,
        HOLDING_REGISTER_INFORMATION_START};

const uint8_t auchCRCH[] =
    {
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
        0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
        0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
        0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
        0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
        0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
        0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
        0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
        0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
        0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
        0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
        0x40};

const uint8_t auchCRCL[] =
    {
        0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
        0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
        0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
        0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
        0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,
        0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
        0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
        0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
        0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,
        0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
        0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,
        0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
        0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
        0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
        0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,
        0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
        0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
        0x40};

t_packet_modbus g_packet_modbus_rx;
uint8_t g_modbus_data[(MODBUS_REGISTER_MAX * 2) + 10];
uint8_t g_modbus_index = 0;

uint8_t g_modbus_state = MODBUS_SOM;
uint8_t g_modbus_slave_address = 1;

uint8_t g_parser_modbus_send_enable = DISABLE;

uint8_t g_buffer[2];

extern UART_HandleTypeDef huart3; /* Uart (USB), 9600 */
extern UART_HandleTypeDef huart4; /* RS485 : 9600 */

void parser_modbus_transmit(uint8_t *p_data, uint8_t length)
{
    for (int i = 0; i < length; i++)
    {
        HAL_UART_Transmit(&huart4, &p_data[i], 1, 0xFFFF);
    }
}

uint8_t parser_modbus_function_code(uint8_t function_code)
{
    uint8_t result = MODBUS_NONE;

    switch (function_code)
    {

    case MODBUS_READ_COILS:             // 4 [01, 0x01] Read Coil
    case MODBUS_READ_DISCRETE_INPUT:    // 4 [02, 0x02] Read Discrete Input
    case MODBUS_READ_HOLDING_REGISTERS: // 4 [03, 0x03] Read Holding Registers
    case MODBUS_READ_INPUT_REGISTERS:   // 4 [04, 0x04] Read Input Registers
    {
        result = MODBUS_READ;
    }
    break;

    case MODBUS_WRITE_SINGLE_COIL:        // 4 [05, 0x05] Write Single Coil
    case MODBUS_WRITE_SINGLE_REGISTER:    // 4 [06, 0x06] Write Single Holding Register
    case MODBUS_WRITE_MULTIPLE_COILS:     // 4 [15, 0x0F] Write Multiple Coils
    case MODBUS_WRITE_MULTIPLE_REGISTERS: // 4 [16, 0x10] Write Multiple Holding Registers
    {
        result = MODBUS_WRITE;
    }
    break;
    }

    return result;
}

uint16_t parser_modbus_crc_rx(void)
{
    uint8_t crc_high = 0xFF;
    uint8_t crc_low = 0xFF;
    uint8_t crc_index = 0;
    uint8_t index = 0;

#if 0
    printf("-- crc\n");
    printf("-- id                   [%d]\n", g_packet_modbus_rx.slave_id);
    printf("-- function_code        [%d]\n", g_packet_modbus_rx.function_code);
    printf("-- start_address        [%d]\n", g_packet_modbus_rx.start_address);
    printf("-- quantity_of_register [%d]\n", g_packet_modbus_rx.quantity_of_register);
#endif

    g_modbus_data[index++] = g_packet_modbus_rx.slave_id;
    g_modbus_data[index++] = g_packet_modbus_rx.function_code;
    g_modbus_data[index++] = (uint8_t)((g_packet_modbus_rx.start_address & 0xFF00) >> 8);
    g_modbus_data[index++] = (uint8_t)((g_packet_modbus_rx.start_address & 0x00FF) >> 0);

    if (g_packet_modbus_rx.function_code != MODBUS_WRITE_SINGLE_REGISTER)
    {
        g_modbus_data[index++] = (uint8_t)((g_packet_modbus_rx.quantity_of_register & 0xFF00) >> 8);
        g_modbus_data[index++] = (uint8_t)((g_packet_modbus_rx.quantity_of_register & 0x00FF) >> 0);
    }

    if (g_packet_modbus_rx.function_code > MODBUS_READ_INPUT_REGISTERS)
    {
        if (g_packet_modbus_rx.function_code != MODBUS_WRITE_SINGLE_REGISTER)
            g_modbus_data[index++] = g_packet_modbus_rx.length_of_data;

        for (int i = 0; i < g_packet_modbus_rx.length_of_data; i++)
        {
            g_modbus_data[index++] = g_packet_modbus_rx.data[i];
        }
    }

    for (int i = 0; i < index; i++)
    {
        crc_index = crc_low ^ g_modbus_data[i];
        crc_low = (uint8_t)(crc_high ^ auchCRCH[crc_index]);
        crc_high = auchCRCL[crc_index];
    }

    return (uint16_t)((crc_high << 8) | (crc_low));
}

uint16_t parser_modbus_crc_tx(int index)
{
    uint8_t crc_high = 0xFF;
    uint8_t crc_low = 0xFF;
    uint8_t crc_index = 0;

    for (int i = 0; i < index; i++)
    {
        crc_index = crc_low ^ g_modbus_data[i];
        crc_low = (uint8_t)(crc_high ^ auchCRCH[crc_index]);
        crc_high = auchCRCL[crc_index];
    }

    return (uint16_t)((crc_high << 8) | (crc_low));
}

uint8_t parser_modbus_address(void)
{
    uint16_t address = g_packet_modbus_rx.start_address + 1;
    uint16_t quantity_of_registers = g_packet_modbus_rx.quantity_of_register;

    if ((HOLDING_REGISTER_SENSOR_START <= address) && (address <= HOLDING_REGISTER_SENSOR_END))
    {
        if ((address + quantity_of_registers) <= HOLDING_REGISTER_SENSOR_END)
        {
            return MODBUS_REGISTER_SENSOR;
        }
    }
    else if ((HOLDING_REGISTER_DEVICE_START <= address) && (address <= HOLDING_REGISTER_DEVICE_END))
    {
        if ((address + quantity_of_registers) <= HOLDING_REGISTER_DEVICE_END)
        {
            return MODBUS_REGISTER_DEVICE;
        }
    }
    else if ((HOLDING_REGISTER_INFORMATION_START <= address) && (address <= HOLDING_REGISTER_INFORMATION_END))
    {
        if ((address + quantity_of_registers) <= HOLDING_REGISTER_INFORMATION_END)
        {
            return MODBUS_REGISTER_INFORMATION;
        }
    }

    return MODBUS_REGISTER_NONE;
}

void parser_modbus_exception(uint8_t function_code, uint8_t exception)
{
    /*
        Exception

        01 : illegal function
        02 : illegal data address
        03 : illegal data value
        04 : slave device failure
        05 : acknowledge
        06 : savle device busy
        08 : memory parity error
        0A : gateway path unavaliable
        0B : gateway target device failed to respond
    */
    printf("-- parser_modbus_exception() : Func [0x%02X], Return [0x%02X], Exception [%d]\n", function_code, (function_code + 0x80), exception);

    uint16_t crc_16 = 0;

    g_modbus_index = 0;
    g_modbus_data[g_modbus_index++] = g_packet_modbus_rx.slave_id;
    g_modbus_data[g_modbus_index++] = g_packet_modbus_rx.function_code + 0x80;
    g_modbus_data[g_modbus_index++] = exception;

    crc_16 = parser_modbus_crc_tx(g_modbus_index);

    g_modbus_data[g_modbus_index++] = (uint8_t)((crc_16 & 0x00FF) >> 0);
    g_modbus_data[g_modbus_index++] = (uint8_t)((crc_16 & 0xFF00) >> 8);

    g_parser_modbus_send_enable = ENABLE;
}

void simple_pack_sensor_data(t_sensor_data *sensor_data, uint8_t *modbus_data, uint16_t *index)
{
    uint16_t data;

    // 먼지 데이터
    data = (uint16_t)sensor_data->dust.pm_01_0;
    modbus_data[(*index)++] = (data >> 8); // 상위 바이트
    modbus_data[(*index)++] = data & 0xFF; // 하위 바이트

    data = (uint16_t)sensor_data->dust.pm_02_5;
    modbus_data[(*index)++] = (data >> 8);
    modbus_data[(*index)++] = data & 0xFF;

    data = (uint16_t)sensor_data->dust.pm_10_0;
    modbus_data[(*index)++] = (data >> 8);
    modbus_data[(*index)++] = data & 0xFF;

    // CO2 데이터
    data = (uint16_t)sensor_data->co2.data;
    modbus_data[(*index)++] = (data >> 8);
    modbus_data[(*index)++] = data & 0xFF;

    // TVOC 데이터
    data = (uint16_t)sensor_data->tvoc.data;
    modbus_data[(*index)++] = (data >> 8);
    modbus_data[(*index)++] = data & 0xFF;

    // 온도 데이터
    data = (uint16_t)(sensor_data->t_h.temperature / 10);
    modbus_data[(*index)++] = (data >> 8);
    modbus_data[(*index)++] = data & 0xFF;

    // 습도 데이터
    data = (uint16_t)(sensor_data->t_h.humidity / 10);
    modbus_data[(*index)++] = (data >> 8);
    modbus_data[(*index)++] = data & 0xFF;
}

void parser_modbus_receive(void)
{
    uint8_t holding_register = 0;
    uint8_t holding_register_index = 0;

    uint16_t data = 0;
    uint16_t crc_16 = 0;

    if (g_packet_modbus_rx.slave_id == 0)
    {
        // �����̺� ��巹���� 0�̸� ��ε��ɽ�Ʈ ���
        return;
    }

    memset(g_modbus_data, 0, sizeof(g_modbus_data));
    g_modbus_index = 0;

    switch (g_packet_modbus_rx.function_code)
    {
    // 4 [01] Read Coil
    // 4 [02] Read Discrete Input
    case MODBUS_READ_COILS:
    case MODBUS_READ_DISCRETE_INPUT:
    {
    }
    break;

    // 4 [03] Read Holding Registers
    // 4 [04] Read Input Registers
    case MODBUS_READ_HOLDING_REGISTERS:
    {
        holding_register = parser_modbus_address();
        if (holding_register >= MODBUS_REGISTER_NONE)
        {
            parser_modbus_exception(g_packet_modbus_rx.function_code, EXCEPTION_ILLEGAL_DATA_ADDRESS);
            break;
        }
        holding_register_index = (g_packet_modbus_rx.start_address + 1) - g_holding_register_address[holding_register];

        // Response
        g_modbus_data[g_modbus_index++] = g_packet_modbus_rx.slave_id;
        g_modbus_data[g_modbus_index++] = g_packet_modbus_rx.function_code;
        g_modbus_data[g_modbus_index++] = g_packet_modbus_rx.quantity_of_register * 2;

        // simple_pack_sensor_data(&g_sensor_current, g_modbus_data, &g_modbus_index);
        for (int i = 0; i < g_packet_modbus_rx.quantity_of_register; i++)
        {
            if (holding_register == MODBUS_REGISTER_SENSOR)
            {
                data = 1;
                g_modbus_data[g_modbus_index++] = (uint8_t)((data & 0xFF00) >> 8);
                g_modbus_data[g_modbus_index++] = (uint8_t)((data & 0x00FF) >> 0);
            }

            if (holding_register == MODBUS_REGISTER_DEVICE)
            {
                data = 1;
                // g_setup.device_id = 3;
                // data = g_setup.device_id;
                g_modbus_data[g_modbus_index++] = (uint8_t)((data & 0xFF00) >> 8);
                g_modbus_data[g_modbus_index++] = (uint8_t)((data & 0xFF00) >> 0);
            }
            printf("holding register%d \r\n", holding_register);
            printf("g_modbus_data[%d] = %d", i, g_modbus_data[g_modbus_index]);
        }
    }
    break;
    case MODBUS_READ_INPUT_REGISTERS:
    {
        holding_register = parser_modbus_address();
        if (holding_register >= MODBUS_REGISTER_NONE)
        {
            parser_modbus_exception(g_packet_modbus_rx.function_code, EXCEPTION_ILLEGAL_DATA_ADDRESS);
            break;
        }
        holding_register_index = (g_packet_modbus_rx.start_address + 1) - g_holding_register_address[holding_register];

        // Response
        g_modbus_data[g_modbus_index++] = g_packet_modbus_rx.slave_id;
        g_modbus_data[g_modbus_index++] = g_packet_modbus_rx.function_code;
        g_modbus_data[g_modbus_index++] = g_packet_modbus_rx.quantity_of_register * 2;

        // simple_pack_sensor_data(&g_sensor_current, g_modbus_data, &g_modbus_index);
        for (int i = 0; i < g_packet_modbus_rx.quantity_of_register; i++)
        {
            /* CO2 값 제외하기위해서 4번을 건너띄기 */
            data = g_holding_register[holding_register][holding_register_index++];
            if (holding_register_index == 4)
            {
                continue;
            }

            g_modbus_data[g_modbus_index++] = (uint8_t)((data & 0xFF00) >> 8);
            g_modbus_data[g_modbus_index++] = (uint8_t)((data & 0x00FF) >> 0);
        }
        /* CO2 값 제외된 만큼 데이터 보충해주기*/
        data = 0;
        g_modbus_data[g_modbus_index++] = (uint8_t)((data & 0xFF00) >> 8);
        g_modbus_data[g_modbus_index++] = (uint8_t)((data & 0x00FF) >> 0);
        // printf("-- holding_register_index : [%d]\n", holding_register_index);
    }
    break;

    // 4 [05] Write Single Coil
    case MODBUS_WRITE_SINGLE_COIL:
    {
    }
    break;

    // 4 [06] Write Single Holding Register
    case MODBUS_WRITE_SINGLE_REGISTER:
    {
        holding_register = parser_modbus_address();
        if (holding_register >= MODBUS_REGISTER_NONE)
        {
            parser_modbus_exception(g_packet_modbus_rx.function_code, EXCEPTION_ILLEGAL_DATA_ADDRESS);
            break;
        }
        holding_register_index = (g_packet_modbus_rx.start_address + 1) - g_holding_register_address[holding_register];

        g_holding_register[holding_register][holding_register_index] = g_packet_modbus_rx.data[0] << 8;
        g_holding_register[holding_register][holding_register_index] += g_packet_modbus_rx.data[1] << 0;

        // Response
        g_modbus_data[g_modbus_index++] = g_packet_modbus_rx.slave_id;
        g_modbus_data[g_modbus_index++] = g_packet_modbus_rx.function_code;
        g_modbus_data[g_modbus_index++] = (uint8_t)((g_packet_modbus_rx.start_address & 0xFF00) >> 8);
        g_modbus_data[g_modbus_index++] = (uint8_t)((g_packet_modbus_rx.start_address & 0x00FF) >> 0);
        g_modbus_data[g_modbus_index++] = g_packet_modbus_rx.data[0];
        g_modbus_data[g_modbus_index++] = g_packet_modbus_rx.data[1];
    }
    break;

    // 4 [15] Write Multiple Coils
    case MODBUS_WRITE_MULTIPLE_COILS:
    {
    }
    break;

    // 4 [16] Write Multiple Holding Registers
    case MODBUS_WRITE_MULTIPLE_REGISTERS:
    {
        holding_register = parser_modbus_address();
        if (holding_register >= MODBUS_REGISTER_NONE)
        {
            parser_modbus_exception(g_packet_modbus_rx.function_code, EXCEPTION_ILLEGAL_DATA_ADDRESS);
            break;
        }
        holding_register_index = (g_packet_modbus_rx.start_address + 1) - g_holding_register_address[holding_register];

        // printf("-- Reg : [%d][%d], [%d]\n", g_packet_modbus_rx.start_address, g_packet_modbus_rx.quantity_of_register, holding_register);

        for (int i = 0; i < g_packet_modbus_rx.quantity_of_register; i++)
        {
            g_holding_register[holding_register][holding_register_index + i] = g_packet_modbus_rx.data[g_modbus_index++] << 8;
            g_holding_register[holding_register][holding_register_index + i] += g_packet_modbus_rx.data[g_modbus_index++] << 0;
            // printf("-- [%d][%d]\n", holding_register_index + i, g_holding_register[holding_register][holding_register_index + i]);
        }
        g_modbus_index = 0;

#if 0
            printf("-- id                   [%d]\n", g_packet_modbus_rx.slave_id);
            printf("-- function_code        [%d]\n", g_packet_modbus_rx.function_code);
            printf("-- start_address        [%d]\n", g_packet_modbus_rx.start_address);
            printf("-- quantity_of_register [%d]\n", g_packet_modbus_rx.quantity_of_register);
#endif

        // Response
        g_modbus_data[g_modbus_index++] = g_packet_modbus_rx.slave_id;
        g_modbus_data[g_modbus_index++] = g_packet_modbus_rx.function_code;
        g_modbus_data[g_modbus_index++] = (uint8_t)((g_packet_modbus_rx.start_address & 0xFF00) >> 8);
        g_modbus_data[g_modbus_index++] = (uint8_t)((g_packet_modbus_rx.start_address & 0x00FF) >> 0);
        g_modbus_data[g_modbus_index++] = (uint8_t)((g_packet_modbus_rx.quantity_of_register & 0xFF00) >> 8);
        g_modbus_data[g_modbus_index++] = (uint8_t)((g_packet_modbus_rx.quantity_of_register & 0x00FF) >> 0);
    }
    break;
    }

    // CRC
    crc_16 = parser_modbus_crc_tx(g_modbus_index);

    g_modbus_data[g_modbus_index++] = (uint8_t)((crc_16 & 0x00FF) >> 0);
    g_modbus_data[g_modbus_index++] = (uint8_t)((crc_16 & 0xFF00) >> 8);

    g_parser_modbus_send_enable = ENABLE;
    printf("enable\r\n");
}

void parser_modbus(uint8_t data)
{
    // printf("DATA = 0x%02X\r\n", data);
    static uint8_t index = 0;
    switch (g_modbus_state)
    {
    case MODBUS_SOM:
    {
        // if ((data == g_modbus_slave_address) || (data == 0))
        if (data == 1)
        {
            g_packet_modbus_rx.slave_id = data;
            index = 0;
            g_modbus_state = MODBUS_FUNCTION_CODE;
        }
        else
        {
            printf("addr\n");
        }
    }
    break;

    case MODBUS_FUNCTION_CODE:
    {
        if (parser_modbus_function_code(data) < MODBUS_NONE)
        {
            g_packet_modbus_rx.function_code = data;
            index = 0;
            g_modbus_state = MODBUS_ADDRESS;
        }
        else
        {
            g_modbus_state = MODBUS_SOM;
        }
    }
    break;

    case MODBUS_ADDRESS:
    {
        g_buffer[index++] = data;

        if (index >= 2)
        {
            index = 0;

            g_packet_modbus_rx.start_address = g_buffer[0] << 8;
            g_packet_modbus_rx.start_address += g_buffer[1] << 0;

            if (g_packet_modbus_rx.function_code == MODBUS_WRITE_SINGLE_REGISTER)
            {
                g_packet_modbus_rx.length_of_data = 2;
                g_modbus_state = MODBUS_DATA;
            }
            else
            {
                g_modbus_state = MODBUS_QUANTITY_OF_REGISTER;
            }
        }
    }
    break;

    case MODBUS_QUANTITY_OF_REGISTER:
    {
        g_buffer[index++] = data;

        if (index >= 2)
        {
            index = 0;

            g_packet_modbus_rx.quantity_of_register = g_buffer[0] << 8;
            g_packet_modbus_rx.quantity_of_register += g_buffer[1] << 0;

            if (g_packet_modbus_rx.quantity_of_register > MODBUS_REGISTER_MAX)
            {
                g_modbus_state = MODBUS_SOM;
            }
            else
            {
                if (g_packet_modbus_rx.function_code >= MODBUS_WRITE_MULTIPLE_REGISTERS)
                {
                    g_modbus_state = MODBUS_LENGTH_OF_DATA;
                }
                else
                {
                    g_modbus_state = MODBUS_CRC;
                }
            }
        }
    }
    break;

    case MODBUS_LENGTH_OF_DATA:
    {
        g_packet_modbus_rx.length_of_data = data;

        index = 0;
        g_modbus_state = MODBUS_DATA;
    }
    break;

    case MODBUS_DATA:
    {
        g_packet_modbus_rx.data[index++] = data;

        if (index >= g_packet_modbus_rx.length_of_data)
        {
            index = 0;
            g_modbus_state = MODBUS_CRC;
        }
    }
    break;

    case MODBUS_CRC:
    {
        g_buffer[index++] = data;

        if (index >= 2)
        {
            index = 0;

            uint16_t crc_16 = parser_modbus_crc_rx();

            g_packet_modbus_rx.crc16 = g_buffer[1] << 8;
            g_packet_modbus_rx.crc16 += g_buffer[0] << 0;
            printf("g_buffer1 =  0x%02X\r\n", g_buffer[1]);
            printf("g_buffer0 =  0x%02X\r\n", g_buffer[0]);

            if (crc_16 == g_packet_modbus_rx.crc16)
            {
                parser_modbus_receive();
                printf("oaky!\r\n");
            }
            else
            {
                printf("-- Modbus CRC : [0x%04X][0x%04X]\n", crc_16, g_packet_modbus_rx.crc16);
            }

            g_modbus_state = MODBUS_SOM;
        }
    }
    break;
    }
}

// 1 ms
void parser_modbus_handle(void)
{
#if 1
    static uint16_t period_parser_id = 0;
    static uint16_t period_parser_modbus = 0;
    static uint16_t period_parser_send = 0;

    uint32_t data_32 = 0;
    if (g_parser_modbus_send_enable == ENABLE)
    {
        if (period_parser_send++ >= 0)
        {
            period_parser_send = 0;

            g_parser_modbus_send_enable = DISABLE;
            parser_modbus_transmit(g_modbus_data, g_modbus_index);
            // printf("parser_transmit!\r\n");
            // printf("parser_transmit!\r\n");
            // printf("parser_transmit!\r\n");
            // printf("parser_transmit!\r\n");
        }
    }

    if (++period_parser_id >= 1)
    {
        period_parser_id = 0;

        if (g_modbus_slave_address != g_holding_register_device[0])
        {
            g_modbus_slave_address = g_setup.device_id = g_holding_register_device[0];
            setup_data_write();
        }
    }

    if (++period_parser_modbus >= 1)
    {
        period_parser_modbus = 0;

        // Dust
        g_holding_register_sensor[MODBUS_PM_01_0] = (uint16_t)g_sensor_current.dust.pm_01_0;
        g_holding_register_sensor[MODBUS_PM_02_5] = (uint16_t)g_sensor_current.dust.pm_02_5;
        g_holding_register_sensor[MODBUS_PM_10_0] = (uint16_t)g_sensor_current.dust.pm_10_0;

        // Co2
        g_holding_register_sensor[MODBUS_CO2] = (uint16_t)g_sensor_current.co2.data;

        // Tvoc
        g_holding_register_sensor[MODBUS_TVOC] = (uint16_t)g_sensor_current.tvoc.data;

        // Temperature
        g_holding_register_sensor[MODBUS_TEMPERATURE] = (uint16_t)(g_sensor_current.t_h.temperature / 10);
        if (g_temperature_minus == ENABLE)
            g_holding_register_sensor[MODBUS_TEMPERATURE] |= 0x8000;

        // Humidity
        g_holding_register_sensor[MODBUS_HUMIDITY] = (uint16_t)(g_sensor_current.t_h.humidity / 10);

        // Particle
        data_32 = (uint32_t)g_sensor_output.particle.pc_00_3;
        g_holding_register_sensor[MODBUS_PC_00_3_MSB] = (uint16_t)((data_32 & 0xFFFF0000) >> 16);
        g_holding_register_sensor[MODBUS_PC_00_3_LSB] = (uint16_t)((data_32 & 0x0000FFFF) >> 0);

        data_32 = (uint32_t)g_sensor_output.particle.pc_00_5;
        g_holding_register_sensor[MODBUS_PC_00_5_MSB] = (uint16_t)((data_32 & 0xFFFF0000) >> 16);
        g_holding_register_sensor[MODBUS_PC_00_5_LSB] = (uint16_t)((data_32 & 0x0000FFFF) >> 0);

        data_32 = (uint32_t)g_sensor_output.particle.pc_01_0;
        g_holding_register_sensor[MODBUS_PC_01_0_MSB] = (uint16_t)((data_32 & 0xFFFF0000) >> 16);
        g_holding_register_sensor[MODBUS_PC_01_0_LSB] = (uint16_t)((data_32 & 0x0000FFFF) >> 0);

        data_32 = (uint32_t)g_sensor_output.particle.pc_02_5;
        g_holding_register_sensor[MODBUS_PC_02_5_MSB] = (uint16_t)((data_32 & 0xFFFF0000) >> 16);
        g_holding_register_sensor[MODBUS_PC_02_5_LSB] = (uint16_t)((data_32 & 0x0000FFFF) >> 0);

        data_32 = (uint32_t)g_sensor_output.particle.pc_05_0;
        g_holding_register_sensor[MODBUS_PC_05_0_MSB] = (uint16_t)((data_32 & 0xFFFF0000) >> 16);
        g_holding_register_sensor[MODBUS_PC_05_0_LSB] = (uint16_t)((data_32 & 0x0000FFFF) >> 0);

        data_32 = (uint32_t)g_sensor_output.particle.pc_10_0;
        g_holding_register_sensor[MODBUS_PC_10_0_MSB] = (uint16_t)((data_32 & 0xFFFF0000) >> 16);
        g_holding_register_sensor[MODBUS_PC_10_0_LSB] = (uint16_t)((data_32 & 0x0000FFFF) >> 0);

#if 0
        printf("-- Device ID [%d]-----------------------------------\n", g_modbus_slave_address);
        printf("-- Sensor      : ");
        for (int i=0; i<HOLDING_REGISTER_QUANTITY_SENSOR; i++)
        {
            printf("%02X ", g_holding_register_sensor[i]);
        }
        printf("\n");

        printf("-- Device      : ");
        for (int i=0; i<HOLDING_REGISTER_QUANTITY_DEVICE; i++)
        {
            printf("%02X ", g_holding_register_device[i]);
        }
        printf("\n");

        printf("-- Information : ");
        for (int i=0; i<HOLDING_REGISTER_QUANTITY_INFORMATION; i++)
        {
            printf("%02X ", g_holding_register_information[i]);
        }
        printf("\n");
#endif
    }
#endif
}

void parser_modbus_init(void)
{
    memset(g_holding_register_sensor, 0, sizeof(g_holding_register_sensor));
    memset(g_holding_register_device, 0, sizeof(g_holding_register_device));
    memset(g_holding_register_information, 0, sizeof(g_holding_register_information));

    // slave id
    g_modbus_slave_address = g_setup.device_id;
    g_holding_register_device[0] = g_setup.device_id;

    // version
    g_holding_register_information[0] = VERSION_MAJOR * 1000;
    g_holding_register_information[0] += VERSION_MINOR * 100;
    g_holding_register_information[0] += VERSION_REVISION;
}
