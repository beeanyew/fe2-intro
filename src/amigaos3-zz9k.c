/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "amigaos3-zz9k.h"
#include <stdio.h>
#include <proto/expansion.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <string.h>

#define ZZWRITE16(a, b) *(volatile short*)((unsigned int)zz9k_addr+a) = (volatile short)b;

// These are offsets in the "scratch area" in the memory on the ZZ9000 board.
// They are located after the main framebuffer/video RAM and are guaranteed to not be in use.
unsigned int zz9k_offsets[ZZ9K_OFFSET_NUM] = {
    0x3300000,
    0x3400000,
    0x3500000,
    0x3600000,
};

unsigned int zz9k_addr = 0;
static unsigned int zz9k_gfxdata = Z3_GFXDATA_ADDR;
unsigned char zz9k_palette[768];
struct zz9k_GFXData *gxd = NULL;

unsigned int find_zz9k() {
	struct Library *expansionBase = NULL;

	if ((expansionBase = OpenLibrary("expansion.library",0L)) == NULL) {
	    //printf("Failed to open expansion.library, could not get ZZ9000 board address.\n");
  	}
	else {
		struct ConfigDev* cd = NULL;
		cd = (struct ConfigDev*)FindConfigDev(cd,0x6d6e,0x4);
		if (cd != NULL) {
			zz9k_addr = (unsigned int)cd->cd_BoardAddr;
            zz9k_gfxdata += zz9k_addr;
			//printf("Found ZZ9000 board at address $%X\n", (unsigned int)cd->cd_BoardAddr);
            gxd = (struct zz9k_GFXData *)zz9k_gfxdata;
		}
		/*else {
			printf("No ZZ9000 board found.\n");
		}*/
        CloseLibrary(expansionBase);
	}

    return zz9k_addr;
}

void zz9k_clearbuf(unsigned int addr, unsigned int color, unsigned short w, unsigned short h, unsigned char color_format) {
    gxd->offset[0] = (addr & 0x0FFFFFFF);
    gxd->x[0] = w; gxd->y[0] = h;
    gxd->pitch[0] = w;
    gxd->rgb[0] = color;
    gxd->u8_user[GFXDATA_U8_COLORMODE] = color_format;

    //printf("zz9k clearbuf to %p: @%.8X %dx%d - Size: %d\n", gfxdata, addr, w, h, sizeof(struct zz9k_GFXData));
    ZZWRITE16(REG_ZZ_ACC_OP, ACC_OP_BUFFER_CLEAR);
}

void zz9k_flip_surface(unsigned int src, unsigned int dest, unsigned short w, unsigned short h, unsigned int bpp) {
    gxd->offset[0] = (src & 0x0FFFFFFF);
    gxd->offset[1] = (dest & 0x0FFFFFFF);
    gxd->x[0] = w; gxd->y[0] = h;
    gxd->pitch[0] = w;
    gxd->u8_user[GFXDATA_U8_COLORMODE] = bpp;

    ZZWRITE16(REG_ZZ_ACC_OP, ACC_OP_BUFFER_FLIP);
}

void zz9k_set_clut_mouse_cursor(short hot_x, short hot_y, unsigned short w, unsigned short h, const void *bmp, unsigned int key_color) {
    gxd->offset[1] = Z3_GFXDATA_ADDR + 0x10000;
    memcpy((uint8_t *)(uint32_t)(zz9k_gfxdata + 0x10000), bmp, w * h);
    memcpy(gxd->clut1, zz9k_palette, 768);
    gxd->x[0] = -(hot_x+1); gxd->y[0] = -hot_y;
    gxd->x[1] = w; gxd->y[1] = h;
    gxd->u8offset = (uint8_t)key_color;

    ZZWRITE16(REG_ZZ_DMA_OP, OP_SPRITE_CLUT_BITMAP);
}

