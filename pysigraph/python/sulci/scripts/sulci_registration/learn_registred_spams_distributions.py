#!/usr/bin/env python
from __future__ import print_function
from __future__ import absolute_import
import os, sys, numpy, pprint, copy
from optparse import OptionParser
import sigraph
from soma import aims
from sulci.common import io, add_translation_option_to_parser
from sulci.models import distribution, distribution_aims
from sulci.registration.common import transformation_to_motion
from sulci.registration.spam import SpamMixtureRegistration, SpamRegistration
from sulci.registration.transformation import RigidTransformation, \
                    SulcusWiseRigidTransformations
import multiprocessing
from six.moves import queue
import threading
from soma import mpfork
import numpy as np
import time
from six.moves import range
from six.moves import zip


################################################################################
def update_sulci_set(sulci_set, i, filename, name, v, c, s, w=None):
    if name in sulci_set:
        h = sulci_set[name]
        if i in h:
            h[i]['vertices'].append(v)
            if w: h[i]['weights'].append(w)
            h[i]['gravity_center'][0].append(c)
            h[i]['gravity_center'][1].append(s)
        else:
            h[i] = {
                'vertices' : [v],
                'name' : filename,
                'weights' : None,
                'gravity_center' : [[c], [s]]
            }
            if w: h[i]['weights'] = [w]
    else:
        sulci_set[name] = {
            i : {'vertices' : [v],
            'name' : filename,
            'weights' : None,
            'gravity_center' : [[c], [s]] }
            }
        if w: sulci_set[name][i]['weights'] = [w]


def update_sulci_set2(sulci_set, selected_sulci, segments_weights_g,
                    i, filename, name, v, no_tal):
    if no_tal:
        if 'gravity_center' not in v or 'size' not in v:
            return
        c = v['gravity_center']
        s = v['size']
    else:
        if 'refgravity_center' not in v or 'refsize' not in v:
            return
        c = v['refgravity_center']
        s = v['refsize']
    if segments_weights_g != None:
        W = segments_weights_g
        weights = {}
        for j, name in enumerate(W.colnames()):
            if not name.startswith('proba_'): continue
            name = name[6:]
            if selected_sulci is not None \
                and name not in selected_sulci: continue
            w = W[W[:, 0] == v['index']][0, j]
            if w < 0.01: continue
            update_sulci_set(sulci_set, i, filename,
                        name, v, c, s, w)
            weights[name] = w
        return weights
    else:
        if selected_sulci is not None and name not in selected_sulci:
            return []
        update_sulci_set(sulci_set, i, filename, name, v, c, s, None)
        return None


################################################################################
def compute_gravity_centers(sulci_set, input_motions=None,
                local_transformations=None):
    '''
    Compute sulcuswise gravity centers.
    sulci_set :     sulcuswise/subjectwise info (local gravity centers
                    in Talairach space)
    input_motions : subjectwise global motion (applied first)
    local_transformations : sulcuswise/subjectwise transformations (applied
                            in a second time)
    '''
    gravity_centers = {}
    for sulcus, data in sulci_set.items():
        db = []
        for i, h in data.items():
            g = h['gravity_center']
            if np.any(np.isnan(np.asarray(g))):
                print('NaN in gravity_centers, sulcus:', sulcus, i)
                continue
            if input_motions:
                motion = input_motions[i]
            else:
                motion = aims.Motion()
                motion.setToIdentity()
            if local_transformations:
                local_trans = local_transformations[i][sulcus]
                local_motion = local_trans.to_motion()
                motion = local_motion * motion
            db.append(motion.transform(g))
        db = numpy.array(db)

        distr = distribution.FixedSphericalGaussian(1.)
        distr.fit(db)
        gravity_centers[sulcus] = distr
    return gravity_centers

def estimate_gravity_center(sulcus, graphs, global_graph_trans):
    Id3 = numpy.identity(3)
    Ra = numpy.asmatrix(numpy.zeros((3, 3)))
    ta = numpy.asmatrix(numpy.zeros((3, 1)))
    for i, g in enumerate(graphs):
        trans = global_graph_trans[i][sulcus]
        if trans is None: continue
        R, t0 = trans._R, trans._t
        A = Id3 - R
        Ra += A + A.T
        ta += A.T * t0
    g = numpy.asarray(Ra.I * ta)
    return g

