#include "main.h"

#if 0
#define ADDR_FLASH_PAGE_60 ((uint32_t)0x0801E000) /* Base @ of Page 60, 2 Kbytes */
#define ADDR_FLASH_PAGE_63 ((uint32_t)0x0801F800) /* Base @ of Page 63, 2 Kbytes */

#endif

#define FLASH_USER_START_ADDR ADDR_FLASH_PAGE_62                 /* Start @ of user Flash area */
#define FLASH_USER_END_ADDR ADDR_FLASH_PAGE_63 + FLASH_PAGE_SIZE /* End @ of user Flash area */
#define FLASH_USER_PAGE_SIZE 4                                   /* 4 Byte */

uint32_t g_page_address = 0;
uint32_t Address = 0, g_page_error = 0;

void flash_lock(void)
{
    /* Lock the Flash to disable the flash control register access (recommended to protect the FLASH memory against possible unwanted operation) */
    HAL_FLASH_Lock();
}

void flash_unlock(void)
{
    /* Unlock the Flash to enable the flash control register access */
    HAL_FLASH_Unlock();
}

void flash_erase_user_page(uint32_t page_address)
{
    /*Variable used for Erase procedure*/
    static FLASH_EraseInitTypeDef EraseInitStruct;

    flash_unlock();

    /* Fill EraseInit structure*/
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = page_address;
    EraseInitStruct.NbPages = 1;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &g_page_error) != HAL_OK)
    {
        /*
        Error occurred while page erase.
        User can add here some code to deal with this error.
        g_page_error will contain the faulty page and then to know the code error on this page,
        user can call function 'HAL_FLASH_GetError()'
        */

        printf("-- flash_erase() : Fail !!!\n");
    }

    flash_lock();
}

void flash_write_user_page(uint32_t page, uint8_t index, uint32_t data)
{
    HAL_StatusTypeDef status = HAL_ERROR;
    uint32_t address = (page + (FLASH_USER_PAGE_SIZE * index));

    flash_unlock();

    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, data);
    if (status == HAL_OK)
    {
    }
    else
    {
        printf("-- Flash Write Fail !!! [0x%X]\n", status);
    }

    flash_lock();
}

uint32_t flash_read_user_page(uint32_t page, uint8_t index)
{
    uint32_t address = (page + (FLASH_USER_PAGE_SIZE * index));

    __IO uint32_t data = *(__IO uint32_t *)address;

    return (uint32_t)data;
}

void flash_init(void)
{
    printf("-- flash_init()\n");
}
