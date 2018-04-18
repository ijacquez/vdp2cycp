import csv
import os
import struct
import sys
import re

PROGNAME = os.path.basename(sys.argv[0])

def usage():
    print >> sys.stderr, "%s in.csv" % (PROGNAME)

if len(sys.argv[1:]) != 1:
    usage()
    sys.exit(2)

SCROLL_SCREENS = {
    "NBG0": "SCRN_NBG0",
    "NBG1": "SCRN_NBG1",
    "NBG2": "SCRN_NBG2",
    "NBG3": "SCRN_NBG3",
    "RBG0": "SCRN_RBG0",
    "RBG1": "SCRN_RBG1"
}

FORMATS = {
    "cell": "SCRN_FORMAT_CELL",
    "bitmap": "SCRN_FORMAT_BITMAP"
}

CCC = {
    "16": "SCRN_CCC_PALETTE_16",
    "256": "SCRN_CCC_PALETTE_256",
    "2048": "SCRN_CCC_PALETTE_2048",
    "32768": "SCRN_CCC_RBG_32768",
    "16770000": "SCRN_CCC_RGB_16770000"
}

CHARACTER_SIZES = {
    "1x1": "1 * 1",
    "2x2": "2 * 2"
};

PND = {
    "1": "1",
    "2": "2"
}

AUXILIARY_MODES = {
    "0": "0",
    "1": "1"
}

REDUCTIONS = {
    "1": "SCRN_REDUCTION_NONE",
    "1/2": "SCRN_REDUCTION_HALF",
    "1/4": "SCRN_REDUCTION_QUARTER"
}

PLANE_SIZES = {
    "1x1": "1 * 1",
    "2x1": "2 * 1",
    "2x2": "2 * 2"
}

