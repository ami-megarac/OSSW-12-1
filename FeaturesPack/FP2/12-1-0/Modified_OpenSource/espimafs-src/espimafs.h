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

#ifndef __ESPIMAFS_H__
#define __ESPIMAFS_H__

#include <linux/types.h>

#define MAX_XFER_BUFF_SIZE          0xFFF        //4096

// AST_ESPI_MAFS defination
#define ESPIMAFS_ADDR_LEN               4

typedef struct 
{
    unsigned char (*num_espimafs_ch) (void);
    int (*flashmafs_read) (uint32_t addr, uint32_t *len, u8 *data);
    int (*flashmafs_write) (uint32_t addr, uint32_t len, u8 *data);
    int (*flashmafs_erase) (uint32_t addr, uint32_t eraseblocksize);
    void (*get_channel_status) (uint32_t *gen_status, uint32_t *ch_status, uint32_t *FlashAccChMaxReadReqSize, uint32_t *FlashAccChMaxPayloadSizeSelected, uint32_t *FlashAccChMaxPayloadSizeSupported, uint32_t *FlashAccChEraseSize);
} espimafs_hal_operations_t;


typedef struct
{
    void (*get_espimafs_core_data) ( int dev_id );
} espimafs_core_funcs_t;


struct espimafs_hal
{
    espimafs_hal_operations_t *pespimafs_hal_ops;
	struct mtd_info *mtd;
	struct mtd_partition partitions;
};


struct espimafs_dev
{
    struct espimafs_hal *pespimafs_hal;
    unsigned char ch_num;
};

#endif

