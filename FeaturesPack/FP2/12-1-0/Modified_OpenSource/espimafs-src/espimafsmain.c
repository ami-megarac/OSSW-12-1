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
 * File name: espimafsmain.c
 * This driver provides MAFS common layer, independent of the hardware, for the eSPI MAFS driver.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/poll.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/vmalloc.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <asm/io.h>
#include "helper.h"
#include "driver_hal.h"
#include "dbgout.h"
#include "espimafs_ioctl.h"
#include "espimafs.h"
#include "../drivers/mtd/mtdcore.h"

#ifdef HAVE_UNLOCKED_IOCTL
  #if HAVE_UNLOCKED_IOCTL
  #define USE_UNLOCKED_IOCTL
  #endif
#endif

//#define ESPI_DEBUG          1
#define ESPIMAFS_MAJOR          178
#define ESPIMAFS_MINOR          0
#define ESPIMAFS_MAX_DEVICES    255
#define ESPIMAFS_DEV_NAME       "espimafs"


static struct cdev *espi_cdev;
static dev_t espimafs_devno = MKDEV(ESPIMAFS_MAJOR, ESPIMAFS_MINOR);
struct espimafs_flash_info_t *g_espimafs_flash_info = NULL;
struct semaphore espimafs_flash_lock;
uint32_t FlashAccChMaxReadReqSize;
uint32_t FlashAccChMaxPayloadSizeSelected;
uint32_t FlashAccChMaxPayloadSizeSupported;
uint32_t FlashAccChEraseSizeType;

static void espimafs_get_flash_cap(espimafs_hal_operations_t *pespimafs_hal_ops, uint32_t *FlashAccChMaxReadReqSize, uint32_t *FlashAccChMaxPayloadSizeSelected, uint32_t *FlashAccChMaxPayloadSizeSupported, uint32_t *FlashAccChEraseSizeType)
{
    unsigned int gen_status = 0;
    unsigned int ch_status = 0;

    gen_status = gen_status;
    ch_status = ch_status;

    pespimafs_hal_ops->get_channel_status(&gen_status,
                                          &ch_status,
                                          FlashAccChMaxReadReqSize,
                                          FlashAccChMaxPayloadSizeSelected,
                                          FlashAccChMaxPayloadSizeSupported,
                                          FlashAccChEraseSizeType);


    switch(*FlashAccChMaxReadReqSize)
    {
        case AST_ESPIMAFS_CH_CAP_MAX_READ_REQ_SIZE_64_BYTES:
            *FlashAccChMaxReadReqSize = 64;
            break;
        case AST_ESPIMAFS_CH_CAP_MAX_READ_REQ_SIZE_128_BYTES:
            *FlashAccChMaxReadReqSize = 128;
            break;
        case AST_ESPIMAFS_CH_CAP_MAX_READ_REQ_SIZE_256_BYTES:
            *FlashAccChMaxReadReqSize = 256;
            break;
        case AST_ESPIMAFS_CH_CAP_MAX_READ_REQ_SIZE_512_BYTES:
            *FlashAccChMaxReadReqSize = 512;
            break;
        case AST_ESPIMAFS_CH_CAP_MAX_READ_REQ_SIZE_1024_BYTES:
            *FlashAccChMaxReadReqSize = 1024;
            break;
        case AST_ESPIMAFS_CH_CAP_MAX_READ_REQ_SIZE_2048_BYTES:
            *FlashAccChMaxReadReqSize = 2048;
            break;
        case AST_ESPIMAFS_CH_CAP_MAX_READ_REQ_SIZE_4096_BYTES:
            *FlashAccChMaxReadReqSize = 4096;
            break;
        default:
            *FlashAccChMaxReadReqSize = 0;
            break;
    }

    switch(*FlashAccChMaxPayloadSizeSelected)
    {
        case AST_ESPIMAFS_CH_CAP_MAX_PAYLOAD_SIZE_64_BYTES:
            *FlashAccChMaxPayloadSizeSelected = 64;
            break;
        case AST_ESPIMAFS_CH_CAP_MAX_PAYLOAD_SIZE_128_BYTES:
            *FlashAccChMaxPayloadSizeSelected = 128;
            break;
        case AST_ESPIMAFS_CH_CAP_MAX_PAYLOAD_SIZE_256_BYTES:
            *FlashAccChMaxPayloadSizeSelected = 256;
            break;
        default:
            *FlashAccChMaxPayloadSizeSelected = 0;
            break;
    }

    switch(*FlashAccChMaxPayloadSizeSupported)
    {
        case AST_ESPIMAFS_CH_CAP_MAX_PAYLOAD_SIZE_64_BYTES:
            *FlashAccChMaxPayloadSizeSupported = 64;
            break;
        case AST_ESPIMAFS_CH_CAP_MAX_PAYLOAD_SIZE_128_BYTES:
            *FlashAccChMaxPayloadSizeSupported = 128;
            break;
        case AST_ESPIMAFS_CH_CAP_MAX_PAYLOAD_SIZE_256_BYTES:
            *FlashAccChMaxPayloadSizeSupported = 256;
            break;
        default:
            *FlashAccChMaxPayloadSizeSupported = 0;
            break;
    }
}

