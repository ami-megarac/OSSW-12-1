/****************************************************************
 **                                                            **
 **    (C)Copyright 2006-2009, American Megatrends Inc.        **
 **                                                            **
 **            All Rights Reserved.                            **
 **                                                            **
 **        5555 Oakbrook Pkwy Suite 200, Norcross              **
 **                                                            **
 **        Georgia - 30093, USA. Phone-(770)-246-8600.         **
 **                                                            **
****************************************************************/

#ifndef _ESPIMAFS_IOCTL_H_
#define _ESPIMAFS_IOCTL_H_

#include <linux/socket.h>
#include <linux/tcp.h>

#define FLASH_CHANNEL_SUPPORTED               (0x1<<3)

// AST_ESPI_CHx_CAPCONF
typedef enum
{
    AST_ESPIMAFS_CH_CAP_MAX_READ_REQ_SIZE_RESERVED = 0,
    AST_ESPIMAFS_CH_CAP_MAX_READ_REQ_SIZE_64_BYTES,
    AST_ESPIMAFS_CH_CAP_MAX_READ_REQ_SIZE_128_BYTES,
    AST_ESPIMAFS_CH_CAP_MAX_READ_REQ_SIZE_256_BYTES,
    AST_ESPIMAFS_CH_CAP_MAX_READ_REQ_SIZE_512_BYTES,
    AST_ESPIMAFS_CH_CAP_MAX_READ_REQ_SIZE_1024_BYTES,
    AST_ESPIMAFS_CH_CAP_MAX_READ_REQ_SIZE_2048_BYTES,
    AST_ESPIMAFS_CH_CAP_MAX_READ_REQ_SIZE_4096_BYTES,
    AST_ESPIMAFS_CH_CAP_MAX_READ_REQ_SIZE_EOF
} AST_ESPIMAFS_CH_CAP_MAX_READ_REQ_SIZE_T;

typedef enum
{
    AST_ESPIMAFS_CH_CAP_MAX_PAYLOAD_SIZE_RESERVED = 0,
    AST_ESPIMAFS_CH_CAP_MAX_PAYLOAD_SIZE_64_BYTES,
    AST_ESPIMAFS_CH_CAP_MAX_PAYLOAD_SIZE_128_BYTES,
    AST_ESPIMAFS_CH_CAP_MAX_PAYLOAD_SIZE_256_BYTES,
    AST_ESPIMAFS_CH_CAP_MAX_PAYLOAD_SIZE_EOF
} AST_ESPIMAFS_CH_CAP_MAX_PAYLOAD_SIZE_T;

typedef enum
{
    AST_ESPIMAFS_CH_CAP_FLASH_BLOCK_ERASE_SIZE_RESERVED = 0,
    AST_ESPIMAFS_CH_CAP_FLASH_BLOCK_ERASE_SIZE_4K_BYTES,
    AST_ESPIMAFS_CH_CAP_FLASH_BLOCK_ERASE_SIZE_64K_BYTES,
    AST_ESPIMAFS_CH_CAP_FLASH_BLOCK_ERASE_SIZE_4K_64K_BYTES,
    AST_ESPIMAFS_CH_CAP_FLASH_BLOCK_ERASE_SIZE_128K_BYTES,
    AST_ESPIMAFS_CH_CAP_FLASH_BLOCK_ERASE_SIZE_256K_BYTES,
    AST_ESPIMAFS_CH_CAP_FLASH_BLOCK_ERASE_SIZE_EOF
} AST_ESPIMAFS_CH_CAP_FLASH_BLOCK_ERASE_SIZE_T;

#define AST_ESPIMAFS_CH_CAP_FLASH_BLOCK_ERASE_SIZE_4K_VAL (4 * 1024)
#define AST_ESPIMAFS_CH_CAP_FLASH_BLOCK_ERASE_SIZE_64K_VAL (64 * 1024)
#define AST_ESPIMAFS_CH_CAP_FLASH_BLOCK_ERASE_SIZE_128K_VAL (128 * 1024)
#define AST_ESPIMAFS_CH_CAP_FLASH_BLOCK_ERASE_SIZE_256K_VAL (256 * 1024)


struct espimafs_data_t {
    unsigned int    header;         //
    unsigned int    buf_len;        // number of bytes
    unsigned char   *buffer;
};

/* IO Command Data Structure */
struct espimafs_get_channel_stat_t {
    unsigned char   num_channel;
    unsigned char   gen_status;
    unsigned char   ch_status;
    unsigned char   FlashAccChMaxReadReqSize;
    unsigned char   FlashAccChMaxPayloadSizeSelected;
    unsigned char   FlashAccChMaxPayloadSizeSupported;
    unsigned char   FlashAccChEraseSizeType;
};

/*************************************************************************************/
/*   Cycle Type     */
/*************************************************************************************/
#define SUCCESS_COMP_WO_DATA            0x06

#define SUCCESS_COMP_W_DATA_MIDDLE      0x09
#define SUCCESS_COMP_W_DATA_FIRST       0x0B
#define SUCCESS_COMP_W_DATA_LAST        0x0D
#define SUCCESS_COMP_W_DATA_ONLY        0x0F

#define USUCCESS_COMP_WO_DATA_LAST      0x0C
#define USUCCESS_COMP_WO_DATA_ONLY      0x0E

#define ESPI_CYCLE_TYPE_FLASH_READ      0x00
#define ESPI_CYCLE_TYPE_FLASH_WRITE     0x01
#define ESPI_CYCLE_TYPE_FLASH_ERASE     0x02

#define ESPIIOC_BASE        'p'

#define GET_FLASH_CHAN_STAT         _IOR(ESPIIOC_BASE, 0x40, struct espimafs_data_t)

#endif /* _ESPIMAFS_IOCTL_H_ */

