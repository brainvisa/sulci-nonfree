from __future__ import absolute_import
import csv
import string
import re
import os.path
import numpy
import datamind.core as dmc
from datamind.ml.database import DbNumpy
from datamind.tools import msg
import sys
from six.moves import range

if sys.version_info[0] >= 3:
    def execfile(filename, globals=None, locals=None):
        if globals is None:
            globals = sys._getframe(1).f_globals
        if locals is None:
            locals = sys._getframe(1).f_locals
        with open(filename, "r") as fh:
            exec(fh.read() + "\n", globals, locals)


# FIXME : ajouter gestion de la colonne GROUP <-> db.getGroups()?
class ReaderHeaderCsv(object):

    '''
Csv reader from minf header dictionary. Usgage :

ReaderHeaderCsv().read(csv_filename, header_dict)

More doc see help : (ReaderHeaderCsv.read).
    '''

    def _read_csv_from_header(self, prefix, header, sep):
        try:
            csv_filename = header['data']
        except KeyError:
            msg.error('no datafile defined')
            return False
        if not csv_filename.startswith(os.sep):
            csv_filename = os.path.join(prefix, csv_filename)
        return self._read_csv(csv_filename, header, sep)

    def _set_dims_if_needed(self, header, dims):
        if 'dims' not in header:
            header['dims'] = dims

    def _check_dims(self, line, header, size):
        '''
Check if current line follow supposed number of dimensions according
to header. Raise IOError exception if it failed.

line :      current line number.
header :    header dictionnary.
size :      number of columns of current line.
        '''
        if size != header['dims']:
            msg.error('line %d' % line + ' : wrong number of ' +
                      'dimensions : %d != %d' % (header['dims'], size))
            raise IOError

    def _read_csv(self, csv_filename, header, sep):
        try:
            fd = open(csv_filename, 'r')
        except IOError as e:
            msg.error("%s" % e)
            return False
        # Try to read header
        first = fd.readline().rstrip('\n\t')
        if sep is None:
            sep = '[\t]+'
        first = re.split(sep, first)
        first_array = numpy.array(first, dtype='O')
        x_ind, y_ind, inf_ind = header['X'], header['Y'], header['INF']
        xy_ind = x_ind + y_ind
        xy_first = first_array[xy_ind]
        try:
            for x in xy_first:
                numpy.double(x)
            fd.seek(0)  # go back to begining
        except ValueError:  # header found : check
            if 'labels' not in header:
                header['labels'] = first
            elif first != header['labels']:
                msg.warning('minf names and csv header differs'
                            ' :\n\t%s != %s' %
                            (str(header['labels']), str(first)))
            self._set_dims_if_needed(header, len(first))
        # FIXME : si quelqu'un trouve une meilleure facon de faire...
        # read data
        xs, ys, infs = [], [], []
        size = None
        for i, line in enumerate(fd.readlines()):
            line = re.split(sep, line.rstrip('\n\t'))
            if size is None:
                self._set_dims_if_needed(header, len(line))
            self._check_dims(i, header, len(line))
            lina = numpy.array(line, dtype='O')
            x, y, inf = lina[x_ind], lina[y_ind], lina[inf_ind]
            try:
                xs.append([numpy.double(s) for s in x.tolist()])
            except ValueError:
                msg.error("l.%d: can't convert X " % (i + 1) +
                          "data to double")
                return False
            try:
                ys.append([numpy.double(s) for s in y.tolist()])
            except ValueError:
                msg.error("l.%d: can't convert Y " % (i + 1) +
                          "data to double")
                return False
            infs.append(inf.tolist())
        X = numpy.array(xs, dtype='double')
        Y = numpy.array(ys, dtype='double')
        # FIXME : 32 chars only for each info data
        INF = numpy.array(infs, dtype='S32')
        fd.close()
        # final check
        xcols_n, ycols_n, icols_n = X.shape[1], Y.shape[1], INF.shape[1]
        if not xcols_n:
            X = None
        if not ycols_n:
            Y = None
        if not icols_n:
            INF = None
        acols_n = xcols_n + ycols_n + icols_n
        if 'dims' in header and acols_n != header['dims']:
            msg.warning('wrong value for dim in header : '
                        '%d (%d + %d + %d) != %d' %
                        (acols_n, xcols_n, ycols_n, icols_n,
                         header['dims']))
        size = X.shape[0]
        if 'size' in header and size != header['size']:
            msg.warning('number of vectors and size value in header'				' differs : %d != %d' %
                        (size, header['size']))
        return DbNumpy(X, Y, INF), header

    def read(self, csv_filename, minf_header, sep=None):
        '''
Read CSV file based on meta informations in header.
Note : Until now, types in header are not important.

csv_filename : text data file storing an array, each data separated by sep.
sep :           csv seperator, default all blanks. It can be a regexp.
        '''
        import os
        prefix = os.path.dirname(csv_filename)
        minf_header['data'] = os.path.basename(csv_filename)
        return self._read_csv_from_header(prefix, minf_header, sep)


