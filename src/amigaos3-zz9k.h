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

// A pointer to the permanent GFXData struct in memory on the ZZ9000 board,
// used for sharing data between the Amiga side and the RTG board.
#define Z3_GFXDATA_ADDR 0x3200000
#define Z3_SCRATCH_ADDR 0x3210000
#define SURFACE_OFFSET(a) ((unsigned int)a.getPixels() & 0x0FFFFFFF)

// ZZ9000 register offsets, will always* be backward compatible.
#define REG_ZZ_DMA_OP 0x5A
#define REG_ZZ_ACC_OP 0x5C

#define ZZWRITE16(a, b) *(volatile short*)((unsigned int)zz9k_addr+a) = (volatile short)b;
#define ZZCHKADDR(a, b) (unsigned int)a > zz9k_addr && (unsigned int)b > zz9k_addr

#define MNTVA_COLOR_8BIT     0
#define MNTVA_COLOR_16BIT565 1
#define MNTVA_COLOR_32BIT    2
#define MNTVA_COLOR_15BIT    3

#define uint32 unsigned int
#define uint16 unsigned short
#define uint8 unsigned char

unsigned int find_zz9k();
unsigned int zz9k_get_surface_offset(int idx);

void zz9k_clearbuf(unsigned int addr, unsigned int col, unsigned short w, unsigned short h, unsigned char color_format);
void zz9k_flip_surface(unsigned int src, unsigned int dest, unsigned short w, unsigned short h, unsigned int bpp);
void zz9k_set_clut_mouse_cursor(short hot_x, short hot_y, unsigned short w, unsigned short h, const void *bmp, unsigned int key_color);
void zz9k_blit_rect(unsigned int src, unsigned int dest, int x, int y, int src_pitch, int dest_pitch, int w, int h, unsigned char src_bpp, unsigned char dest_bpp, unsigned char reverse);
void zz9k_blit_rect_mask(unsigned int src, unsigned int dest, int x, int y, int src_pitch, int dest_pitch, int w, int h, unsigned char mask_color, unsigned char src_bpp, unsigned char dest_bpp);
void zz9k_set_16_to_8_colormap(void *src);

void zz9k_drawline(unsigned int dest, int dest_pitch, int x, int y, int x2, int y2, unsigned int color, unsigned char bpp, int pen_width, int pen_height);
void zz9k_fill_rect(unsigned int dest, int dest_pitch, int x, int y, int w, int h, unsigned int color, unsigned char bpp);
void zz9k_draw_circle(unsigned int dest, int dest_pitch, int x, int y, int w, int h, int r, unsigned int color, unsigned char bpp);
void zz9k_fill_circle(unsigned int dest, int dest_pitch, int x, int y, int w, int h, int r, unsigned int color, unsigned char bpp);
void zz9k_draw_flat_tri(unsigned int dest, int dest_pitch, int w, int h, void *pts_, unsigned int color, unsigned char bpp);

unsigned int zz9k_alloc_surface(unsigned short w, unsigned short h, unsigned char bpp);
unsigned int zz9k_alloc_mem(unsigned int size);
void zz9k_free_surface(unsigned int p_);

void zz9k_debugme(unsigned int off1, unsigned int off2, const char *txt);

enum gfx_dma_op {
  OP_NONE,
  OP_DRAWLINE,
  OP_FILLRECT,
  OP_COPYRECT,
  OP_COPYRECT_NOMASK,
  OP_RECT_TEMPLATE,
  OP_RECT_PATTERN,
  OP_P2C,
  OP_P2D,
  OP_INVERTRECT,
  OP_PAN,
  OP_SPRITE_XY,
  OP_SPRITE_COLOR,
  OP_SPRITE_BITMAP,
  OP_SPRITE_CLUT_BITMAP,
  OP_ETH_USB_OFFSETS,
  OP_NUM,
};

enum gfx_acc_op {
  ACC_OP_NONE,
  ACC_OP_BUFFER_FLIP,
  ACC_OP_BUFFER_CLEAR,
  ACC_OP_BLIT_RECT,
  ACC_OP_ALLOC_SURFACE,
  ACC_OP_FREE_SURFACE,
  ACC_OP_SET_BPP_CONVERSION_TABLE,
  ACC_OP_DRAW_LINE,
  ACC_OP_FILL_RECT,
  ACC_OP_DRAW_CIRCLE,
  ACC_OP_FILL_CIRCLE,
  ACC_OP_DRAW_FLAT_TRI,
  ACC_OP_DRAW_TEX_TRI,
  ACC_OP_NUM,
};

enum gfxdata_offsets {
  GFXDATA_DST,
  GFXDATA_SRC,
};

enum gfxdata_u8_types {
  GFXDATA_U8_COLORMODE,
  GFXDATA_U8_DRAWMODE,
  GFXDATA_U8_LINE_PATTERN_OFFSET,
  GFXDATA_U8_LINE_PADDING,
};

// GFXData struct has a maximum size of 64KB
#pragma pack(4)
struct zz9k_GFXData {
  uint32 offset[2];
  uint32 rgb[2];
  uint16 x[4], y[4];
  uint16 user[4];
  uint16 pitch[4];
  uint8 u8_user[8];
  uint8 op, mask, minterm, u8offset;
  uint32 u32_user[8];
  uint8 clut1[768];
  uint8 clut2[768];
  uint8 clut3[768];
  uint8 clut4[768];
};

enum ZZ9K_SURFACE_OFFSETS {
    ZZ9K_OFFSET_GAME_SCREEN,
    ZZ9K_OFFSET_TMP_SCREEN,
    ZZ9K_OFFSET_OVERLAY,
    ZZ9K_OFFSET_OVERLAY16,
    ZZ9K_OFFSET_NUM,
};
