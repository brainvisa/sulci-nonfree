#!/usr/bin/env python2
from __future__ import print_function
import os, sys, numpy
from optparse import OptionParser
from soma import aims
from sulci.registration.procrust import vector_from_rotation
from sulci.common import io
from sulci.models import distribution, distribution_aims
try:
    import queue
except ImportError:
    # python 2.6 and early 2.7.x
    import Queue as queue
from soma import mpfork
import tempfile
import shutil

################################################################################


################################################################################
def parseOpts(argv):
    description = 'Learn sulcuswise transformation prior from output ' + \
    'data of command learn_registered_spams_distributions.py in ' + \
    'local mode \n' \
    'learn_transformation_prior.py [OPTIONS] trans1.dat trans2.dat...'
    parser = OptionParser(description)
    for s in ['translation', 'direction', 'angle']:
        parser.add_option('--%s-distribdir' %s,
            dest='%s_distribdir' % s, metavar = 'FILE',
            action='store', default = 'bayesian_%s_distribs' %s,
            help='output distribution directory for %s ' % s + \
            '(default : %default). A file named FILE.dat is ' + \
            'created to store labels/databases links.')
    parser.add_option('--threads', dest='thread', type='int', default=1,
        help='use the given number of threads when parallelisation is '
        'possible. 0 means all available CPU cores, a negative number means '
        'all available CPU cores except this number. Default=1.')

    return parser, parser.parse_args(argv)

