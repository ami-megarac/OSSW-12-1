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
 * File name: ast_espioob.c
 * eSPI OOB hardware driver is implemented for hardware controller.
 */

#include <linux/module.h>
#include <linux/pci.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/interrupt.h>

#include "driver_hal.h"
#include "espioob_ioctl.h"
#include "espioob.h"
#include "ast_espioob.h"


static void *ast_espioob_virt_base;

static int ast_espioob_hal_id;
static espioob_core_funcs_t *espioob_core_ops;

static void ast_espi_oob_tx(void);
static int espioob_rx_wait_for_int( int ms_timeout );

struct espioob_ch_data        oob_rx_channel;
struct espioob_ch_data        oob_tx_channel;

unsigned char tag = 0;

/* -------------------------------------------------- */

static inline u32 ast_espi_read_reg(u32 offset)
{
    return( ioread32( (void * __iomem)ast_espioob_virt_base + offset ) );
}

static inline void ast_espi_write_reg(u32 value, u32 offset) 
{
    iowrite32(value, (void * __iomem)ast_espioob_virt_base + offset);
}

/* -------------------------------------------------- */

static unsigned char ast_espioob_num_ch(void)
{
    return AST_ESPIOOB_CHANNEL_NUM;
}

static void oob_read(struct espioob_data_t *espioob_data)
{
    if (espioob_rx_wait_for_int(1000) == 0)
    {
        espioob_data->header = oob_rx_channel.header;
        espioob_data->buf_len = oob_rx_channel.buf_len;
        memcpy(espioob_data->buffer, oob_rx_channel.buff, oob_rx_channel.buf_len);
    }
    else
    {
        espioob_data->header = 0;
        espioob_data->buf_len = 0;
        memset(espioob_data->buffer, 0, MAX_XFER_BUFF_SIZE);
    }
    /* Clear RX Data */
    memset(oob_rx_channel.buff, 0, oob_rx_channel.buf_len);
    oob_rx_channel.header = 0;
    oob_rx_channel.buf_len = 0;
}

static void oob_write (struct espioob_data_t *espioob_data)
{
    unsigned int    header = 0;
    header = espioob_data->header;
    oob_tx_channel.header = (GET_LEN(header) << 12) | ((tag & 0xf) << 8) | GET_CYCLE_TYPE(header);
    oob_tx_channel.buf_len = espioob_data->buf_len;
    memcpy(oob_tx_channel.buff, espioob_data->buffer, oob_tx_channel.buf_len);
    ast_espi_oob_tx();

    tag ++;
    tag %= 0xf;
}

static void oob_writeread(struct espioob_data_t *espioob_data)
{
    // Transmitting
    oob_write(espioob_data);
    // Receiving
    oob_read(espioob_data);
}

static void get_oob_channel_status(u32 *gen_status, u32 *ch_status, u32 *OOBChMaxPayloadSizeSelected, u32 *OOBChMaxPayloadSizeSupported )
{    
    *gen_status = ast_espi_read_reg(AST_ESPI_GEN_CAPCONF);

    *ch_status = 0;
    
    if (ast_espi_read_reg(AST_ESPI_CH2_CAPCONF) & 0x3)
        *ch_status |= OOB_CHANNEL_SUPPORTED;

    *OOBChMaxPayloadSizeSelected = (ast_espi_read_reg(AST_ESPI_CH2_CAPCONF) & 0x700) >> 8;
    *OOBChMaxPayloadSizeSupported = (ast_espi_read_reg(AST_ESPI_CH2_CAPCONF) & 0x70) >> 4;

#ifdef ESPI_DEBUG
    // Slave Registers
    printk("AST_ESPI_GEN_CAPCONF    (0A0h) 0x%x\n", (unsigned int)ast_espi_read_reg(AST_ESPI_GEN_CAPCONF));
    printk("AST_ESPI_CH0_CAPCONF    (0A4h) 0x%x\n", (unsigned int)ast_espi_read_reg(AST_ESPI_CH0_CAPCONF));
    printk("AST_ESPI_CH1_CAPCONF    (0A8h) 0x%x\n", (unsigned int)ast_espi_read_reg(AST_ESPI_CH1_CAPCONF));
    printk("AST_ESPI_CH2_CAPCONF    (0ACh) 0x%x\n", (unsigned int)ast_espi_read_reg(AST_ESPI_CH2_CAPCONF));
    printk("AST_ESPI_CH3_CAPCONF    (0B0h) 0x%x\n", (unsigned int)ast_espi_read_reg(AST_ESPI_CH3_CAPCONF));
#endif
}

