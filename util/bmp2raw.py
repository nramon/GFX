#!/usr/bin/env python3
################################################################################
# Generate an array of 8-bit color values and a 6-bit palette (as an
# 8-bit array) from an 8-bit BMP image.
#
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.
# 
# This program is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE. See the GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License along with
# this program. If not, see <https://www.gnu.org/licenses/>.
################################################################################
import logging
import numpy
import os
import sys
from PIL import Image

# Configure logging.
logging.basicConfig(format='[%(levelname)s] %(message)s', level=logging.INFO)

# Check command line arguments.
if len(sys.argv) != 2:
    logging.error("Usage: %s <BMP file>", sys.argv[0])
    sys.exit(1)

# Get the name of the image without extension.
basename = os.path.splitext(os.path.basename(sys.argv[1]))[0]

# Load the BMP image file.
img = Image.open(sys.argv[1])

# Extract the pixel values.
raw = numpy.array(img.getdata())
if len(raw.shape) != 1:
    logging.error("Image does not look like an 8-bit BMP image.")
    sys.exit(1)

# Extract the palette.
pal = numpy.array(img.getpalette(rawmode='RGB'))
if pal is None:
    logging.error("No palette information found.")
    sys.exit(1)

# Convert the palette from 8-bit RGB to 6-bit VGA.
pal = (pal * 63) / 255

# Write the pixel values and the palette to disk.
raw.astype('int8').tofile(f"{basename}.raw")
logging.info("Wrote file: %s.raw", basename)
pal.astype('int8').tofile(f"{basename}.pal")
logging.info("Wrote file: %s.pal", basename)