################################################################################
class SpamLearner(object):
    def __init__(self, graphs, segments_weights, input_motions, ss,
        selected_sulci, label_mode, sigmas, sigma_value, fromlog,
        no_tal, is_affine):
        self._graphs = graphs
        self._segments_weights = segments_weights
        self._input_motions = input_motions
        self._ss = ss
        self._selected_sulci = selected_sulci
        self._label_mode = label_mode
        self._sigmas = sigmas
        self._sigma_value = sigma_value
        self._fromlog = fromlog
        self._no_tal = no_tal
        self._is_affine = is_affine
        self._init_data()
        # number of threads to be used
        self.nthread = 1

    def get_labels(self): return self._labels

    def _init_data(self):
        subjects_data = []
        subjects_data_weights = []
        labels = set()
        sulci_set = {}
        motions = []

        if self._ss:
            bucket_name = 'aims_ss'
        else:    bucket_name = 'aims_bottom'
        gravity_centers = {}

        for i, g in enumerate(self._graphs):
            if self._segments_weights:
                segments_weights_g = self._segments_weights[i]
            else:    segments_weights_g = None
            if self._no_tal:
                motion = aims.Motion()
                motion.setToIdentity()
            else:    motion = aims.GraphManip.talairach(g)
            filename = g['aims_reader_filename']
            if self._input_motions:
                motion = self._input_motions[i] * motion
            motions.append(motion)
            if self._segments_weights and \
                segments_weights_g != None:
                data = []
            else:    data = {}
            data_weights = []
            for v in g.vertices():
                if v.getSyntax() != 'fold': continue
                name = v[self._label_mode]
                labels.add(name)
                weights = update_sulci_set2(sulci_set,
                    self._selected_sulci,
                    segments_weights_g,i, filename,
                    name, v, self._no_tal)
                if weights == []: continue
                ss_map = v[bucket_name].get()
                size_in = numpy.array([ss_map.sizeX(),
                    ss_map.sizeY(), ss_map.sizeZ()])
                X = []
                for p_in in ss_map[0].keys():
                    p_in = aims.Point3df(p_in * size_in)
                    p_out = motion.transform(p_in)
                    X.append(p_out)
                if segments_weights_g is None:
                    if X != []:
                        if name in data:
                            data[name].append(X)
                        else:    data[name] = [X]
                else:
                    data.append(X)
                    data_weights.append(weights)
            for sulcus, h in sulci_set.items():
                if i not in h: continue
                d = h[i]
                C = d['gravity_center'][0]
                W = d['gravity_center'][1]
                d['gravity_center'] = numpy.dot(W, C)
                d['gravity_center'] /= numpy.sum(W)

            subjects_data.append(data)
            subjects_data_weights.append(data_weights)
        self._gravity_centers = compute_gravity_centers(sulci_set,
                            self._input_motions)
        self._subjects_data = subjects_data
        self._subjects_data_weights = subjects_data_weights
        self._sulci_set = sulci_set
        self._labels = labels
        self._motions = motions


