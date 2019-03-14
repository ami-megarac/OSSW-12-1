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

 /*
 * File name: ast_espimafs.c
 * eSPI FLASH MAFS hardware driver is implemented for hardware controller.
 */

#include <linux/module.h>
#include <linux/pci.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#include "driver_hal.h"
#include "espimafs_ioctl.h"
#include "espimafs.h"
#include "ast_espimafs.h"


static void *ast_espimafs_virt_base;

static int ast_espimafs_hal_id;
static espimafs_core_funcs_t *espimafs_core_ops;

struct espimafs_ch_data        mafs_rx_channel;
struct espimafs_ch_data        mafs_tx_channel;

static void ast_espimafs_flash_tx(void);
static int espimafs_rx_wait_for_int( int ms_timeout );

unsigned char tag = 0;

/* -------------------------------------------------- */

static inline u32 ast_espimafs_read_reg(u32 offset)
{
    return( ioread32( (void * __iomem)ast_espimafs_virt_base + offset ) );
}

static inline void ast_espimafs_write_reg(u32 value, u32 offset) 
{
    iowrite32(value, (void * __iomem)ast_espimafs_virt_base + offset);
}

/* -------------------------------------------------- */

static unsigned char ast_espimafs_num_ch(void)
{
    return AST_ESPIMAFS_CHANNEL_NUM;
}

static void flashmafs_rx (struct espimafs_data_t *espimafs_data)
{
    if (espimafs_rx_wait_for_int(1000) == 0)
    {
        espimafs_data->header = mafs_rx_channel.header;
        espimafs_data->buf_len = mafs_rx_channel.buf_len;
        memcpy(espimafs_data->buffer, mafs_rx_channel.buff, mafs_rx_channel.buf_len);
    }
    else
    {
        espimafs_data->header = 0;
        espimafs_data->buf_len = 0;
        memset(espimafs_data->buffer, 0, MAX_XFER_BUFF_SIZE);
    }
    /* Clear RX Data */
    memset(mafs_rx_channel.buff, 0, mafs_rx_channel.buf_len);
    mafs_rx_channel.header = 0;
    mafs_rx_channel.buf_len = 0;
}

static void flashmafs_tx (struct espimafs_data_t *espimafs_data)
{
    mafs_tx_channel.header = espimafs_data->header;
    mafs_tx_channel.buf_len = espimafs_data->buf_len;
    memcpy(mafs_tx_channel.buff, espimafs_data->buffer, mafs_tx_channel.buf_len);
    ast_espimafs_flash_tx();
}

static int flashmafs_read (uint32_t addr, uint32_t *len, u8 *data)
{
    struct espimafs_data_t espi_data;
    int ret = -1;

    if (*len > MAX_XFER_BUFF_SIZE)
        return (ret);

    mafs_tx_channel.header = (*len << 12) | ((tag & 0xf) << 8) | ESPI_CYCLE_TYPE_FLASH_READ;
    mafs_tx_channel.buf_len = ESPIMAFS_ADDR_LEN;
    mafs_tx_channel.buff[0] = (addr >> 24) & 0xff;
    mafs_tx_channel.buff[1] = (addr >> 16) & 0xff;
    mafs_tx_channel.buff[2] = (addr >> 8) & 0xff;
    mafs_tx_channel.buff[3] = addr & 0xff;
    ast_espimafs_flash_tx();

    tag ++;
    tag %= 0xf;

    memset(&espi_data, 0 ,sizeof(struct espimafs_data_t));
    espi_data.buffer = data;
    flashmafs_rx (&espi_data);
    *len = GET_LEN(espi_data.header);

    switch(GET_CYCLE_TYPE(espi_data.header))
    {
        case SUCCESS_COMP_WO_DATA:
            //printk("FLASH_RX_COMPLETION SUCCESS_COMP_WO_DATA \n");
            ret = 0;
            break;

        case SUCCESS_COMP_W_DATA_MIDDLE:
            //printk("FLASH_RX_COMPLETION SUCCESS_COMP_W_DATA_MIDDLE \n");
            ret = 0;
            break;

        case SUCCESS_COMP_W_DATA_FIRST:
            //printk("FLASH_RX_COMPLETION SUCCESS_COMP_W_DATA_FIRST \n");
            ret = 0;
            break;

        case SUCCESS_COMP_W_DATA_LAST:
            //printk("FLASH_RX_COMPLETION SUCCESS_COMP_W_DATA_LAST \n");
            ret = 0;
            break;

        case SUCCESS_COMP_W_DATA_ONLY:
            //printk("FLASH_RX_COMPLETION SUCCESS_COMP_W_DATA_ONLY \n");
            ret = 0;
            break;

        case USUCCESS_COMP_WO_DATA_LAST:
            //printk("FLASH_RX_COMPLETION USUCCESS_COMP_WO_DATA_LAST \n");
            break;

        case USUCCESS_COMP_WO_DATA_ONLY:
            //printk("FLASH_RX_COMPLETION USUCCESS_COMP_WO_DATA_ONLY \n");
            break;
    }

    return (ret);
}

