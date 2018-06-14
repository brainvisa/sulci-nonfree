import numpy
import re

sulcus_surface = ['F.Cal.ant.-Sc.Cal.', 'F.C.M.ant.', 'F.C.M.post.', 'F.Coll.',
                  'F.I.P.', 'F.I.P.Po.C.inf.', 'F.I.P.r.int.1', 'F.I.P.r.int.2', 'F.P.O.',
                  'OCCIPITAL', 'S.Call.', 'S.C.', 'S.C.LPC.', 'S.C.sylvian.', 'S.Cu.',
                  'S.F.inf.ant.', 'S.F.inf.', 'S.F.inter.', 'S.F.int.', 'S.F.marginal.',
                  'S.F.median.', 'S.F.orbitaire.', 'S.F.polaire.tr.', 'S.F.sup.',
                  'S.GSM.', 'S.Li.ant.', 'S.Li.post.', 'S.Olf.', 'S.O.p.', 'S.Or.',
                  'S.O.T.lat.ant.', 'S.O.T.lat.int.', 'S.O.T.lat.med.', 'S.O.T.lat.post.',
                  'S.Pa.int.', 'S.Pa.sup.', 'S.Pa.t.', 'S.p.C.', 'S.Pe.C.inf.',
                  'S.Pe.C.inter.', 'S.Pe.C.marginal.', 'S.Pe.C.median.', 'S.Pe.C.sup.',
                  'S.Po.C.sup.', 'S.Rh.', 'S.R.inf.', 'S.s.P.', 'S.T.i.ant.',
                  'S.T.i.post.', 'S.T.pol.', 'S.T.s.', 'S.T.s.ter.asc.ant.', 'INSULA',
                  'S.T.s.ter.asc.post.', 'unknown', 'ventricle', 'F.C.L.a.', 'F.C.L.p.',
                  'F.C.L.r.diag.']

sulcus_broca = ['F.C.L.r.ant.', 'F.C.L.r.asc.']
sulcus_buried = ['F.C.L.r.retroC.tr.', 'F.C.L.r.sc.ant.', 'F.C.L.r.sc.post.']

orientations_map = {}
orientations_map.update(
    zip(sulcus_surface, ['surface'] * len(sulcus_surface)))
orientations_map.update(zip(sulcus_buried, ['buried'] * len(sulcus_buried)))
orientations_map.update(zip(sulcus_broca, ['broca'] * len(sulcus_broca)))


def get_sulcus_rotation_axe(vertex):
    rootlabel = re.sub("_(left|right)", '', vertex['name'])
    hull_normal = vertex['refhull_normal'].arraydata()
    normal = vertex['refnormal'].arraydata()
    omap = orientations_map[rootlabel]
    o = None
    if omap == 'surface':
        return hull_normal, vertex['hull_normal_weight']
    if omap == 'buried':
        refs = ['F.C.L.a.', 'F.C.L.p.']
    elif omap == 'broca':
        refs = ['INSULA']
    dirs = []
    sizes = []
    for e in vertex.edges():
        if e.getSyntax() != 'junction':
            continue
        v1, v2 = e.vertices()
        if v2['name'] == vertex['name']:
            v2, v1 = v1, v2
        rootlabel2 = re.sub("_(left|right)", '', v2['name'])
        if rootlabel2 not in refs:
            continue
        try:
            dir = e['refdirection'].arraydata()
        except KeyError:
            continue
        dirs.append(dir)
        sizes.append(e['refsize'])
    if not len(sizes):
        return None
    w = numpy.sum(sizes)
    dir = numpy.dot(sizes, dirs) / w
    o = numpy.cross(dir, normal)
    o *= numpy.sign(numpy.dot(o, hull_normal))
    return o, w


def reorient(dirs, mean_dir=None):
    if mean_dir is None:
        return dirs
    dirs = numpy.vstack(dirs)
    s = numpy.sign(numpy.dot(dirs, mean_dir))
    return numpy.sign(numpy.dot(dirs, mean_dir))[None].T * dirs
