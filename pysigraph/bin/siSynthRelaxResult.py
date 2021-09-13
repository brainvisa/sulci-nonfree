#!/usr/bin/env python
from __future__ import print_function
from __future__ import absolute_import
import sys
import os
import numpy
from optparse import OptionParser
from datamind.tools import *
import datamind.io as io
import datamind.core
from six.moves import range

train = ['caca', 'cronos', 'dionysos2', 'jah2', 'morphee', 'jupiter', 'neptune',
         'ammon', 'horus', 'osiris', 'ra', 'ares', 'hades', 'jason', 'poseidon',
         'zeus']
test = ['chaos', 'hyperion', 'anubis', 'isis', 'moon']
gen2005 = ['athena', 'atlas', 'demeter', 'eros', 'icbm100T', 'icbm125T',
           'icbm200T', 'icbm201T', 'icbm300T', 'icbm310T', 'icbm320T', 'beflo',
           'shiva', 'vayu', 'vishnu']
gen2000 = ['athena', 'demeter', 'beflo', 'shiva',
           'vishnu']

gen = None

################################################################################
# Old #
#######


def old_read_csv_result(filename):
    def check_data(data_list):
        error_msg = []
        kept_lines = []
        for i, d in enumerate(data_list):
            if d[0].startswith('#'):
                continue
            if len(d) < 2 or len(d) > 3:
                error_msg.append('l.%d : should have '
                                 'two or three columns.\n' % i)
                continue
            try:
                float(d[1])
                if len(d) == 3:
                    float(d[2])
                kept_lines.append(d)
            except ValueError as msg:
                error_msg.append('l.%d : %s.\n' % (i, msg))
        if len(error_msg) > 0:
            sys.stderr.write("error : read error\n")
            for e in error_msg:
                sys.stderr.write('\t' + e)
            return {}
        else:
            return kept_lines

    fd = open(filename)
    data_list = [l.rstrip(' \n\t').split('\t') for l in fd.readlines()][1:]
    data_list = check_data(data_list)
    if data_list == {}:
        return {}
    data = {}
    for d in data_list:
        res = Result(*[float(v) for v in d[1:]])
        if d[0] in data:
            data[d[0]].append(res)
        else:
            data[d[0]] = [res]
    fd.close()
    print(repr(data))
    return data


def old_compute_all_info(filename, details):
    data = old_read_csv_result(filename)
    if data == {}:
        msg.write("\t(skip)", 'thin_gray')
        print()
        return
    msg.write_list([(' * ', 'bold_yellow'), "Rates and Energies :\n"])
    infos = {'train': compute_db_info(data, train),
             'test': compute_db_info(data, test),
             'gen': compute_db_info(data, gen),
             'all': compute_db_info(data, train + test + gen)}
    msg.write_list([('\t  \tRATE\t ', 'red'), ('\t|\t', 'cyan'),
                    ('ENERGY\t \n', 'red')])
    msg.write_list([('\t  \tMEAN\tSTD', 'red'),
                    ('\t|\t', 'cyan'), ('MEAN\tSTD\n', 'red')])
    for n in ['train', 'test', 'gen', 'all']:
        rmean, rstd, emean, estd, s = infos[n]
        msg.write('\t%s' % n, 'green')
        # try:
        #	rmean.mask
        #	msg.write('\tNA', 'thin_gray')
        # except AttributeError:
        msg.write('\t%2.2f' % rmean)
        # try:
        #	rstd.mask
        #	msg.write('\tNA', 'thin_gray')
        # except AttributeError:
        msg.write('\t%2.2f' % rstd)
        msg.write('\t|', 'cyan')
        try:
            emean.mask
            msg.write('\tNA', 'thin_gray')
        except AttributeError:
            msg.write('\t%2.2f' % emean)
        if numpy.isnan(estd):
            msg.write('\tNA\n', 'thin_gray')
        else:
            msg.write('\t%2.2f\n' % estd)
    for n in ['train', 'test', 'gen']:
        skipped = infos[n][4]
        if len(skipped) > 0:
            msg.write("(skip %s)\t" % n, 'thin_gray')
            for name in skipped:
                msg.write("%s " % name, 'thin_gray')
            print()
    if not details:
        return
    numbers = compute_number_of_results(data)
    for (n, db) in [('train', train), ('test', test), ('gen', gen)]:
        print()
        msg.write_list(['----- ', (n, 'green'), ' -----'])
        cor = compute_correlations_synth_info(data, db)
        subject_cor, mean_cor, std_cor = \
            compute_correlations_subject_info(data, db)
        print()
        msg.write_list([(' * ', 'bold_yellow'),
                        "Total correlation rate / energy : %s\n" % cor])
        msg.write_list([(' * ', 'bold_yellow'),
                        "Subjectwise mean (std) correlation "
                        "rate / energy :\n\t",
                        "%f (%f)\n" % (mean_cor, std_cor)])
        msg.write_list([(' * ', 'bold_yellow'),
                        "Subjectwise correlation rate / energy (qty) :\n"])
        for (subject, correlation) in subject_cor.items():
            if numpy.isnan(correlation):
                continue
            msg.write('\t%s\t%f (%d)\n' % (subject,
                                           correlation, numbers[subject][0]))
        subject_std, mean_rates_std, mean_energies_std = \
            compute_std_subject_info(data, db)
        print()
        msg.write_list([(' * ', 'bold_yellow'),
                        "Subjectwise mean std rate, energy (qty) :\n\t",
                        "\t%f\t%f\n" % (mean_rates_std, mean_energies_std)])
        msg.write_list([(' * ', 'bold_yellow'),
                        "Subjectwise std rate, energy :\n"])
        for (subject, (rate, energy)) in subject_std.items():
            if numpy.isnan(rate) and numpy.isnan(energy):
                continue
            msg.write('\t%s\t%f (%d)\t%f (%d)\n' %
                      (subject, rate, numbers[subject][1],
                       energy, numbers[subject][2]))