static int flashmafs_write (uint32_t addr, uint32_t len, u8 *data)
{
    struct espimafs_data_t espi_data;
    unsigned char cmd[ESPIMAFS_ADDR_LEN] = {0};
    int ret = -1;

    if (len > MAX_XFER_BUFF_SIZE)
        return (ret);

    mafs_tx_channel.header = (len << 12) | ((tag & 0xf) << 8) | ESPI_CYCLE_TYPE_FLASH_WRITE;
    mafs_tx_channel.buf_len = len + ESPIMAFS_ADDR_LEN;
    mafs_tx_channel.buff[0] = (addr >> 24) & 0xff;
    mafs_tx_channel.buff[1] = (addr >> 16) & 0xff;
    mafs_tx_channel.buff[2] = (addr >> 8) & 0xff;
    mafs_tx_channel.buff[3] = addr & 0xff;
    memcpy(&mafs_tx_channel.buff[ESPIMAFS_ADDR_LEN], data, len);
    ast_espimafs_flash_tx();

    tag ++;
    tag %= 0xf;

    memset(&espi_data, 0 ,sizeof(struct espimafs_data_t));
    espi_data.buffer = cmd;
    flashmafs_rx (&espi_data);

    switch(GET_CYCLE_TYPE(espi_data.header))
    {
        case SUCCESS_COMP_WO_DATA:
            //printk("FLASH_RX_COMPLETION SUCCESS_COMP_WO_DATA \n");
            ret = 0;
            break;

        case SUCCESS_COMP_W_DATA_MIDDLE:
            //printk("FLASH_RX_COMPLETION SUCCESS_COMP_W_DATA_MIDDLE \n");
            ret = 0;
            break;

        case SUCCESS_COMP_W_DATA_FIRST:
            //printk("FLASH_RX_COMPLETION SUCCESS_COMP_W_DATA_FIRST \n");
            ret = 0;
            break;

        case SUCCESS_COMP_W_DATA_LAST:
            //printk("FLASH_RX_COMPLETION SUCCESS_COMP_W_DATA_LAST \n");
            ret = 0;
            break;

        case SUCCESS_COMP_W_DATA_ONLY:
            //printk("FLASH_RX_COMPLETION SUCCESS_COMP_W_DATA_ONLY \n");
            ret = 0;
            break;

        case USUCCESS_COMP_WO_DATA_LAST:
            //printk("FLASH_RX_COMPLETION USUCCESS_COMP_WO_DATA_LAST \n");
            break;

        case USUCCESS_COMP_WO_DATA_ONLY:
            //printk("FLASH_RX_COMPLETION USUCCESS_COMP_WO_DATA_ONLY \n");
            break;
    }

    return (ret);
}

