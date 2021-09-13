#!/usr/bin/env python

from __future__ import print_function
import os
import sys
import numpy
import pickle
import time
import copy
from optparse import OptionParser, OptionGroup
import random
from soma import aims
import sigraph
from sulci.common import io, add_translation_option_to_parser
from sulci.features.descriptors import descriptorFactory


################################################################################
class Observer(object):
    def update(self, tagger):
        state = tagger.getState()
        self.__getattribute__(state)(tagger)

    def label_choosen(self, tagger): pass

    def init(self, tagger): pass

    def label_changed(self, tagger): pass

    def after_pass(self, tagger): pass

    def unchanged(self, tagger): pass


class GuiObserver(Observer):
    def __init__(self):
        Observer.__init__(self)
        import anatomist.direct.api as anatomist
        self._a = anatomist.Anatomist()
        self._win = self._a.createWindow(wintype='3D')
        self._win.setHasCursor(0)
        self._aobjects = []
        self._hie = None
        self._n = 0
        self.load_hie()

    def load_hie(self):
        shared_path = aims.carto.Paths.shfjShared()
        hiename = os.path.join(shared_path, 'nomenclature',
                               'hierarchy', 'sulcal_root_colors.hie')
        self._hie = self._a.loadObject(hiename)

    def init(self, tagger):
        import anatomist.cpp as cpp
        import brainvisa.quaternion as quaternion
        import soma.qt_gui.qt_backend.QtCore as qt
        self._ag = self._a.toAObject(tagger.graph())
        m = aims.Reader().read('/home/mp210984/ccrt/base2008/mesh_white/Lammon_white.tri')
        self._white = self._a.toAObject(m)
        self._aobjects = [self._ag, self._white]
        # default : names color, we want labels
        #self._aobjects = [self._ag]
        self._a.addObjects(self._aobjects, [self._win])
        motion = aims.GraphManip.talairach(self._ag.graph())
        vector = motion.toMatrix()[:3, 3].tolist() + \
            motion.toMatrix()[:3, :3].T.ravel().tolist()
        destination = self._a.centralRef
        origin = self._a.createReferential()
        t = self._a.execute("LoadTransformation",
                            **{'matrix': vector,
                               'origin': origin.getInternalRep(),
                               'destination': destination.getInternalRep()})
        t = self._a.Transformation(self._a, t.trans())
        #if mesh: amesh.setReferential(origin.internalRep)
        self._ag.setReferential(origin.internalRep)
        self._white.setReferential(origin.internalRep)
        c = self._a.getControlWindow()
        c.SelectWindow(self._win.internalRep)
        cpp.ObjectActions.displayGraphChildrenMenuCallback().doit(
            [self._ag.internalRep])
        c.UnselectAllWindows()
        self._a.execute("GraphParams", label_attribute='label')

        self._win.setWindowState(self._win.windowState() |
                                 qt.Qt.WindowFullScreen)
        self._win.setFocus()
        self._win.raiseW()
        q, q2 = quaternion.Quaternion(), quaternion.Quaternion()
        q.fromAxis([0, 1, 0], numpy.pi / 2.)
        q2.fromAxis([1, 0, 0], numpy.pi / 2.)
        q = q.compose(q2)
        self._a.camera(windows=[self._win], zoom=1,
                       view_quaternion=q.vector(), force_redraw=True)
        imagename = 'snapshot.jpg'
        self._a.execute("WindowConfig", windows=[self._win],
                        record_mode=1, record_basename=imagename)
        # build the video :
        # mencoder "mf://*.jpg" -mf fps=5 -ovc lavc -o plop.avi

    def after_pass(self, tagger):
        import soma.qt_gui.qt_backend.QtGui as qt
        for s in tagger._segments:
            id = s['index']
            tag_label = tagger._taglabels[id]
            s['label'] = tagger._availablelabels[id][tag_label]
        self._ag.updateColors()
        self._ag.setChanged()
        self._ag.notifyObservers()
        qt.qApp.processEvents()
        time.sleep(0.001)
        desktop_geometry = qt.qApp.desktop().geometry()
        self._n += 1


class GuiLabelsObserver(GuiObserver):
    def __init__(self):
        GuiObserver.__init__(self)

    def init(self, tagger):
        GuiObserver.init(self, tagger)
        self._ag.setColorMode(self._ag.Normal)
        self._ag.updateColors()
        self._ag.notifyObservers()
        self._ag.setChanged()

    def after_pass(self, tagger):
        import soma.qt_gui.qt_backend.QtGui as qt
        for s in tagger._segments:
            id = s['index']
            tag_label = tagger._taglabels[id]
            s['label'] = tagger._availablelabels[id][tag_label]
        self._ag.updateColors()
        self._ag.setChanged()
        self._ag.notifyObservers()
        qt.qApp.processEvents()
        time.sleep(0.001)
        self._n += 1


