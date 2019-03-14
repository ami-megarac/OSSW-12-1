/****************************************************************
 ****************************************************************
 **                                                            **
 **    (C)Copyright 2009-2018, American Megatrends Inc.        **
 **                                                            **
 **            All Rights Reserved.                            **
 **                                                            **
 **        5555 Oakbrook Pkwy Suite 200, Norcross              **
 **                                                            **
 **        Georgia - 30093, USA. Phone-(770)-246-8600.         **
 **                                                            **
 ****************************************************************
 ****************************************************************/

/****************************************************************
 *
 * ast_fb_vnc.c
 * ASPEED AST frame buffer driver for VNC server
 *
 ****************************************************************/

#include <linux/version.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/tty.h>
#include <linux/slab.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/selection.h>
#include <linux/bigphysarea.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <linux/vt_kern.h>
#include <linux/capability.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#ifdef CONFIG_MTRR
#include <asm/mtrr.h>
#endif

#include <linux/platform_device.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28)
#include <mach/platform.h>
#include <mach/hardware.h>
#else
#include <asm/arch/platform.h>
#include <asm/arch/hardware.h>
#endif

#include "ast_fb_vnc.h"
#include "ast_fb_vnc_mode.h"
#include "fb_vnc_ioctl.h"

#define AST_FB_VNC_DRV_NAME "astfb_vnc"

struct astfb_info *dinfo;

static int ypan = 0;

struct astvga_par {
	struct platform_device 	*pdev;
	struct fb_info			*info;
	void __iomem *video_base;
	u32 pseudo_palette[17];
	
};

/*
 * Here we define the default structs fb_fix_screeninfo and fb_var_screeninfo
 * if we don't use modedb. If we do use modedb see astvgafb_init how to use it
 * to get a fb_var_screeninfo. Otherwise define a default var as well. 
 */
static struct fb_fix_screeninfo astvgafb_fix = {
	.id =		AST_FB_VNC_DRV_NAME, 
	.type =		FB_TYPE_PACKED_PIXELS,
	.visual =	FB_VISUAL_PSEUDOCOLOR,
	.xpanstep =	1,
	.ypanstep =	1,
	.ywrapstep =	1, 
	.accel =	FB_ACCEL_NONE,
};
/*****************************************************************************/

static int astfb_set_fix(struct fb_info *info)
{
	struct fb_fix_screeninfo *fix;
	struct astfb_info *dinfo = GET_DINFO(info);

	fix = &(info->fix);
	memset(fix, 0, sizeof(struct fb_fix_screeninfo));
	strcpy(fix->id, dinfo->name);
	fix->smem_start = dinfo->frame_buf_phys;
	fix->smem_len = dinfo->frame_buf_sz;
	fix->type = FB_TYPE_PACKED_PIXELS;
	fix->type_aux = 0;
	fix->visual = (dinfo->bpp == 8) ? FB_VISUAL_PSEUDOCOLOR : FB_VISUAL_TRUECOLOR;
	fix->xpanstep = 0;
	fix->ypanstep = ypan ? 1 : 0;
	fix->ywrapstep = 0;
	fix->line_length = dinfo->pitch;
	fix->mmio_start = dinfo->ulMMIOPhys;
	fix->mmio_len = dinfo->ulMMIOSize;
	fix->accel = FB_ACCEL_NONE;

	return 0;
}

static int astfb_blank(int blank, struct fb_info *info)
{
	return 0;
}

static int astfb_pan_display(struct fb_var_screeninfo *var, struct fb_info* info)
{
	uint32_t base;
	u32 xoffset, yoffset;
	
	xoffset = (var->xoffset + 3) & ~3; /* DW alignment */
	yoffset = var->yoffset;

	if ((xoffset + var->xres) > var->xres_virtual) {
		return -EINVAL;
	}

	if ((yoffset + var->yres) > var->yres_virtual) {
		return -EINVAL;
	}
	
	info->var.xoffset = xoffset;
	info->var.yoffset = yoffset;

	base = (var->yoffset * var->xres_virtual) + var->xoffset;

    /* calculate base bpp depth */
	switch(var->bits_per_pixel) {
	case 32:
		break;
	case 16:
		base >>= 1;
		break;
	case 8:
	default:
		base >>= 2;
		break;
	}

	return 0;
}

static int astfb_vnc_ioctl(struct fb_info *info, unsigned int cmd, unsigned long arg)
{
	struct astfb_info *dinfo = GET_DINFO(info);
	struct astfb_dfbinfo dfbinfo;
	
	switch(cmd) {
	case AMIFB_GET_DFBINFO:
		dfbinfo.ulFBSize = dinfo->frame_buf_sz;
		dfbinfo.ulFBPhys = dinfo->frame_buf_phys;
		dfbinfo.ulCMDQSize = dinfo->cmd_q_sz;
		dfbinfo.ulCMDQOffset = dinfo->cmd_q_offset;
		dfbinfo.ul2DMode = dinfo->use_2d_engine;
		if (copy_to_user((void __user *)arg, &dfbinfo, sizeof(struct astfb_dfbinfo)))
			return -EFAULT;
		return 0;
		
	default:
		return -EINVAL;
	}

	return 0;
}