static int flashmafs_erase (uint32_t addr, uint32_t eraseblocksize)
{
    struct espimafs_data_t espi_data;
    unsigned char cmd[ESPIMAFS_ADDR_LEN] = {0};
    int ret = -1;

    mafs_tx_channel.header = (eraseblocksize << 12) | ((tag & 0xf) << 8) | ESPI_CYCLE_TYPE_FLASH_ERASE;
    mafs_tx_channel.buf_len = ESPIMAFS_ADDR_LEN;
    mafs_tx_channel.buff[0] = (addr >> 24) & 0xff;
    mafs_tx_channel.buff[1] = (addr >> 16) & 0xff;
    mafs_tx_channel.buff[2] = (addr >> 8) & 0xff;
    mafs_tx_channel.buff[3] = addr & 0xff;
    ast_espimafs_flash_tx();

    tag ++;
    tag %= 0xf;

    memset(&espi_data, 0 ,sizeof(struct espimafs_data_t));
    espi_data.buffer = cmd;
    flashmafs_rx (&espi_data);

    switch(GET_CYCLE_TYPE(espi_data.header))
    {
        case SUCCESS_COMP_WO_DATA:
            //printk("FLASH_RX_COMPLETION SUCCESS_COMP_WO_DATA \n");
            ret = 0;
            break;

        case SUCCESS_COMP_W_DATA_MIDDLE:
            //printk("FLASH_RX_COMPLETION SUCCESS_COMP_W_DATA_MIDDLE \n");
            ret = 0;
            break;

        case SUCCESS_COMP_W_DATA_FIRST:
            //printk("FLASH_RX_COMPLETION SUCCESS_COMP_W_DATA_FIRST \n");
            ret = 0;
            break;

        case SUCCESS_COMP_W_DATA_LAST:
            //printk("FLASH_RX_COMPLETION SUCCESS_COMP_W_DATA_LAST \n");
            ret = 0;
            break;

        case SUCCESS_COMP_W_DATA_ONLY:
            //printk("FLASH_RX_COMPLETION SUCCESS_COMP_W_DATA_ONLY \n");
            ret = 0;
            break;

        case USUCCESS_COMP_WO_DATA_LAST:
            //printk("FLASH_RX_COMPLETION USUCCESS_COMP_WO_DATA_LAST \n");
            break;

        case USUCCESS_COMP_WO_DATA_ONLY:
            //printk("FLASH_RX_COMPLETION USUCCESS_COMP_WO_DATA_ONLY \n");
            break;
    }

    return (ret);
}

static void get_flash_channel_status(uint32_t *gen_status, uint32_t *ch_status, uint32_t *FlashAccChMaxReadReqSize, uint32_t *FlashAccChMaxPayloadSizeSelected, uint32_t *FlashAccChMaxPayloadSizeSupported, uint32_t *FlashAccChEraseSizeType )
{
    *gen_status = ast_espimafs_read_reg(AST_ESPI_GEN_CAPCONF);
    
    *ch_status = 0;
    if (ast_espimafs_read_reg(AST_ESPI_CH3_CAPCONF) & 0x3)
        *ch_status |= FLASH_CHANNEL_SUPPORTED;

    *FlashAccChMaxReadReqSize = (ast_espimafs_read_reg(AST_ESPI_CH3_CAPCONF) & 0x7000) >> 12;
    *FlashAccChMaxPayloadSizeSelected = (ast_espimafs_read_reg(AST_ESPI_CH3_CAPCONF) & 0x700) >> 8;
    *FlashAccChMaxPayloadSizeSupported = (ast_espimafs_read_reg(AST_ESPI_CH3_CAPCONF) & 0xE0) >> 5;
    *FlashAccChEraseSizeType = (ast_espimafs_read_reg(AST_ESPI_CH3_CAPCONF) & 0x1C) >> 2;

#ifdef ESPI_DEBUG
    // Slave Registers
    printk("AST_ESPI_GEN_CAPCONF    (0A0h) 0x%x\n", (unsigned int)ast_espimafs_read_reg(AST_ESPI_GEN_CAPCONF));
    printk("AST_ESPI_CH0_CAPCONF    (0A4h) 0x%x\n", (unsigned int)ast_espimafs_read_reg(AST_ESPI_CH0_CAPCONF));
    printk("AST_ESPI_CH1_CAPCONF    (0A8h) 0x%x\n", (unsigned int)ast_espimafs_read_reg(AST_ESPI_CH1_CAPCONF));
    printk("AST_ESPI_CH2_CAPCONF    (0ACh) 0x%x\n", (unsigned int)ast_espimafs_read_reg(AST_ESPI_CH2_CAPCONF));
    printk("AST_ESPI_CH3_CAPCONF    (0B0h) 0x%x\n", (unsigned int)ast_espimafs_read_reg(AST_ESPI_CH3_CAPCONF));
#endif
}

static int espimafs_rx_wait_for_int( int ms_timeout )
{
    int status = 0;
    if (wait_event_timeout(mafs_rx_channel.as_wait, mafs_rx_channel.op_status, (ms_timeout*HZ/1000)) == 0)
    {
        status = -1;
        //printk("!!! time out !!!!\n");
    }
    mafs_rx_channel.op_status = 0;

    return status;
}

static void ast_espimafs_flash_completion(unsigned int header, unsigned char data_len,unsigned char *data)
{
    struct espimafs_data_t espimafs_data;

    espimafs_data.header = header;
    espimafs_data.buf_len = data_len;
    if(data)
        espimafs_data.buffer = data;
    else
        espimafs_data.buffer = NULL;    

    flashmafs_tx(&espimafs_data);
}