class GuiNrjObserver(GuiObserver):
    def __init__(self):
        GuiObserver.__init__(self)

    def init(self, tagger):
        GuiObserver.init(self, tagger)
        self._ag.setColorMode(self._ag.PropertyMap)
        self._ag.setColorProperty('nrj')
        self._ag.updateColors()
        self._ag.notifyObservers()
        self._ag.setChanged()

    def after_pass(self, tagger):
        import soma.qt_gui.qt_backend.QtGui as qt
        for s in tagger._segments:
            id = s['index']
            if tagger._changes[id] or self._n == 0:
                p = numpy.exp(-tagger.local_energy(id))  # / \
                # tagger._temp)
                proba = p[tagger._taglabels[id]] / p.sum()
                s['nrj'] = max(min(proba, 1.), 0.0)
                #en = tagger.local_energy(id)
                #s['nrj'] = en[tagger._taglabels[id]]
        #palette = self._ag.getOrCreatePalette()
        # palette.setMin1(0.)
        # palette.setMax1(1.)
        self._ag.setInternalsChanged()
        self._ag.updateAfterAimsChange()
        self._ag.updateColors()
        self._ag.setChanged()
        self._ag.notifyObservers()
        # self._a.setObjectPalette([self._ag], 'Blue-Red',
        #	minVal=0., maxVal=1., absoluteMode=1)
        qt.qApp.processEvents()
        time.sleep(0.001)
        self._n += 1


class GuiLabelsChangedObserver(GuiLabelsObserver):
    def __init__(self):
        GuiLabelsObserver.__init__(self)

    def init(self, tagger):
        GuiLabelsObserver.init(self, tagger)

    def after_pass(self, tagger): pass

    def label_changed(self, tagger):
        GuiLabelsObserver.after_pass(self, tagger)


class GuiChangesObserver(GuiObserver):
    def __init__(self):
        GuiObserver.__init__(self)

    def init(self, tagger):
        GuiObserver.init(self, tagger)
        self._ag.setColorMode(self._ag.PropertyMap)
        self._ag.setColorProperty('changes')
        self._ag.updateColors()
        self._ag.notifyObservers()
        self._ag.setChanged()

    def after_pass(self, tagger):
        import soma.qt_gui.qt_backend.QtGui as qt
        for s in tagger._segments:
            id = s['index']
            s['changes'] = tagger._changes[id]
        self._ag.updateColors()
        self._ag.setChanged()
        self._ag.notifyObservers()
        qt.qApp.processEvents()
        time.sleep(0.001)
        self._n += 1


class SegmentObserver(Observer):
    def __init__(self, indices):
        Observer.__init__(self)
        self._indices = indices

    def label_changed(self, tagger):
        def proba(en):
            P = numpy.exp(-en)
            P /= P.sum()
            return P

        def s(a):
            return "[" + ', '.join(("%2.3f" % x) for x in a) + "]"

        id1 = tagger.getCurrenSegmentID()
        if id1 not in self._indices:
            return
        print(" -- label has changed on segment n.%d --" % id1)
        print(" - relations : ")
        availablelabels = tagger.getAvailableLabels(id1)
        availablelabels_n = len(availablelabels)
        if availablelabels_n == 1:
            return 0, 0
        relcum = numpy.zeros(availablelabels_n, dtype=numpy.longdouble)
        w = tagger.getSegWeights(id1)
        if not w:
            w = 1.
        neighbours_n = len(tagger.getNeighbours(id1))
        for id2 in tagger.getNeighbours(id1):
            l2 = tagger.tagLabel(id2)
            P12 = tagger.relPotential(id1, id2)
            P12_id2 = P12[:, l2].T
            relcum += P12_id2
            label = tagger.getSegmentLabel(id2)
            print("    n.%3d : %s %s (%s) " % (id2, s(P12_id2 / w),
                                               s(proba(neighbours_n * P12_id2 / w)), label))
        relcum /= w
        print("  =        ", s(relcum), s(proba(relcum)))
        priors = tagger.eval_priors(id1) / w
        print(" - priors :", s(priors), s(proba(priors)))
        seg_pot = tagger.segPotential(id1) / w
        print(" - seg :   ", s(seg_pot), s(proba(seg_pot)))
        en = relcum + priors + seg_pot
        print(" - En :    ", s(en), s(proba(en)))
        print(" - Labels :", availablelabels)

    def label_choosen(self, tagger):
        print(" -- chosen labels --")
        for id in self._indices:
            label = tagger.getSegmentLabel(id)
            print("n.%d : %s" % (id, label))


