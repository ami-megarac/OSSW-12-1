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

#ifndef _ESPIOOB_IOCTL_H_
#define _ESPIOOB_IOCTL_H_

#include <linux/socket.h>
#include <linux/tcp.h>

#define OOB_CHANNEL_SUPPORTED               (0x1<<2)

// AST_ESPI_CHx_CAPCONF
typedef enum
{
    AST_ESPIOOB_CH_CAP_MAX_READ_REQ_SIZE_RESERVED = 0,
    AST_ESPIOOB_CH_CAP_MAX_READ_REQ_SIZE_64_BYTES,
    AST_ESPIOOB_CH_CAP_MAX_READ_REQ_SIZE_128_BYTES,
    AST_ESPIOOB_CH_CAP_MAX_READ_REQ_SIZE_256_BYTES,
    AST_ESPIOOB_CH_CAP_MAX_READ_REQ_SIZE_512_BYTES,
    AST_ESPIOOB_CH_CAP_MAX_READ_REQ_SIZE_1024_BYTES,
    AST_ESPIOOB_CH_CAP_MAX_READ_REQ_SIZE_2048_BYTES,
    AST_ESPIOOB_CH_CAP_MAX_READ_REQ_SIZE_4096_BYTES,
    AST_ESPIOOB_CH_CAP_MAX_READ_REQ_SIZE_EOF
} AST_ESPIOOB_CH_CAP_MAX_READ_REQ_SIZE_T;

typedef enum
{
    AST_ESPIOOB_CH_CAP_MAX_PAYLOAD_SIZE_RESERVED = 0,
    AST_ESPIOOB_CH_CAP_MAX_PAYLOAD_SIZE_64_BYTES,
    AST_ESPIOOB_CH_CAP_MAX_PAYLOAD_SIZE_128_BYTES,
    AST_ESPIOOB_CH_CAP_MAX_PAYLOAD_SIZE_256_BYTES,
    AST_ESPIOOB_CH_CAP_MAX_PAYLOAD_SIZE_EOF
} AST_ESPIOOB_CH_CAP_MAX_PAYLOAD_SIZE_T;

struct espioob_data_t {
    unsigned int    header;         //
    unsigned int    buf_len;        // number of bytes
    unsigned char   *buffer;
};

/* IO Command Data Structure */
struct espioob_get_channel_stat_t {
    unsigned char   num_channel;
    unsigned char   gen_status;
    unsigned char   ch_status;
    unsigned char   OOBChMaxPayloadSizeSelected;
    unsigned char   OOBChMaxPayloadSizeSupported;
};

#define ESPI_CYCLE_TYPE_OOB_MESSAGE     0x21       

#define ESPIIOC_BASE        'p'

#define GET_OOB_CHAN_STAT       _IOR(ESPIIOC_BASE, 0x20, struct espioob_data_t)
#define OOB_READ                _IOR(ESPIIOC_BASE, 0x21, struct espioob_data_t)
#define OOB_WRITE               _IOW(ESPIIOC_BASE, 0x22, struct espioob_data_t)
#define OOB_WRITEREAD           _IOWR(ESPIIOC_BASE, 0x23, struct espioob_data_t)

#endif /* _ESPIOOB_IOCTL_H_ */