def old_compute_best_info(filename):
    data = old_read_csv_result(filename)
    if data == {}:
        msg.write("\t(skip)", 'thin_gray')
        print()
        return
    msg.write_list([(' * ', 'bold_yellow'), "Rates and Energies :\n"])
    infos = {'train': compute_db_best_info(data, train),
             'test': compute_db_best_info(data, test),
             'gen': compute_db_best_info(data, gen),
             'all': compute_db_best_info(data, train + test + gen)}
    msg.write_list([('\t  \tRATE\t ', 'red'), ('\t|\t', 'cyan'),
                    ('ENERGY\t \n', 'red')])
    msg.write_list([('\t  \tMEAN\tSTD', 'red'),
                    ('\t|\t', 'cyan'), ('MEAN\tSTD\n', 'red')])
    for n in ['train', 'test', 'gen', 'all']:
        rmean, rstd, emean, estd, s = infos[n]
        msg.write('\t%s' % n, 'green')
        if rmean.mask:
            msg.write('\tNA', 'thin_gray')
        else:
            msg.write('\t%2.2f' % rmean)
        if rstd.mask:
            msg.write('\tNA', 'thin_gray')
        else:
            msg.write('\t%2.2f' % rstd)
        msg.write('\t|', 'cyan')
        if emean.mask:
            msg.write('\tNA', 'thin_gray')
        else:
            msg.write('\t%2.2f' % emean)
        if numpy.isnan(estd):
            msg.write('\tNA\n', 'thin_gray')
        else:
            msg.write('\t%2.2f\n' % estd)


################################################################################
# One garde ? #
###############
class Result(object):
    def __init__(self, rate, energy=None):
        self.rate = rate
        self.energy = energy

    def __repr__(self):
        if self.energy is None:
            return "rate = %f, energy = NA" % (self.rate)
        return "rate = %f, energy = %f" % (self.rate, self.energy)

    def to_list(self):
        if self.energy is None:
            return [self.rate, numpy.nan]
        return [self.rate, self.energy]


def read_csv_result(filename):
    r = io.csvIO.ReaderCsv()
    m = r.read(filename)
    return m


def compute_db_info(data, db):
    rates, energies, skipped = [], [], []
    for name in db:
        results = data.get(name, [Result(numpy.nan, numpy.nan)])
        if len(results) == 0:
            skipped += [name]
        else:
            local_rates = []
            local_energies = []
            for res in results:
                local_rates += [res.rate]
                if res.energy is not None:
                    local_energies += [res.energy]
            rates += [numpy.array(local_rates).mean()]
            if len(local_energies) == 0:
                energies += [numpy.nan]
            else:
                energies += [numpy.array(local_energies).mean()]
    a = numpy.array(rates)
    ma = numpy.ma.masked_where(numpy.isnan(a), a)
    e = numpy.array(energies)
    me = numpy.ma.masked_where(numpy.isnan(e), e)
    if me.mask.all():  # all is nan
        e_std = numpy.nan
    else:
        e_std = me.std()
    # ma.MaskedArray is used because std return masked array or float64
    return ma.mean(), numpy.ma.MaskedArray(ma.std()), \
        me.mean(), e_std, skipped


