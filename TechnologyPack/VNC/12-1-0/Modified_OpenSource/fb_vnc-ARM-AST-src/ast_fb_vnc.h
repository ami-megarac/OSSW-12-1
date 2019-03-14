#ifndef __AST_FB_VNC_H__
#define __AST_FB_VNC_H__

#define GET_DINFO(info)		(struct astfb_info *)(info->par)
#define GET_DISP(info, con)	((con) < 0) ? (info)->disp : &fb_display[con]

struct astfb_info {
	/* fb info */
	struct fb_info *info;
	struct device *dev;

	struct fb_var_screeninfo var;
	struct fb_fix_screeninfo fix;
	u32 pseudo_palette[17];
	
	/* driver registered */
	int registered;
	
	/* chip info */
	char name[16];

	/* resource stuff */
	unsigned long frame_buf_phys;
	unsigned long frame_buf_sz;
	void *frame_buf;

	unsigned long ulMMIOPhys;
	unsigned long ulMMIOSize;

	void __iomem *io;
	void __iomem *io_2d;

	/* Options */

	/* command queue */
	unsigned long cmd_q_sz;
	unsigned long cmd_q_offset;
	int use_2d_engine;

	/* mode stuff */
	int xres;
	int yres;
	int xres_virtual;
	int yres_virtual;
	int bpp;
	int pixclock;
	int pitch;
	int refreshrate;
};

#endif /* !__AST_FB_VNC_H__ */