class GlobalSpamLearner(SpamLearner):
    def __init__(self, *args, **kwargs):
        SpamLearner.__init__(self, *args, **kwargs)

    def _init_data(self):
        SpamLearner._init_data(self)

        # compute weights, groups, priors (voxels/segments)
        self._weights = []
        self._groups = []
        for i, g in enumerate(self._graphs):
            self._init_data_one_graph(i)

    def _init_data_one_graph(self, i):
        data = self._subjects_data[i]
        data_weights = self._subjects_data_weights[i]
        subject_voxels = []
        subweights = [[] for k in range(len(self._labels))]
        subgroups = []
        if self._segments_weights == None:
            for j, label in enumerate(self._labels):
                try:
                    segments = data[label]
                except KeyError:
                    segments = []
                for k, w in enumerate(subweights):
                    if k == j:
                        w += [1.]
                    else:    w += [0.]
                label_voxels = []
                for s in segments: label_voxels += s
                voxels_size = len(label_voxels)
                subject_voxels += label_voxels
                subgroups += [j] * voxels_size
        else:
            for j, segment in enumerate(data):
                voxels_size = len(segment)
                for k, label in enumerate(self._labels):
                    w = subweights[k]
                    if label in data_weights[j]:
                        w += [data_weights[j][label]]
                    else:    w += [0.]
                subject_voxels += segment
                subgroups += [j] * voxels_size
        X = numpy.asmatrix(numpy.vstack(subject_voxels))
        self._subjects_data[i] = X
        self._weights.append(numpy.asmatrix(numpy.vstack(subweights)))
        self._groups.append(numpy.array(subgroups))

    def learn_spams(self, motions, sulci_set):
        def do_fit(self, s, infos, motions):
            s.fit_graphs(infos, motions, self._ss)
            return s

        models = [None] * len(self._labels)
        if self.nthread != 1:
            q = queue.Queue()
            workers = mpfork.allocate_workers(q, self.nthread, motions)
        for i, sulcus in enumerate(self._labels):
            if self._selected_sulci != None and \
                sulcus not in self._selected_sulci:
                continue
            try: infos = sulci_set[sulcus]
            except KeyError:
                s = distribution.Dummy(1.)
            else:
                sigma = self._sigmas['sulci'].get(sulcus,
                            self._sigma_value)
                s = distribution_aims.Spam(sigma, self._fromlog)
                if self.nthread != 1:
                    job = (i, do_fit, (self, s, infos), {}, models)
                    q.put(job)
                else:
                    models[i] = do_fit(self, s, infos, motions)
                #s.fit_graphs(infos, motions, self._ss)
            #models.append(s)
        if self.nthread != 1:
            for i in range(len(workers)):
                q.put(None)
            q.join()
            for worker in workers:
                worker.join()

        mixture = distribution_aims.SpamMixtureModel(models, None)
        return mixture

    def register(self, i, mixture, verbose):
        eps = 0.001
        X = self._subjects_data[i]
        reg = SpamMixtureRegistration(X.T, self._weights[i],
            mixture, self._groups[i], self._is_affine, verbose - 1)
        old_R, old_t = self._old_params[i]
        reg.set_initialization(old_R, old_t)
        R, t = reg.optimize(eps=eps, mode='powell',
                affine=self._is_affine)
        self._old_params[i] = R, t
        energy = reg.energy()
        trans = RigidTransformation(R, t)
        #print('trans', i, ':', R, '\n', t)
        return trans, energy

    def learn_onestep(self, motions, verbose=0):
        transformations = []
        total_energy = 0.
        # spams learning
        if verbose > 0: print("spams learning...")
        mixture = self.learn_spams(motions, self._sulci_set)
        # registration
        if verbose > 0: print("registration...")

        #def do_stuff(self, q, mixture, verbose, res):
            #while True:
                #i = q.get()
                #if i is None:
                    #q.task_done()
                    #break
                #print('registering:', i)
                #trans, energy = self.register(i, mixture, verbose - 1)
                #q.task_done()
                #print('done:', i)
                #res[i] = (trans, energy)


        # multiprocessing cannot be used because graphs and vertices
        # cannot be pickled.
        #pool = multiprocessing.Pool(processes=n_thread)
        #res = pool.map(do_register,
                       #[(self, i, mixture, verbose - 1)
                        #for i in range(len(self._graphs))])

        if self.nthread != 1:
            q = queue.Queue()
            workers = mpfork.allocate_workers(q, self.nthread, mixture,
                                              verbose - 1)
        res = [None] * len(self._graphs)

        for i, g in enumerate(self._graphs):
            if verbose > 1:
                print("graph %d/%d" % (i + 1, len(self._graphs)))
            if self.nthread != 1:
                job = (i, self.register, (i, ), {}, res)
                q.put(job)
            else:
                trans, energy = self.register(i, mixture, verbose - 1)
                #total_energy += energy
                #transformations.append(trans)
                res[i] = (trans, energy)
        if self.nthread != 1:
            for i in range(len(workers)):
                q.put(None)

            q.join()
            for worker in workers:
                worker.join()

        for i, r in enumerate(res):
            self._old_params[i] = r[0]._R, r[0]._t

        total_energy = sum([x[1] for x in res]) / len(self._graphs)
        transformations = [x[0] for x in res]
        #transformations.append(trans)
        #total_energy /= len(self._graphs)
        return transformations, total_energy

    def learn(self, miniter=0, maxiter=numpy.inf, verbose=0):
        energy_eps = 1.
        cur_motions = self._motions
        n = 0
        self._old_params = []
        id = numpy.asmatrix(numpy.identity(3))
        z = numpy.asmatrix(numpy.zeros((3, 1)))
        transformations = []
        for i in range(len(self._graphs)):
            R, t = id.copy(), z.copy()
            trans = RigidTransformation(R, t)
            self._old_params.append((R, t))
            transformations.append(trans)
        old_energy = numpy.inf
        while 1:
            if verbose > 0:
                print("*********")
                print("**  %d   " % n)
                print("*********")
            n += 1
            transformations, energy = self.learn_onestep(\
                        cur_motions, verbose)
            if verbose > 0: print("subjects mean energy :", energy)
            if (n >= miniter) and \
                (old_energy - energy < energy_eps):
                break
            else:   old_energy = energy
            # compute transformation from subject space
            cur_motions = [(transformations[i].to_motion() * \
                self._motions[i]) \
                for i in range(len(self._graphs))]
            if n >= maxiter: break
        mixture = self.learn_spams(cur_motions, self._sulci_set)

        return transformations, mixture


