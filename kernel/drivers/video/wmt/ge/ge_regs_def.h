#ifndef GE_REGS_DEF_H
#define GE_REGS_DEF_H

struct ge_regs_8710 {
	unsigned int ge_command;	/* 0x0000 GE Command */
	unsigned int color_depth;	/* 0x0004 GE Color Depth */
	unsigned int hm_sel;		/* 0x0008 GE High Color Mode Select */
	unsigned int pat_tran_en;	/* 0x000c GE Pattern Transp. Enable */

	unsigned int font_tran_en;	/* 0x0010 GE Font Transparency Enable */
	unsigned int rop_code;		/* 0x0014 GE ROP Code */
	unsigned int ge_fire;		/* 0x0018 GE Fire Code */
	unsigned int rop_bg_code;	/* 0x001c */

	unsigned int src_baddr;		/* 0x0020 GE Source Base Address */
	unsigned int src_disp_w;	/* 0x0024 GE Source Display width */
	unsigned int src_disp_h;	/* 0x0028 GE Source DIsplay height */
	unsigned int src_x_start;	/* 0x002c GE Source X start point */

	unsigned int src_y_start;	/* 0x0030 GE Source Y start point */
	unsigned int src_width;		/* 0x0034 GE Source Width */
	unsigned int src_height;	/* 0x0038 GE Source Height */
	unsigned int des_baddr;		/* 0x003c GE Dest Base Address */

	unsigned int des_disp_w;	/* 0x0040 GE Dest Display width */
	unsigned int des_disp_h;	/* 0x0044 GE Dest DIsplay height */
	unsigned int des_x_start;	/* 0x0048 GE Dest X start point */
	unsigned int des_y_start;	/* 0x004c GE Dest Y start point */

	unsigned int des_width;		/* 0x0050 GE Dest Width */
	unsigned int des_height;	/* 0x0054 GE Dest Height */
	unsigned int font0_buf;		/* 0x0058 GE FONT Color 0 Buffer */
	unsigned int font1_buf;		/* 0x005c GE FONT Color 1 Buffer */

	unsigned int font2_buf;		/* 0x0060 GE FONT Color 2 Buffer*/
	unsigned int font3_buf;		/* 0x0064 GE FONT Color 3 Buffer*/
	unsigned int pat0_buf;		/* 0x0068 GE Pattern 0 Buffer */
	unsigned int pat1_buf;		/* 0x006c GE Pattern 1 Buffer */

	unsigned int pat2_buf;		/* 0x0070 GE Pattern 2 Buffer */
	unsigned int pat3_buf;		/* 0x0074 GE Pattern 3 Buffer */
	unsigned int pat4_buf;		/* 0x0078 GE Pattern 4 Buffer */
	unsigned int pat5_buf;		/* 0x007c GE Pattern 5 Buffer */

	unsigned int pat6_buf;		/* 0x0080 GE Pattern 6 Buffer */
	unsigned int pat7_buf;		/* 0x0084 GE Pattern 7 Buffer */
	unsigned int pat0_color;	/* 0x0088 GE Pattern 0 Color */
	unsigned int pat1_color;	/* 0x008c GE Pattern 1 Color */

	unsigned int pat2_color;	/* 0x0090 GE Pattern 2 Color */
	unsigned int pat3_color;	/* 0x0094 GE Pattern 3 Color */
	unsigned int pat4_color;	/* 0x0098 GE Pattern 4 Color */
	unsigned int pat5_color;	/* 0x009c GE Pattern 5 Color */

	unsigned int pat6_color;	/* 0x00a0 GE Pattern 6 Color */
	unsigned int pat7_color;	/* 0x00a4 GE Pattern 7 Color */
	unsigned int pat8_color;	/* 0x00a8 GE Pattern 8 Color */
	unsigned int pat9_color;	/* 0x00ac GE Pattern 9 Color */

	unsigned int pat10_color;	/* 0x00b0 GE Pattern 10 Color */
	unsigned int pat11_color;	/* 0x00b4 GE Pattern 11 Color */
	unsigned int pat12_color;	/* 0x00b8 GE Pattern 12 Color */
	unsigned int pat13_color;	/* 0x00bc GE Pattern 13 Color */

	unsigned int pat14_color;	/* 0x00c0 GE Pattern 14 Color */
	unsigned int pat15_color;	/* 0x00c4 GE Pattern 15 Color */
	unsigned int ck_sel;		/* 0x00c8 Color Key Select */
	unsigned int src_ck;		/* 0x00cc GE Source Colorkey */

	unsigned int des_ck;		/* 0x00d0 GE Destination Colorkey */
	unsigned int alpha_sel;		/* 0x00d4 GE Alpha Select */
	unsigned int bitblt_alpha;	/* 0x00d8 */
	unsigned int des_path_en;	/* 0x00dc DES Read Path Enable */

	unsigned int rotate_mode;	/* 0x00e0 GE Rotate Mode */
	unsigned int mirror_mode;	/* 0x00e4 GE Mirror Mode */
	unsigned int ge_delay;		/* 0x00e8 GE Start Cycle Delay */
	unsigned int ge_eng_en;		/* 0x00ec GE Engine Enable */

	unsigned int ge_int_en;		/* 0x00f0 GE Intr. Enable signal */
	unsigned int ge_int_flag;	/* 0x00f4 GE Interrupt Flag */
	unsigned int ge_status;		/* 0x00f8 GE AMX Status */
	unsigned int ge_swid;		/* 0x00fc GE Software Identify */

	unsigned int ln_x_start;	/* 0x0100 */
	unsigned int ln_x_end;		/* 0x0104 */
	unsigned int ln_y_start;	/* 0x0108 */
	unsigned int reserved2;		/* 0x010c Reserved */

	unsigned int ln_y_end;		/* 0x0110 */
	unsigned int ln_tck;		/* 0x0114 */
	unsigned int amx_csc_bypass;	/* 0x0118 GE AMX CSC Bypass */
	unsigned int c1_coef;		/* 0x011c */