class SCRNFormat:
    def __init__(self, name, *args):
        try:
            self._name = self._trim(name)
            self._format = self._trim(args[1])

            self.scroll_screen = self._parse_scroll_screen(args[0])
            self.format = self._parse_format(args[1])
            self.cc_count = self._parse_cc_count(args[2])
            self.character_size = self._parse_character_size(args[3])
            self.pnd_size = self._parse_pnd_size(args[4])
            self.auxiliary_mode = self._parse_auxiliary_mode(args[5])
            self.cp_table = self._parse_cp_table(args[6])
            self.color_palette = self._parse_color_palette(args[7])
            self.vcs_table = self._parse_vcs_table(args[8])
            self.reduction = self._parse_reduction(args[9])
            self.plane_size = self._parse_plane_size(args[10])
            self.plane_a = self._parse_plane(args[11])
            self.plane_b = self._parse_plane(args[12])
            self.plane_c = self._parse_plane(args[13])
            self.plane_d = self._parse_plane(args[14])
        except IndexError:
            raise ValueError("Invalid arguments")

    def __str__ (self):
        format = ""
        if self._format == "cell":
            format += """static const struct scrn_cell_format _%s_%s_config = {
        .scf_character_size = %s,
        .scf_pnd_size = %s,
        .scf_auxiliary_mode = %s,
        .scf_cp_table = %s,
        .scf_color_palette = %s,
        .scf_vcs_table = %s,
        .scf_reduction = %s,
        .scf_plane_size = %s,
        .scf_map.plane_a = %s,
        .scf_map.plane_b = %s,
        .scf_map.plane_c = %s,
        .scf_map.plane_d = %s
};
""" % (self._name,
         self._format,
         self.character_size,
         self.pnd_size,
         self.auxiliary_mode,
         self.cp_table,
         self.color_palette,
         self.vcs_table,
         self.reduction,
         self.plane_size,
         self.plane_a,
         self.plane_b,
         self.plane_c,
         self.plane_d)
        elif self._format == "bitmap":
            format += """static const struct scrn_bitmap_format _%s_%s_config = {
};
""" % (self._name,
         self._format)
        format += """
static const struct scrn_format _%s_config = {
        .sf_scroll_screen = %s,
        .sf_format = %s,
        .sf_cc_count = %s,
        .sf_config = &%s_%s_config
};""" % (self._name,
        self.scroll_screen,
        self.format,
        self.cc_count,
        self._name,
        self._format)
        return format

    def _trim(self, value):
        pattern = re.compile(r'\s+')
        return re.sub(pattern, '', value)

    def _parse(self, value, kvmap):
        try:
            svalue = self._trim(value)
            return kvmap[svalue]
        except KeyError:
            raise ValueError("Invalid value: %s" % (", ".join(kvmap.keys())))

    def _parse_hex(self, value):
        try:
            return int(value, 0)
        except ValueError:
            raise ValueError("Cannot convert to a valid address")

    def _parse_address_range(self, value, from_addr, to_addr):
        ivalue = self._parse_hex(value)
        if (ivalue < from_addr) or (ivalue > to_addr):
            raise ValueError("Address 0x%08X is not within range 0x%08X:0x%08X" % (ivalue, from_addr, to_addr))
        return ("0x%08X" % (ivalue))

    def _parse_scroll_screen(self, value):
        return self._parse(value, SCROLL_SCREENS)

    def _parse_format(self, value):
        return self._parse(value, FORMATS)

    def _parse_cc_count(self, value):
        return self._parse(value, CCC)

    def _parse_character_size(self, value):
        return self._parse(value, CHARACTER_SIZES)

    def _parse_pnd_size(self, value):
        return self._parse(value, PND)

    def _parse_auxiliary_mode(self, value):
        return self._parse(value, AUXILIARY_MODES)

    def _parse_cp_table(self, value):
        return self._parse_address_range(value, 0x25E00000, 0x25EFFFFF)

    def _parse_color_palette(self, value):
        return self._parse_address_range(value, 0x25F00000, 0x25F0FFFF)

    def _parse_vcs_table(self, value):
        # Special case
        ivalue = self._parse_hex(value)
        if ivalue == 0x00000000:
            return value
        return self._parse_address_range(value, 0x25E00000, 0x25EFFFFF)

    def _parse_reduction(self, value):
        return self._parse(value, REDUCTIONS)

    def _parse_plane_size(self, value):
        return self._parse(value, PLANE_SIZES)

    def _parse_plane(self, value):
        return self._parse_address_range(value, 0x25E00000, 0x25EFFFFF)

def convert_filename(filename):
    return os.path.splitext(filename)[0].replace(".", "_") \
                                        .replace(" ", "_")

csv_count = len(sys.argv[1:])
idx = 0

print """#include "vdp2.h"
"""

csv_file = sys.argv[1]
try:
    # Count number of rows
    row_count = 0
    with open(csv_file, "r") as ifp:
        reader = csv.reader(ifp, delimiter = ',')
        row_count = sum(1 for row in ifp) - 1
    # Parse rows
    config_names = []
    with open(csv_file, "r") as ifp:
        reader = csv.reader(ifp, delimiter = ',')
        # Skip the header
        reader.next()
        row_idx = 0
        for row in reader:
            row_idx += 1
            name = "%s_%04i" % (convert_filename(csv_file), idx)
            config_names.append(name)
            scrn = SCRNFormat(name, *row)
            if (row_idx > 1) and (row_idx <= row_count):
                print
            print scrn
            idx += 1
except IOError as e:
    print >> sys.stderr, "%s: error: %s" % (PROGNAME, e.strerror)
except csv.Error as e:
    print >> sys.stderr, "%s: error: File %s, line %i: %s" % (PROGNAME, csv_file, reader.line_num, e)
print
print """static const struct scrn_format *_%s_configs[] = {""" % (convert_filename(csv_file))
print ",\n".join(["        &_%s_config" % (config_name) for config_name in config_names] + ["        NULL"])
print """};"""