def create_empty_minf_header():
    '''
Return empty minf header under dictionary shape,
with following keys :

format :   database_1.0 : to identify current version of minf file
'labels' : list of names of each columns (optional).
'types' :  list of type ('double', 'string') of each columns (unused).
'X' :      list of column indices for matrix of data
           (mandatory : may be empty).
'Y' :      list of column indices for class or data to regress to
           (mandatory : may be empty).
'INF' :    list of column indices for matrix of information
           (mandatory : may be empty).
'size' :   number of vectors (line) in database (optional).
'dims' :   number of features (columns) in database (optional).
'data' :   filename of csv (mandatory : data values).

.minf minimal file format is :

$ cat test.minf
attributes = {
    'format' : 'database_1.0',
    'data' : 'test.data'
}

If unknown fields are added in dictionnary, they are read and you can
access to them later from read header.
    '''
    h = {
        'format': 'database_1.0',
            'labels': [],
            'types': [],
            'X': [], 'Y': [], 'INF': [],
            'size': 0,
            'dims': 0,
            'data': '',
    }
    return h


class ReaderMinfCsv(ReaderHeaderCsv):

    '''
Simple Csv reader with minf header file. Usage :

ReaderMinfCsv().read(minf_filename)

More doc see help : (ReaderMinfCsv.read).
    '''

    def _read_minf(self, minf_filename):
        try:
            exec(compile(open(minf_filename).read(), minf_filename, 'exec'))
            header = locals()['attributes']
            return header
        except Exception as e:
            msg.error(e)
            return False

    def read(self, minf_filename, sep=None):
        '''
Read minf and nested CSV file.
Note : Until now, types in minf file are not important.

minf_filename : filename of metainformation file. For format see
            help(create_empty_minf_header).
sep :           csv seperator, default all blanks. It can be a regexp.
        '''
        import os
        prefix = os.path.dirname(minf_filename)
        header = self._read_minf(minf_filename)
        if not header:
            return False
        return self._read_csv_from_header(prefix, header, sep)