static int espimafs_flash_erase(struct mtd_info *mtd, struct erase_info *instr)
{
    espimafs_hal_operations_t *pespimafs_hal_ops;
    uint32_t addr, len;
#if (LINUX_VERSION_CODE >=  KERNEL_VERSION(3,4,11))
	uint64_t instr_addr = 0, instr_len = 0;
#endif
    uint32_t eraseblocksize = mtd->erasesize;
    uint32_t espimafs_erasesizetype;

    /* sanity checks */
    if (instr->addr + instr->len > mtd->size)
	{
		return -EINVAL;
	}

#if (LINUX_VERSION_CODE >=  KERNEL_VERSION(3,4,11))
	/* Code to solve : WARNING: "__umoddi3". Need to check for alternative solution. */
	instr_addr = instr->addr;
	instr_len = instr->len;
	if (do_div(instr_addr, eraseblocksize) != 0 || do_div(instr_len, eraseblocksize) != 0)
		return -EINVAL;
#else
    if ((instr->addr % eraseblocksize) != 0 || (instr->len % eraseblocksize) != 0)
        return -EINVAL;
#endif

    pespimafs_hal_ops = mtd->priv;
    addr = instr->addr;
    len = instr->len;

    switch(eraseblocksize)
    {
        case AST_ESPIMAFS_CH_CAP_FLASH_BLOCK_ERASE_SIZE_4K_VAL:
            espimafs_erasesizetype = AST_ESPIMAFS_CH_CAP_FLASH_BLOCK_ERASE_SIZE_4K_BYTES;
            break;
        case AST_ESPIMAFS_CH_CAP_FLASH_BLOCK_ERASE_SIZE_64K_VAL:
            espimafs_erasesizetype = AST_ESPIMAFS_CH_CAP_FLASH_BLOCK_ERASE_SIZE_64K_BYTES;
            break;
        case AST_ESPIMAFS_CH_CAP_FLASH_BLOCK_ERASE_SIZE_128K_VAL:
            espimafs_erasesizetype = AST_ESPIMAFS_CH_CAP_FLASH_BLOCK_ERASE_SIZE_128K_BYTES;
            break;
        case AST_ESPIMAFS_CH_CAP_FLASH_BLOCK_ERASE_SIZE_256K_VAL:
            espimafs_erasesizetype = AST_ESPIMAFS_CH_CAP_FLASH_BLOCK_ERASE_SIZE_256K_BYTES;
            break;
        default:
            return -EINVAL;
            break;
    }

    while (len)
    {
        /* the least significat 3 bits of the length field of Flash Erase command specifies the
           size of block to be erased with the encoding matches the value of the Flash Block Erase
           Size filed of the Channel Capabilities and Configuration register */
        if (pespimafs_hal_ops->flashmafs_erase(addr, espimafs_erasesizetype))
        {
            instr->state = MTD_ERASE_FAILED;
            up(&espimafs_flash_lock);
            return -EIO;
        }

        addr += eraseblocksize;
        len -= eraseblocksize;
    }

    up(&espimafs_flash_lock);

    instr->state = MTD_ERASE_DONE;
    mtd_erase_callback(instr);

    return 0;
}