class GlobalSpamLearnerLoo(GlobalSpamLearner):
    def __init__(self, *args, **kwargs):
        GlobalSpamLearner.__init__(self, *args, **kwargs)

    def learn_spams_loo(self, i, motions):
        sulci_set = copy.copy(self._sulci_set)
        for label, h in sulci_set.items():
            h2 = copy.copy(h)
            try: del h2[i]
            except KeyError: pass
            if len(h2) == 0:
                del sulci_set[label]
            else:    sulci_set[label] = h2
        mixture = self.learn_spams(motions, sulci_set)
        return mixture

    def learn_onestep(self, motions, verbose=0):
        def learn_onestep_job(self, i, motions, verbose):
            try:
                self.nthread = 1 # disable sub-divisions inside workers
                # learn spams
                mixture = self.learn_spams_loo(i, motions)
                # registration
                trans, energy = self.register(i, mixture, verbose)
                self._old_params[i] = trans._R, trans._t
                return trans, energy
            except Exception as e:
                print('exception:', e)
                raise

        eps = 1
        transformations = []
        total_energy = 0.
        q = queue.Queue()
        workers = mpfork.allocate_workers(q, self.nthread)
        res = [None] * len(self._graphs)

        for i, g in enumerate(self._graphs):
            if verbose > 1:
                print("graph %d/%d" % (i + 1, len(self._graphs)))
            job = (i, learn_onestep_job, (self, i, motions, verbose), {}, res)
            q.put(job)
            ## learn spams
            #mixture = self.learn_spams_loo(i, motions)
            ## registration
            #trans, energy = self.register(i, mixture, verbose)
            #total_energy += energy
            #transformations.append(trans)

        for i in range(len(workers)):
            q.put(None)

        q.join()
        for worker in workers:
            worker.join()

        for i, r in enumerate(res):
            self._old_params[i] = r[0]._R, r[0]._t
            total_energy += r[1]
            transformations.append(r[0])

        total_energy /= len(self._graphs)
        return transformations, total_energy


#class Job(object):
    #def __init__(self, data, lock=None):
        #self.data = data
        #if lock is None:
            #lock = threading.RLock()
        #self.lock = lock
        #self._done = false

    #def done(self):
        #with self.lock:
            #return self._done

    #def set_done(self, state=True):
        #with self.lock:
            #self._done = state

    #def join(self):
        #while True:
            #done = self.done()
            #if done:
                #return
            #time.sleep(0.2)

