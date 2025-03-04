#ifndef SENSOR_AM1002_H
#define SENSOR_AM1002_H

#if (__USE_SENSOR_AM1002_ == 1)

#define AM1002_READ_MEASUREMENT_RESULT                              0x16
#define AM1002_READ_SOFTWARE_VERSION_NUMBER                         0x1E
#define AM1002_READ_SERIAL_NUMBER                                   0x1F

#define AM1002_HEAD_SEND                                            0x11
#define AM1002_HEAD_RECEIVE                                         0x16

enum
{
    AM1002_STATE_SOM = 0,               /* 00 : Search for start of message */
    // ---------------------------------------
    AM1002_STATE_LENGTH,                /* 01 */
    AM1002_STATE_COMMAND,               /* 02 */
    AM1002_STATE_DATA,                  /* 03 */
    AM1002_STATE_CHECK_SUM,             /* 04 */
};

typedef struct
{
    uint8_t header;
    uint8_t length;
    uint8_t command;
    uint8_t data[MAX_DATA];
    uint8_t crc;
} t_packet_am1002;

void am1002_parser (uint8_t data);
void am1002_handle (void);
void am1002_init (void);

#endif /* __USE_SENSOR_AM1002_ == 1 */

#endif /* SENSOR_AM1002_H */