#
class WriterCsv(object):

    '''
CSV writer. Usage :

1) WriterCsv().write(csv_filename, db)
2) WriterCsv().write(csv_filename, db, header)
3) WriterCsv().write(csv_filename, db, header, minf_ilename)

More doc see : help(WriterCsv.write).
    '''

    def _get_cols_length(self, db):
        X, Y, I = db.getX(), db.getY(), db.getINF()
        if X is None:
            xcols_n = 0
        else:
            xcols_n = X.shape[1]
        if Y is None:
            ycols_n = 0
        else:
            ycols_n = Y.shape[1]
        if I is None:
            icols_n = 0
        else:
            icols_n = I.shape[1]
        return xcols_n, ycols_n, icols_n

    def _read_header(self, db, header):
        xcols_n, ycols_n, icols_n = self._get_cols_length(db)
        xcols = header.get('X', [])
        ycols = header.get('Y', [])
        icols = header.get('INF', [])
        h = {	'X': (xcols, xcols_n),
              'Y': (ycols, ycols_n),
              'INF': (icols, icols_n)}
        error = False
        for k, (t, s) in h.items():
            if len(t) == s:
                continue
            error = True
            msg.error('%s header does not match with ' % k +
                      'data shape : %d != %d' % (len(t), s))
            return False
        return xcols + ycols + icols, header.get('user', None)

    def _write_minf(self, minf, csv, db, names, user):
        '''
Write minf file with metainformation.

types are fixed to 'double' for X and Y and 'string' to INF.

Return True if success.
        '''
        try:
            fd = open(minf, 'w')
        except IOError as e:
            msg.error("%s" % e)
            return False
        xcols_n, ycols_n, icols_n = self._get_cols_length(db)
        xycols_n = xcols_n + ycols_n
        dim = db.getColsNumber()
        xind = list(range(xcols_n))
        yind = list(range(xcols_n, xycols_n))
        infind = list(range(xycols_n, xycols_n + icols_n))
        h = create_empty_minf_header()
        h['labels'] = names
        h['types'] = ['double'] * xycols_n + ['string'] * icols_n
        h['X'], h['Y'], h['INF'] = xind, yind, infind
        h['size'] = db.getX().shape[0]
        h['dims'] = dim
        h['data'] = csv
        fd.write('attributes = {\n')
        keys = ['format', 'labels', 'types', 'X', 'Y', 'INF',
                'size', 'dims',	'data']
        for k in keys:
            v = h[k]
            fd.write('\t%s : %s,\n' % (repr(k), repr(v)))
        if user is not None:
            k, v = user
            fd.write('\t%s : %s,\n' % (repr(k), repr(v)))
        fd.write('}\n')
        fd.close()
        return True

    def _write_csv_header(self, fd, names):
        '''
Write csv header to fd.

Return True if success.
        '''
        fd.write('\t'.join(names) + '\n')
        return True

    def _write_csv(self, file, db, names):
        '''
Write csv file.

file : csv file.
db :   database.

        '''
        try:
            fd = open(file, 'w')
        except IOError as e:
            msg.error("%s" % e)
            return False
        X, Y, I = db.getX(), db.getY(), db.getINF()
        if names is not None and not self._write_csv_header(fd, names):
            fd.close()
            return False
        size = len(X)
        for i in range(size):
            if X is None:
                xs = []
            else:
                xs = [str(j) for j in X[i]]
            if Y is None:
                ys = []
            else:
                ys = [str(j) for j in Y[i]]
            if I is None:
                infs = []
            else:
                infs = [str(j) for j in I[i]]
            hlist = xs + ys + infs
            fd.write('\t'.join(hlist) + '\n')
        fd.close()
        return True

    def _read_db(self, db):
        '''
Read data and transform it to datamind database.

db : database, matix, or list of 1, 2, or 3 matrices.
        '''
        from datamind.ml import database
        if isinstance(db, database.Db):
            return db
        if isinstance(db, tuple):
            if len(db) == 1:
                X, Y, INF = db + [None, None]
            if len(db) == 2:
                X, Y, INF = db + [None]
            if len(db) == 3:
                X, Y, INF = db
            return DbNumpy(*db)
        # I guess it's a matrix
        return DbNumpy(db)

    def write(self, file, db, header=None, minf=None):
        '''
Write csv in given file. First row gives columns name, others : data matrix.
Write X matric columns first, Y matrix columns second, INFO matrix columns
finally.

file :    csv output file.
db :      database or matrix or list of 1, 2 or 3 matrices to dump.
      if 1 matrix is given or a list of 1 matrix : taken as X data.
      if a list of 2 matrices is given : taken as (X, Y).
      if a list of 3 matrices is given : taken as (X, Y, INF).
header :  dictionnary of columns names.
      ex : {'X' : ['age', 'size'], 'Y' : ['class'], 'INF' : ['number']}
      One other key, can be added : 'user' to add specific application
      data in a dictionnary.
      ex : {'user' : ('mysubdic', {'myfield1' : 1, 'myfield2' : 2})}
      if defined, first line of csv is the header.
minf :    if defined (filename), a meta information file is written. For
      format see help(create_empty_minf_header).

Return True if success.
        '''
        error = False
        names = None
        db = self._read_db(db)
        if header is not None:
            val = self._read_header(db, header)
            if val == False:
                return False
            else:
                names, user = val
        if minf is not None and \
                not self._write_minf(minf, file, db, names, user):
            return False
        if minf is not None and os.path.dirname(file) == '':
            file = os.path.join(os.path.dirname(minf), file)
        return self._write_csv(file, db, names)