################################################################################
class Tagger(object):
    def __init__(self, sulcimodel, init_with_segments, init_prior_distr,
                 graph, motion, nrjname, csvname, selected_sulci,
                 node_indices, anneal_opt, normalize_weights):
        self._sulcimodel = sulcimodel
        self._init_with_segments = init_with_segments
        self._init_prior_distr = init_prior_distr
        self._segments_distrib = sulcimodel.segments_distrib()
        if self._segments_distrib:
            segments_data_type = self._segments_distrib['data_type']
            self._segments_descriptor = \
                descriptorFactory(segments_data_type, 'segments')
        else:
            self._segments_descriptor = None
        self._relations_distrib = sulcimodel.relations_distrib()
        if self._relations_distrib:
            relations_data_type = self._relations_distrib['data_type']
            self._relations_descriptor = \
                descriptorFactory(relations_data_type, 'relations')
        else:
            self._relations_descriptor = None
        self._sulci_distrib = sulcimodel.sulci_distrib()
        if self._sulci_distrib:
            sulci_data_type = self._sulci_distrib['data_type']
            self._sulci_descriptor = descriptorFactory(
                sulci_data_type, 'sulci')
        else:
            self._sulci_descriptor = None
        self._labels_prior = sulcimodel.labels_prior()
        self._states = sulcimodel.labels()
        self._states_n = len(self._states)
        self._graph = graph
        self._motion = aims.GraphManip.talairach(graph)
        if motion:
            self._motion = motion * self._motion
        self._selected_sulci = selected_sulci
        self._node_indices = node_indices
        self._anneal_opt = anneal_opt
        self._observers = []
        self._state = 'init'
        # output nrj file
        self._nrj_fd = open(nrjname, 'w')
        self._nrj_fd.write("Iteration\tTemperature\tEnergy\tChanges\n")
        self._csvname = csvname
        self._normalize_weights = normalize_weights
        self._current_seg_id = None

    def addObserver(self, observer):
        self._observers.append(observer)

    def getState(self): return self._state

    def notifyObservers(self):
        for o in self._observers:
            o.update(self)

    def getCurrenSegmentID(self): return self._current_seg_id

    def getSegmentLabel(self, id):
        return self._availablelabels[id][self._taglabels[id]]

    def getAvailableLabels(self, id):
        return self._availablelabels[id]

    def graph(self): return self._graph

    def getNeighbours(self, id):
        return self._neighbours[id]

    def tagLabel(self, id):
        return self._taglabels[id]

    def relPotential(self, id1, id2):
        if id1 > id2:
            P12 = self._rel_potentials[id2, id1].T
        else:
            P12 = self._rel_potentials[id1, id2]
        return P12

    def segPotential(self, id1):
        return self._seg_potentials[id1]

    def getSegWeights(self, id1):
        return self._seg_weights[id1]

    def initialize_prior(self):
        if not self._labels_prior:
            return
        prior_data_type = self._labels_prior['data_type']
        if prior_data_type in ['size_generalized_dirichlet',
                               'size_dirichlet']:
            self._prior_descriptor = \
                descriptorFactory('size_global_frequency')
        elif prior_data_type in ['label_generalized_dirichlet',
                                 'label_dirichlet']:
            self._prior_descriptor = \
                descriptorFactory('label_global_frequency')
        elif prior_data_type in ['size_frequency', 'label_frequency']:
            self._prior_descriptor = \
                descriptorFactory('local_frequency')
        else:
            self._prior_descriptor = None
            return
        self._prior_distrib = self._labels_prior['prior']
        # FIXME : marche uniquement avec les priors Freq
        # les autres priors ne fonctionnent plus
        priors = numpy.ravel(numpy.asarray(
            self._prior_distrib.frequencies()))
        prior_labels = self._labels_prior['labels']
        self._prior_descriptor.compute_data(self._graph,
                                            self._taglabels, self._availablelabels,
                                            self._seg_indices, prior_labels)
        if self._selected_sulci is None:
            p = numpy.array(prior_labels)
            s = numpy.array(self._states)
            if self._states != prior_labels:
                if len(self._states) != \
                        len(prior_labels):
                    print("error : labels size " +
                          "differs between " +
                          "sulcimodel and prior.")
                    sys.exit(1)
                print("warning : labels order differs" +
                      " between sulcimodel and " +
                      "prior : order fixed.")
                indices = [numpy.argwhere(p == x)[0, 0]
                           for x in s]
                priors = numpy.asarray(priors)[indices]
        else:
            indices = []
            for sulcus in self._states:
                ind = numpy.argwhere(numpy.array(
                    [x == sulcus
                        for x in prior_labels]))[0, 0]
                indices.append(ind)
            priors = numpy.asarray(priors)[0][indices]
        self._prior_distrib.set_frequencies(priors)

    def initialize_init_prior(self):
        if not self._init_prior_distr:
            return
        d = self._init_prior_distr['prior']
        priors = numpy.ravel(numpy.asarray(
            d.frequencies()))
        prior_labels = self._init_prior_distr['labels']
        if self._selected_sulci is None:
            p = numpy.array(prior_labels)
            s = numpy.array(self._states)
            if self._states != prior_labels:
                if len(self._states) != \
                        len(prior_labels):
                    print("error : labels size " +
                          "differs between " +
                          "sulcimodel and prior.")
                    sys.exit(1)
                print("warning : labels order differs" +
                      " between sulcimodel and " +
                      "prior : order fixed.")
                indices = [numpy.argwhere(p == x)[0, 0]
                           for x in s]
                priors = numpy.asarray(priors)[indices]
        else:
            indices = []
            for sulcus in self._states:
                ind = numpy.argwhere(numpy.array(
                    [x == sulcus
                        for x in prior_labels]))[0, 0]
                indices.append(ind)
            priors = numpy.asarray(priors)[0][indices]
        self._init_priors = priors

    def precompute_from_data(self, init_mode, weighting_mode,
                             select_mode, picklename=None):
        segdescr = self._segments_descriptor
        reldescr = self._relations_descriptor
        if self._segments_distrib:
            segdistr = self._segments_distrib['vertices']
        else:
            segdistr = None
        if self._relations_distrib:
            reldistr = self._relations_distrib['edges']
        else:
            reldistr = None

        print("compute initialization...")
        self._availablelabels = {}
        self._labelsind = {}
        self._taglabels = {}
        self._seg_indices = []
        self._neighbours = {}
        self._seg_potentials = {}
        self._seg_weights = {}
        seg_edges = {}
        for s in self._segments:
            node_index = s['index']
            self._neighbours[node_index] = set()
            logli = numpy.zeros(self._states_n, dtype=numpy.longdouble)
            li = numpy.zeros(self._states_n, dtype=numpy.longdouble)
            if segdistr:
                for i, label in enumerate(self._states):
                    distrib = segdistr[label]
                    logli[i], li[i] = segdescr.likelihood(
                        distrib, self._motion, s)
                if self._init_prior_distr:
                    li *= self._init_priors
                p = li / li.sum()
            else:
                p = li
            if select_mode == 'threshold':
                sel = p >= 0.01
            elif select_mode == 'all':
                sel = (p == p)
            elif select_mode != 'store_labels':
                sel = [(t == s['label'])
                       for i, t in enumerate(self._states)]
            if not self._init_with_segments:
                self._seg_potentials[node_index] = -logli[sel]
            else:
                self._seg_potentials[node_index] = li[sel] * 0.
            labels = [t for i, t in enumerate(self._states) if sel[i]]
            self._availablelabels[node_index] = labels
            self._labelsind[node_index] = [i for i, t in
                                           enumerate(self._states) if sel[i]]
            if init_mode == 'store_labels':
                init_label = s['label']
            elif init_mode == 'segments_potentials':
                init_label = self._states[numpy.argmax(p)]
            elif init_mode == 'random':
                r = numpy.random.randint(0, len(labels))
                init_label = labels[r]
            # convert indice from all labels to available labels set
            self._taglabels[node_index] = [i for i, l in
                                           enumerate(labels) if (l == init_label)][0]
            if ((self._node_indices is None) or
                    (node_index in self._node_indices)):
                self._seg_indices.append(node_index)
            self._seg_weights[node_index] = 0.
            seg_edges[node_index] = []

        print("compute potentials and neighbourhood...")
        if reldescr:
            graph_edges = reldescr.edges_from_graph(self._graph)
            if weighting_mode == 'number':
                for edge_infos in graph_edges.items():
                    (r1, r2), ((v1, v2), edges) = edge_infos
                    seg_edges[r1].append(edges)
                    seg_edges[r2].append(edges)
            self._rel_potentials = {}
            for edge_infos in graph_edges.items():
                (r1, r2), ((v1, v2), edges) = edge_infos
                self._neighbours[r1].add(r2)
                self._neighbours[r2].add(r1)
                # relation potential
                P12, indices = reldescr.potential_matrix(self._motion,
                                                         reldistr, edge_infos, self._availablelabels)
                # segment potential
                self._rel_potentials[indices] = P12
                if weighting_mode == 'sizes':
                    w = v1['refsize'] * v2['refsize']
                elif weighting_mode == 'contact_area':
                    if edges.has_key('cortical'):
                        w = edges['cortical']['reflength']
                    elif edges.has_key('junction'):
                        # note : some buried segments may have cortical
                        # relation missing because the voronoi used to
                        # define the relation is only define in surface.
                        # In this case, I choose to use the length of
                        # the contact area between the 2 segments rather
                        # than those between the 2 voronoi ROIs.
                        w = edges['junction']['reflength']
                    else:  # note : strange rare relations
                        w = 0.
                elif weighting_mode in ['number', 'none']:
                    w = 1.
                self._rel_potentials[indices] *= w
                self._seg_weights[r1] += w
                self._seg_weights[r2] += w
            if weighting_mode != 'none':
                for ind, p in self._seg_potentials.items():
                    self._seg_potentials[ind] *= \
                        self._seg_weights[ind]
            else:
                self._normalize_weights = False
        else:
            self._rel_potentials = None

        if picklename:
            fd = open(picklename, 'w')
            obj = (self._availablelabels, self._labelsind,
                   self._taglabels, self._seg_indices,
                   self._neighbours, self._seg_potentials,
                   self._rel_potentials, self._seg_weights)
            pickle.dump(obj, fd)
            fd.close()

    def precompute_from_pickle(self, filename):
        print("read from precomputing data from '%s'" % filename)
        fd = open(filename, 'r')
        obj = pickle.load(fd)
        (self._availablelabels, self._labelsind, self._taglabels,
         self._seg_indices, self._neighbours,
         self._seg_potentials, self._rel_potentials,
         self._seg_weights) = obj
        fd.close()

    def restore_state(self):
        self._taglabels = self._best_taglabels
        if self._labels_prior:
            self._prior_descriptor.set_data(self._best_prior_data)

    def store_state(self, en):
        self._best_en = en
        self._best_taglabels = copy.copy(self._taglabels)
        if self._labels_prior:
            self._best_prior_data = copy.copy(
                self._prior_descriptor.get_data())

    def tag(self, mode='icm', init_mode='store_labels',
            weighting_mode='none', select_mode='threshold',
            precomputing='from_data', picklename=None):
        if mode in ['energy', 'error']:
            select_mode = 'store_labels'
            init_mode = 'store_labels'
        self._segments = []
        for xi in self._graph.vertices():
            if xi.getSyntax() != 'fold':
                continue
            sulcus = xi['name']
            self._segments.append(xi)

        self.initialize_init_prior()
        # init data / potentials
        if precomputing == 'from_data':
            self.precompute_from_data(init_mode, weighting_mode,
                                      select_mode, picklename)
        elif precomputing == 'from_pickle':
            self.precompute_from_pickle(picklename)
        self.initialize_prior()
        self.init_sulci_potentials()

        # infered labels
        self.algo(mode)

        # fill graph with infered labels
        for s in self._segments:
            id = s['index']
            tag_label = self._taglabels[id]
            s['label'] = self._availablelabels[id][tag_label]
        self._state = 'label_choosen'
        self.notifyObservers()
        self.write_csv()

    def algo(self, mode='icm'):
        self.notifyObservers()
        self._changes = {}
        for id in self._seg_indices:
            self._changes[id] = 0.
        if mode == 'init':
            return
        elif mode == 'icm':
            return self.algo_icm()
        elif mode == 'mpm':
            return self.algo_mpm()
        elif mode == 'sa':
            return self.algo_simulated_annealing()
        elif mode == 'energy':
            print("energy = ", self.energy())
        elif mode == 'error':
            pass  # FIXME

    def algo_simulated_annealing(self):
        print("start simulated annealing...")
        self._temp, rate, stopRate, tmax = self._anneal_opt
        segments_n = len(self._segments)
        en = self.energy()
        self._best_en = numpy.inf
        self._best_taglabels = copy.copy(self._taglabels)
        self._current_seg_id = None
        t = 0
        while 1:
            numpy.random.shuffle(self._seg_indices)
            chgmt = 0.
            for id in self._seg_indices:
                self._current_seg_id = id
                c, d = self.gibbs(id)
                self._changes[id] += c
                chgmt += c
                en -= d
                if c:
                    self._state = 'label_changed'
                else:
                    self._state = 'unchanged'
                self.notifyObservers()
            self._state = 'after_pass'
            self.notifyObservers()
            chgmt /= segments_n
            print("t = %d, temp = %3.4f, en = %3.3f, chgmt = %2.2f"
                  % (t, self._temp, en, chgmt))
            self._nrj_fd.write("%d\t%3.4f\t%3.3f\t%2.2f\n" % (t,
                                                              self._temp, en, chgmt))
            t += 1
            self._temp *= rate
            if en < self._best_en:
                self.store_state(en)
            if chgmt < stopRate:
                break
            if t > tmax:
                break
        self.restore_state()
        self.algo_icm()

    def algo_icm(self):
        print("start icm...")
        self._temp, rate, stopRate, tmax = self._anneal_opt
        en = self.energy()
        best_en = numpy.inf
        best_taglabels = copy.copy(self._taglabels)
        self._current_seg_id = None
        t = 0
        while 1:
            numpy.random.shuffle(self._seg_indices)
            chgmt = 0.
            for id in self._seg_indices:
                self._current_seg_id = id
                c, d = self.icm(id)
                self._changes[id] += c
                chgmt += c
                en -= d
                if c:
                    self._state = 'label_changed'
                else:
                    self._state = 'unchanged'
                self.notifyObservers()
            self._state = 'after_pass'
            self.notifyObservers()
            print("t = %d, en = %3.3f, chgmt = %2.2f" % (t,
                                                         en, chgmt))
            t += 1
            if en < best_en:
                self.store_state(en)
            if chgmt == 0:
                break
            if t > tmax:
                break
        self.restore_state()

    def algo_mpm(self):
        print("start mpm...")
        self._freq = {}
        for id in self._seg_indices:
            size = len(self._availablelabels[id1])
            self._freq[id] = numpy.zeros(size, dtype=numpy.longdouble)
        self._temp, rate, stopRate, tmax = self._anneal_opt
        self._current_seg_id = None
        t = 0
        print("burning period...")
        self._state = 'running'
        while 1:
            numpy.random.shuffle(self._seg_indices)
            if t >= 100:
                if t == 100:
                    print("start computing frequencies...")
                for id in self._seg_indices:
                    self._current_seg_id = id
                    self.gibbs(id)
                    self._freq[id][self._taglabels[id]] += 1
                    self.notifyObservers()
            self._state = 'after_pass'
            self.notifyObservers()
            print("t = %d" % t)
            t += 1
            if t > tmax:
                break
        for id in self._seg_indices:
            self._taglabels[id] = numpy.argmax(self._freq[id])

    def energy(self):
        en_rel = en_seg = 0.
        for id1 in self._seg_indices:
            l1 = self._taglabels[id1]
            for id2 in list(self._neighbours[id1]):
                l2 = self._taglabels[id2]
                if id1 > id2:
                    P12 = self._rel_potentials[id2, id1].T
                else:
                    P12 = self._rel_potentials[id1, id2]
                en_rel += P12[l1, l2]
            P1 = self._seg_potentials[id1]
            en_seg += P1[l1]
        en_rel /= 2.
        if self._sulci_descriptor:
            en_sulci = self._sulci_en.sum()
        else:
            en_sulci = 0.
        print(en_rel, en_seg, self.eval_prior(), en_sulci)
        return en_rel + en_seg + self.eval_prior() + en_sulci

    def local_energy(self, id1):
        en = numpy.longdouble(0.)
        for id2 in self._neighbours[id1]:
            l2 = self._taglabels[id2]
            if id1 > id2:
                P12 = self._rel_potentials[id2, id1].T
            else:
                P12 = self._rel_potentials[id1, id2]
            en += P12[:, l2].T
        priors = self.eval_priors(id1)
        en += self._seg_potentials[id1] + priors
        en += self.eval_sulci_potentials(id1)
        return en

    def gibbs(self, id1):
        if len(self._availablelabels[id1]) == 1:
            return 0, 0
        l1 = self._taglabels[id1]
        en = self.local_energy(id1)
        delta_e = en[l1] - en
        if self._normalize_weights:
            w = self._seg_weights[id1]
            if not w:
                w = 1.
            p = numpy.exp(delta_e / (self._temp * w))
        else:
            p = numpy.exp(delta_e / self._temp)
        p = p.cumsum()
        p /= p[-1]
        r = numpy.random.uniform(0, 1)
        l2 = (p < r).sum()  # new label
        chgmt = (l2 != l1)
        self._taglabels[id1] = l2
        self.update(id1, l1, l2)

        return chgmt, delta_e[l2]

    def icm(self, id1):
        if len(self._availablelabels[id1]) == 1:
            return 0, 0
        l1 = self._taglabels[id1]
        en = self.local_energy(id1)
        delta_e = en[l1] - en
        l2 = numpy.argmin(en)  # new label
        chgmt = (l2 != l1)
        self._taglabels[id1] = l2
        self.update(id1, l1, l2)
        return chgmt, delta_e[l2]

    def init_sulci_potentials(self):
        if not self._sulci_descriptor:
            return
        print("init sulci potentials")
        self._sulci_local_en = {}
        label_segments = {}
        for label in range(self._states_n):
            label_segments[label] = {}
        for s in self._segments:
            id = s['index']
            label = self._labelsind[id][self._taglabels[id]]
            label_segments[label][id] = s
        self._label_segments = label_segments
        self._sulci_descriptor.init(self._motion, self._sulci_distrib,
                                    self._states, self._segments)

        # init energy
        self._sulci_en = numpy.zeros(self._states_n, dtype=numpy.longdouble)
        d = self._sulci_descriptor
        for i in range(self._states_n):
            self._sulci_en[i] = d.likelihood(self._sulci_distrib,
                                             i, self._label_segments[i])

    def eval_sulci_potentials(self, id):
        if not self._sulci_descriptor:
            return 0.
        l1 = self._taglabels[id]
        tl1 = self._labelsind[id][l1]
        en = numpy.zeros(len(self._labelsind[id]), dtype=numpy.longdouble)
        en2 = numpy.array(self._sulci_en)
        en2[tl1] = 0.

        segments_tl1 = copy.copy(self._label_segments[tl1])
        seg = segments_tl1[id]
        del segments_tl1[id]

        en_tl1 = self._sulci_descriptor.likelihood(self._sulci_distrib,
                                                   tl1, segments_tl1)
        for i, tl2 in enumerate(self._labelsind[id]):
            segments_tl2 = copy.copy(self._label_segments[tl2])
            segments_tl2[id] = seg
            en_tl2 = self._sulci_descriptor.likelihood(
                self._sulci_distrib, tl2, segments_tl2)
            self._sulci_local_en[tl2] = en_tl2
            en3 = numpy.array(en2)
            en3[tl2] = 0.
            if tl1 != tl2:
                en[i] = en3.sum() + en_tl1 + en_tl2
            else:
                en[i] = en3.sum() + en_tl2
        return en

    def eval_priors(self, id):
        '''
id : segment id
        '''
        if not self._labels_prior:
            return 0.
        l1 = self._taglabels[id]
        en = numpy.zeros(len(self._labelsind[id]), dtype=numpy.longdouble)
        if self._prior_descriptor:
            old_l2 = l1 = self._labelsind[id][l1]
            for i, l2 in enumerate(self._labelsind[id]):
                self._prior_descriptor.update_data(
                    id, old_l2, l2)
                logli = self._prior_descriptor.likelihood(
                    self._prior_distrib, l2)
                en[i] = -logli
                old_l2 = l2
            self._prior_descriptor.update_data(id, old_l2, l1)
            en *= self._seg_weights[id]  # new behaviour
        return en

    def eval_prior(self):
        if not self._labels_prior:
            return 0.
        en = 0.
        if self._prior_descriptor:
            logli = self._prior_descriptor.full_likelihood(
                self._prior_distrib, self._taglabels)
            en = -logli
        return en

    def update(self, id, l1, l2):
        '''
id : segment id
l1 : local label id (old)
l2 : local label id (new)
        '''
        if not self._labels_prior and not self._sulci_descriptor:
            return
        # true labels indices
        l1 = self._labelsind[id][l1]
        l2 = self._labelsind[id][l2]
        if self._labels_prior:
            self._prior_descriptor.update_data(id, l1, l2)
        if self._sulci_descriptor:
            s = self._label_segments[l1][id]
            del self._label_segments[l1][id]
            self._label_segments[l2][id] = s
            self._sulci_en[l1] = self._sulci_local_en[l1]
            self._sulci_en[l2] = self._sulci_local_en[l2]

    def write_csv(self):
        fd = open(self._csvname, 'w')
        s = "nodes\tproba"
        for si in range(self._states_n):
            s += '\tproba_' + self._states[si]
        fd.write(s + '\n')
        P = numpy.zeros(self._states_n)
        for id1 in self._seg_indices:
            l1 = self._taglabels[id1]
            availablelabels_n = len(self._availablelabels[id1])
            en = numpy.zeros(availablelabels_n, dtype=numpy.longdouble)
            for id2 in self._neighbours[id1]:
                l2 = self._taglabels[id2]
                if id1 > id2:
                    P12 = self._rel_potentials[id2, id1].T
                else:
                    P12 = self._rel_potentials[id1, id2]
                en += P12[:, l2].T
            priors = self.eval_priors(id1)
            en += self._seg_potentials[id1] + priors
            en /= self._seg_weights[id1]
            P[:] = 0.
            for i, ind in enumerate(self._labelsind[id1]):
                P[ind] = numpy.exp(-en[i])
            P /= P.sum()
            m = P.max()
            s = '\t'.join(str(e) for e in P) + '\n'
            fd.write('%d\t%f\t' % (id1, m) + s)