static int espioob_rx_wait_for_int( int ms_timeout )
{
    int status = 0;
    if (wait_event_timeout(oob_rx_channel.as_wait, oob_rx_channel.op_status, (ms_timeout*HZ/1000)) == 0)
    {
        status = -1;
        //printk("!!! time out !!!!\n");
    }
    oob_rx_channel.op_status = 0;

    return status;
}

// Channel 2: OOB
static void ast_espi_oob_rx(void)
{
    int i = 0;
    u32 ctrl = ast_espi_read_reg(AST_ESPI_OOB_RX_CTRL);
    //printk("cycle type = %x , tag = %x, len = %d byte \n",GET_CYCLE_TYPE(ctrl), GET_TAG(ctrl), GET_LEN(ctrl));

    oob_rx_channel.header = ctrl;
    oob_rx_channel.buf_len = GET_LEN(ctrl);

    for(i = 0;i< oob_rx_channel.buf_len; i++) 
        oob_rx_channel.buff[i] = ast_espi_read_reg(AST_ESPI_OOB_RX_DATA);

    ast_espi_write_reg(TRIGGER_TRANSACTION , AST_ESPI_OOB_RX_CTRL);
    // Wake up RX process
    oob_rx_channel.op_status = 1;
    wake_up( &oob_rx_channel.as_wait );
}

static void ast_espi_oob_tx(void)
{
    int i=0;    

    for(i = 0;i < oob_tx_channel.buf_len; i++)
        ast_espi_write_reg(oob_tx_channel.buff[i] , AST_ESPI_OOB_TX_DATA);

    ast_espi_write_reg(TRIGGER_TRANSACTION | oob_tx_channel.header , AST_ESPI_OOB_TX_CTRL);
}

/* -------------------------------------------------- */
static espioob_hal_operations_t ast_espi_hw_ops = {
    .num_espioob_ch         = ast_espioob_num_ch,
    .oob_read               = oob_read,
    .oob_write              = oob_write,
    .oob_writeread          = oob_writeread,
    .get_channel_status     = get_oob_channel_status,
};

static hw_hal_t ast_espi_hw_hal = {
    .dev_type = EDEV_TYPE_ESPIOOB,
    .owner = THIS_MODULE,
    .devname = AST_ESPIOOB_DRIVER_NAME,
    .num_instances = AST_ESPIOOB_CHANNEL_NUM,
    .phal_ops = (void *) &ast_espi_hw_ops
};

static irqreturn_t ast_espioob_handler(int this_irq, void *dev_id)
{
    unsigned int handled = 0;
    unsigned long status;
    unsigned long intrEn;
    unsigned long espi_intr;

    status = ast_espi_read_reg(AST_ESPI_INTR_STATUS);
    intrEn  = ast_espi_read_reg(AST_ESPI_INTR_EN);
    //printk(KERN_WARNING "%s: ESPI OOB AST_ESPI_INTR_STATUS status %lx intrEn %lx\n", AST_ESPIOOB_DRIVER_NAME, status, intrEn);
    espi_intr = status & intrEn;

    // OOB Channel
    if(espi_intr & OOB_TX_COMPLETE) {
        //printk("OOB_TX_COMPLETE \n");
        ast_espi_write_reg(OOB_TX_COMPLETE, AST_ESPI_INTR_STATUS);
        handled |= OOB_TX_COMPLETE;
    }

    if(espi_intr & OOB_RX_COMPLETE) {
        //printk("OOB_RX_COMPLETE \n");        
        ast_espi_oob_rx();
        ast_espi_write_reg(OOB_RX_COMPLETE, AST_ESPI_INTR_STATUS);
        handled |= OOB_RX_COMPLETE;
    }

    if(espi_intr & OOB_TX_ERROR) {
        //printk("OOB_TX_ERROR \n");
        ast_espi_write_reg(OOB_TX_ERROR, AST_ESPI_INTR_STATUS);
        handled |= OOB_TX_ERROR;
    }

    if(espi_intr & OOB_TX_ABORT) {
        //printk("OOB_TX_ABORT \n");
        ast_espi_write_reg(OOB_TX_ABORT, AST_ESPI_INTR_STATUS);
        handled |= OOB_TX_ABORT;
    }
    
    if(espi_intr & OOB_RX_ABORT) {
        //printk("OOB_RX_ABORT \n");
        ast_espi_write_reg(OOB_RX_ABORT, AST_ESPI_INTR_STATUS);
        handled |= OOB_RX_ABORT;
    }

    if ((espi_intr & (~handled)) == 0)
        return IRQ_HANDLED;
    else
        return IRQ_NONE;
}

