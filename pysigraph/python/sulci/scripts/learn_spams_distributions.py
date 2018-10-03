#!/usr/bin/env python2
from __future__ import print_function
import os, sys, numpy, pprint, re, glob
from optparse import OptionParser
import sigraph
from soma import aims
from sulci.common import io, add_translation_option_to_parser
from sulci.models import distribution, distribution_aims


################################################################################
def update_sulci_set(sulci_set, i, filename, name, v, w=None):
    if sulci_set.has_key(name):
        h = sulci_set[name]
        if h.has_key(i):
            h[i]['vertices'].append(v)
            if w: h[i]['weights'].append(w)
        else:
            h[i] = {
                'vertices' : [v],
                'name' : filename,
                'weights' : None
            }
            if w: h[i]['weights'] = [w]
    else:
        sulci_set[name] = {
            i : {'vertices' : [v],
            'name' : filename,
            'weights' : None
            }
        }
        if w: sulci_set[name][i]['weights'] = [w]

################################################################################
def compute_spams(graphs, segments_weights, distribdir, sigma_value,
                  sigma_file, data_type, bucket_name, ss, selected_sulcus,
                  options):
    reader = aims.Reader()
    if sigma_file is not None:
        sigmas = io.read_from_exec(sigma_file, 'sigma')
    else:    sigmas = {'sulci' : {}}
    if options.depth_weighted or options.k:
        need_data = True
    else:    need_data = False

    print("compute sulci hash...")
    sulci_set = {}
    motions = []
    data = []
    data_depth = []
    # Create an hash table : sulci <-> [(vertices, motion) for each graph]
    for i, g in enumerate(graphs):
        if segments_weights:
            segments_weights_g = segments_weights[i]
        filename = g['aims_reader_filename']
        subject = os.path.splitext(os.path.basename(filename))[0]
        side = subject[0]
        subject = re.sub('_.*', '', subject)[1:]
        if need_data: data.append({})
        if options.depth_weighted:
            data_depth.append({})
            depthmapname = '%s_depthmap.ima' % subject
            if options.mode != 'loo' :
                depthmapname = os.path.join('..', depthmapname)
            if os.path.exists(depthmapname):
                reader = aims.Reader()
                print("find depthmap for subject '%s'" % subject)
                depthmap = reader.read(depthmapname)
                depthmap = aims.AimsData_FLOAT(depthmap)
            else:
                print("compute depthmap for subject '%s'" % \
                                subject)
                direc = os.path.join(os.path.dirname(filename),
                        '..', 'skeleton', '%s*%s*.' \
                            % (side, subject))
                patterns = [ direc + 'vimg', direc + 'ima']
                for p in patterns:
                    res = glob.glob(p)
                    if len(res) == 1:
                        skelname = res[0]
                        break
                    elif len(res) == 0:
                        continue
                    else:
                        print("error: find more than "+\
                            "one skeleton file : "+\
                            str(res))
                        sys.exit(1)
                skel = reader.read(skelname)
                fat = aims.FoldGraphAttributes(skel, g)
                fat.prepareBrainDepthMap()
                depthmap = fat.getBrainDepth()
                writer = aims.Writer()
                writer.write(depthmap, depthmapname)
        motion = aims.GraphManip.talairach(g)
        motions.append(motion)
        for v in g.vertices():
            if v.getSyntax() != 'fold': continue
            # select how to group data to build SPAMs
            if options.label_mode == 'local_errors':
                if v['name'] != v['label']:
                    name = v['name']
                else:    continue
            elif options.label_mode == 'local_ok':
                if v['name'] == v['label']:
                    name = v['name']
                else:    continue
            elif options.label_mode == 'global_errors':
                if v['name'] != v['label']:
                    name = 'error'
                else:    continue
            elif options.label_mode == 'global_ok':
                if v['name'] == v['label']:
                    name = 'ok'
                else:    continue
            elif options.label_mode in ['name', 'label']:
                name = v[options.label_mode]
            if segments_weights and segments_weights_g != None:
                W = segments_weights_g
                for j, name in enumerate(W.colnames()):
                    if not name.startswith('proba_'):
                        continue
                    name = name[6:]
                    if selected_sulcus is not None and \
                        name not in selected_sulcus:
                            continue
                    w = W[W[:, 0] == v['index']][0, j]
                    if w < 0.01: continue
                    update_sulci_set(sulci_set, i,
                        filename, name, v, w)
            else:
                if selected_sulcus is not None and \
                    name not in selected_sulcus: continue
                update_sulci_set(sulci_set, i, \
                    filename, name, v, None)

            ss_map = v[bucket_name].get()
            size_in = numpy.array([ss_map.sizeX(),
                ss_map.sizeY(), ss_map.sizeZ()])
            if need_data:
                X, D = [], []
                for p_in in ss_map[0].keys():
                    if options.depth_weighted:
                        d = depthmap.value(p_in[0],
                            p_in[1], p_in[2])
                    p_in = aims.Point3df(p_in * size_in)
                    p_out = motion.transform(p_in)
                    X.append(p_out)
                    if options.depth_weighted: D.append(d)
                if data[-1].has_key(name):
                    data[-1][name].append(X)
                else:    data[-1][name] = [X]
                if options.depth_weighted:
                    if data_depth[-1].has_key(name):
                        data_depth[-1][name] += D
                    else:    data_depth[-1][name] = D
    # extraction of 5% deepest value of voxels position per subject
    # and per sulcus
    if need_data:
        data2 = {}
        if options.depth_weighted:
            data_depth2 = {}
            data_pct = {}
        for i, g in enumerate(graphs):
            datai = data[i]
            if options.depth_weighted:
                data_depthi = data_depth[i]
            for sulcus, Xlist in datai.items():
                X = numpy.vstack(Xlist)
                if options.depth_weighted:
                    Dlist = data_depthi[sulcus]
                    D = numpy.array(Dlist)
                if data2.has_key(sulcus):
                    data2[sulcus].append(X)
                else:    data2[sulcus]= [X]
                if options.depth_weighted:
                    if data_depth2.has_key(sulcus):
                        data_depth2[sulcus] += Dlist
                    else:    data_depth2[sulcus] = Dlist
                    th = numpy.sort(D)[int(len(D) * 0.95)]
                    Ds = D[D > th]
                    if data_pct.has_key(sulcus):
                        data_pct[sulcus] += Ds.tolist()
                    else:    data_pct[sulcus] = Ds.tolist()
        data = {}
        if options.depth_weighted: data_depth = {}
        for sulcus in data2.keys():
            if options.depth_weighted:
                Dlist = data_pct[sulcus]
                data_pct[sulcus] = numpy.vstack(Dlist)
                data_depth[sulcus] = numpy.array(\
                        data_depth2[sulcus])
            data[sulcus] = numpy.vstack(data2[sulcus])
        del data2
        if options.depth_weighted: data_depth2

    # create output directory
    prefix = distribdir
    try:    os.mkdir(prefix)
    except OSError, e:
        print("warning: directory '%s' allready exists" % prefix)
    if options.depth_weighted:
        model_type = 'depth_weighted_spam'
    else:    model_type = 'spam'

    # learn and write spams
    h = {'level' : 'segments', 'data_type' : data_type, 'files' : {}}
    for sulcus, infos in sulci_set.items():
        if selected_sulcus != None and selected_sulcus != sulcus:
            continue
        print("*** %s ***" % sulcus)
        sigma = sigmas['sulci'].get(sulcus, sigma_value)
        if options.depth_weighted:
            s = distribution_aims.DepthWeightedSpam(sigma)
            try:
                X = data[sulcus]
            except KeyError: continue
            deepest_depth = data_pct[sulcus]
            g = s.gaussian()
            g.fit(deepest_depth)
            depth = data_depth[sulcus][None].T
            logli, li = g.likelihoods(depth)
            if options.k:
                s.fit_knn(X, k=int(options.k), weights=li)
            else:    s.fit(X, weights=li)
        else:
            s = distribution_aims.Spam(sigma)
            if options.k:
                try:
                    X = data[sulcus]
                except KeyError: continue
                s.fit_knn(X, k=int(options.k))
            else:    s.fit_graphs(infos, motions, ss)
        filename = io.node2densityname(prefix, 'spam', sulcus)
        if options.depth_weighted:
            filename = re.sub('\.ima$', '.data', filename)
        s.write(filename)
        relfilename = re.sub('^%s%s' % (os.path.dirname(prefix), \
                        os.path.sep), '', filename)
        h['files'][sulcus] = (model_type, relfilename)

    # write distribution summary file
    summary_file = distribdir + '.dat'
    io.write_pp('distributions', summary_file, h)


