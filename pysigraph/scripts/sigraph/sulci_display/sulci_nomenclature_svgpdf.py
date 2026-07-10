#!/usr/bin/env python

from soma import aims
import sigraph
from reportlab.graphics.shapes import Drawing, Rect, String, Line
from reportlab.lib.colors import Color
import math
import json


def save_pdf_labels_table(labels, svg_filename):

    if svg_filename.endswith('.svg'):
        from reportlab.graphics import renderSVG as render
    else:
        from reportlab.graphics import renderPDF as render

    width = 600
    height = 400
    left = 20
    top = 20
    line_height = 10
    hwidth = 290
    cwidth = 270
    c0 = 80
    n2 = int(math.ceil(len(labels) / 2))
    n_2 = len(labels) // 2
    fs = 6

    print(n2, n_2)

    d = Drawing(width, height)
    d.add(Line(left + c0, height - top - n2 * line_height,
               left + c0, height - top))
    d.add(Line(left + hwidth + c0,
               height - top - n_2 * line_height,
               left + hwidth + c0, height - top))
    d.add(Rect(left, height - top - n2 * line_height,
               cwidth, n2 * line_height, stroke=1, fillColor=None))
    d.add(Rect(left + hwidth, height - top - n_2 * line_height,
               cwidth, n_2 * line_height, stroke=1, fillColor=None))

    for i, label in enumerate(sorted(labels)):
        desc = labels[label]
        col = int(i / len(labels) * 2)
        row = i % n2
        if row != 0:
            d.add(Line(left + col * hwidth,
                       height - top - row * line_height,
                       left + col * hwidth + cwidth,
                       height - top - row * line_height))
        d.add(Rect(left + col * hwidth,
                   height - top - (row + 1) * line_height, 20,
                   line_height, fillColor=Color(*desc[1])))
        d.add(String(left + col * hwidth + 25,
                     height - top - (row + 1) * line_height + 2, label,
                     fontSize=fs, fontName='Helvetica',
                     fillcolor=[0., 0., 0.]))
        d.add(String(left + col * hwidth + c0 + 5,
                     height - top - (row + 1) * line_height + 2, desc[0],
                     fontSize=fs, fontName='Helvetica'))

    render.drawToFile(d, svg_filename)


ft = sigraph.FoldLabelsTranslator()
ft.readLabels(aims.carto.Paths.findResourceFile(
    'nomenclature/translation/sulci_model_2018.trl'))
nom = aims.read(aims.carto.Paths.findResourceFile(
    'nomenclature/hierarchy/sulcal_root_colors.hie'))


def iter_tree(nom):
    todo = list(nom.children())
    while todo:
        item = todo.pop(0)
        todo = list(item.children()) + todo
        yield item


with open(aims.carto.Paths.findResourceFile(
        'nomenclature/translation/sulci_long_names.json')) as f:
    longnames = json.load(f)


labels = {}
for item in iter_tree(nom):
    label = item.get('name')
    print(label)
    if label is not None:
        tlabel = ft.lookupLabel(label)
        if tlabel is not None:
            color = nom.find_color(tlabel)
            if tlabel.endswith('_left'):
                tlabel = tlabel[:-5]
            elif tlabel.endswith('_right'):
                tlabel = tlabel[:-6]
            longlabel = longnames.get(tlabel)
            if longlabel is None:
                continue
            if longlabel.startswith('Left '):
                longlabel = longlabel[5:]
            elif longlabel.startswith('Right '):
                longlabel = longlabel[6:]
            longlabel = longlabel[0].upper() + longlabel[1:]
            labels[tlabel] = (longlabel, color)

# print(labels)
save_pdf_labels_table(labels, '/tmp/sulci_labels.svg')