	unsigned int ln_stl_tb;		/* 0x0120 */
	unsigned int ln_stl_rtn;	/* 0x0124 */
	unsigned int ln_stl_data;	/* 0x0128 */
	unsigned int ln_stl_apa;	/* 0x012c */

	unsigned int bc_p1x;		/* 0x0130 */
	unsigned int bc_p1y;		/* 0x0134 */
	unsigned int bc_p2x;		/* 0x0138 */
	unsigned int bc_p2y;		/* 0x013c */

	unsigned int bc_p3x;		/* 0x0140 */
	unsigned int bc_p3y;		/* 0x0144 */
	unsigned int bc_color;		/* 0x0148 */
	unsigned int bc_alpha;		/* 0x014c */

	unsigned int bc_delta_t;	/* 0x0150 */
	unsigned int bc_l_stl;		/* 0x0154 */
	unsigned int bc_l_stl_rtn;	/* 0x0158 */
	unsigned int c2_coef;		/* 0x015c */

	unsigned int c3_coef;		/* 0x0160 */
	unsigned int c4_coef;		/* 0x0164 */
	unsigned int c5_coef;		/* 0x0168 */
	unsigned int c6_coef;		/* 0x016c */

	unsigned int c7_coef;		/* 0x0170 */
	unsigned int c8_coef;		/* 0x0174 */
	unsigned int yuy2_y_baddr;	/* 0x0178 YUY2 Y Base Address */
	unsigned int yuy2_c_baddr;	/* 0x017c YUY2 C Base Address */

	unsigned int vq_en;		/* 0x0180 VQ Enable */
	unsigned int vq_size;		/* 0x0184 VQ Buffer Size */
	unsigned int vq_udptr;		/* 0x0188 VQ Pointer Update */
	unsigned int vq_baseaddr;	/* 0x018c VQ Buffer Base Address */

	unsigned int vq_wrsize;		/* 0x0190 VQ Free Buffer Space */
	unsigned int vq_staddrw;	/* 0x0194 DRAM Access Address */
	unsigned int vq_thr;		/* 0x0198 VQ Lower Intr. Threshold */
	unsigned int yuy2_y_fbw;	/* 0x019c YUY2 Y FB Width */

	unsigned int rop4_en;		/* 0x01a0 */
	unsigned int alpha_plane_en;	/* 0x01a4 */
	unsigned int mask_baddr;	/* 0x01a8 */
	unsigned int mask_disp_w;	/* 0x01ac */

	unsigned int mask_disp_h;	/* 0x01b0 */
	unsigned int mask_x_start;	/* 0x01b4 */
	unsigned int mask_y_start;	/* 0x01b8 */
	unsigned int mask_width;	/* 0x01bc */

	unsigned int mask_height;	/* 0x01c0 */
	unsigned int dw_mask_baddr;	/* 0x01c4 */
	unsigned int alpha_plane_wbe;	/* 0x01c8 */
	unsigned int yuy2_c_fbw;	/* 0x01cc YUY2 C FB Width */

	unsigned int adap_blend_en;	/* 0x01d0 (New!) */
	unsigned int src_alpha_sel;	/* 0x01d4 (New!) */
	unsigned int src_blend_apa;	/* 0x01d8 (New!) */
	unsigned int des_alpha_sel;	/* 0x01dc (New!) */

	unsigned int des_blend_apa;	/* 0x01e0 (New!) */
	unsigned int adap_clamp_en;	/* 0x01e4 (New!) */
	unsigned int yuy2_c_blend_sel;	/* 0x01e8 YUY2 C Blending Select */
	unsigned int src_indep_mode;	/* 0x01ec Source Indep. color mode */

	unsigned int c9_coef;		/* 0x01f0 */
	unsigned int coef_i;		/* 0x01f4 */
	unsigned int coef_j;		/* 0x01f8 */
	unsigned int coef_k;		/* 0x01fc */

	unsigned int g1_cd;		/* 0x0200 G1 Color Depth */
	unsigned int g2_cd;		/* 0x0204 G2 Color Depth */
	unsigned int reserved5[2];	/* 0x0208 - 0x020c Reserved */

	unsigned int g1_fg_addr;	/* 0x0210 G1 FG Start Address */
	unsigned int g1_bg_addr;	/* 0x0214 G1 BG Start Address */
	unsigned int g1_fb_sel;		/* 0x0218 G1 Framebuffer Select */
	unsigned int g2_fg_addr;	/* 0x021c G2 FG Start Address */

	unsigned int g2_bg_addr;	/* 0x0220 G2 BG Start Address */
	unsigned int g2_fb_sel;		/* 0x0224 G2 Framebuffer Select */
	unsigned int reserved6[2];	/* 0x0228 - 0x022c Reserved */

	unsigned int g1_x_start;	/* 0x0230 G1 X-COOR Start Point */
	unsigned int g1_x_end;		/* 0x0234 G1 X-COOR End Point */
	unsigned int g1_y_start;	/* 0x0238 G1 Y-COOR Start Point */
	unsigned int g1_y_end;		/* 0x023c G1 Y-COOR End Point */

	unsigned int g2_x_start;	/* 0x0240 G2 X-COOR Start Point */
	unsigned int g2_x_end;		/* 0x0244 G2 X-COOR End Point */
	unsigned int g2_y_start;	/* 0x0248 G2 Y-COOR Start Point */
	unsigned int g2_y_end;		/* 0x024c G2 Y-COOR End Point */

	unsigned int disp_x_end;	/* 0x0250 GE Display X End Point */
	unsigned int disp_y_end;	/* 0x0254 GE Display Y End Point */
	unsigned int ge_amx_cb;		/* 0x0258 GE AMX color bar */
	unsigned int g1_yuv_mode_en;	/* 0x025c G1 YUV Mode Enable */

