#!/usr/bin/env python3
########################################################################
# Generate an array of 8-bit color values and a 6-bit palette (as an
# 8-bit array) from an 8-bit BMP image.
########################################################################
import numpy
import os
import sys
from PIL import Image

# Check command line arguments.
if len(sys.argv) != 2:
    print(f"Usage: {sys.argv[0]} <BMP file>\n")
    sys.exit(1)

# Get the name of the image without extension.
basename = os.path.splitext(os.path.basename(sys.argv[1]))[0]

# Load the BMP image file.
img = Image.open(sys.argv[1])

# Extract the pixel values.
raw = numpy.array(img.getdata())
if len(raw.shape) != 1:
    print("[ERR] Image does not look like an 8-bit BMP image.")
    sys.exit(1)

# Extract the palette.
pal = numpy.array(img.getpalette(rawmode='RGB'))
if pal is None:
    print("[ERR] No palette information found.")
    sys.exit(1)

# Convert the palette from 8-bit RGB to 6-bit VGA.
pal = (pal * 63) / 255

# Write the pixel values and the palette to disk.
raw.astype('int8').tofile(f"{basename}.raw")
print(f"[LOG] Wrote file: {basename}.raw")
pal.astype('int8').tofile(f"{basename}.pal")
print(f"[LOG] Wrote file: {basename}.pal")
