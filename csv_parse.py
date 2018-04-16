import csv
import os
import struct
import sys
import re

PROGNAME = os.path.basename(sys.argv[0])

def usage():
    print >> sys.stderr, "%s in.csv out.fmt" % (PROGNAME)

if len(sys.argv[1:]) != 2:
    usage()
    sys.exit(2)

SCROLL_SCREENS = {
    "NBG0": 1 << 0,
    "NBG1": 1 << 1,
    "NBG2": 1 << 2,
    "NBG3": 1 << 3,
    "RBG0": 1 << 4,
    "RBG1": 1 << 5
}

FORMATS = [
    "cell",
    "format"
]

CCC = [
    16,
    256,
    2048,
    32768,
    16770000
]

PND = [
    1,
    2
]

AUXILIARY_MODES = [
    0,
    1
]

class SCRNCellFormat:
    def __init__(self, *args):
        try:
            self.scroll_screen = self._parse_scroll_screen(args[0])
            self.format = self._parse_format(args[1])
            self.cc_count = self._parse_cc_count(args[2])
            self.pnd_size = self._parse_pnd_size(args[3])
            self.auxiliary_mode = self._parse_auxiliary_mode(args[4])
            self.cp_table = self._parse_cp_table(args[5])
            self.color_palette = self._parse_color_palette(args[6])
            self.vcs_table = self._parse_vcs_table(args[7])
            self.reduction = self._parse_reduction(args[8])
        except IndexError:
            raise ValueError("Invalid arguments")

    def _parse_scroll_screen(self, value):
        if value not in SCROLL_SCREENS.keys():
            raise ValueError("Invalid scroll screen value: %s" % (", ".join(SCROLL_SCREENS.keys())))
        return value

    def _parse_format(self, value):
        if value not in FORMATS:
            raise ValueError("Invalid format: %s" % (", ".join(FORMATS)))
        return value

    def _parse_cc_count(self, value):
        try:
            cc_count = int(value, 0)
        except ValueError:
            raise ValueError("Cannot convert to a valid character color count value")
        if cc_count not in CCC:
            raise ValueError("Invalid character color count: %s" % (", ".join(map(str, CCC))))

    def _parse_pnd_size(self, value):
        try:
            pnd = int(value, 0)
        except ValueError:
            raise ValueError("Cannot convert to a valid pattern name data size value")
        if pnd not in PND:
            raise ValueError("Invalid pattern name data size: %s" % (", ".join(map(str, PND))))

    def _parse_auxiliary_mode(self, value):
        try:
            aux = int(value, 0)
        except ValueError:
            raise ValueError("Cannot convert to a valid auxiliary mode")
        if aux not in AUXILIARY_MODES:
            raise ValueError("Invalid auxiliary mode: %s" % (", ".join(map(str, AUXILIARY_MODES))))

    def _parse_cp_table(self, value):
        try:
            cp_table = int(value, 0)
        except ValueError:
            raise ValueError("Cannot convert to a valid character pattern table address")
        return cp_table

    def _parse_color_palette(self, value):
        try:
            color_palette = int(value, 0)
        except ValueError:
            raise ValueError("Cannot convert to a valid color palette address")
        return color_palette

    def _parse_vcs_table(self, value):
        try:
            return int(value, 0)
        except ValueError:
            raise ValueError("Cannot convert to a valid vertical cell scroll table address")

    def _parse_reduction(self, value):
        print value
        if not re.match(r"(?:^1$)|(?:^1\s*/\s*2$)|(?:^1\s*/\s*4$)", value):
            raise ValueError("Cannot parse reduction value")

class SCRNBitmapFormat:
    def __init__(self, *args):
        self.scroll_screen = args[0]

s = SCRNCellFormat("NBG2", "cell", "16", "1", "0", "0x25E00000", "0x25F80000", "0x00000000", "1/2","0x0","0x0","0x0","0x0")

with open(sys.argv[2], "w+") as ofp:
    try:
        with open(sys.argv[1], "r") as ifp:
            reader = csv.reader(ifp, delimiter = ',')
            # Skip the header
            reader.next()
            for row in reader:
                pass
    except IOError as e:
        print >> sys.stderr, "%s: error: %s" % (PROGNAME, e.strerror)
    except csv.Error as e:
        print >> sys.stderr, "%s: error: File %s, line %i: %s" % (PROGNAME, sys.argv[1], reader.line_num, e)