class LocalSpamLearner(SpamLearner):
    def __init__(self, graphs, segments_weights, input_motions,
        ss, selected_sulci, label_mode, sigmas,
        sigma_value, fromlog, no_tal, is_affine, dir_priors):
        SpamLearner.__init__(self, graphs, segments_weights,
            input_motions, ss, selected_sulci, label_mode, sigmas,
            sigma_value, fromlog, no_tal, is_affine)
        self._dir_priors = dir_priors

    def _init_data(self):
        SpamLearner._init_data(self)

        data_by_label = {}
        for sulcus in self._labels: data_by_label[sulcus] = []
        for i, g in enumerate(self._graphs):
            data = self._subjects_data[i]
            for label in self._labels:
                try:    segments = data[label]
                except KeyError:
                    segments = None
                if segments is not None:
                    segments = numpy.vstack(segments)
                data_by_label[label].append(segments)
        self._data_by_label = data_by_label
        del self._subjects_data

    def register(self, sulcus, i, X, spam, dir_prior, verbose=0):
        eps = 0.001
        if dir_prior:
            kappa, mu = dir_prior.kappa(), dir_prior.mu()
            dir_var = 10. / kappa
            angle_var = (numpy.pi / 4.) ** 2
            t_var = 100.
        else:    dir_var = mu = angle_var = t_var = None
        g = numpy.asarray(self._gravity_centers[sulcus].mean()).T

        reg = SpamRegistration(spam, g, X.T,
            R_angle_var=angle_var, R_dir_var=dir_var,
            R_dir_mean=mu, t_var=t_var, verbose=verbose-1)
        old_R, old_t = self._old_params[i]
        reg.set_initialization(old_R, old_t)
        R, t = reg.optimize(eps=eps, mode='powell')
        self._old_params[i] = R, t
        energy = reg.energy()
        # global expression of local sulcus transformation
        # from t_g : local translation expressed in g referential
        # R.(X - g) + t_g + g = R.X + (t_g + g - R.g)
        # t0 : translation expressed in global referential : (g = 0)
        t0 = t + g - R * g
        global_trans = RigidTransformation(R, t0)
        return global_trans, energy

    def learn_spam(self, sulcus, motions, sulci_set):
        try:
            infos = sulci_set[sulcus]
            sigma = self._sigmas['sulci'].get(sulcus, self._sigma_value)
            spam = distribution_aims.Spam(sigma, self._fromlog)
            spam.fit_graphs(infos, motions, self._ss)
            return spam
        except Exception as e:
            print('exception in learn_spam:', type(e), e)
            import traceback
            traceback.print_exc()
            raise

    def learn_sulcus_onestep(self, sulcus, motions, verbose=0):
        transformations = []
        total_energy = 0.
        if self._dir_priors:
            dir_prior = self._dir_priors['vertices'][sulcus]
        else:    dir_prior = None
        X_subjects = self._data_by_label[sulcus]
        if verbose > 0: print("spams learning...")
        spam = self.learn_spam(sulcus, motions, self._sulci_set)
        for i, g in enumerate(self._graphs):
            if verbose > 1:
                print("graph %d/%d" % (i + 1, len(self._graphs)))
            X = X_subjects[i]
            if X is None:
                transformations.append(None)
                if verbose > 1: print("(skip graph)")
                continue
            global_trans, energy = self.register(sulcus, i, X, spam,
                    dir_prior, verbose - 1)
            if verbose > 1: print("graph energy :", energy)
            total_energy += energy
            transformations.append(global_trans)
        total_energy /= len(self._graphs)
        return transformations, total_energy

    def learn_sulcus(self, sulcus, miniter=0, maxiter=numpy.inf, verbose=0):
        try:
            energy_eps = 0.1
            cur_motions = self._motions
            n = 0
            self._old_params = []
            id = numpy.asmatrix(numpy.identity(3))
            z = numpy.asmatrix(numpy.zeros((3, 1)))
            transformations = []
            for i in range(len(self._graphs)):
                R, t = id.copy(), z.copy()
                trans = RigidTransformation(R, t)
                self._old_params.append((R, t))
                transformations.append(trans) # global transformation
            old_energy = numpy.inf
            n = 0
            while 1:
                if verbose > 0:
                    print("*********")
                    print("**  %d   " % n)
                    print("*********")
                n += 1
                transformations, energy = \
                    self.learn_sulcus_onestep(sulcus,
                            cur_motions, verbose - 1)
                if verbose > 0: print("subjects mean energy :", energy)
                if (n >= miniter) and \
                    (old_energy - energy < energy_eps): break
                else:   old_energy = energy
                # compute transformation from subject space
                cur_motions = []
                for i in range(len(self._graphs)):
                    global_trans = transformations[i]
                    m = self._motions[i]
                    if global_trans:
                        m = global_trans.to_motion() * m
                    cur_motions.append(m)
                if n >= maxiter: break
            spam = self.learn_spam(sulcus, cur_motions, self._sulci_set)
            return transformations, spam
        except Exception as e:
            print('exception in learn_sulcus:', type(e), e)
            import traceback
            traceback.print_exc()
            raise

    def learn(self, optimized_gravity_centers,
        miniter=0., maxiter=numpy.inf, verbose=0):
        d = dict(list(zip(self._labels, [None] * len(self._labels))))
        local_graph_trans = [copy.copy(d) for i in self._graphs]
        global_graph_trans = [copy.copy(d) for i in self._graphs]
        spams = []

        q = queue.Queue()
        workers = mpfork.allocate_workers(q, self.nthread)
        res = [None] * len(self._labels)

        for i, sulcus in enumerate(self._labels):
            if self._selected_sulci != None and \
                sulcus not in self._selected_sulci:
                continue
            if verbose > 0: print("========== %s =========" % sulcus)
            job = (i, self.learn_sulcus,
                   (sulcus, miniter, maxiter, verbose - 1), {}, res)
            q.put(job)

        for w in range(len(workers)):
            q.put(None)

        q.join()
        for w in workers:
            w.join()

        for i, sulcus in enumerate(self._labels):
            if res[i] is None:
                continue
            if len(res[i]) == 3 and isinstance(res[i][1], Exception):
                print('error in learning of sulcus %s:' % sulcus)
                print(res[i][0], res[i][1])
                import traceback
                traceback.print_exception(*res[i])
                raise res[i][0]
            try:
                transformations, spam = res[i]
                spams.append(spam)
                for i, global_trans in enumerate(transformations):
                    global_graph_trans[i][sulcus] = global_trans
            except Exception as e:
                print('exception:', type(e), e)
                raise

        # gravity_centers in optimized referential space
        self._new_gravity_centers = compute_gravity_centers(\
            self._sulci_set, self._input_motions,global_graph_trans)
        for sulcus in self._labels:
            if optimized_gravity_centers:
                g = estimate_gravity_center(sulcus,
                    self._graphs, global_graph_trans)
            else:    g = numpy.asarray(self._new_gravity_centers[\
                    sulcus].mean()).T
            for i, graph in enumerate(self._graphs):
                trans = global_graph_trans[i][sulcus]
                if trans is None:
                    local_graph_trans[i][sulcus] = None
                    continue
                R, t0 = trans._R, trans._t
                tg = t0 + R * g - g
                local_trans = RigidTransformation(R, tg)
                local_graph_trans[i][sulcus] = local_trans

        local_trans = [SulcusWiseRigidTransformations(t) \
                    for t in local_graph_trans]
        global_trans = [SulcusWiseRigidTransformations(t) \
                    for t in global_graph_trans]
        mixture = distribution_aims.SpamMixtureModel(spams, None)
        return list(zip(local_trans, global_trans)), mixture