static int espimafs_flash_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf)
{
    espimafs_hal_operations_t *pespimafs_hal_ops;
    uint32_t transfer_len = 0;

    if (len == 0)
    {
        *retlen = 0;
        return 0;
    }

    if (from + len > mtd->size)
        return -EINVAL;

    pespimafs_hal_ops = mtd->priv;

    /* Byte count starts at zero. */
    if (retlen != 0)
        *retlen = 0;

    down(&espimafs_flash_lock);

    while (len > 0)
    {
        if (len > FlashAccChMaxReadReqSize)
            transfer_len = FlashAccChMaxReadReqSize;
        else
            transfer_len = len;

        if (pespimafs_hal_ops->flashmafs_read(from, &transfer_len, (u8 *)buf))
        {
            up(&espimafs_flash_lock);
            return -EIO;
        }
        
        buf += transfer_len;
        from += transfer_len;
        len -= transfer_len;
        (*retlen) += transfer_len;
    }

    up(&espimafs_flash_lock);

    return 0;
}

static int espimafs_flash_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf)
{
    espimafs_hal_operations_t *pespimafs_hal_ops;
    uint32_t transfer_len = 0;

    if (len == 0)
    {
        *retlen = 0;
        return 0;
    }

    if (to + len > mtd->size)
        return -EINVAL;

    pespimafs_hal_ops = mtd->priv;

    /* Byte count starts at zero. */
    if (retlen != 0)
        *retlen = 0;

    down(&espimafs_flash_lock);

    while (len > 0)
    {
        if (len > FlashAccChMaxPayloadSizeSelected)
            transfer_len = FlashAccChMaxPayloadSizeSelected;
        else
            transfer_len = len;

        if (pespimafs_hal_ops->flashmafs_write(to, transfer_len, (u8 *)buf))
        {
            up(&espimafs_flash_lock);
            return -EIO;
        }
        
        buf += transfer_len;
        to += transfer_len;
        len -= transfer_len;
        (*retlen) += transfer_len;
    }

    up(&espimafs_flash_lock);

    return 0;
}

static struct mtd_info *espimafs_flash_probe(espimafs_hal_operations_t *pespimafs_hal_ops)
{
    struct mtd_info *new_mtd;

    new_mtd = kzalloc(sizeof(struct mtd_info), GFP_KERNEL);
    if (!new_mtd)
        return NULL;

    espimafs_get_flash_cap(pespimafs_hal_ops,
                           &FlashAccChMaxReadReqSize,
                           &FlashAccChMaxPayloadSizeSelected,
                           &FlashAccChMaxPayloadSizeSupported,
                           &FlashAccChEraseSizeType);

    new_mtd->name = CONFIG_SPX_FEATURE_ESPI_MAFS_FLASH_MTD_NAME;
    new_mtd->type = MTD_NORFLASH;
    new_mtd->flags = MTD_CAP_NORFLASH;
    new_mtd->size = CONFIG_SPX_FEATURE_ESPI_MAFS_FLASH_SIZE *1024;