################################################################################
def parseOpts(argv):
    description = 'Compute Spam from a list of graphs.\n' \
    'In Talairach space mode, no registration - see ' \
    'learn_registred_spams_distributions.py for registration included with ' \
    'model estimation.\n' \
    'Needs: labels translation file (.trl), sulci graphs (.arg)\n' \
    'learn_spams_distributions.py [OPTIONS] graph1.arg graph2.arg...\n'
    'learn_spams_distributions.py [OPTIONS] graph1.arg graph2.arg ... == posterior1.csv posterior2.csv...\n'
    "use 'None' as csv filename to ignore the segments weights"
    parser = OptionParser(description)
    add_translation_option_to_parser(parser)
    parser.add_option('-d', '--distribdir', dest='distribdir',
        metavar = 'FILE', action='store',
        default = 'bayesian_spam_distribs',
        help='output distribution directory (default : %default).' \
            'A file named FILE.dat is created to store ' \
            'labels/databases links.')
    parser.add_option('-s', '--sulcus', dest='sulcus',
        metavar = 'NAME', action='store', default = None,
        help='Compute spam only for one sulcus ' \
            '(default : compute all spams)')
    parser.add_option('--label-mode', dest='label_mode',
        metavar = 'STR', type='choice', choices=('name', 'label',
        'local_errors', 'global_errors', 'local_ok', 'global_ok'),
        action='store', default='name',
        help="'name': build SPAMs from manual " + \
        "labeling, 'label': build SPAMs from automatic labeling, " + \
        "'local_errors': build SPAMs from sulciwise errors, " + \
        "'global_errors': build SPAMs from unnamed errors " + \
        "(default: %default)")
    parser.add_option('--data-type', dest='data_type',
        metavar = 'TYPE', action='store', default = 'simple_surface',
        help="data type on which spam are learned. Choose one of " \
        "'simple_surface', 'bottom' (default : %default)")
    parser.add_option('--sigma-value', dest='sigma_value',
        metavar = 'FILE', action='store', default = 2.,
        help='fixed smoothing parameter for all sulci ' \
        '(default : %default)')
    parser.add_option('--sigma-file', dest='sigma_file',
        metavar = 'FILE', action='store', default = None,
        help='dictionnary storing sigma values (smoothing parameters)'\
        'for each sulci (file produced by learn_spam_sigma.py)')
    parser.add_option('-k', '--knn', dest='k',
        metavar = 'INT', action='store', default = None,
        help='enable knn estimation with a specified number of ' + \
        'neighbours (default : disable). Alternative methods for kernel density estimation - do not use in routine...')
    parser.add_option('--mode', dest='mode',
        metavar = 'FILE', action='store', default = 'normal',
        help="'normal' : compute spams on given graphs, 'loo' : " + \
        "leave one out on graphs : create several models " + \
        "(default : %default), all given reference FILE options " + \
        "must be located in './all/' relative directory and similar " +\
        "data can be found in './cv_*/' directories relative to " + \
        "leave one out graphs folds.")
    parser.add_option('--depth-weighted', dest='depth_weighted',
        action='store_true', default = None,
        help='gives more weight to buried structures')
    parser.add_option('--threads', dest='thread', type='int', default=1,
        help='use the given number of threads when parallelisation is '
        'possible. 0 means all available CPU cores, a negative number means '
        'all available CPU cores except this number. Default=1.')

    return parser, parser.parse_args(argv)