	unsigned int g2_yuv_mode_en;	/* 0x0260 G2 YUV Mode Enable */
	unsigned int g1_yuv_fmt_sel;	/* 0x0264 G1 YUV Format Select */
	unsigned int g1_yuv_outp_sel;	/* 0x0268 G1 YUV Output Select */
	unsigned int g2_yuv_fmt_sel;	/* 0x026c G2 YUV Format Select */

	unsigned int g2_yuv_outp_sel;	/* 0x0270 G2 YUV Output Select */
	unsigned int ge_amx_csc_cfg;	/* 0x0274 GE AMX CSC Config */
	unsigned int ge_amx_csc_mode;	/* 0x0278 GE AMX CSC Mode */
	unsigned int ge_amx_y_sub_16_en;/* 0x027c GE AMX Y Sub 16 Enable */

	unsigned int g1_yuv_addr;	/* 0x0280 G1 YUV Address */
	unsigned int g2_yuv_addr;	/* 0x0284 G2 YUV Address */
	unsigned int reserved8[2];	/* 0x0288 - 0x028c Reserved */

	unsigned int reserved9[2];	/* 0x0290 - 0x0294 Reserved */
	unsigned int g1_ck_en;		/* 0x0298 G1 Color Key Enable */
	unsigned int g2_ck_en;		/* 0x029c G2 Color Key Enable */

	unsigned int g1_c_key;		/* 0x02a0 G1 Color Key */
	unsigned int g2_c_key;		/* 0x02a4 G2 Color Key */
	unsigned int g1_amx_en;		/* 0x02a8 G1 Alpha Mixing Enable */
	unsigned int g2_amx_en;		/* 0x02ac G2 Alpha Mixing Enable */

	unsigned int ge_ck2_apa;	/* 0x02b0 */
	unsigned int ge_amx_ctl;	/* 0x02b4 GE Alpha Mixing Control */
	unsigned int ge_ck_apa;		/* 0x02b8 GE Color Key alpha */
	unsigned int ge_fix_apa;	/* 0x02bc GE Fix Alpha */

	unsigned int g1_amx_hm;		/* 0x02c0 G1 AMX Hi Color Mode */
	unsigned int g2_amx_hm;		/* 0x02c4 G2 AMX Hi Color Mode */
	unsigned int ge_nh_data;	/* 0x02c8 G1/G2 No Hit Data Output */
	unsigned int ge_vsync_sts;	/* 0x02cc GE Vsync Status (New!) */

	unsigned int ge_reg_upd;	/* 0x02d0 GE Register Updata */
	unsigned int ge_reg_sel;	/* 0x02d4 GE Register Read Select */
	unsigned int ge_amx2_ctl;	/* 0x02d8 GE AMX Output Control */
	unsigned int ge_fix2_apa;	/* 0x02dc GE Fix Output Alpha */

	unsigned int g1_h_scale;	/* 0x02e0 G1 H Scaling Enable */
	unsigned int g2_h_scale;	/* 0x02e4 G2 H Scaling Enable */
	unsigned int g1_fbw;		/* 0x02e8 G1 Frame Buffer Width */
	unsigned int g1_vcrop;		/* 0x02ec G1 Vertical Cropping */

	unsigned int g1_hcrop;		/* 0x02f0 G1 Horizontal Cropping */
	unsigned int g2_fbw;		/* 0x02f4 G2 Frame Buffer Width */
	unsigned int g2_vcrop;		/* 0x02f8 G2 Vertical Cropping */
	unsigned int g2_hcrop;		/* 0x02fc G2 Horizontal Cropping */
};

struct ge_regs_8850 {

	/* Base1 address: 0xd8050400 */

	unsigned int ge_command;	/* 0x0000 GE Command */
	unsigned int _0004;		/* 0x0004 */
	unsigned int _0008;		/* 0x0008 */
	unsigned int pat_tran_en;	/* 0x000c GE Pattern Transp. Enable */

	unsigned int font_tran_en;	/* 0x0010 GE Font Transparency Enable */
	unsigned int rop_code;		/* 0x0014 GE ROP Code */
	unsigned int ge_fire;		/* 0x0018 GE Fire Code */
	unsigned int rop_bg_code;	/* 0x001c */

	unsigned int src_baddr;		/* 0x0020 GE Source Base Address */
	unsigned int src_disp_w;	/* 0x0024 GE Source Display width */
	unsigned int src_disp_h;	/* 0x0028 GE Source DIsplay height */
	unsigned int src_x_start;	/* 0x002c GE Source X start point */

	unsigned int src_y_start;	/* 0x0030 GE Source Y start point */
	unsigned int src_width;		/* 0x0034 GE Source Width */
	unsigned int src_height;	/* 0x0038 GE Source Height */
	unsigned int des_baddr;		/* 0x003c GE Dest Base Address */

	unsigned int des_disp_w;	/* 0x0040 GE Dest Display width */
	unsigned int des_disp_h;	/* 0x0044 GE Dest DIsplay height */
	unsigned int des_x_start;	/* 0x0048 GE Dest X start point */
	unsigned int des_y_start;	/* 0x004c GE Dest Y start point */

	unsigned int des_width;		/* 0x0050 GE Dest Width */
	unsigned int des_height;	/* 0x0054 GE Dest Height */
	unsigned int font0_buf;		/* 0x0058 GE FONT Color 0 Buffer */
	unsigned int font1_buf;		/* 0x005c GE FONT Color 1 Buffer */

	unsigned int font2_buf;		/* 0x0060 GE FONT Color 2 Buffer*/
	unsigned int font3_buf;		/* 0x0064 GE FONT Color 3 Buffer*/
	unsigned int pat0_buf;		/* 0x0068 GE Pattern 0 Buffer */
	unsigned int pat1_buf;		/* 0x006c GE Pattern 1 Buffer */