    switch(FlashAccChEraseSizeType)
    {
        case AST_ESPIMAFS_CH_CAP_FLASH_BLOCK_ERASE_SIZE_4K_BYTES:
            new_mtd->erasesize = AST_ESPIMAFS_CH_CAP_FLASH_BLOCK_ERASE_SIZE_4K_VAL;
            break;
        case AST_ESPIMAFS_CH_CAP_FLASH_BLOCK_ERASE_SIZE_64K_BYTES:
            new_mtd->erasesize = AST_ESPIMAFS_CH_CAP_FLASH_BLOCK_ERASE_SIZE_64K_VAL;
            break;
        case AST_ESPIMAFS_CH_CAP_FLASH_BLOCK_ERASE_SIZE_4K_64K_BYTES:
            new_mtd->erasesize = AST_ESPIMAFS_CH_CAP_FLASH_BLOCK_ERASE_SIZE_64K_VAL;
            break;
        case AST_ESPIMAFS_CH_CAP_FLASH_BLOCK_ERASE_SIZE_128K_BYTES:
            new_mtd->erasesize = AST_ESPIMAFS_CH_CAP_FLASH_BLOCK_ERASE_SIZE_128K_VAL;
            break;
        case AST_ESPIMAFS_CH_CAP_FLASH_BLOCK_ERASE_SIZE_256K_BYTES:
            new_mtd->erasesize = AST_ESPIMAFS_CH_CAP_FLASH_BLOCK_ERASE_SIZE_128K_VAL;
            break;
        default:
            new_mtd->erasesize = 0;
            break;
    }

#if (LINUX_VERSION_CODE >=  KERNEL_VERSION(3,4,11))
	new_mtd->_erase = espimafs_flash_erase;
	new_mtd->_read = espimafs_flash_read;
	new_mtd->_write = espimafs_flash_write;
#else
    new_mtd->erase = espimafs_flash_erase;
    new_mtd->read = espimafs_flash_read;
    new_mtd->write = espimafs_flash_write;
#endif
    new_mtd->writesize = 1;

    return new_mtd;
}

int register_espimafs_hal_module (unsigned char num_instances, void *phal_ops, void **phw_data)
{
    struct espimafs_hal *pespimafs_hal;

    if (phal_ops == NULL)
    {
        return -ENOMEM;
    }

    pespimafs_hal = (struct espimafs_hal*) kmalloc (sizeof(struct espimafs_hal), GFP_KERNEL);
    if (!pespimafs_hal)
    {
        return -ENOMEM;
    }

    pespimafs_hal->pespimafs_hal_ops = ( espimafs_hal_operations_t *) phal_ops;
    pespimafs_hal->mtd = espimafs_flash_probe(pespimafs_hal->pespimafs_hal_ops);
    if (pespimafs_hal->mtd == NULL)
    {
        kfree(pespimafs_hal);
        return -ENODEV;
    }

    pespimafs_hal->mtd->owner = THIS_MODULE;
    pespimafs_hal->mtd->priv = phal_ops;

    pespimafs_hal->partitions.name = CONFIG_SPX_FEATURE_ESPI_MAFS_FLASH_MTD_NAME;
    pespimafs_hal->partitions.offset = 0;
    pespimafs_hal->partitions.size = pespimafs_hal->mtd->size;
    pespimafs_hal->partitions.mask_flags = 0;
#if !(LINUX_VERSION_CODE >=  KERNEL_VERSION(3,4,11))
    pespimafs_hal->partitions.mtdp = NULL;
#endif

    add_mtd_partitions(pespimafs_hal->mtd, &pespimafs_hal->partitions, 1);

    *phw_data = (void *) pespimafs_hal;    

    return 0;    
}


int unregister_espimafs_hal_module (void *phw_data)
{
    struct espimafs_hal *pespimafs_hal = (struct espimafs_hal*) phw_data;

    kfree (pespimafs_hal);

    return 0;
}

