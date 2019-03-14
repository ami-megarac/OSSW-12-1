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
 * File name: espioobmain.c
 * This driver provides OOB common layer, independent of the hardware, for the eSPI OOB driver.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/poll.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/vmalloc.h>
#include <asm/io.h>
#include "helper.h"
#include "driver_hal.h"
#include "dbgout.h"
#include "espioob_ioctl.h"
#include "espioob.h"

#ifdef HAVE_UNLOCKED_IOCTL
  #if HAVE_UNLOCKED_IOCTL
  #define USE_UNLOCKED_IOCTL
  #endif
#endif

//#define ESPI_DEBUG          1
#define ESPIOOB_MAJOR           177
#define ESPIOOB_MINOR           0
#define ESPIOOB_MAX_DEVICES     255
#define ESPIOOB_DEV_NAME        "espioob"


static int get_oob_channel_status (struct espioob_dev *pdev, unsigned long arg);
static int oob_read (struct espioob_dev *pdev, unsigned long arg);
static int oob_write (struct espioob_dev *pdev, unsigned long arg);
static int oob_writeread (struct espioob_dev *pdev, unsigned long arg);

static struct cdev *espi_cdev;
static dev_t espioob_devno = MKDEV(ESPIOOB_MAJOR, ESPIOOB_MINOR);


int register_espioob_hal_module (unsigned char num_instances, void *phal_ops, void **phw_data)
{
    struct espioob_hal *pespioob_hal;

    pespioob_hal = (struct espioob_hal*) kmalloc (sizeof(struct espioob_hal), GFP_KERNEL);
    if (!pespioob_hal)
    {
        return -ENOMEM;
    }

    pespioob_hal->pespioob_hal_ops = ( espioob_hal_operations_t *) phal_ops;
    *phw_data = (void *) pespioob_hal;    

    return 0;    
}


int unregister_espioob_hal_module (void *phw_data)
{
    struct espioob_hal *pespioob_hal = (struct espioob_hal*) phw_data;

    kfree (pespioob_hal);

    return 0;
}

int get_oob_channel_status (struct espioob_dev *pdev, unsigned long arg)
{
    struct espioob_data_t espi_data;
    unsigned int num_channel;
    unsigned int gen_status;
    unsigned int ch_status;
    unsigned int OOBChMaxPayloadSizeSelected;
    unsigned int OOBChMaxPayloadSizeSupported;

    if  (copy_from_user (&espi_data, (void*) arg, sizeof(struct espioob_data_t)))
        return -EFAULT;
    
    num_channel = pdev->pespioob_hal->pespioob_hal_ops->num_espioob_ch();
    pdev->pespioob_hal->pespioob_hal_ops->get_channel_status(&gen_status, &ch_status, &OOBChMaxPayloadSizeSelected, &OOBChMaxPayloadSizeSupported);

#ifdef ESPI_DEBUG
    printk ("Number of OOB channel support %d \n", num_channel);

    if (gen_status & OOB_CHANNEL_SUPPORTED)
        printk("OOB Channel Supported\n");
    
    if (ch_status & OOB_CHANNEL_SUPPORTED)
        printk("Host OOB Channel Supported\n");
#endif

    espi_data.buf_len = sizeof(struct espioob_get_channel_stat_t);
    ((struct espioob_get_channel_stat_t *) espi_data.buffer)->num_channel = num_channel;
    ((struct espioob_get_channel_stat_t *) espi_data.buffer)->gen_status = gen_status;
    ((struct espioob_get_channel_stat_t *) espi_data.buffer)->ch_status = ch_status;
    ((struct espioob_get_channel_stat_t *) espi_data.buffer)->OOBChMaxPayloadSizeSelected = OOBChMaxPayloadSizeSelected;
    ((struct espioob_get_channel_stat_t *) espi_data.buffer)->OOBChMaxPayloadSizeSupported = OOBChMaxPayloadSizeSupported;
    if  (copy_to_user ((void*) arg, &espi_data, sizeof(struct espioob_data_t)))
        return -EFAULT;
    
    return 0;
}

int oob_read (struct espioob_dev *pdev, unsigned long arg)
{
    struct espioob_data_t espi_data;

    if  (copy_from_user (&espi_data, (void*) arg, sizeof(struct espioob_data_t)))
        return -EFAULT;

    pdev->pespioob_hal->pespioob_hal_ops->oob_read (&espi_data);

    if  (copy_to_user ((void*) arg, &espi_data, sizeof(struct espioob_data_t)))
        return -EFAULT;

    return 0;
}

int oob_write (struct espioob_dev *pdev, unsigned long arg)
{
    struct espioob_data_t espi_data;

    if  (copy_from_user (&espi_data, (void*) arg, sizeof(struct espioob_data_t)))
        return -EFAULT;

    pdev->pespioob_hal->pespioob_hal_ops->oob_write (&espi_data);
    return 0;
}

int oob_writeread (struct espioob_dev *pdev, unsigned long arg)
{
    struct espioob_data_t espi_data;

    if  (copy_from_user (&espi_data, (void*) arg, sizeof(struct espioob_data_t)))
        return -EFAULT;

    pdev->pespioob_hal->pespioob_hal_ops->oob_writeread (&espi_data);

    if  (copy_to_user ((void*) arg, &espi_data, sizeof(struct espioob_data_t)))
        return -EFAULT;

    return 0;
}