	unsigned int pat2_buf;		/* 0x0070 GE Pattern 2 Buffer */
	unsigned int pat3_buf;		/* 0x0074 GE Pattern 3 Buffer */
	unsigned int pat4_buf;		/* 0x0078 GE Pattern 4 Buffer */
	unsigned int pat5_buf;		/* 0x007c GE Pattern 5 Buffer */

	unsigned int pat6_buf;		/* 0x0080 GE Pattern 6 Buffer */
	unsigned int pat7_buf;		/* 0x0084 GE Pattern 7 Buffer */
	unsigned int pat0_color;	/* 0x0088 GE Pattern 0 Color */
	unsigned int pat1_color;	/* 0x008c GE Pattern 1 Color */

	unsigned int pat2_color;	/* 0x0090 GE Pattern 2 Color */
	unsigned int pat3_color;	/* 0x0094 GE Pattern 3 Color */
	unsigned int pat4_color;	/* 0x0098 GE Pattern 4 Color */
	unsigned int pat5_color;	/* 0x009c GE Pattern 5 Color */

	unsigned int pat6_color;	/* 0x00a0 GE Pattern 6 Color */
	unsigned int pat7_color;	/* 0x00a4 GE Pattern 7 Color */
	unsigned int pat8_color;	/* 0x00a8 GE Pattern 8 Color */
	unsigned int pat9_color;	/* 0x00ac GE Pattern 9 Color */

	unsigned int pat10_color;	/* 0x00b0 GE Pattern 10 Color */
	unsigned int pat11_color;	/* 0x00b4 GE Pattern 11 Color */
	unsigned int pat12_color;	/* 0x00b8 GE Pattern 12 Color */
	unsigned int pat13_color;	/* 0x00bc GE Pattern 13 Color */

	unsigned int pat14_color;	/* 0x00c0 GE Pattern 14 Color */
	unsigned int pat15_color;	/* 0x00c4 GE Pattern 15 Color */
	unsigned int ck_sel;		/* 0x00c8 Color Key Select */
	unsigned int src_ck;		/* 0x00cc GE Source Colorkey */

	unsigned int des_ck;		/* 0x00d0 GE Destination Colorkey */
	unsigned int alpha_sel;		/* 0x00d4 GE Alpha Select */
	unsigned int bitblt_alpha;	/* 0x00d8 */
	unsigned int _00dc;		/* 0x00dc */

	unsigned int rotate_mode;	/* 0x00e0 GE Rotate Mode */
	unsigned int mirror_mode;	/* 0x00e4 GE Mirror Mode */
	unsigned int ge_delay;		/* 0x00e8 GE Start Cycle Delay */
	unsigned int ge_eng_en;		/* 0x00ec GE Engine Enable */

	unsigned int ge_int_en;		/* 0x00f0 GE Intr. Enable signal */
	unsigned int ge_int_flag;	/* 0x00f4 GE Interrupt Flag */
	unsigned int ge_status;		/* 0x00f8 GE AMX Status */
	unsigned int ge_swid;		/* 0x00fc GE Software Identify */

	/* Base2 address: 0xd8050500 */

	unsigned int ln_x_start;	/* 0x0100 */
	unsigned int ln_x_end;		/* 0x0104 */
	unsigned int ln_y_start;	/* 0x0108 */
	unsigned int _010c;		/* 0x010c */

	unsigned int ln_y_end;		/* 0x0110 */
	unsigned int ln_tck;		/* 0x0114 */
	unsigned int amx_csc_bypass;	/* 0x0118 GE AMX CSC Bypass */
	unsigned int c1_coef;		/* 0x011c */

	unsigned int ln_stl_tb;		/* 0x0120 */
	unsigned int ln_stl_rtn;	/* 0x0124 */
	unsigned int ln_stl_data;	/* 0x0128 */
	unsigned int ln_stl_apa;	/* 0x012c */

	unsigned int bc_p1x;		/* 0x0130 */
	unsigned int bc_p1y;		/* 0x0134 */
	unsigned int bc_p2x;		/* 0x0138 */
	unsigned int bc_p2y;		/* 0x013c */

	unsigned int bc_p3x;		/* 0x0140 */
	unsigned int bc_p3y;		/* 0x0144 */
	unsigned int bc_color;		/* 0x0148 */
	unsigned int bc_alpha;		/* 0x014c */

	unsigned int bc_delta_t;	/* 0x0150 */
	unsigned int bc_l_stl;		/* 0x0154 */
	unsigned int bc_l_stl_rtn;	/* 0x0158 */
	unsigned int c2_coef;		/* 0x015c */

	unsigned int c3_coef;		/* 0x0160 */
	unsigned int c4_coef;		/* 0x0164 */
	unsigned int c5_coef;		/* 0x0168 */
	unsigned int c6_coef;		/* 0x016c */

	unsigned int c7_coef;		/* 0x0170 */
	unsigned int c8_coef;		/* 0x0174 */
	unsigned int _0178;		/* 0x0178 */
	unsigned int _017c;		/* 0x017c */

	unsigned int vq_en;		/* 0x0180 VQ Enable */
	unsigned int vq_size;		/* 0x0184 VQ Buffer Size */
	unsigned int vq_udptr;		/* 0x0188 VQ Pointer Update */
	unsigned int vq_baseaddr;	/* 0x018c VQ Buffer Base Address */

	unsigned int vq_wrsize;		/* 0x0190 VQ Free Buffer Space */
	unsigned int vq_staddrw;	/* 0x0194 DRAM Access Address */
	unsigned int vq_thr;		/* 0x0198 VQ Lower Intr. Threshold */
	unsigned int _019c;		/* 0x019c */

	unsigned int rop4_en;		/* 0x01a0 */
	unsigned int alpha_plane_en;	/* 0x01a4 */
	unsigned int mask_baddr;	/* 0x01a8 */
	unsigned int mask_disp_w;	/* 0x01ac */

	unsigned int mask_disp_h;	/* 0x01b0 */
	unsigned int mask_x_start;	/* 0x01b4 */
	unsigned int mask_y_start;	/* 0x01b8 */
	unsigned int mask_width;	/* 0x01bc */