static void flash_read_completion(unsigned int header, unsigned char *data)
{
    int i = 0;
    int len = GET_LEN(header);
    //printk("FLASH_COMPLETION cycle type = %x , tag = %x, len = %d byte  \n", GET_CYCLE_TYPE(header), GET_TAG(header), GET_LEN(header));
    if(len <= 64) {
        //single - only
        ast_espimafs_flash_completion(header |SUCCESS_COMP_W_DATA_ONLY, GET_LEN(header), data);
    } else {
        //split  ---
        header &= 0xffffff00;
        for(i = 0; i < len; i+=64) {
            if(i ==0) {
                ast_espimafs_flash_completion(header |SUCCESS_COMP_W_DATA_FIRST, 64, data);
            } else if (i + 64 >= len) {
                ast_espimafs_flash_completion(header |SUCCESS_COMP_W_DATA_LAST, header | 64, data);
            } else {
                ast_espimafs_flash_completion(header |SUCCESS_COMP_W_DATA_MIDDLE, header | (len - i), data);
            }
        }
    }

}

static void flash_write_completion(unsigned int tag)
{
    //single - only
    ast_espimafs_flash_completion((tag << 8)  |SUCCESS_COMP_WO_DATA, 0, NULL);

}

static void flash_erase_completion(unsigned int tag)
{
    //single - only
    ast_espimafs_flash_completion((tag << 8) |SUCCESS_COMP_WO_DATA, 0, NULL);
}

// Channel 3: Runtime Flash
static void ast_espimafs_flash_rx_completion(void)
{
    struct espimafs_ch_data *flash_rx = &mafs_rx_channel;

    //printk("FLASH_RX_COMPLETION cycle type = %x , tag = %x, len = %d byte  \n", GET_CYCLE_TYPE(flash_rx->header), GET_TAG(flash_rx->header), GET_LEN(flash_rx->header));

    switch(GET_CYCLE_TYPE(flash_rx->header))
    {
        case ESPI_CYCLE_TYPE_FLASH_READ:
            //printk("RX COMPLETION FLASH_READ len %d \n", GET_LEN(flash_rx->header));
            flash_read_completion(flash_rx->header , NULL);
            break;

        case ESPI_CYCLE_TYPE_FLASH_WRITE:
            //printk("RX COMPLETION FLASH_WRITE len %d \n", GET_LEN(flash_rx->header));
            flash_write_completion(GET_TAG(flash_rx->header));
            break;

        case ESPI_CYCLE_TYPE_FLASH_ERASE:
            //printk("RX COMPLETION FLASH_ERASE len %d \n", GET_LEN(flash_rx->header));
            flash_erase_completion(GET_TAG(flash_rx->header));
            break;

        case SUCCESS_COMP_WO_DATA:
            //printk("FLASH_RX_COMPLETION SUCCESS_COMP_WO_DATA \n");
            break;

        case SUCCESS_COMP_W_DATA_MIDDLE:
            //printk("FLASH_RX_COMPLETION SUCCESS_COMP_W_DATA_MIDDLE \n");
            break;

        case SUCCESS_COMP_W_DATA_FIRST:
            //printk("FLASH_RX_COMPLETION SUCCESS_COMP_W_DATA_FIRST \n");
            break;

        case SUCCESS_COMP_W_DATA_LAST:
            //printk("FLASH_RX_COMPLETION SUCCESS_COMP_W_DATA_LAST \n");
            break;

        case SUCCESS_COMP_W_DATA_ONLY:
            //printk("FLASH_RX_COMPLETION SUCCESS_COMP_W_DATA_ONLY \n");
            break;

        case USUCCESS_COMP_WO_DATA_LAST:
            //printk("FLASH_RX_COMPLETION USUCCESS_COMP_WO_DATA_LAST \n");
            break;

        case USUCCESS_COMP_WO_DATA_ONLY:
            //printk("FLASH_RX_COMPLETION USUCCESS_COMP_WO_DATA_ONLY \n");
            break;
    }
}