def main():
    parser, (options, args) = parseOpts(sys.argv)
    inputs = args[1:]
    if len(inputs) == 0:
        print("missing graph")
        sys.exit(1)
    ind = [i for i, input in enumerate(inputs) if input == '==']
    if len(ind) == 0:
        graphnames = inputs
        input_segments_weights = None
    else:
        ind = ind[0]
        graphnames = inputs[:ind]
        input_segments_weights = inputs[ind + 1:]
    if options.label_mode in ['local_errors', 'local_ok',
                'global_errors', 'global_ok']:
        label_mode = 'both'
    else:
        label_mode = options.label_mode
    graphs = io.load_graphs(options.transfile, graphnames, label_mode,
                            nthread=options.thread)
    if input_segments_weights:
        segments_weights = io.read_segments_weights(\
                    input_segments_weights)
    else:    segments_weights = None

    # options
    if options.data_type == 'simple_surface':
        ss = True
        data_type = 'voxels_aims_ss'
        bucket_name = 'aims_ss'
    elif options.data_type == 'bottom' :
        ss = False
        data_type = 'voxels_bottom'
        bucket_name = 'aims_bottom'
    else:
        print("error : '%s' is not a valid data type" % \
                        options.data_type)
        sys.exit(1)
    sigma_value = float(options.sigma_value)

    if options.mode == 'normal' :
        compute_spams(graphs, segments_weights, options.distribdir,
            sigma_value, options.sigma_file, data_type, bucket_name,
            ss, options.sulcus, options)
    elif options.mode == 'loo' :
        print("-- all --")
        distribdir = os.path.join('all', options.distribdir)
        if options.sigma_file is None:
            sigma_file = None
        else:    sigma_file = os.path.join('all', options.sigma_file)
        compute_spams(graphs, segments_weights, distribdir,
            sigma_value, sigma_file,
            data_type, bucket_name, ss, options.sulcus, options)
        for i in range(len(graphs)):
            subgraphs = graphs[:i] + graphs[i+1:]
            subsegments_weights = segments_weights[:i] + \
                        segments_weights[i+1:]
            directory = 'cv_%d' % i
            print('-- %s --' % directory)
            distribdir = os.path.join(directory, options.distribdir)
            if options.sigma_file is None:
                sigma_file = None
            else:    sigma_file = os.path.join(directory,
                    options.sigma_file)
            compute_spams(subgraphs, subsegments_weights,
                    distribdir, sigma_value,
                    sigma_file, data_type, bucket_name,
                    ss, options.sulcus, options)
    else:
        print("error : '%s' unknown mode" % options.mode)



if __name__ == '__main__' : main()
