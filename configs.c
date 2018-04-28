#include "vdp2.h"

static const struct scrn_format _bg_00_format = {
        .sf_enable = true,
        .sf_scroll_screen = SCRN_NBG2,
        .sf_type = SCRN_TYPE_CELL,
        .sf_cc_count = SCRN_CCC_PALETTE_16,
        .sf_format = {
                .cell = {
                        .scf_character_size = 1 * 1,
                        .scf_pnd_size = 1,
                        .scf_auxiliary_mode = 0,
                        .scf_cp_table = 0x05E00000,
                        .scf_color_palette = 0x05F00000,
                        .scf_vcs_table = 0x00000000,
                        .scf_reduction = SCRN_REDUCTION_QUARTER,
                        .scf_plane_size = 1 * 1,
                        .scf_map.plane_a = 0x05E00000,
                        .scf_map.plane_b = 0x05E00000,
                        .scf_map.plane_c = 0x05E00000,
                        .scf_map.plane_d = 0x05E00000
                }
        }
};

static const struct scrn_format _bg_01_format = {
        .sf_enable = true,
        .sf_scroll_screen = SCRN_NBG3,
        .sf_type = SCRN_TYPE_CELL,
        .sf_cc_count = SCRN_CCC_PALETTE_16,
        .sf_format = {
                .cell = {
                        .scf_character_size = 1 * 1,
                        .scf_pnd_size = 1,
                        .scf_auxiliary_mode = 0,
                        .scf_cp_table = 0x05E00000,
                        .scf_color_palette = 0x05F00000,
                        .scf_vcs_table = 0x00000000,
                        .scf_reduction = SCRN_REDUCTION_QUARTER,
                        .scf_plane_size = 1 * 1,
                        .scf_map.plane_a = 0x05E00000,
                        .scf_map.plane_b = 0x05E00000,
                        .scf_map.plane_c = 0x05E00000,
                        .scf_map.plane_d = 0x05E00000
                }
        }
};

static const struct scrn_format _bg_02_format = {
        .sf_enable = true,
        .sf_scroll_screen = SCRN_NBG1,
        .sf_type = SCRN_TYPE_BITMAP,
        .sf_cc_count = SCRN_CCC_PALETTE_16,
        .sf_format = {
                .bitmap = {
                        .sbf_bitmap_size.width = 512,
                        .sbf_bitmap_size.height = 256,
                        .sbf_bitmap_pattern = 0x05E00000,
                        .sbf_color_palette = 0x05F00000
                }
        }
};

const struct scrn_format *bg_formats[] = {
        &_bg_00_format,
        &_bg_01_format,
        &_bg_02_format,
        NULL
};
