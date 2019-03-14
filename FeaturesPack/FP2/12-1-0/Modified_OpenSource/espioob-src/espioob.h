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

#ifndef __ESPIOOB_H__
#define __ESPIOOB_H__

#include <linux/types.h>

typedef struct 
{
    unsigned char (*num_espioob_ch) (void);
    void (*oob_read) (struct espioob_data_t *espioob_data);
    void (*oob_write) (struct espioob_data_t *espioob_data);
    void (*oob_writeread) (struct espioob_data_t *espioob_data);
    void (*get_channel_status) (u32 *gen_status, u32 *ch_status, u32 *OOBChMaxPayloadSizeSelected, u32 *OOBChMaxPayloadSizeSupported);
} espioob_hal_operations_t;


typedef struct
{
    void (*get_espioob_core_data) ( int dev_id );
} espioob_core_funcs_t;


struct espioob_hal
{
    espioob_hal_operations_t *pespioob_hal_ops;
};


struct espioob_dev
{
    struct espioob_hal *pespioob_hal;
    unsigned char ch_num;
};

#endif

