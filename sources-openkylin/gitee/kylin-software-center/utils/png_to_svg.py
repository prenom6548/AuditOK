import sys
import os
import shutil
from PIL import Image

def convertPixel(r, g, b, a=1):
    color = "#%02X%02X%02X" % (r, g, b)
    opacity = a
    return (color, opacity)

def png_to_svg(png_file_name,dest_path,pkgname):
    root, ext = os.path.splitext(png_file_name)
    root = dest_path + pkgname

    image = Image.open(png_file_name)
    mode = image.mode
    pixels = image.load()
    width, height = image.size

    if "RGB" in mode:
        output = "<svg width=\"%d\" height=\"%d\" viewBox=\"0 0 %d %d\" xmlns=\"http://www.w3.org/2000/svg\">" % (width,height,width, height)
        for r in range(height):
            print(r)
            for c in range(width):
                color, opacity = convertPixel(*pixels[c, r])
                output += "<rect x=\"%d\" y=\"%d\" width=\"1\" height=\"1\" fill=\"%s\" fill-opacity=\"%s\"/>" % (c, r, color, opacity)

        output += "</svg>"

        with open(root + ".svg", "w") as f:
            f.write(output)

    # shutil.copy(png_file_name,dest_path)