	unsigned int mask_height;	/* 0x01c0 */
	unsigned int dw_mask_baddr;	/* 0x01c4 */
	unsigned int alpha_plane_wbe;	/* 0x01c8 */
	unsigned int _01cc;		/* 0x01cc */

	unsigned int adap_blend_en;	/* 0x01d0 (New!) */
	unsigned int src_alpha_sel;	/* 0x01d4 (New!) */
	unsigned int src_blend_apa;	/* 0x01d8 (New!) */
	unsigned int des_alpha_sel;	/* 0x01dc (New!) */

	unsigned int des_blend_apa;	/* 0x01e0 (New!) */
	unsigned int adap_clamp_en;	/* 0x01e4 (New!) */
	unsigned int yuy2_c_blend_sel;	/* 0x01e8 YUY2 C Blending Select */
	unsigned int _01ec;		/* 0x01ec */

	unsigned int c9_coef;		/* 0x01f0 */
	unsigned int coef_i;		/* 0x01f4 */
	unsigned int coef_j;		/* 0x01f8 */
	unsigned int coef_k;		/* 0x01fc */

	/* Base3 address: 0xd8050600 */

	unsigned int g1_cd;		/* 0x0200 G1 Color Depth */
	unsigned int g2_cd;		/* 0x0204 G2 Color Depth */
	unsigned int _0208;		/* 0x0208 */
	unsigned int _020c;		/* 0x020c */

	unsigned int g1_fg_addr;	/* 0x0210 G1 FG Start Address */
	unsigned int g1_bg_addr;	/* 0x0214 G1 BG Start Address */
	unsigned int g1_fb_sel;		/* 0x0218 G1 Framebuffer Select */
	unsigned int g2_fg_addr;	/* 0x021c G2 FG Start Address */

	unsigned int g2_bg_addr;	/* 0x0220 G2 BG Start Address */
	unsigned int g2_fb_sel;		/* 0x0224 G2 Framebuffer Select */
	unsigned int _0228;		/* 0x0228 */
	unsigned int _022c;		/* 0x022c */

	unsigned int g1_x_start;	/* 0x0230 G1 X-COOR Start Point */
	unsigned int g1_x_end;		/* 0x0234 G1 X-COOR End Point */
	unsigned int g1_y_start;	/* 0x0238 G1 Y-COOR Start Point */
	unsigned int g1_y_end;		/* 0x023c G1 Y-COOR End Point */

	unsigned int g2_x_start;	/* 0x0240 G2 X-COOR Start Point */
	unsigned int g2_x_end;		/* 0x0244 G2 X-COOR End Point */
	unsigned int g2_y_start;	/* 0x0248 G2 Y-COOR Start Point */
	unsigned int g2_y_end;		/* 0x024c G2 Y-COOR End Point */

	unsigned int disp_x_end;	/* 0x0250 GE Display X End Point */
	unsigned int disp_y_end;	/* 0x0254 GE Display Y End Point */
	unsigned int ge_amx_cb;		/* 0x0258 GE AMX color bar */
	unsigned int g1_yuv_mode_en;	/* 0x025c G1 YUV Mode Enable */

	unsigned int g2_yuv_mode_en;	/* 0x0260 G2 YUV Mode Enable */
	unsigned int g1_yuv_fmt_sel;	/* 0x0264 G1 YUV Format Select */
	unsigned int g1_yuv_outp_sel;	/* 0x0268 G1 YUV Output Select */
	unsigned int g2_yuv_fmt_sel;	/* 0x026c G2 YUV Format Select */

	unsigned int g2_yuv_outp_sel;	/* 0x0270 G2 YUV Output Select */
	unsigned int ge_amx_csc_cfg;	/* 0x0274 GE AMX CSC Config */
	unsigned int ge_amx_csc_mode;	/* 0x0278 GE AMX CSC Mode */
	unsigned int ge_amx_y_sub_16_en;/* 0x027c GE AMX Y Sub 16 Enable */

	unsigned int g1_yuv_addr;	/* 0x0280 G1 YUV Address */
	unsigned int g2_yuv_addr;	/* 0x0284 G2 YUV Address */
	unsigned int _0288;		/* 0x0288 */
	unsigned int _028c;		/* 0x028c */

	unsigned int _0290;		/* 0x0290 */
	unsigned int _0294;		/* 0x0294 */
	unsigned int g1_ck_en;		/* 0x0298 G1 Color Key Enable */
	unsigned int g2_ck_en;		/* 0x029c G2 Color Key Enable */

	unsigned int g1_c_key;		/* 0x02a0 G1 Color Key */
	unsigned int g2_c_key;		/* 0x02a4 G2 Color Key */
	unsigned int g1_amx_en;		/* 0x02a8 G1 Alpha Mixing Enable */
	unsigned int g2_amx_en;		/* 0x02ac G2 Alpha Mixing Enable */

	unsigned int ge_ck2_apa;	/* 0x02b0 */
	unsigned int ge_amx_ctl;	/* 0x02b4 GE Alpha Mixing Control */
	unsigned int ge_ck_apa;		/* 0x02b8 GE Color Key alpha */
	unsigned int ge_fix_apa;	/* 0x02bc GE Fix Alpha */

	unsigned int g1_amx_hm;		/* 0x02c0 G1 AMX Hi Color Mode */
	unsigned int g2_amx_hm;		/* 0x02c4 G2 AMX Hi Color Mode */
	unsigned int ge_nh_data;	/* 0x02c8 G1/G2 No Hit Data Output */
	unsigned int ge_vsync_sts;	/* 0x02cc GE Vsync Status (New!) */

	unsigned int ge_reg_upd;	/* 0x02d0 GE Register Updata */
	unsigned int ge_reg_sel;	/* 0x02d4 GE Register Read Select */
	unsigned int ge_amx2_ctl;	/* 0x02d8 GE AMX Output Control */
	unsigned int ge_fix2_apa;	/* 0x02dc GE Fix Output Alpha */