def compute_correlations_synth_info(data, db):
    db_res = []
    for name in db:
        if name in data:
            for r in data[name]:
                db_res.append(r.to_list())
    db_res = numpy.array(db_res)
    if db_res.shape[0] == 0:
        return numpy.nan
    db_res = db_res[numpy.isnan(db_res).any(axis=1) != True]
    if db_res.shape[0] == 0:
        return numpy.nan
    return compute_correlation(db_res[:, 0], db_res[:, 1])


def compute_correlations_subject_info(data, db):
    '''
Return :
    - dict of subject/correlations.
    - correlations mean.
    - correlations std.
    '''
    correlations = {}
    for name in db:
        if name not in data:
            c = numpy.nan
        else:
            db_res = []
            for r in data[name]:
                db_res.append(r.to_list())
            db_res = numpy.array(db_res)
            db_res = db_res[numpy.isnan(db_res).any(axis=1) != True]
            if db_res.shape[0] == 0:
                c = numpy.nan
            else:
                c = compute_correlation(db_res[:, 0],
                                        db_res[:, 1])
        correlations[name] = c
    c = numpy.array(list(correlations.values()))
    return correlations, c.mean(), c.std()


def compute_std_subject_info(data, db):
    '''
Return :
    - dict of subject/std.
    - std mean.
    '''
    stds = {}
    for name in db:
        if name not in data:
            stds[name] = numpy.nan, numpy.nan
        else:
            db_res = []
            for r in data[name]:
                db_res.append(r.to_list())
            db_res = numpy.array(db_res)
            db_rates = db_res[:, 0]
            db_rates = db_rates[numpy.isnan(db_rates) != True]
            db_nrj = db_res[:, 1]
            db_nrj = db_nrj[numpy.isnan(db_nrj) != True]
            if db_rates.shape[0] != 0:
                std_rates = db_rates.std()
            else:
                std_rates = numpy.nan
            if db_nrj.shape[0] != 0:
                std_nrj = db_nrj.std()
            else:
                std_nrj = numpy.nan
            stds[name] = std_rates, std_nrj
    c = numpy.array(list(stds.values()))
    return stds, c[:, 0].mean(), c[:, 1].mean()


def compute_number_of_results(data):
    '''
Return : number of data to compute : correlations, rates and energies
    '''
    n = {}
    for name in train + test + gen:
        if name not in data:
            n[name] = 0, 0, 0
        else:
            db_res = []
            for r in data[name]:
                db_res.append(r.to_list())
            db_res = numpy.array(db_res)
            db_rates = db_res[:, 0]
            db_rates = db_rates[numpy.isnan(db_rates) != True]
            db_nrj = db_res[:, 1]
            db_nrj = db_nrj[numpy.isnan(db_nrj) != True]
            db_res = db_res[numpy.isnan(db_res).all(1) != True]
            n[name] = db_res.shape[0], db_rates.shape[0], \
                db_nrj.shape[0]
    return n


def compute_db_best_info(data, db):
    rates, energies, skipped = [], [], []
    for name in db:
        results = data.get(name, [Result(numpy.nan, numpy.nan)])
        if len(results) == 0:
            skipped += [name]
        else:
            best_rate = numpy.inf
            best_energy = numpy.inf
            for res in results:
                if res.energy is not None and \
                        res.energy < best_energy:
                    best_energy = res.energy
                    best_rate = res.rate
            rates += [best_rate]
            if best_energy != numpy.inf:
                energies += [best_energy]
    a = numpy.array(rates)
    ma = numpy.ma.masked_where(numpy.isnan(a), a)
    e = numpy.array(energies)
    me = numpy.ma.masked_where(numpy.isnan(e), e)
    if me.mask.all():  # all is nan
        e_std = numpy.nan
    else:
        e_std = me.std()
    # ma.MaskedArray is used because std return masked array or float64
    return ma.mean(), numpy.ma.MaskedArray(ma.std()), \
        me.mean(), e_std, skipped