class LocalSpamLearnerLoo(LocalSpamLearner):
    def __init__(self, graphs, segments_weights, input_motions, ss,
        selected_sulci, label_mode, sigmas, sigma_value,
        fromlog, no_tal, is_affine, dir_priors):
        LocalSpamLearner.__init__(self, graphs, segments_weights,
            input_motions, ss, selected_sulci, label_mode,
            sigmas, sigma_value, fromlog, no_tal,
            is_affine,dir_priors)

    def learn_spam_loo(self, sulcus, i, motions):
        sulci_set = copy.copy(self._sulci_set)
        for label, h in sulci_set.items():
            h2 = copy.copy(h)
            try: del h2[i]
            except KeyError: pass
            if len(h2) == 0:
                del sulci_set[label]
            else:    sulci_set[label] = h2
        spam = self.learn_spam(sulcus, motions, sulci_set)
        return spam

    def learn_sulcus_onestep(self, sulcus, motions, verbose=0):
        transformations = []
        total_energy = 0.
        if self._dir_priors:
            dir_prior = self._dir_priors['vertices'][sulcus]
        else:    dir_prior = None
        X_subjects = self._data_by_label[sulcus]
        if verbose > 0: print("spams learning...")
        for i, g in enumerate(self._graphs):
            if verbose > 1:
                print("graph %d/%d" % (i + 1, len(self._graphs)))
            spam = self.learn_spam_loo(sulcus, i, motions)
            X = X_subjects[i]
            if X is None: continue
            global_trans, energy = self.register(i, X, spam,
                    dir_prior, verbose - 1)
            if verbose > 1: print("graph energy :", energy)
            total_energy += energy
            transformations.append(global_trans)
        total_energy /= len(self._graphs)
        return transformations, total_energy