	unsigned int g1_h_scale;	/* 0x02e0 G1 H Scaling Enable */
	unsigned int g2_h_scale;	/* 0x02e4 G2 H Scaling Enable */
	unsigned int g1_fbw;		/* 0x02e8 G1 Frame Buffer Width */
	unsigned int g1_vcrop;		/* 0x02ec G1 Vertical Cropping */

	unsigned int g1_hcrop;		/* 0x02f0 G1 Horizontal Cropping */
	unsigned int g2_fbw;		/* 0x02f4 G2 Frame Buffer Width */
	unsigned int g2_vcrop;		/* 0x02f8 G2 Vertical Cropping */
	unsigned int g2_hcrop;		/* 0x02fc G2 Horizontal Cropping */

	/* Reserved address: 0xd8050700 - 0xd80512fc */

	unsigned int _0300[768];	/* 0x0300 - 0xefc */

	/* Base4 address: 0xd8051300 */

	unsigned int alpha_bitblt_mode;	/* 0x0f00 */
	unsigned int src_input_sel;	/* 0x0f04 */
	unsigned int period_stop;	/* 0x0f08 */
	unsigned int _0f0c;		/* 0x0f0c */

	unsigned int src_c_exp_bypass;		/* 0x0f10 */
	unsigned int src_c_exp_rgb_mode;	/* 0x0f14 */
	unsigned int src_c_exp_cplt_mode;	/* 0x0f18 */
	unsigned int src_csc_en;		/* 0x0f1c */

	unsigned int src_csc_mode;	/* 0x0f20 */
	unsigned int src_y_sub_16_en;	/* 0x0f24 */
	unsigned int src_c1_coef;	/* 0x0f28 */
	unsigned int src_c2_coef;	/* 0x0f2c */

	unsigned int src_c3_coef;	/* 0x0f30 */
	unsigned int src_c4_coef;	/* 0x0f34 */
	unsigned int src_c5_coef;	/* 0x0f38 */
	unsigned int src_c6_coef;	/* 0x0f3c */

	unsigned int src_c7_coef;	/* 0x0f40 */
	unsigned int src_c8_coef;	/* 0x0f44 */
	unsigned int src_c9_coef;	/* 0x0f48 */
	unsigned int src_coef_i;	/* 0x0f4c */

	unsigned int src_coef_j;	/* 0x0f50 */
	unsigned int src_coef_k;	/* 0x0f54 */
	unsigned int src_igs_mode;	/* 0x0f58 */
	unsigned int _0f5c;		/* 0x0f5c */

	unsigned int des_c_exp_bypass;		/* 0x0f60 */
	unsigned int des_c_exp_rgb_mode;	/* 0x0f64 */
	unsigned int des_c_exp_cplt_mode;	/* 0x0f68 */
	unsigned int des_csc_en;		/* 0x0f6c */

	unsigned int des_csc_mode;	/* 0x0f70 */
	unsigned int des_y_sub_16_en;	/* 0x0f74 */
	unsigned int des_c1_coef;	/* 0x0f78 */
	unsigned int des_c2_coef;	/* 0x0f7c */

	unsigned int des_c3_coef;	/* 0x0f80 */
	unsigned int des_c4_coef;	/* 0x0f84 */
	unsigned int des_c5_coef;	/* 0x0f88 */
	unsigned int des_c6_coef;	/* 0x0f8c */

	unsigned int des_c7_coef;	/* 0x0f90 */
	unsigned int des_c8_coef;	/* 0x0f94 */
	unsigned int des_c9_coef;	/* 0x0f98 */
	unsigned int des_coef_i;	/* 0x0f9c */

	unsigned int des_coef_j;	/* 0x0fa0 */
	unsigned int des_coef_k;	/* 0x0fa4 */
	unsigned int des_igs_mode;	/* 0x0fa8 */
	unsigned int vpu_rcmd;		/* 0x0fac */

	unsigned int vpu_field_mode;	/* 0x0fb0 */
	unsigned int vpu_field_sel;	/* 0x0fb4 */
	unsigned int vpu_vpu_pvbi;	/* 0x0fb8 */
	unsigned int time_out_sel;	/* 0x0fbc */

	unsigned int time_out;		/* 0x0fc0 */
	unsigned int src_rgb32_fmt;	/* 0x0fc4 */
	unsigned int src_yuv_en;	/* 0x0fc8 */
	unsigned int src_fmt444;	/* 0x0fcc */

	unsigned int src_vfmt;		/* 0x0fd0 */
	unsigned int src_src_fmt;	/* 0x0fd4 */
	unsigned int src_src_out;	/* 0x0fd8 */
	unsigned int src_field_mode;	/* 0x0fdc */

	unsigned int src_field_sel;	/* 0x0fe0 */
	unsigned int src_c_baddr;	/* 0x0fe4 */
	unsigned int src_cb;		/* 0x0fe8 */
	unsigned int src_color_depth;	/* 0x0fec */

	unsigned int src_hm_sel;	/* 0x0ff0 */
	unsigned int src_req_sel;	/* 0x0ff4 */
	unsigned int src_y_req_num;	/* 0x0ff8 */
	unsigned int src_c_req_num;	/* 0x0ffc */

	/* Base5 address: 0xd8051400 */

	unsigned int des_yuv_en;	/* 0x1000 */
	unsigned int des_fmt444;	/* 0x1004 */
	unsigned int des_vfmt;		/* 0x1008 */
	unsigned int des_src_fmt;	/* 0x100c */

	unsigned int des_src_out;	/* 0x1010 */
	unsigned int des_field_mode;	/* 0x1014 */
	unsigned int des_field_sel;	/* 0x1018 */
	unsigned int des_c_baddr;	/* 0x101c */