int get_flash_channel_status (struct espimafs_dev *pdev, unsigned long arg)
{
    struct espimafs_data_t espi_data;
    unsigned int num_channel;
    unsigned int gen_status;
    unsigned int ch_status;
    unsigned int FlashAccChMaxReadReqSize;
    unsigned int FlashAccChMaxPayloadSizeSelected;
    unsigned int FlashAccChMaxPayloadSizeSupported;
    unsigned int FlashAccChEraseSizeType;

    if  (copy_from_user (&espi_data, (void*) arg, sizeof(struct espimafs_data_t)))
        return -EFAULT;
    
    num_channel = pdev->pespimafs_hal->pespimafs_hal_ops->num_espimafs_ch();
    pdev->pespimafs_hal->pespimafs_hal_ops->get_channel_status(&gen_status,
                                                               &ch_status,
                                                               &FlashAccChMaxReadReqSize,
                                                               &FlashAccChMaxPayloadSizeSelected,
                                                               &FlashAccChMaxPayloadSizeSupported,
                                                               &FlashAccChEraseSizeType);

#ifdef ESPI_DEBUG
    printk ("Number of FLASH channel support %d \n", num_channel);

    if (gen_status & FLASH_CHANNEL_SUPPORTED)
        printk("FLASH Channel Supported\n");
    
    if (ch_status & FLASH_CHANNEL_SUPPORTED)
        printk("Host FLASH Channel Supported\n");
#endif

    espi_data.buf_len = sizeof(struct espimafs_get_channel_stat_t);
    ((struct espimafs_get_channel_stat_t *) espi_data.buffer)->num_channel = num_channel;
    ((struct espimafs_get_channel_stat_t *) espi_data.buffer)->gen_status = gen_status;
    ((struct espimafs_get_channel_stat_t *) espi_data.buffer)->ch_status = ch_status;
    ((struct espimafs_get_channel_stat_t *) espi_data.buffer)->FlashAccChMaxReadReqSize = FlashAccChMaxReadReqSize;
    ((struct espimafs_get_channel_stat_t *) espi_data.buffer)->FlashAccChMaxPayloadSizeSelected = FlashAccChMaxPayloadSizeSelected;
    ((struct espimafs_get_channel_stat_t *) espi_data.buffer)->FlashAccChMaxPayloadSizeSupported = FlashAccChMaxPayloadSizeSupported;
    ((struct espimafs_get_channel_stat_t *) espi_data.buffer)->FlashAccChEraseSizeType = FlashAccChEraseSizeType;
    if  (copy_to_user ((void*) arg, &espi_data, sizeof(struct espimafs_data_t)))
        return -EFAULT;
    
    return 0;
}

static int espi_open(struct inode *inode, struct file *file)
{
    unsigned int minor = iminor(inode);
    struct espimafs_hal *pespimafs_hal;
    struct espimafs_dev *pdev;
    hw_info_t espi_hw_info;
    unsigned char open_count;
    int ret;

    ret = hw_open (EDEV_TYPE_ESPIMAFS, minor,&open_count, &espi_hw_info);
    if (ret)
        return -ENXIO;

    pespimafs_hal = espi_hw_info.pdrv_data;

    pdev = (struct espimafs_dev*)kmalloc(sizeof(struct espimafs_dev), GFP_KERNEL);
    
    if (!pdev)
    {
        hw_close (EDEV_TYPE_ESPIMAFS, minor, &open_count);
        printk (KERN_ERR "%s: failed to allocate espi MAFS private dev structure for espi iminor: %d\n", ESPIMAFS_DEV_NAME, minor);
        return -ENOMEM;
    }

    pdev->pespimafs_hal = pespimafs_hal;
    file->private_data = pdev;

    if (open_count == 1)
    {
        // initialization buffer
    }

    return 0;
}


static int espi_release(struct inode *inode, struct file *file)
{
    int ret;
    unsigned char open_count;
    struct espimafs_dev *pdev = (struct espimafs_dev*)file->private_data;
    
    ret = hw_close (EDEV_TYPE_ESPIMAFS, iminor(inode), &open_count);
    if(ret) { return -1; }

    pdev->pespimafs_hal = NULL;
    file->private_data = NULL;

    kfree (pdev);
    return 0;
}

#ifdef USE_UNLOCKED_IOCTL
static long espi_ioctl(struct file *file,unsigned int cmd, unsigned long arg)
#else
static int espi_ioctl(struct inode *inode, struct file *file,unsigned int cmd, unsigned long arg)
#endif
{
    int ret = 0;
    struct espimafs_dev *pdev = (struct espimafs_dev*) file->private_data;
    //struct espimafs_data_t *xfer = (void __user *)arg;
    
    switch (cmd)
    {
        case GET_FLASH_CHAN_STAT:
            ret = get_flash_channel_status ( pdev, arg );

            break;
        default:
            printk ( "Invalid eSPI MAFS IOCTL\n");
            return -EINVAL;
    }
    return ret;
}