void zz9k_blit_rect(unsigned int src, unsigned int dest, int x, int y, int src_pitch, int dest_pitch, int w, int h, unsigned char src_bpp, unsigned char dest_bpp, unsigned char reverse) {
    gxd->offset[0] = (src & 0x0FFFFFFF);
    gxd->offset[1] = (dest & 0x0FFFFFFF);
    gxd->pitch[0] = (unsigned short)src_pitch;
    gxd->pitch[1] = (unsigned short)dest_pitch;
    gxd->x[0] = (unsigned short)x; gxd->y[0] = (unsigned short)y;
    gxd->x[1] = (unsigned short)w; gxd->y[1] = (unsigned short)h;
    gxd->u8_user[0] = src_bpp;
    gxd->u8_user[1] = dest_bpp;
    gxd->u8_user[2] = reverse;

    ZZWRITE16(REG_ZZ_ACC_OP, ACC_OP_BLIT_RECT);
}

void zz9k_blit_rect_mask(unsigned int src, unsigned int dest, int x, int y, int src_pitch, int dest_pitch, int w, int h, unsigned char mask_color, unsigned char src_bpp, unsigned char dest_bpp) {
    gxd->offset[0] = (src & 0x0FFFFFFF);
    gxd->offset[1] = (dest & 0x0FFFFFFF);
    gxd->pitch[0] = (unsigned short)src_pitch;
    gxd->pitch[1] = (unsigned short)dest_pitch;
    gxd->x[0] = (unsigned short)x; gxd->y[0] = (unsigned short)y;
    gxd->x[1] = (unsigned short)w; gxd->y[1] = (unsigned short)h;
    gxd->u8_user[0] = src_bpp;
    gxd->u8_user[1] = dest_bpp;
    gxd->u8_user[2] = 2;
    gxd->u8offset = mask_color;

    ZZWRITE16(REG_ZZ_ACC_OP, ACC_OP_BLIT_RECT);
}

void zz9k_drawline(unsigned int dest, int dest_pitch, int x, int y, int x2, int y2, unsigned int color, unsigned char bpp, int pen_width, int pen_height) {
    gxd->offset[0] = (dest & 0x0FFFFFFF);
    gxd->pitch[0] = (unsigned short)dest_pitch;
    gxd->x[0] = (unsigned short)x; gxd->y[0] = (unsigned short)y;
    gxd->x[1] = (unsigned short)x2; gxd->y[1] = (unsigned short)y2;
    gxd->rgb[0] = color;
    gxd->u8_user[0] = bpp;
    gxd->u8_user[1] = (char)pen_width;
    gxd->u8_user[2] = (char)pen_height;

    ZZWRITE16(REG_ZZ_ACC_OP, ACC_OP_DRAW_LINE);
}

void zz9k_fill_rect(unsigned int dest, int dest_pitch, int x, int y, int w, int h, unsigned int color, unsigned char bpp) {
    gxd->offset[0] = (dest & 0x0FFFFFFF);
    gxd->pitch[0] = (unsigned short)dest_pitch;
    gxd->x[0] = (unsigned short)x; gxd->y[0] = (unsigned short)y;
    gxd->x[1] = (unsigned short)w; gxd->y[1] = (unsigned short)h;
    gxd->rgb[0] = color;
    gxd->u8_user[0] = bpp;

    ZZWRITE16(REG_ZZ_ACC_OP, ACC_OP_FILL_RECT);
}

void zz9k_draw_circle(unsigned int dest, int dest_pitch, int x, int y, int w, int h, int r, unsigned int color, unsigned char bpp) {
    gxd->offset[0] = (dest & 0x0FFFFFFF);
    gxd->pitch[0] = (unsigned short)dest_pitch;
    gxd->x[0] = (unsigned short)x; gxd->y[0] = (unsigned short)y;
    gxd->x[1] = (unsigned short)w; gxd->y[1] = (unsigned short)h;
    gxd->x[2] = (unsigned short)r;
    gxd->rgb[0] = color;
    gxd->u8_user[0] = bpp;

    ZZWRITE16(REG_ZZ_ACC_OP, ACC_OP_DRAW_CIRCLE);
}