	unsigned int des_cb;		/* 0x1020 */
	unsigned int des_color_depth;	/* 0x1024 */
	unsigned int des_hm_sel;	/* 0x1028 */
	unsigned int des_req_sel;	/* 0x102c */

	unsigned int des_y_req_num;	/* 0x1030 */
	unsigned int des_c_req_num;	/* 0x1034 */
	unsigned int dw_color_depth;	/* 0x1038 */
	unsigned int dw_hm_sel;		/* 0x103c */

	unsigned int dw_baddr;		/* 0x1040 */
	unsigned int dw_disp_w;		/* 0x1044 */
	unsigned int dw_disp_h;		/* 0x1048 */
	unsigned int dw_x_start;	/* 0x104c */

	unsigned int dw_y_start;	/* 0x1050 */
	unsigned int dw_width;		/* 0x1054 */
	unsigned int dw_height;		/* 0x1058 */
	unsigned int dw_yuv_en;		/* 0x105c */

	unsigned int dw_fmt444;		/* 0x1060 */
	unsigned int dw_vfmt;		/* 0x1064 */
	unsigned int dw_c_baddr;	/* 0x1068 */
	unsigned int _106c;		/* 0x106c */

	unsigned int govr_src_en;	/* 0x1070 */
	unsigned int govr_des_en;	/* 0x1074 */
	unsigned int govr_src_x_start;	/* 0x1078 */
	unsigned int govr_src_y_start;	/* 0x107c */

	unsigned int govr_des_x_start;		/* 0x1080 */
	unsigned int govr_des_y_start;		/* 0x1084 */
	unsigned int govr_no_hit_data;		/* 0x1088 */
	unsigned int govr_src_blend_data;	/* 0x108c */

	unsigned int govr_des_blend_data;	/* 0x1090 */
	unsigned int src_fix_00_apa;		/* 0x1094 */
	unsigned int src_fix_01_apa;		/* 0x1098 */
	unsigned int src_fix_10_apa;		/* 0x109c */

	unsigned int src_fix_11_apa;		/* 0x10a0 */
	unsigned int src_govr_hit_01_sel;	/* 0x10a4 */
	unsigned int src_govr_hit_10_sel;	/* 0x10a8 */
	unsigned int src_govr_hit_11_sel;	/* 0x10ac */

	unsigned int des_fix_00_apa;	/* 0x10b0 */
	unsigned int des_fix_01_apa;	/* 0x10b4 */
	unsigned int des_fix_10_apa;	/* 0x10b8 */
	unsigned int des_fix_11_apa;	/* 0x10bc */

	unsigned int des_govr_hit_01_sel;	/* 0x10c0 */
	unsigned int des_govr_hit_10_sel;	/* 0x10c4 */
	unsigned int des_govr_hit_11_sel;	/* 0x10c8 */
	unsigned int src_ckey_en;		/* 0x10cc */

	unsigned int des_ckey_en;	/* 0x10d0 */
	unsigned int src_ckey_ctrl;	/* 0x10d4 */
	unsigned int des_ckey_ctrl;	/* 0x10d8 */
	unsigned int src_ckey;		/* 0x10dc */

	unsigned int des_ckey;		/* 0x10e0 */
	unsigned int dw_watch_dog_en;	/* 0x10e4 */
	unsigned int dw_watch_dog;	/* 0x10e8 */
	unsigned int mask_bypass;	/* 0x10ec */

	unsigned int _10f0;		/* 0x10f0 */
	unsigned int _10f4;		/* 0x10f4 */
	unsigned int _10f8;		/* 0x10f8 */
	unsigned int no_hit_alpha;	/* 0x10fc */

	/* Base6 address: 0xd8051500 */

	unsigned int src_output_alpha_01;	/* 0x1100 */
	unsigned int src_output_alpha_0101;	/* 0x1104 */
	unsigned int src_output_alpha_10;	/* 0x1108 */
	unsigned int src_output_alpha_1010;	/* 0x110c */

	unsigned int src_output_alpha_1100;	/* 0x1110 */
	unsigned int src_output_alpha_1101;	/* 0x1114 */
	unsigned int src_output_alpha_1110;	/* 0x1118 */
	unsigned int src_output_alpha_1111;	/* 0x111c */

	unsigned int src_output_alpha_01_sel;	/* 0x1120 */
	unsigned int src_output_alpha_0101_sel;	/* 0x1124 */
	unsigned int src_output_alpha_10_sel;	/* 0x1128 */
	unsigned int src_output_alpha_1010_sel;	/* 0x112c */

	unsigned int src_output_alpha_1100_sel;	/* 0x1130 */
	unsigned int src_output_alpha_1101_sel;	/* 0x1134 */
	unsigned int src_output_alpha_1110_sel;	/* 0x1138 */
	unsigned int src_output_alpha_1111_sel;	/* 0x113c */

	unsigned int des_output_alpha_01;	/* 0x1140 */
	unsigned int des_output_alpha_0101;	/* 0x1144 */
	unsigned int des_output_alpha_10;	/* 0x1148 */
	unsigned int des_output_alpha_1010;	/* 0x114c */

	unsigned int des_output_alpha_1100;	/* 0x1150 */
	unsigned int des_output_alpha_1101;	/* 0x1154 */
	unsigned int des_output_alpha_1110;	/* 0x1158 */
	unsigned int des_output_alpha_1111;	/* 0x115c */

	unsigned int des_output_alpha_01_sel;	/* 0x1160 */
	unsigned int des_output_alpha_0101_sel;	/* 0x1164 */
	unsigned int des_output_alpha_10_sel;	/* 0x1168 */
	unsigned int des_output_alpha_1010_sel;	/* 0x116c */

	unsigned int des_output_alpha_1100_sel;	/* 0x1170 */
	unsigned int des_output_alpha_1101_sel;	/* 0x1174 */
	unsigned int des_output_alpha_1110_sel;	/* 0x1178 */
	unsigned int des_output_alpha_1111_sel;	/* 0x117c */