static int espi_open(struct inode *inode, struct file *file)
{
    unsigned int minor = iminor(inode);
    struct espioob_hal *pespioob_hal;
    struct espioob_dev *pdev;
    hw_info_t espi_hw_info;
    unsigned char open_count;
    int ret;

    ret = hw_open (EDEV_TYPE_ESPIOOB, minor,&open_count, &espi_hw_info);
    if (ret)
        return -ENXIO;

    pespioob_hal = espi_hw_info.pdrv_data;

    pdev = (struct espioob_dev*)kmalloc(sizeof(struct espioob_dev), GFP_KERNEL);
    
    if (!pdev)
    {
        hw_close (EDEV_TYPE_ESPIOOB, minor, &open_count);
        printk (KERN_ERR "%s: failed to allocate espi OOB private dev structure for espi iminor: %d\n", ESPIOOB_DEV_NAME, minor);
        return -ENOMEM;
    }

    pdev->pespioob_hal = pespioob_hal;
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
    struct espioob_dev *pdev = (struct espioob_dev*)file->private_data;
    
    ret = hw_close (EDEV_TYPE_ESPIOOB, iminor(inode), &open_count);
    if(ret) { return -1; }

    pdev->pespioob_hal = NULL;
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
    struct espioob_dev *pdev = (struct espioob_dev*) file->private_data;
    
    switch (cmd)
    {
        case GET_OOB_CHAN_STAT:
            ret = get_oob_channel_status ( pdev, arg );
            break;

        case OOB_READ:
            ret = oob_read ( pdev, arg );
            break;

        case OOB_WRITE:
            ret = oob_write ( pdev, arg );
            break;

        case OOB_WRITEREAD:
            ret = oob_writeread ( pdev, arg );
            break;

        default:
            printk ( "Invalid eSPI OOB IOCTL\n");
            return -EINVAL;
    }
    return ret;
}


/* ----- Driver registration ---------------------------------------------- */
static struct file_operations espi_ops = {
    owner:        THIS_MODULE,
    read:        NULL,
    write:        NULL,
#ifdef USE_UNLOCKED_IOCTL
    unlocked_ioctl: espi_ioctl,
#else
    ioctl:        espi_ioctl,
#endif
    open:        espi_open,
    release:    espi_release,
};


static espioob_core_funcs_t espioob_core_funcs = {
    .get_espioob_core_data = NULL,
};

static core_hal_t espioob_core_hal = {
    .owner                  = THIS_MODULE,
    .name                   = "ESPI OOB CORE",
    .dev_type               = EDEV_TYPE_ESPIOOB,
    .register_hal_module    = register_espioob_hal_module,
    .unregister_hal_module  = unregister_espioob_hal_module,
    .pcore_funcs            = (void *)&espioob_core_funcs
};


/*
 * eSPI driver init function
 */
int __init espi_init(void)
{
    int ret =0 ;
  
    /* espi device initialization */ 
    if ((ret = register_chrdev_region (espioob_devno, ESPIOOB_MAX_DEVICES, ESPIOOB_DEV_NAME)) < 0)
    {
        printk (KERN_ERR "failed to register espi OOB device <%s> (err: %d)\n", ESPIOOB_DEV_NAME, ret);
        return ret;
    }
    
    espi_cdev = cdev_alloc ();
    if (!espi_cdev)
    {
        unregister_chrdev_region (espioob_devno, ESPIOOB_MAX_DEVICES);
        printk (KERN_ERR "%s: failed to allocate espi OOB cdev structure\n", ESPIOOB_DEV_NAME);
        return -1;
    }
    
    cdev_init (espi_cdev, &espi_ops);
    
    espi_cdev->owner = THIS_MODULE;

    if ((ret = cdev_add (espi_cdev, espioob_devno, ESPIOOB_MAX_DEVICES)) < 0)
    {
        cdev_del (espi_cdev);
        unregister_chrdev_region (espioob_devno, ESPIOOB_MAX_DEVICES);
        printk    (KERN_ERR "failed to add <%s> char device\n", ESPIOOB_DEV_NAME);
        ret = -ENODEV;
        return ret;
    }

    if ((ret = register_core_hal_module (&espioob_core_hal)) < 0)
    {
        printk(KERN_ERR "failed to register the Core espi OOB module\n");
        cdev_del (espi_cdev);
        unregister_chrdev_region (espioob_devno, ESPIOOB_MAX_DEVICES);
        ret = -EINVAL;
        goto out_no_mem;
    }

    printk("The eSPI OOB Driver is loaded successfully.\n" );
    return 0;
  
out_no_mem:
    cdev_del (espi_cdev);
    unregister_chrdev_region (espioob_devno, ESPIOOB_MAX_DEVICES);    

    return ret;
}

/*!
 * eSPI driver exit function
 */
void __exit espi_exit(void)
{
    unregister_core_hal_module (EDEV_TYPE_ESPIOOB);
    unregister_chrdev_region (espioob_devno, ESPIOOB_MAX_DEVICES);
    
    if (NULL != espi_cdev)
    {
        cdev_del (espi_cdev);
    }

    printk ( "Unregistered the eSPI OOB driver successfully\n");

    return;
}

int espioob_core_loaded =1;
EXPORT_SYMBOL(espioob_core_loaded);

module_init(espi_init);
module_exit(espi_exit);

MODULE_AUTHOR("American Megatrends Inc.");
MODULE_DESCRIPTION("eSPI OOB Common Driver");
MODULE_LICENSE ("GPL");