static void ast_espimafs_flash_rx(void)
{
    int i = 0;
    struct espimafs_ch_data *flash_rx = &mafs_rx_channel;
    u32 ctrl = ast_espimafs_read_reg(AST_ESPI_FLASH_RX_CTRL);
    //printk("flash_rx cycle type = %x , tag = %x, len = %d byte \n", GET_CYCLE_TYPE(ctrl), GET_TAG(ctrl), GET_LEN(ctrl));

    flash_rx->full = 1;
    flash_rx->header = ctrl;
    if((GET_CYCLE_TYPE(ctrl) == 0x00) || (GET_CYCLE_TYPE(ctrl) == 0x02))
        flash_rx->buf_len = 4;
    else if (GET_CYCLE_TYPE(ctrl) == 0x01)
        flash_rx->buf_len = GET_LEN(ctrl) + 4;
    else if ((GET_CYCLE_TYPE(ctrl) & 0x09) == 0x09)
        flash_rx->buf_len = GET_LEN(ctrl);
    else
        flash_rx->buf_len = 0;

    for(i = 0;i< flash_rx->buf_len; i++) 
        flash_rx->buff[i] = ast_espimafs_read_reg(AST_ESPI_FLASH_RX_DATA);

    ast_espimafs_write_reg(TRIGGER_TRANSACTION, AST_ESPI_FLASH_RX_CTRL);

    // Wake up RX process
    mafs_rx_channel.op_status = 1;
    wake_up( &mafs_rx_channel.as_wait );
}

static void ast_espimafs_flash_tx(void)
{
    int i=0;    

    //printk("flash_tx cycle type = %x , tag = %x, len = %d byte \n", GET_CYCLE_TYPE(mafs_tx_channel.header), GET_TAG(mafs_tx_channel.header), GET_LEN(mafs_tx_channel.header));
    mafs_rx_channel.op_status = 0;
    for(i = 0;i < mafs_tx_channel.buf_len; i++)
        ast_espimafs_write_reg(mafs_tx_channel.buff[i], AST_ESPI_FLASH_TX_DATA);

    ast_espimafs_write_reg(TRIGGER_TRANSACTION | mafs_tx_channel.header, AST_ESPI_FLASH_TX_CTRL);
}

/* -------------------------------------------------- */
static espimafs_hal_operations_t ast_espimafs_hw_ops = {
    .num_espimafs_ch        = ast_espimafs_num_ch,
    .flashmafs_read 		= flashmafs_read,
    .flashmafs_write 		= flashmafs_write,
    .flashmafs_erase 		= flashmafs_erase,
    .get_channel_status     = get_flash_channel_status,
};

static hw_hal_t ast_espimafs_hw_hal = {
    .dev_type = EDEV_TYPE_ESPIMAFS,
    .owner = THIS_MODULE,
    .devname = AST_ESPIMAFS_DRIVER_NAME,
    .num_instances = AST_ESPIMAFS_CHANNEL_NUM,
    .phal_ops = (void *) &ast_espimafs_hw_ops
};

static irqreturn_t ast_espimafs_handler(int this_irq, void *dev_id)
{
    unsigned int handled = 0;
    unsigned long status;
    unsigned long intrEn;
    unsigned long espi_intr;

    status = ast_espimafs_read_reg(AST_ESPI_INTR_STATUS);
    intrEn  = ast_espimafs_read_reg(AST_ESPI_INTR_EN);
    //printk(KERN_WARNING "%s: AST_ESPI_INTR_STATUS status %lx intrEn %lx\n", AST_ESPIMAFS_DRIVER_NAME, status, intrEn);
    espi_intr = status & intrEn;

    // Flash Channel
    if(espi_intr & FLASH_TX_COMPLETE) {
        //printk("FLASH_TX_COMPLETE \n");
        ast_espimafs_write_reg(FLASH_TX_COMPLETE, AST_ESPI_INTR_STATUS);
        handled |= FLASH_TX_COMPLETE;
    }

    if(espi_intr & FLASH_RX_COMPLETE) {
        //printk("FLASH_RX_COMPLETE \n");
        ast_espimafs_flash_rx();
        ast_espimafs_write_reg(FLASH_RX_COMPLETE, AST_ESPI_INTR_STATUS);
        handled |= FLASH_RX_COMPLETE;
        ast_espimafs_flash_rx_completion();
    }
    
    if(espi_intr & FLASH_TX_ERROR) {
        //printk("FLASH_TX_ERROR \n");
        ast_espimafs_write_reg(FLASH_TX_ERROR, AST_ESPI_INTR_STATUS);
        handled |= FLASH_TX_ERROR;
    }
    
    if(espi_intr & FLASH_TX_ABORT) {
        //printk("FLASH_TX_ABORT \n");
        ast_espimafs_write_reg(FLASH_TX_ABORT, AST_ESPI_INTR_STATUS);
        handled |= FLASH_TX_ABORT;
    }
    
    if(espi_intr & FLASH_RX_ABORT) {
        //printk("FLASH_RX_ABORT \n");
        ast_espimafs_write_reg(FLASH_RX_ABORT, AST_ESPI_INTR_STATUS);
        handled |= FLASH_RX_ABORT;
    }

    if ((espi_intr & (~handled)) == 0)
        return IRQ_HANDLED;
    else
        return IRQ_NONE;
}