################################################################################
# New #
#######
def select_subdatabase(db, selected_subjects):
    ind = db[:, 0]
    all_subjects = ind.tolist()
    selected_ind = numpy.zeros(ind.shape, dtype='bool')
    for s in selected_subjects:
        selected_ind |= (db[:, 0] == datamind.core.DataFrame.code(s))
    subdb = db[selected_ind]
    return subdb


def compute_correlation(x0, x1):
    # warning numpy.std, std with bias while cov is biased.
    def std_nobias(x):
        return numpy.sqrt(((x - x.mean()) ** 2).sum() / (len(x) - 1))
    cov = numpy.cov(x0, x1)[0, 1]  # no bias
    corr = cov / (std_nobias(x0) * std_nobias(x1))
    return corr


def compute_correlations(db):
    nrj = db[:, -1]
    corrs = []
    for error_dim in range(1, db.shape[1] - 1):
        corr = compute_correlation(db[:, error_dim], nrj)
        corrs.append(corr)
    corrs = datamind.core.DataFrame(data=numpy.array(corrs).reshape(1, -1),
                                    colnames=db.colnames().tolist()[1:-1],
                                    celltypes=db._celltypes[1:-1])
    corrs = numpy.hstack((numpy.zeros((corrs.shape[0], 2)), corrs,
                          numpy.nan + numpy.zeros((corrs.shape[0], 1))))
    corrs = datamind.core.DataFrame(data=corrs,
                                    colnames=['Type'] + db.colnames().tolist(),
                                    celltypes=numpy.hstack(('string', db._celltypes)))
    corrs[:, 1] = datamind.core.DataFrame.code('all')
    corrs[:, 0] = datamind.core.DataFrame.code('correlation')
    return corrs


def min_each_subject(db):
    ind = db[:, 0]
    all_subjects = ind.tolist()
    if hasattr(numpy, 'unique1d'):
        all_subjects = numpy.unique1d(all_subjects).tolist()
    else:
        all_subjects = numpy.unique(all_subjects).tolist()
    mins = []
    for s in all_subjects:
        s_db = db[db[:, 0] == datamind.core.DataFrame.code(s)]
        s_min = s_db.min(axis=0)
        mins.append(s_min)
    mins = datamind.core.DataFrame(data=numpy.vstack(mins),
                                   colnames=db.colnames().tolist(),
                                   celltypes=db._celltypes)
    return mins


def mean_each_subject(db):
    ind = db[:, 0]
    all_subjects = ind.tolist()
    if hasattr(numpy, 'unique1d'):
        all_subjects = numpy.unique1d(all_subjects).tolist()
    else:
        all_subjects = numpy.unique(all_subjects).tolist()
    means = []
    for s in all_subjects:
        s_db = db[db[:, 0] == datamind.core.DataFrame.code(s)]
        s_mean = s_db.mean(axis=0)
        means.append(s_mean)
    means = datamind.core.DataFrame(data=numpy.vstack(means),
                                    colnames=db.colnames().tolist(),
                                    celltypes=db._celltypes)
    return means


def compute_mean(db, mode, show_std):
    # summarize one error per subject
    if mode == 'mean':
        db_subject = mean_each_subject(db)
    elif mode == 'best':
        db_subject = min_each_subject(db)

    train_db = select_subdatabase(db_subject, train)
    test_db = select_subdatabase(db_subject, test)
    gen_db = select_subdatabase(db_subject, gen)

    all_mean = db_subject[:, 1:].mean(axis=0)
    train_mean = train_db[:, 1:].mean(axis=0)
    test_mean = test_db[:, 1:].mean(axis=0)
    gen_mean = gen_db[:, 1:].mean(axis=0)

    if show_std:
        all_std = db_subject[:, 1:].std(axis=0)
        train_std = train_db[:, 1:].std(axis=0)
        test_std = test_db[:, 1:].std(axis=0)
        gen_std = gen_db[:, 1:].std(axis=0)
        means = [all_mean, all_std, train_mean, train_std,
                 test_mean, test_std, gen_mean, gen_std]
    else:
        means = [all_mean, train_mean, test_mean, gen_mean]

    means = numpy.vstack(means)
    means = numpy.hstack((numpy.zeros((means.shape[0], 2)), means))
    means = datamind.core.DataFrame(data=means,
                                    colnames=['Type'] + db.colnames().tolist(),
                                    celltypes=numpy.hstack(('string', db._celltypes)))
    means[:, 1] = datamind.core.DataFrame.code('all')
    if show_std:
        types = ['all_mean', 'all_std', 'train_mean', 'train_std',
                 'test_mean', 'test_std', 'gen_mean', 'gen_std']
    else:
        types = ['all_mean', 'train_mean', 'test_mean', 'gen_mean']
    types = [(x + '_' + mode) for x in types]
    means[:, 0] = datamind.core.DataFrame.code(types)
    return means


