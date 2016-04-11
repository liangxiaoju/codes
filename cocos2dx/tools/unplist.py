#!/usr/bin/env python
import os,sys
from xml.etree import ElementTree
from optparse import OptionParser
from PIL import Image

def elementTreeToDict(root):
    d = {}
    for index, element in enumerate(root):
        if element.tag == "key":
            key = element.text
            nextElement = root[index + 1]
            if nextElement.tag == "dict":
                d[key] = elementTreeToDict(nextElement)
            elif nextElement.tag == "string":
                if nextElement.text.find("{") >= 0:
                    to_list = lambda x: x.replace('{','').replace('}','').split(',')
                    d[key] = [ int(float(i)) for i in to_list(nextElement.text)]
                else:
                    d[key] = nextElement.text
            elif nextElement.tag in ("integer", "real"):
                d[key] = int(float(nextElement.text))
            elif nextElement.tag in ("true", "false"):
                d[key] = nextElement.tag == "true"
    return d

def extractPlist(plist, outputDir):
    with open(plist, "r") as f:
        elementRoot = ElementTree.fromstring(f.read())

    elementDict = elementTreeToDict(elementRoot[0])

    metadata = elementDict["metadata"]
    png = metadata["realTextureFileName"]
    png = os.path.join(os.path.dirname(plist), png)
    format = metadata["format"]
    size = metadata["size"]

    frames = elementDict["frames"]
    rotated = False

    for name, frame in frames.items():
        rect = [0, 0, 0, 0]
        if format == 2:
            rect = frame["frame"]
            offset = frame["offset"]
            rect[0] += offset[0]
            rect[1] += offset[1]
            rotated = frame["rotated"]
        elif format == 0:
            rect = [frame["x"], frame["y"], frame["width"], frame["height"]]
            offset = [frame["offsetX"], frame["offsetY"]]
            rect[0] += offset[0]
            rect[1] += offset[1]

        size = rect[2:4]
        pasteBox = [0, 0] + size

        if rotated:
            w,h = rect[3],rect[2]
        else:
            w,h = rect[2],rect[3]

        cropBox = [rect[0], rect[1], rect[0]+w, rect[1]+h]

        bigImage = Image.open(png)
        rectImage = bigImage.crop(cropBox)

        if rotated:
            rectImage = rectImage.transpose(Image.ROTATE_90)

        pasteImage = Image.new("RGBA", size, (0,0,0,0))

        try:
            pasteImage.paste(rectImage, pasteBox, mask=0)
        except:
            print name, "Failed to paste:", rectImage.getbbox(), "==>", pasteBox
            continue

        dirname = os.path.join(outputDir, plist.split(".")[0])
        filename = os.path.join(dirname, name)
        subdirname = os.path.dirname(filename)
        if not os.path.isdir(subdirname):
            os.makedirs(subdirname)

        print "Generate: " + filename
        pasteImage.save(filename)


if __name__ == '__main__':
    parser = OptionParser()
    parser.add_option("-d", "--dir", dest="dir",
        help="output directory", default=None)
    parser.add_option("-p", "--plist", dest="plist",
        help=".plist file path")

    (options, args) = parser.parse_args()

    plist = options.plist
    outputDir = options.dir

    if not plist or not os.path.isfile(plist):
        raise Exception("No plist")

    if not outputDir:
        outputDir = os.path.dirname(plist)

    extractPlist(plist, outputDir)