static int astfb_get_cmap_len(struct fb_var_screeninfo *var)
{
	return (var->bits_per_pixel == 8) ? 256 : 16;
}

static int astfb_setcolreg(unsigned regno, unsigned red, unsigned green, unsigned blue, unsigned transp, struct fb_info *info)
{
	if (regno >= astfb_get_cmap_len(&info->var))
		return 1;

	switch(info->var.bits_per_pixel) {
	case 8:
		return 1;
	case 16:
		((u32 *) (info->pseudo_palette))[regno] = (red & 0xf800) | ((green & 0xfc00) >> 5) | ((blue & 0xf800) >> 11);
		break;
	case 32:
		red >>= 8;
		green >>= 8;
		blue >>= 8;
		((u32 *)(info->pseudo_palette))[regno] =
				(red << 16) | (green << 8) | (blue);
		break;
	default:
		return 1;
	}

	return 0;
}

static int astfb_set_par(struct fb_info *info)
{

	astfb_set_fix(info);
	
	return 0;
}

static int astfb_check_var(struct fb_var_screeninfo *var, struct fb_info *info)
{
	return 0;
}

static struct fb_ops astvgafb_ops = {
	.owner          = THIS_MODULE,
	.fb_check_var   = astfb_check_var,
	.fb_set_par     = astfb_set_par,
	.fb_setcolreg   = astfb_setcolreg,
	.fb_blank       = astfb_blank,
	.fb_pan_display = astfb_pan_display,
	.fb_fillrect    = cfb_fillrect,
	.fb_copyarea    = cfb_copyarea,
	.fb_imageblit   = cfb_imageblit,
	.fb_ioctl       = astfb_vnc_ioctl
};

void check_vga_mem_ast2500(unsigned int *pVGA_mem_addr, unsigned int *pVGA_mem_size)
{
	uint32_t reg;
	unsigned int total_index, vga_index;
	unsigned int vga_mem_size;
#if defined(SOC_AST2300) || defined(SOC_AST2400)
	unsigned int vga_mem_address[4][4]={
		{0x43800000, 0x47800000, 0x4F800000, 0x5F800000},
		{0x43000000, 0x47000000, 0x4F000000, 0x5F000000},
		{0x43000000, 0x46000000, 0x4E000000, 0x5E000000},
		{0x00000000, 0x44000000, 0x4C000000, 0x5C000000}};
#elif defined(SOC_AST2500) || defined(SOC_AST2530)
	unsigned int vga_mem_address[4][4]={
		{0x87800000, 0x8F800000, 0x9F800000, 0xBF800000},
		{0x87000000, 0x8F000000, 0x9F000000, 0xBF000000},
		{0x86000000, 0x8E000000, 0x9E000000, 0xBE000000},
		{0x84000000, 0x8C000000, 0x9C000000, 0xBC000000}};
#endif
	
	reg = ioread32((void * __iomem)SDRAM_CONFIG_REG);
	total_index = (reg & 0x03);
	
	reg = ioread32((void * __iomem)SCU_HW_STRAPPING_REG);
	vga_index = ((reg >> 2) & 0x03);
	vga_mem_size = 0x800000 << ((reg >> 2) & 0x03);
	// printk("vga_index: %d,  total_index: %d\n", vga_index, total_index);
	if (pVGA_mem_addr)
	{
		*pVGA_mem_addr = vga_mem_address[vga_index][total_index];
	}
	if (pVGA_mem_size)
	{
		*pVGA_mem_size = vga_mem_size;
	}
	
	printk("vga memory address: %08lx, size: %d \n", (unsigned long)vga_mem_address[vga_index][total_index], vga_mem_size);
	
}

static void cleanup(struct astfb_info *dinfo)
{
	if (!dinfo)
		return;

	if (dinfo->frame_buf != NULL)
		iounmap(dinfo->frame_buf);

	if (dinfo->registered) {
		unregister_framebuffer(dinfo->info);
		framebuffer_release(dinfo->info);
	}

	dev_set_drvdata(dinfo->dev, NULL);
}

static void astfb_vnc_release(struct device *dev)
{

}