def process(f, options):
    if options.old:
        if options.show_mean:
            old_compute_all_info(f, 'short')
        if options.show_best:
            old_compute_best_info(f)
        if options.show_correlation:
            old_compute_all_info(f, 'all')
    else:
        print('------ %s ------' % f)
        try:
            db = read_csv_result(f)
        except:
            msg.write("\t(skip)\n", 'thin_gray')
            return

        r = []
        if options.show_mean:
            r += [compute_mean(db, 'mean', options.show_std)]
        if options.show_best:
            r += [compute_mean(db, 'best', options.show_std)]
        if options.show_correlation:
            r += [compute_correlations(db)]
        res = numpy.vstack(r)
        res = numpy.hstack((numpy.zeros((res.shape[0], 2)), res))
        res = datamind.core.DataFrame(data=res,
                                      colnames=['File', 'Base'] +
                                      r[0].colnames().tolist(),
                                      celltypes=numpy.hstack((['string']*2, r[0]._celltypes)))
        res[:, 0] = datamind.core.DataFrame.code(f)
        res[:, 1] = datamind.core.DataFrame.code(options.database_name)
        print(res)
        return res


def write_csv(csvname, res):
    w = io.csvIO.WriterCsv()
    w.write(res, csvname)


def parseOpts(argv):
    description = 'Synthetize error rates results (default : if no ' \
        'option is passed : --mean is activated\n' \
        'siSynthRelaxResult.py [OPTIONS] file1.csv file2.csv...'
    parser = OptionParser(description)
    parser.add_option('-d', '--database', action='store',
                      dest='database_name', metavar='TYPE', default='2008',
                      help='one among 2000, 2005, 2008 (default : %default)')
    parser.add_option('--csv', action='store',
                      dest='csvname', metavar='FILE', default=None,
                      help='store computed data in csv')
    parser.add_option('-m', '--mean', action='store_true',
                      dest='show_mean', default=False,
                      help='Show mean values. Take the mean value of each subject '
                      'and then compute the mean over subjects')
    parser.add_option('-c', '--correlation', action='store_true',
                      dest='show_correlation', default=False,
                      help='Show correlations between errors and energy')
    parser.add_option('-b', '--best', action='store_true',
                      dest='show_best', default=False,
                      help='Show best values. Take the best value of each subject '
                      'and then compute the mean over subjects')
    parser.add_option('-s', '--std', action='store_true',
                      dest='show_std', default=False,
                      help='Show standard deviation mesures. Take the best/mean '
                      ' (see -m option) value of each subject and then compute '
                      'the std over subjects')
    parser.add_option('-o', '--old', action='store_true',
                      dest='old', default=False,
                      help='old behaviour of siSynthRelaxResult.py')

    return parser, parser.parse_args(argv)


def main():
    global gen

    parser, (options, args) = parseOpts(sys.argv)
    csvlist = args[1:]
    if not (True in [options.show_mean, options.show_correlation,
                     options.show_best]):
        options.show_mean = True

    if options.database_name == '2000':
        gen = gen2000
    elif options.database_name == '2005':
        gen = gen2005
    else:
        print("error : %s is not a valid database" %
              options.database_name)
        sys.exit(1)

    all_res = {}
    for f in csvlist:
        res = process(f, options)
        all_res[f] = res

    if options.csvname:
        # write each result in a separated file
        for i, (file, res) in enumerate(all_res.items()):
            if res is None:
                continue
            csvname = options.csvname
            ind = csvname.rfind('.')
            file = csvname[:ind]
            ext = csvname[ind:]
            csvname = file + ('_%d' % i) + ext
            write_csv(csvname, res)


if __name__ == '__main__':
    main()