################################################################################
class OptionParser2(OptionParser):
    def check_required(self, opt):
        option = self.get_option(opt)

        # Assumes the option's 'default' is set to None!
        if getattr(self.values, option.dest) is None:
            self.error("%s option not supplied" % option)

    def check(self):
        option1 = self.get_option('--distrib_segments')
        option2 = self.get_option('--distrib_relations')
        option3 = self.get_option('--distrib_sulci')
        if (getattr(self.values, option1.dest) is None) and \
                (getattr(self.values, option2.dest) is None) and \
                (getattr(self.values, option3.dest) is None):
            self.error("need at least one model among segments " +
                       ", relations and sulci.")


def parseOpts(argv):
    description = 'Simulated annealing to tag sulci.'
    parser = OptionParser2(description)
    add_translation_option_to_parser(parser)

    # inputs data
    data_group = OptionGroup(parser, "Input data")
    data_group.add_option('-i', '--ingraph', dest='input_graphname',
                          metavar='FILE', action='store', default=None,
                          help='data graph')
    data_group.add_option('--motion', dest='motion',
                          metavar='FILE', action='store', default=None,
                          help='motion file (.trm) from Talairach to the ' +
                          'space of segments model')
    parser.add_option_group(data_group)

    # input models
    models_group = OptionGroup(parser, "Input models")
    models_group.add_option('-d', '--distrib_segments',
                            dest='distribsegmentsname', metavar='FILE', action='store',
                            default=None, help='distribution models for segments ' +
                            '(default : no model)')
    models_group.add_option('--distrib_sulci',
                            dest='distribsulciname', metavar='FILE', action='store',
                            default=None, help='distribution models for sulci ' +
                            '(default : no model)')
    models_group.add_option('--distrib_relations', dest='distribrelname',
                            metavar='FILE', action='store', default=None,
                            help='distribution models for relations (default : no model)')
    models_group.add_option('-p', '--prior', dest='priorname',
                            metavar='FILE', action='store', default=None,
                            help='prior file (default : no prior)')
    models_group.add_option('--init-prior', dest='init_priorname',
                            metavar='FILE', action='store', default=None,
                            help='prior model for label initialization ' +
                            '(default : no prior)')
    models_group.add_option('--init-with-segments',
                            dest='init_with_segments', action='store_true', default=False,
                            help='use distribution models for segments only to ' +
                            'initialize labels and select which labels are available ' +
                            'on each segment')
    parser.add_option_group(models_group)

    # outputs
    outputs_group = OptionGroup(parser, "Outputs")
    outputs_group.add_option('-o', '--outgraph', dest='output_graphname',
                             metavar='FILE', action='store', default='output.arg',
                             help='output tagged graph (default : %default)')
    outputs_group.add_option('--nrj', dest='nrj', metavar='FILE',
                             action='store', default='output.nrj',
                             help='output markov field energy along temperature decrease')
    outputs_group.add_option('-c', '--csv', dest='csvname',
                             metavar='FILE', action='store',
                             default='output.csv',
                             help='csv storing posterior probabilities (default : %default)')
    parser.add_option_group(outputs_group)

    # algo options
    algo_group = OptionGroup(parser, "Algorithms options")
    algo_group.add_option('--mode', dest='mode',
                          metavar='MODE', action='store', default='icm', type='choice',
                          help="'init' : only initialisation with segments potential " +
                          "is done, 'icm' : init + Iterated Conditional Mode, 'mpm' : " +
                          "init + Marginal Posterior Mode, 'sa' : init + Simulated " +
                          "Annealing), 'energy' : compute energy of labels store in " +
                          "'label' attribute, 'error' : compute the mean posterior " +
                          "error of labels stored in the input graph attribute 'label' " +
                          "according to true labels store in the input graph attribute " +
                          "'name'", choices=('icm', 'sa', 'mpm', 'init',
                                             'energy', 'error'))
    algo_group.add_option('--init-mode', dest='init_mode',
                          metavar='MODE', action='store', default='store_labels',
                          type='choice', help="initialization of labels. " +
                          "'segments_potentials': argmax from segments potentials, " +
                          "'store_labels' : from data graph label attributes, " +
                          "'random' : random initialization from available labels",
                          choices=('segments_potentials', 'store_labels', 'random'))
    algo_group.add_option('--select-mode', dest='select_mode',
                          metavar='MODE', action='store', default='threshold',
                          type='choice', help="select available labels on each " +
                          "segment. 'threshold' : select labels with posterior of " +
                          "local segment potential over the threshold, 'all' : keep all" +
                          " labels defined in segment models, 'store_labels' : select " +
                          "only the labels stored in 'label' attribute of the " +
                          "input graph", choices=('threshold', 'all', 'store_labels'))
    algo_group.add_option('--init-temperature', dest='init_temp',
                          metavar='FLOAT', action='store', default='50', type='float',
                          help='initial temperature for simulated annealing ' +
                          '(default : %default)')
    algo_group.add_option('--temperature-rate', dest='temp_rate',
                          metavar='FLOAT', action='store', default='0.995', type='float',
                          help='decreasing rate for temperature in simulated annealing ' +
                          '(default : %default)')
    algo_group.add_option('--stop-rate', dest='stop_rate', metavar='FLOAT',
                          action='store', default='0.02', type='float',
                          help='rate of label modified over the whole segments on ' +
                          'one algo pass. Below this rate, the  simulated annealing ' +
                          'is stopped (default : %default)')
    algo_group.add_option('--tmax', dest='tmax', metavar='FLOAT',
                          action='store', default='10000', type='float',
                          help='max number of iteration (for all modes) (default : ' +
                          '%default)')
    parser.add_option_group(algo_group)

    # misc
    misc_group = OptionGroup(parser, "Miscelleneous")
    misc_group.add_option('-s', '--sulci', dest='sulci',
                          metavar='LIST', action='store', default=None,
                          help='tag only specified manually tagged sulci.')
    misc_group.add_option('-n', '--nodes', dest='node_indices',
                          metavar='INDEX', action='store', default=None, type='str',
                          help='init label on all segments, then start algo ' +
                          'only on specified segments (according to their indices)')
    misc_group.add_option('--look-at-segments', dest='lookat_seg_id',
                          metavar='LIST', action='store', default=None,
                          help='monitor information on the specified segments ' +
                          '(according to their indices) : local energies, current ' +
                          'label, local probabilities.')
    misc_group.add_option('--precomputing-mode', dest='precomputing',
                          metavar='MODE', action='store', default='from_data',
                          type='choice', choices=('from_data', 'from_pickle'),
                          help="'from_data' : based on input data graph, " +
                          "'from_pickle' : based on pickled information from another " +
                          "run of this program (see --picklename) (default: %default)")
    misc_group.add_option('--picklename', dest='picklename',
                          metavar='FILE', action='store', default=None,
                          help="if specified precomputed data are stored in this file " +
                          "and can be reused with --precomputing-mode from_pickle")
    misc_group.add_option('--gui', dest='gui', action='store_true',
                          default=False, help="display and save images of labels " +
                          "with anatomist during labeling process")
    misc_group.add_option('--gui-mode', dest='gui_mode', action='store',
                          type='choice', choices=('labels', 'labels_changed', 'changes',
                                                  'nrj'),	default='labels', help="'labels': display labels " +
                          "(refresh after each pass), 'labels_changed': display labels " +
                          "(refresh after each change), 'changes': display " +
                          "number of changes, 'nrj': display local nrj")
    misc_group.add_option('--weighting-mode', dest='weighting_mode',
                          metavar='MODE', action='store', default='none', type='choice',
                          choices=('contact_area', 'sizes', 'number', 'none'),
                          help="to weight segments potential and relations ones " +
                          "from each other (to take into accounte unbalance number " +
                          "of relations). 'contact_area' : the length of the contact " +
                          "area between the ROIs of the 2 related segments, 'sizes' : " +
                          "volumes product of the 2 connected ROIs, 'none' : no " +
                          "weighting, 'number' : number of relation of the relation")
    misc_group.add_option('--normalize-weights', dest='normalize_weights',
                          action='store_true', default=False, help="divide energies " +
                          "difference by the weights of the considered segment")
    parser.add_option_group(misc_group)
    parser.add_option('--seed', dest='random_seed', type='int', default=0,
                      help='set a fixed random seed before processing. '
                      'Default=0, means random initialization.')

    return parser, parser.parse_args(argv)