static int astvgafb_probe(struct platform_device *pdev)
{
	struct fb_info *info;
	struct astvga_par *par;
	struct device *device = &pdev->dev; /* or &pdev->dev */
	int cmap_len = 256;
	unsigned int VGA_mem_addr;
	unsigned int VGA_mem_size;
	/*
	 * * Dynamically allocate info and par
	 */
	
	info = framebuffer_alloc(sizeof(struct astvga_par), device);

	if (!info) {
		/* goto error path */
		printk(KERN_ERR "Could not allocate memory for astfb_info.\n");
		return -ENODEV;
	}

	check_vga_mem_ast2500(&VGA_mem_addr, &VGA_mem_size);
	
	// init device info.
	dinfo = (struct astfb_info *) info->par;
	memset(dinfo, 0, sizeof(struct astfb_info));
	dinfo->info = info;
	dinfo->dev = device;
	strcpy(dinfo->name, AST_FB_VNC_DRV_NAME);
	dev_set_drvdata(device, (void *) dinfo);
	par = info->par;
	par->video_base = ioremap(VGA_mem_addr, VGA_mem_size);

	/*
	 * Here we set the screen_base to the virtual memory address
	 * for the framebuffer. Usually we obtain the resource address
	 * from the bus layer and then translate it to virtual memory
	 * space via ioremap. Consult ioport.h.
	 */
	dinfo->frame_buf = ioremap(VGA_mem_addr, VGA_mem_size);//framebuffer_virtual_memory; 16MB VGA
	dinfo->frame_buf_phys = VGA_mem_addr;
	dinfo->frame_buf_sz = VGA_mem_size;
	info->screen_base = dinfo->frame_buf;
	info->fbops = &astvgafb_ops;
	info->fix = astvgafb_fix;

	info->fix.smem_start = VGA_mem_addr;
	info->fix.smem_len = VGA_mem_size;
	
	info->pseudo_palette = par->pseudo_palette;
	
	if (fb_alloc_cmap(&info->cmap, cmap_len, 0) < 0) {
		printk(KERN_ERR "Could not allocate cmap for astfb_info.\n");
		framebuffer_release(info);
		return -ENODEV;
	}

	/*
	 * Set up flags to indicate what sort of acceleration your
	 * can provide (pan/wrap/copyarea/etc.) and whether it
	 * is a module -- see FBINFO_* in include/linux/fb.h
	 * 
	 * If your hardware can support any of the hardware accelerated functions
	 * fbcon performance will improve if info->flags is set properly.
	 * 
	 * FBINFO_HWACCEL_COPYAREA - hardware moves
	 * FBINFO_HWACCEL_FILLRECT - hardware fills
	 * FBINFO_HWACCEL_IMAGEBLIT - hardware mono->color expansion
	 * FBINFO_HWACCEL_YPAN - hardware can pan display in y-axis
	 * FBINFO_HWACCEL_YWRAP - hardware can wrap display in y-axis
	 * FBINFO_HWACCEL_DISABLED - supports hardware accels, but disabled
	 * FBINFO_READS_FAST - if set, prefer moves over mono->color expansion
	 * FBINFO_MISC_TILEBLITTING - hardware can do tile blits
	 * 
	 * NOTE: These are for fbcon use only.
	 */
	info->flags = FBINFO_DEFAULT;

	device->release = astfb_vnc_release;

	if (register_framebuffer(info) < 0) {
		fb_dealloc_cmap(&info->cmap);
		cleanup(dinfo);
		framebuffer_release(info);
		return -EINVAL;
	}
	dinfo->registered = 1;
	
	// printk("fb%d: %s frame buffer device\n", info->node, info->fix.id);
	fb_info(info, "%s frame buffer device\n", info->fix.id);
	printk(KERN_INFO "FB: got physical memory pool for size (%d on %08lx bus)\n", info->fix.smem_len, (unsigned long)info->fix.smem_start);
	platform_set_drvdata(pdev, info); /* or platform_set_drvdata(pdev, info) */
    return 0;
}

static int astvgafb_remove(struct platform_device *pdev)
{
	struct fb_info *info = platform_get_drvdata(pdev);
	/* or platform_get_drvdata(pdev); */
	// printk("astvgafb_remove \n");
	if (info) {
		unregister_framebuffer(info);
		fb_dealloc_cmap(&info->cmap);
		/* ... */
		framebuffer_release(info);
	}
	return 0;
}

#ifdef CONFIG_PM
/**
 *	astvgafb_resume - Optional but recommended function. Resume the device.
 *	@dev: platform device
 *
 *      See Documentation/power/devices.txt for more information
 */
static int astvgafb_resume(struct platform_dev *dev)
{
	struct fb_info *info = platform_get_drvdata(dev);
	struct astvgafb_par *par = info->par;

	/* resume here */
	return 0;
}
#else
#define astvgafb_suspend NULL
#define astvgafb_resume NULL
#endif /* CONFIG_PM */

static struct platform_driver astvgafb_driver = {
	.probe = astvgafb_probe,
	.remove = astvgafb_remove,
	.suspend = astvgafb_suspend, /* optional but recommended */
	.resume = astvgafb_resume,   /* optional but recommended */
	.driver = {
		.name = AST_FB_VNC_DRV_NAME,
	},
};

static struct platform_device astfb_device = {
	.name = AST_FB_VNC_DRV_NAME,
};

int __init astfb_init(void)
{
	int ret;

	ret = platform_driver_register(&astvgafb_driver);

	if (!ret) {
		ret = platform_device_register(&astfb_device);
		if (ret)
			platform_driver_unregister(&astvgafb_driver);
	}

	return ret;
}

static void __exit astfb_exit(void)
{
	cleanup(dinfo);
	platform_device_unregister(&astfb_device);
	platform_driver_unregister(&astvgafb_driver);
}

module_init(astfb_init);
module_exit(astfb_exit);

MODULE_AUTHOR("American Megatrends Inc.");
MODULE_DESCRIPTION("AST frame buffer driver for VNC");
MODULE_LICENSE("GPL");