/* ----- Driver registration ---------------------------------------------- */
static struct file_operations espi_ops = {
    owner:          THIS_MODULE,
    read:           NULL,
    write:          NULL,
#ifdef USE_UNLOCKED_IOCTL
    unlocked_ioctl: espi_ioctl,
#else
    ioctl:          espi_ioctl,
#endif
    open:           espi_open,
    release:        espi_release,
};


static espimafs_core_funcs_t espimafs_core_funcs = {
    .get_espimafs_core_data = NULL,
};

static core_hal_t espimafs_core_hal = {
    .owner                      = THIS_MODULE,
    .name                       = "ESPI MAFS CORE",
    .dev_type                   = EDEV_TYPE_ESPIMAFS,
    .register_hal_module        = register_espimafs_hal_module,
    .unregister_hal_module      = unregister_espimafs_hal_module,
    .pcore_funcs                = (void *)&espimafs_core_funcs
};


/*
 * eSPI driver init function
 */
int __init espi_init(void)
{
    int ret =0 ;
  
    /* espi device initialization */ 
    if ((ret = register_chrdev_region (espimafs_devno, ESPIMAFS_MAX_DEVICES, ESPIMAFS_DEV_NAME)) < 0)
    {
        printk (KERN_ERR "failed to register espi MAFS device <%s> (err: %d)\n", ESPIMAFS_DEV_NAME, ret);
        return ret;
    }
    
    espi_cdev = cdev_alloc ();
    if (!espi_cdev)
    {
        unregister_chrdev_region (espimafs_devno, ESPIMAFS_MAX_DEVICES);
        printk (KERN_ERR "%s: failed to allocate espi MAFS cdev structure\n", ESPIMAFS_DEV_NAME);
        return -1;
    }
    
    cdev_init (espi_cdev, &espi_ops);
    
    espi_cdev->owner = THIS_MODULE;
    
    if ((ret = cdev_add (espi_cdev, espimafs_devno, ESPIMAFS_MAX_DEVICES)) < 0)
    {
        cdev_del (espi_cdev);
        unregister_chrdev_region (espimafs_devno, ESPIMAFS_MAX_DEVICES);
        printk    (KERN_ERR "failed to add <%s> char device\n", ESPIMAFS_DEV_NAME);
        ret = -ENODEV;
        return ret;
    }

    sema_init(&espimafs_flash_lock, 1);

    if ((ret = register_core_hal_module (&espimafs_core_hal)) < 0)
    {
        printk(KERN_ERR "failed to register the Core espi MAFS module\n");
        cdev_del (espi_cdev);
        unregister_chrdev_region (espimafs_devno, ESPIMAFS_MAX_DEVICES);
        ret = -EINVAL;
        goto out_no_mem;
    }

    printk("The eSPI MAFS Driver is loaded successfully.\n" );
    return 0;
  
out_no_mem:
    cdev_del (espi_cdev);
    unregister_chrdev_region (espimafs_devno, ESPIMAFS_MAX_DEVICES);    

    return ret;
}

/*!
 * eSPI driver exit function
 */
void __exit espi_exit(void)
{
    unregister_core_hal_module (EDEV_TYPE_ESPIMAFS);
    unregister_chrdev_region (espimafs_devno, ESPIMAFS_MAX_DEVICES);
    
    if (NULL != espi_cdev)
    {
        cdev_del (espi_cdev);
    }

    printk ( "Unregistered the eSPI MAFS driver successfully\n");

    return;    
}

int espimafs_core_loaded =1;
EXPORT_SYMBOL(espimafs_core_loaded);

module_init(espi_init);
module_exit(espi_exit);

MODULE_AUTHOR("American Megatrends Inc.");
MODULE_DESCRIPTION("eSPI MAFS Common Driver");
MODULE_LICENSE ("GPL");