int ast_espimafs_init(void)
{
    int ret;
    uint32_t status;    

    extern int espimafs_core_loaded;
    if (!espimafs_core_loaded)
        return -1;
    
    // Detect eSPI mode
    *(volatile u32 *)(IO_ADDRESS(AST_SCU_REG_BASE)) = (0x1688A8A8); //Unlock SCU register

    status = *(volatile u32 *)(IO_ADDRESS( (AST_SCU_REG_BASE + 0x70) ));
    if(!(status & (0x1 << 25)))
    {
        printk(KERN_WARNING "%s: eSPI mode is not enable\n", AST_ESPIMAFS_DRIVER_NAME);
        return -EPERM;
    }
    *(volatile u32 *)(IO_ADDRESS(AST_SCU_REG_BASE)) = 0;            //Lock SCU register

    ast_espimafs_virt_base = ioremap_nocache(AST_ESPI_REG_BASE, SZ_4K);
    if (!ast_espimafs_virt_base) {
        printk(KERN_WARNING "%s: ioremap failed\n", AST_ESPIMAFS_DRIVER_NAME);
        unregister_hw_hal_module(EDEV_TYPE_ESPIMAFS, ast_espimafs_hal_id);
        return -ENOMEM;
    }

    ast_espimafs_hal_id = register_hw_hal_module(&ast_espimafs_hw_hal, (void **) &espimafs_core_ops);
    if (ast_espimafs_hal_id < 0) {
        printk(KERN_WARNING "%s: register HAL HW module failed\n", AST_ESPIMAFS_DRIVER_NAME);
        iounmap(ast_espimafs_virt_base);
        return ast_espimafs_hal_id;
    }
    
    ret = request_irq(AST_ESPI_IRQ, ast_espimafs_handler, IRQF_SHARED, "ast_espimafs", ast_espimafs_virt_base + AST_ESPI_FLASH_CHAN_ID);
    if (ret) {
        printk(KERN_WARNING "%s: AST_ESPI_IRQ request irq failed\n", AST_ESPIMAFS_DRIVER_NAME);
    }
    
    // Enable eSPI mode
    *(volatile u32 *)(IO_ADDRESS(AST_SCU_REG_BASE)) = (0x1688A8A8); //Unlock SCU register

    status = *(volatile u32 *)(IO_ADDRESS( (AST_SCU_REG_BASE + 0x70) ));
    if(!(status & (0x1 << 25)))
    {
        printk(KERN_WARNING "%s: eSPI mode is not enable\n", AST_ESPIMAFS_DRIVER_NAME);
    }
    *(volatile u32 *)(IO_ADDRESS(AST_SCU_REG_BASE)) = 0;            //Lock SCU register
    
    mafs_rx_channel.buff = kmalloc(MAX_XFER_BUFF_SIZE, GFP_KERNEL);
    mafs_tx_channel.buff = kmalloc(MAX_XFER_BUFF_SIZE, GFP_KERNEL);
    mafs_rx_channel.op_status = 0;
    mafs_tx_channel.op_status = 0;
    init_waitqueue_head( &(mafs_rx_channel.as_wait));

    printk("The eSPI MAFS HW Driver is loaded successfully.\n" );
    return 0;
}

void ast_espimafs_exit(void)
{
    kfree(mafs_rx_channel.buff);
    kfree(mafs_tx_channel.buff);
    
    free_irq(AST_ESPI_IRQ, ast_espimafs_virt_base + AST_ESPI_FLASH_CHAN_ID);
    
    iounmap (ast_espimafs_virt_base);
    unregister_hw_hal_module(EDEV_TYPE_ESPIMAFS, ast_espimafs_hal_id);
    
    return;
}

module_init (ast_espimafs_init);
module_exit (ast_espimafs_exit);

MODULE_AUTHOR("American Megatrends Inc.");
MODULE_DESCRIPTION("ASPEED AST SoC eSPI MAFS Driver");
MODULE_LICENSE ("GPL");