def main():
    parser, (options, args) = parseOpts(sys.argv)
    transformations = args[1:]
    if len(transformations) == 0:
        parser.print_help()
        sys.exit(1)

    aims.carto.PluginLoader.load() # do it before forking

    use_copy = True
    data = {}

    def copy_trm(source, dest):
        if not use_copy:
            return source
        shutil.copy(source, dest)
        if os.path.exists(source + '.minf'):
            shutil.copy(source + '.minf', dest + '.minf')
        elif os.path.exists(dest + '.minf'):
            os.unlink(dest + '.minf')
        return dest

    def read_transfo(filenames, tmp):
        motions = []
        #print('reading', len(filenames), '...')
        for filename in filenames:
            tmp = copy_trm(filename, tmp)
            #print('read:', tmp)
            motion = aims.read(tmp)
            motions.append(motion.toMatrix())
        print('read', len(motions), 'transforms')
        return motions

    nthread = options.thread
    temps = []
    if nthread > 1:
        q = queue.Queue()
        workers = mpfork.allocate_workers(q, nthread)
        res = []
        shunklen = 50
        shunks = []
        shunk = 0
    else:
        tmp = tempfile.mkstemp(suffix='.trm')
        os.close(tmp[0])
        temps.append(tmp[1])

    motions_local_all = []
    i = 0
    for t in transformations:
        motions_local = io.read_from_exec(t, 'transformations')
        motions_local_all.append(motions_local)
        i += len(motions_local)
    if nthread > 1:
        res = [None] * int(numpy.ceil(float(i) / shunklen))
        print('reading:', i, 'transforms in', len(res), 'shunks')
        for i in range(len(res)):
            tmp = tempfile.mkstemp(suffix='.trm')
            os.close(tmp[0])
            temps.append(tmp[1])

    i = 0
    for t, motions_local in zip(transformations, motions_local_all):
        prefix = os.path.dirname(t)
        for sulcus, filename in motions_local.items():
            filename = os.path.join(prefix, filename)
            if nthread > 1:
                if len(shunks) <= shunk:
                    shunks.append([filename])
                else:
                    shunks[shunk].append(filename)
                if (i + 1) % shunklen == 0:
                    job = (shunk, read_transfo, (shunks[shunk], temps[shunk]),
                           {}, res)
                    shunk += 1
                    q.put(job)
            else:
                tmp = copy_trm(filename, temps[0])
                motion = aims.read(tmp)
                print(i)
                if data.has_key(sulcus):
                    data[sulcus].append(motion)
                else:    data[sulcus] = [motion]
            i += 1
    if nthread > 1:
        # last shunk
        if len(shunks) == shunk + 1:
            job = (shunk, read_transfo, (shunks[shunk], temps[shunk]), {}, res)
            q.put(job)

        for i in range(len(workers)):
            q.put(None)
        print('waiting for IO:', q.qsize())
        q.join()
        for worker in workers:
            worker.join()
        print('IO done.')
        i = 0
        shunk = 0
        for t, motions_local in zip(transformations, motions_local_all):
            for sulcus in motions_local.keys():
                motion = aims.AffineTransformation3d(res[shunk][i])
                if data.has_key(sulcus):
                    data[sulcus].append(motion)
                else:    data[sulcus] = [motion]
                i += 1
                if i == shunklen:
                    i = 0
                    shunk += 1
    for tmp in temps:
        try:
            os.unlink(tmp + '.minf')
        except:
            pass
        try:
            os.unlink(tmp)
        except:
            pass

    print('motions OK')

    translation_priors = {}
    angle_priors = {}
    dir_priors = {}

    for distribdir in [options.translation_distribdir,
                options.direction_distribdir,
                options.angle_distribdir]:
        prefix = distribdir
        try:    os.makedirs(prefix)
        except OSError, e:
            print("warning: directory '%s' already exists" % prefix)

    print()
    for sulcus, motions in data.items():
        print('sulcus:', sulcus)
        n = len(motions)
        translations = []
        directions = []
        thetas = []
        for m in motions:
            T = m.translation().arraydata().copy()
            R = m.rotation().volume().get().arraydata().copy()
            R = R.reshape(3, 3)
            w = vector_from_rotation(R)
            theta = numpy.linalg.norm(w)
            if theta:
                direction = w / theta
                directions.append(numpy.asarray(direction).ravel())
            translations.append(T)
            thetas.append(theta)
        translations = numpy.vstack(translations)
        if len(directions): directions = numpy.vstack(directions)
        thetas = numpy.array(thetas)
        # init
        dir_prior = distribution_aims.Bingham()
        angle_prior = distribution.VonMises()
        translation_prior = distribution.Gaussian()
        # fit
        if len(directions) < 3:
            dir_prior.setUniform(3)
        else:
            r = dir_prior.fit(directions)
            if not r: dir_prior.setUniform(directions.shape[1])
        if n < 3:
            angle_prior.setUniform()
        else:    angle_prior.fit(thetas[None].T)
        r = translation_prior.fit(translations, robust=True)
        if (n < 3) or (not r):
            translation_prior.set_cov(numpy.identity(3) * 2)
            translation_prior.update()
        # store
        dir_priors[sulcus] = dir_prior
        angle_priors[sulcus] = angle_prior
        translation_priors[sulcus] = translation_prior

    # save translation prior
    h = {'data_type' : 'translation_prior', 'files' : {}, 'level' : 'sulci'}
    prefix = options.translation_distribdir
    for sulcus, g in translation_priors.items():
        filename = io.node2densityname(prefix, 'gaussian', sulcus)
        g.write(filename)
        h['files'][sulcus] = (g.name(),
                              os.path.relpath(filename,
                                              os.path.dirname(prefix)))
    summary_file = options.translation_distribdir + '.dat'
    io.write_pp('distributions', summary_file, h)

    # save direction prior
    h = {'data_type' : 'direction_prior', 'files' : {}, 'level' : 'sulci'}
    prefix = options.direction_distribdir
    for sulcus, d in dir_priors.items():
        filename = io.node2densityname(prefix,
                'bingham', sulcus)
        d.write(filename)
        h['files'][sulcus] = (d.name(),
                              os.path.relpath(filename,
                                              os.path.dirname(prefix)))
    summary_file = options.direction_distribdir + '.dat'
    io.write_pp('distributions', summary_file, h)

    # save angle prior
    h = {'data_type' : 'angle_prior', 'files' : {}, 'level' : 'sulci'}
    prefix = options.angle_distribdir
    for sulcus, d in angle_priors.items():
        filename = io.node2densityname(prefix, 'von_mises', sulcus)
        d.write(filename)
        h['files'][sulcus] = (d.name(),
                              os.path.relpath(filename,
                                              os.path.dirname(prefix)))
    summary_file = options.angle_distribdir + '.dat'
    io.write_pp('distributions', summary_file, h)


if __name__ == '__main__' : main()