################################################################################
def parseOpts(argv):
    description = 'Compute Spam from a list of graphs, with registration.\n' \
    'Global registration or local registration modes\n\n' \
    'Needs: labels translation file (.trl), sulci graphs (.arg).\n\n' \
    'learn_spams_distributions.py [OPTIONS] graph1.arg graph2.arg...\n' \
    'learn_spams_distributions.py [OPTIONS] graph1.arg graph2.arg... == ' \
    'input_motion1.trm input_motion2.trm...\n' \
    'learn_spams_distributions.py [OPTIONS] graph1.arg graph2.arg... == '\
    'input_motion1.trm input_motion2.trm... === posterior1.csv posterior2.csv...\n'
    'learn_spams_distributions.py [OPTIONS] graph1.arg graph2.arg... === '\
    'posterior1.csv posterior2.csv...\n'
    "use 'None' as csv filename to ignore the segments weights"
    parser = OptionParser(description)
    add_translation_option_to_parser(parser)
    parser.add_option('-d', '--distribdir', dest='distribdir',
        metavar = 'FILE', action='store',
        default = 'bayesian_spam_distribs',
        help='output distribution directory (default : %default).' \
            'A file named FILE.dat is created to store ' \
            'labels/databases links.')
    parser.add_option('--distrib-gaussians', dest='distrib_gaussians',
        metavar = 'DIR', action='store', default = None,
        help='output distrib model of gravity centers. Local mode only.')
    parser.add_option('--dir-priors', dest='dir_priorsname',
        metavar = 'FILE', action='store', default = None,
        help='input von mises Fisher prior on sulcuswise rotation directions. '
        'Obtained using --- ????')
    parser.add_option('-s', '--sulci', dest='sulci',
        metavar = 'LIST', action='store', default = None,
        help='tag only specified manually tagged sulci.')
    parser.add_option('--label-mode', dest='label_mode',
        metavar = 'STR', type='choice', choices=('name', 'label'),
        action='store', default = 'name',
        help="'name' : build SPAMs from manual labeling, " + \
        "'label' : build SPAMs from automatic labeling " + \
        "(default: %default)")
    parser.add_option('--data-type', dest='data_type',
        metavar = 'TYPE', action='store', default = 'simple_surface',
        help="data type on which spam are learned. Choose one among " \
        "'simple_surface', 'bottom' (default : %default)")
    parser.add_option('--sigma-value', dest='sigma_value',
        metavar = 'FILE', action='store', default = 2.,
        help='fixed smoothing parameter for all sulci ' \
        '(default : %default)')
    parser.add_option('--sigma-file', dest='sigma_file',
        metavar = 'FILE', action='store', default = None,
        help='dictionnary storing sigma values (smoothing parameters)'\
        'for each sulci (file produced by learn_spam_sigma.py)')
    parser.add_option('--verbose', dest='verbose',
        metavar = 'INT', action='store', default = 0,
        help='verbosity level (default : disable)')
    parser.add_option('--fromlog', dest='fromlog',
        action='store_true', default = False,
        help='loglikelihoods are stored in spams rather than ' + \
        'likelihoods')
    parser.add_option('--mode', dest='mode', metavar = 'MODE',
        action='store', default = 'global',
        help="use 'global' or 'local' (sulcuswise) registration")
    parser.add_option('--maxiter', dest='maxiter', metavar = 'INT',
        action='store', default = numpy.inf, type='int',
        help="max iterations number of optimization process")
    parser.add_option('--miniter', dest='miniter', metavar = 'INT',
        action='store', default = 0,
        help="min iterations number of optimization process")
    parser.add_option('--no-talairach', dest='no_tal',
        action='store_true', default = False,
        help="if not specified the internal transformation from " + \
        "subject to Talairach is used before any other given " + \
        "transformation.")
    parser.add_option('--affine', dest='is_affine',
        action='store_true', default = False,
        help="if not specified: rigid transformation." + \
        " if specified: affine transformation")
    parser.add_option('--optimized-gravity-centers',
        dest='optimized_gravity_centers', action='store_true',
        default = False, help='if not specified, gravity centers ' \
        'and their related local transformations are expressed ' \
        'from the mean sulci gravity centers from the estimated ' \
        'output space. If specified the gravity centers are ' \
        'optimized to reduce the variability of translations.')
    parser.add_option('--loo', dest='loo', action='store_true',
        default = False,
        help="leave one out in registration estimation (each subject" +\
        "is registred on SPAMs computed without this subject)")
    parser.add_option('--threads', dest='thread', type='int', default=1,
        help='use the given number of threads when parallelisation is '
        'possible. 0 means all available CPU cores, a negative number means '
        'all available CPU cores except this number. Default=1.')

    return parser, parser.parse_args(argv)

