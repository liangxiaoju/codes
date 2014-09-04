#!/usr/bin/env python

import os, sys
from xml.etree import ElementTree
from PIL import Image

def tree_to_dict(tree):
    d = {}
    for index, item in enumerate(tree):
        if item.tag == "SubTexture":
            d[item.get("name")] = dict(item.items())
    return d

def gen_png_from_xml(xml_filename, png_filename=None):
    file_path = xml_filename.replace(".xml", "");
    xmlroot = ElementTree.fromstring(open(xml_filename, 'r').read())
    if png_filename == None:
        png_filename = xmlroot.get("imagePath")
    big_image = Image.open(png_filename)
    xmldict = tree_to_dict(xmlroot)

    for k,v in xmldict.items():
        box = (
            int(v["x"]),
            int(v["y"]),
            int(v["x"]) + int(v["width"]),
            int(v["y"]) + int(v["height"]),
            )

        sizelist = [ int(v["width"]), int(v["height"])]

        rect_on_big = big_image.crop(box);
        result_image = Image.new("RGBA", sizelist, (0,0,0,0))
        result_box = (0, 0, int(v["width"]), int(v["height"]))
        result_image.paste(rect_on_big, result_box, mask=0)

        if not os.path.isdir(file_path):
            os.mkdir(file_path)
        outfile = file_path+os.path.sep+k+".png"
        print outfile
        result_image.save(outfile)

if __name__ == "__main__":
    currentPath = os.getcwd()
    xml_filename = currentPath + os.path.sep + sys.argv[1]
    png_filename = xml_filename.replace(".xml", ".png")
    if os.path.exists(xml_filename) and os.path.exists(png_filename):
        gen_png_from_xml(xml_filename, png_filename)