int ast_espi_init(void)
{
    int ret;
    uint32_t status;    

    extern int espioob_core_loaded;
    if (!espioob_core_loaded)
        return -1;
    
    // Detect eSPI mode
    *(volatile u32 *)(IO_ADDRESS(AST_SCU_REG_BASE)) = (0x1688A8A8); //Unlock SCU register

    status = *(volatile u32 *)(IO_ADDRESS( (AST_SCU_REG_BASE + 0x70) ));
    if(!(status & (0x1 << 25)))
    {
        printk(KERN_WARNING "%s: eSPI mode is not enable\n", AST_ESPIOOB_DRIVER_NAME);
        return -EPERM;
    }
    *(volatile u32 *)(IO_ADDRESS(AST_SCU_REG_BASE)) = 0;            //Lock SCU register

    ast_espioob_virt_base = ioremap_nocache(AST_ESPI_REG_BASE, SZ_4K);
    if (!ast_espioob_virt_base) {
        printk(KERN_WARNING "%s: ioremap failed\n", AST_ESPIOOB_DRIVER_NAME);
        unregister_hw_hal_module(EDEV_TYPE_ESPIOOB, ast_espioob_hal_id);
        return -ENOMEM;
    }

    ast_espioob_hal_id = register_hw_hal_module(&ast_espi_hw_hal, (void **) &espioob_core_ops);
    if (ast_espioob_hal_id < 0) {
        printk(KERN_WARNING "%s: register HAL HW module failed\n", AST_ESPIOOB_DRIVER_NAME);
        iounmap(ast_espioob_virt_base);
        return ast_espioob_hal_id;
    }

    ret = request_irq(AST_ESPI_IRQ, ast_espioob_handler, IRQF_SHARED, AST_ESPIOOB_DRIVER_NAME, ast_espioob_virt_base + AST_ESPI_OOB_CHAN_ID);
    if (ret) {
        printk(KERN_WARNING "%s: AST_ESPI_IRQ request irq failed\n", AST_ESPIOOB_DRIVER_NAME);
    }
    
    // Enable eSPI mode
    *(volatile u32 *)(IO_ADDRESS(AST_SCU_REG_BASE)) = (0x1688A8A8); //Unlock SCU register

    status = *(volatile u32 *)(IO_ADDRESS( (AST_SCU_REG_BASE + 0x70) ));
    if(!(status & (0x1 << 25)))
    {
        printk(KERN_WARNING "%s: eSPI mode is not enable\n", AST_ESPIOOB_DRIVER_NAME);
    }
    *(volatile u32 *)(IO_ADDRESS(AST_SCU_REG_BASE)) = 0;            //Lock SCU register
    oob_rx_channel.buff = kmalloc(MAX_XFER_BUFF_SIZE, GFP_KERNEL);
    oob_tx_channel.buff = kmalloc(MAX_XFER_BUFF_SIZE, GFP_KERNEL);
    oob_rx_channel.op_status = 0;
    oob_tx_channel.op_status = 0;
    init_waitqueue_head( &(oob_rx_channel.as_wait));
    
    printk("The eSPI OOB HW Driver is loaded successfully.\n" );
    return 0;
}

void ast_espi_exit(void)
{
    kfree(oob_rx_channel.buff);
    kfree(oob_tx_channel.buff);
    
    free_irq(AST_ESPI_IRQ, ast_espioob_virt_base + AST_ESPI_OOB_CHAN_ID);
    
    iounmap (ast_espioob_virt_base);
    unregister_hw_hal_module(EDEV_TYPE_ESPIOOB, ast_espioob_hal_id);
    
    return;
}

module_init (ast_espi_init);
module_exit (ast_espi_exit);

MODULE_AUTHOR("American Megatrends Inc.");
MODULE_DESCRIPTION("ASPEED AST SoC eSPI OOB Driver");
MODULE_LICENSE ("GPL");