def main():
    parser, (options, args) = parseOpts(sys.argv)

    if options.distrib_gaussians is None and options.mode == 'local':
            print('--distrib-gaussians option is mandatory', file=sys.stderr)
            parser.print_help()
            sys.exit( 1 )

    # read inputs
    inputs = args[1:]
    if len(inputs) == 0:
        parser.print_help()
        sys.exit(1)
    ind = [i for i, input in enumerate(inputs) \
        if (input == '==' or input == '===')]
    if len(ind) == 0:
        graphnames = inputs
        input_motions_names = None
        input_segments_weights = None
    elif len(ind) == 1:
        ind = ind[0]
        graphnames = inputs[:ind]
        if inputs[ind] == '==':
            input_motions_names = inputs[ind + 1:]
            input_segments_weights = None
        elif inputs[ind] == '===':
            input_motions_names = None
            input_segments_weights = inputs[ind + 1:]
    elif len(ind) == 2:
        ind1, ind2 = ind
        graphnames = inputs[:ind1]
        input_motions_names = inputs[ind1 + 1:ind2]
        input_segments_weights = inputs[ind2 + 1:]

    graphs = io.load_graphs(options.transfile, graphnames,
                            nthread=options.thread)
    if input_motions_names:
        input_motions = [aims.read(f) for f in input_motions_names]
    else:    input_motions = None
    if input_segments_weights:
        segments_weights = io.read_segments_weights(\
                    input_segments_weights)
    else:    segments_weights = None



    # options
    if options.sulci is None:
        selected_sulci = None
    else:    selected_sulci = options.sulci.split(',')
    if options.data_type == 'simple_surface':
        ss = True
        data_type = 'voxels_aims_ss'
    elif options.data_type == 'bottom' :
        ss = False
        data_type = 'voxels_bottom'
    else:
        print("error : '%s' is not a valid data type" % \
                        options.data_type)
        sys.exit(1)
    if options.sigma_file is not None:
        sigmas = io.read_from_exec(options.sigma_file, 'sigma')
    else:    sigmas = {'sulci' : {}}
    sigma_value = int(options.sigma_value)

    # create output directory
    prefix = options.distribdir
    try:    os.makedirs(prefix)
    except OSError as e:
        print("warning: directory '%s' could not be created" % prefix)
        print(e)

    # learn
    if options.mode == 'global':
        opt = [graphs, segments_weights, input_motions, ss,
            selected_sulci,    options.label_mode, sigmas,
            sigma_value, options.fromlog, options.no_tal,
            options.is_affine]
        if options.loo:
            Learner = GlobalSpamLearnerLoo
        else:    Learner = GlobalSpamLearner
    elif options.mode == 'local':
        if options.dir_priorsname:
            dir_priors = io.read_segments_distrib(\
                    options.dir_priorsname,
                    selected_sulci=selected_sulci)
        else:    dir_priors = None
        opt = [graphs, segments_weights, input_motions, ss,
            selected_sulci,    options.label_mode, sigmas, sigma_value,
            options.fromlog, options.no_tal, options.is_affine,
            dir_priors]
        if options.loo:
            Learner = LocalSpamLearnerLoo
        else:    Learner = LocalSpamLearner
    learner = Learner(*opt)
    learner.nthread = options.thread
    if options.mode == 'global':
        transformations, mixture = learner.learn(float(options.miniter),
                float(options.maxiter), verbose=int(options.verbose))
    elif options.mode == 'local':
        transformations, mixture = learner.learn(
            options.optimized_gravity_centers,
            float(options.miniter),
            float(options.maxiter), verbose=int(options.verbose))
    spams = mixture.get_models()
    labels = learner.get_labels()
    print('learning done.')

    # write motions
    for i, g in enumerate(graphs):
        transformation = transformations[i]
        subject = os.path.splitext(os.path.basename(\
                g['aims_reader_filename']))[0]
        base = subject + '_motion'
        if options.mode == 'global':
            filename = os.path.join(prefix, base + '.trm')
            transformation.write(filename)
        elif options.mode == 'local':
            local_trans, global_trans = transformation
            dir = os.path.join(prefix, base)
            local_trans.write(dir + '_local', dir + '_local.dat')
            global_trans.write(dir + '_global', dir + '_global.dat')
    print('transforms written.')
    if options.mode == 'local':
        gravity_centers = learner._new_gravity_centers
        gprefix = options.distrib_gaussians
        print('write local gaussians:', gprefix)
        try:    os.makedirs(gprefix)
        except OSError as e:
            print("warning: directory '%s' already exists" %gprefix)
        h = {'data_type' : 'refgravity_center', 'files' : {},
            'level' : 'segments'}
        for sulcus, distr in gravity_centers.items():
            filename = io.node2densityname(gprefix, 'gaussian', sulcus)
            print('writing:', filename)
            distr.write(filename)
            h['files'][sulcus] = (distr.name(),
                                  os.path.relpath(filename,
                                                  os.path.dirname(gprefix)))
        summary_file = options.distrib_gaussians + '.dat'
        print('write gravity_centers:', summary_file)
        fd = open(summary_file, 'w')
        fd.write('distributions = \\\n')
        p = pprint.PrettyPrinter(indent=4, width=1000, stream=fd)
        p.pprint(h)
        fd.close()

    # write distributions
    h = {'model' : 'spam', 'data_type' : data_type, 'files' : {}}
    for j, sulcus in enumerate(labels):
        if selected_sulci != None and sulcus not in selected_sulci:
            continue
        filename = io.node2densityname(prefix, 'spam', sulcus)
        s = spams[j]
        print('write:', filename)
        s.write(filename)
        h['files'][sulcus] = ('spam',
                              os.path.relpath(filename,
                                              os.path.dirname(prefix)))

    # write distribution summary file
    summary_file = options.distribdir + '.dat'
    print('write distribution:', summary_file)
    fd = open(summary_file, 'w')
    fd.write('distributions = \\\n')
    p = pprint.PrettyPrinter(indent=4, width=1000, stream=fd)
    p.pprint(h)
    fd.close()


if __name__ == '__main__' : main()