def main():
    # read options
    parser, (options, args) = parseOpts(sys.argv)
    parser.check_required('-i')
    parser.check()

    if options.random_seed != 0:
        random.seed(options.random_seed)

    print("read...")
    if options.sulci is None:
        selected_sulci = None
    else:
        selected_sulci = options.sulci.split(',')
    if options.node_indices:
        node_indices = options.node_indices.split(','),
    else:
        node_indices = None
    anneal_opt = options.init_temp, options.temp_rate, \
        options.stop_rate, options.tmax
    if options.motion:
        motion = aims.Reader().read(options.motion)
    else:
        motion = None

    # read graph_model and distrib models
    graph = io.load_graph(options.transfile, options.input_graphname)
    sulcimodel = io.read_full_model(None,
                                    segmentsdistribname=options.distribsegmentsname,
                                    reldistribname=options.distribrelname,
                                    sulcidistribname=options.distribsulciname,
                                    labelspriorname=options.priorname,
                                    selected_sulci=selected_sulci)
    if options.init_priorname:
        init_prior_distr = io.read_labels_prior_model(
            options.init_priorname)
    else:
        init_prior_distr = None

    # sulci tag
    tagger = Tagger(sulcimodel, options.init_with_segments,
                    init_prior_distr, graph, motion, options.nrj, options.csvname,
                    selected_sulci, node_indices, anneal_opt,
                    options.normalize_weights)
    if options.gui:
        if options.gui_mode == 'labels':
            obs = GuiLabelsObserver()
        elif options.gui_mode == 'labels_changed':
            obs = GuiLabelsChangedObserver()
        elif options.gui_mode == 'changes':
            obs = GuiChangesObserver()
        elif options.gui_mode == 'nrj':
            obs = GuiNrjObserver()
        tagger.addObserver(obs)
    if options.lookat_seg_id:
        indices = [int(id) for id in options.lookat_seg_id.split(',')]
        tagger.addObserver(SegmentObserver(indices))
    print("tag...")
    tagger.tag(options.mode, options.init_mode, options.weighting_mode,
               options.select_mode, options.precomputing, options.picklename)

    graph['filename_base'] = '*'
    w = sigraph.FoldWriter(options.output_graphname)
    w.write(graph)


if __name__ == '__main__':
    main()