void zz9k_fill_circle(unsigned int dest, int dest_pitch, int x, int y, int w, int h, int r, unsigned int color, unsigned char bpp) {
    gxd->offset[0] = (dest & 0x0FFFFFFF);
    gxd->pitch[0] = (unsigned short)dest_pitch;
    gxd->x[0] = (unsigned short)x; gxd->y[0] = (unsigned short)y;
    gxd->x[1] = (unsigned short)w; gxd->y[1] = (unsigned short)h;
    gxd->x[2] = (unsigned short)r;
    gxd->rgb[0] = color;
    gxd->u8_user[0] = bpp;

    ZZWRITE16(REG_ZZ_ACC_OP, ACC_OP_FILL_CIRCLE);
}

typedef struct {
    short x;
    short y;
} vec2;

void zz9k_draw_flat_tri(unsigned int dest, int dest_pitch, int w, int h, void *pts_, unsigned int color, unsigned char bpp) {
    int pts_idx = 0;
    vec2 *pts = (vec2 *)pts_;
    unsigned int *pts_ptr = (unsigned int *)gxd->clut4;

    gxd->offset[0] = (dest & 0x0FFFFFFF);
    gxd->pitch[0] = (unsigned short)dest_pitch;
    gxd->x[0] = (unsigned short)w; gxd->y[0] = (unsigned short)h;
    gxd->rgb[0] = color;
    gxd->u8_user[0] = bpp;

    for (int i = 0; i < 3; i++) {
        pts_ptr[pts_idx++] = pts[i].x;
        pts_ptr[pts_idx++] = pts[i].y;
    }

    ZZWRITE16(REG_ZZ_ACC_OP, ACC_OP_DRAW_FLAT_TRI);
}

unsigned int zz9k_alloc_surface(unsigned short w, unsigned short h, unsigned char bpp) {
    gxd->x[0] = w;
    gxd->y[0] = h;
    gxd->u8_user[0] = bpp;
    gxd->u8_user[1] = 0;
    
    ZZWRITE16(REG_ZZ_ACC_OP, ACC_OP_ALLOC_SURFACE);
    
    unsigned int p = gxd->offset[0];
    return p + zz9k_addr;
}

unsigned int zz9k_alloc_mem(unsigned int size) {
    gxd->offset[1] = size;
    gxd->u8_user[1] = 1;
    
    ZZWRITE16(REG_ZZ_ACC_OP, ACC_OP_ALLOC_SURFACE);
    
    unsigned int p = gxd->offset[0];
    return p + zz9k_addr;
}

void zz9k_free_surface(unsigned int p_) {
    if (p_ == 0)
        return;

    unsigned int p = p_ - zz9k_addr;

    gxd->offset[0] = p;
    gxd->u8_user[0] = 0;

    ZZWRITE16(REG_ZZ_ACC_OP, ACC_OP_FREE_SURFACE);
}

unsigned int zz9k_get_surface_offset(int idx) {
    return zz9k_offsets[idx];
}

void zz9k_set_16_to_8_colormap(void *src) {
    gxd->offset[0] = Z3_SCRATCH_ADDR;
    memcpy((void *)((uint32_t)zz9k_addr + Z3_SCRATCH_ADDR), src, 65536);
    ZZWRITE16(REG_ZZ_ACC_OP, ACC_OP_SET_BPP_CONVERSION_TABLE);
}

void zz9k_debugme(unsigned int off1, unsigned int off2, const char *txt) {
	gxd->offset[0] = off1;
	gxd->offset[1] = off2;
    if (txt) {
        sprintf((char *)gxd->clut2, txt);
    } else {
        sprintf((char *)gxd->clut2, "Debug");
    }
    ZZWRITE16(REG_ZZ_ACC_OP, ACC_OP_NONE);
}