	unsigned int src_select_alpha_01;	/* 0x1180 */
	unsigned int src_select_alpha_0101;	/* 0x1184 */
	unsigned int src_select_alpha_10;	/* 0x1188 */
	unsigned int src_select_alpha_1010;	/* 0x118c */

	unsigned int src_select_alpha_1100;	/* 0x1190 */
	unsigned int src_select_alpha_1101;	/* 0x1194 */
	unsigned int src_select_alpha_1110;	/* 0x1198 */
	unsigned int src_select_alpha_1111;	/* 0x119c */

	unsigned int src_select_alpha_01_sel;	/* 0x11a0 */
	unsigned int src_select_alpha_0101_sel;	/* 0x11a4 */
	unsigned int src_select_alpha_10_sel;	/* 0x11a8 */
	unsigned int src_select_alpha_1010_sel;	/* 0x11ac */

	unsigned int src_select_alpha_1100_sel;	/* 0x11b0 */
	unsigned int src_select_alpha_1101_sel;	/* 0x11b4 */
	unsigned int src_select_alpha_1110_sel;	/* 0x11b8 */
	unsigned int src_select_alpha_1111_sel;	/* 0x11bc */

	unsigned int des_select_alpha_01;	/* 0x11c0 */
	unsigned int des_select_alpha_0101;	/* 0x11c4 */
	unsigned int des_select_alpha_10;	/* 0x11c8 */
	unsigned int des_select_alpha_1010;	/* 0x11cc */

	unsigned int des_select_alpha_1100;	/* 0x11d0 */
	unsigned int des_select_alpha_1101;	/* 0x11d4 */
	unsigned int des_select_alpha_1110;	/* 0x11d8 */
	unsigned int des_select_alpha_1111;	/* 0x11dc */

	unsigned int des_select_alpha_01_sel;	/* 0x11e0 */
	unsigned int des_select_alpha_0101_sel;	/* 0x11e4 */
	unsigned int des_select_alpha_10_sel;	/* 0x11e8 */
	unsigned int des_select_alpha_1010_sel;	/* 0x11ec */

	unsigned int des_select_alpha_1100_sel;	/* 0x11f0 */
	unsigned int des_select_alpha_1101_sel;	/* 0x11f4 */
	unsigned int des_select_alpha_1110_sel;	/* 0x11f8 */
	unsigned int des_select_alpha_1111_sel;	/* 0x11fc */

	/* Base7 address: 0xd8051600 */

	unsigned int _1200;	/* 0x1200 */
	unsigned int _1204;	/* 0x1204 */
	unsigned int _1208;	/* 0x1208 */
	unsigned int _120c;	/* 0x120c */

	unsigned int _1210;	/* 0x1210 */
	unsigned int _1214;	/* 0x1214 */
	unsigned int _1218;	/* 0x1218 */
	unsigned int _121c;	/* 0x121c */

	unsigned int _1220;	/* 0x1220 */
	unsigned int _1224;	/* 0x1224 */
	unsigned int _1228;	/* 0x1228 */
	unsigned int _122c;	/* 0x122c */

	unsigned int _1230;	/* 0x1230 */
	unsigned int _1234;	/* 0x1234 */
	unsigned int _1238;	/* 0x1238 */
	unsigned int _123c;	/* 0x123c */

	unsigned int _1240;	/* 0x1240 */
	unsigned int _1244;	/* 0x1244 */
	unsigned int _1248;	/* 0x1248 */
	unsigned int _124c;	/* 0x124c */

	unsigned int _1250;	/* 0x1250 */
	unsigned int _1254;	/* 0x1254 */
	unsigned int _1258;	/* 0x1258 */
	unsigned int _125c;	/* 0x125c */

	unsigned int _1260;	/* 0x1260 */
	unsigned int _1264;	/* 0x1264 */
	unsigned int _1268;	/* 0x1268 */
	unsigned int _126c;	/* 0x126c */

	unsigned int _1270;	/* 0x1270 */
	unsigned int _1274;	/* 0x1274 */
	unsigned int _1278;	/* 0x1278 */
	unsigned int _127c;	/* 0x127c */

	unsigned int _1280;	/* 0x1280 */
	unsigned int _1284;	/* 0x1284 */
	unsigned int _1288;	/* 0x1288 */
	unsigned int _128c;	/* 0x128c */

	unsigned int _1290;	/* 0x1290 */
	unsigned int _1294;	/* 0x1294 */
	unsigned int _1298;	/* 0x1298 */
	unsigned int _129c;	/* 0x129c */

	unsigned int _12a0;	/* 0x12a0 */
	unsigned int _12a4;	/* 0x12a4 */
	unsigned int _12a8;	/* 0x12a8 */
	unsigned int _12ac;	/* 0x12ac */

	unsigned int _12b0;	/* 0x12b0 */
	unsigned int _12b4;	/* 0x12b4 */
	unsigned int _12b8;	/* 0x12b8 */
	unsigned int _12bc;	/* 0x12bc */

	unsigned int _12c0;	/* 0x12c0 */
	unsigned int _12c4;	/* 0x12c4 */
	unsigned int _12c8;	/* 0x12c8 */
	unsigned int _12cc;	/* 0x12cc */

	unsigned int _12d0;	/* 0x12d0 */
	unsigned int _12d4;	/* 0x12d4 */
	unsigned int _12d8;	/* 0x12d8 */
	unsigned int _12dc;	/* 0x12dc */

	unsigned int _12e0;	/* 0x12e0 */
	unsigned int _12e4;	/* 0x12e4 */
	unsigned int _12e8;	/* 0x12e8 */
	unsigned int _12ec;	/* 0x12ec */

	unsigned int _12f0;	/* 0x12f0 */
	unsigned int _12f4;	/* 0x12f4 */
	unsigned int _12f8;	/* 0x12f8 */
	unsigned int _12fc;	/* 0x12fc */
};

#endif /* GE_REGS_DEF_H */
