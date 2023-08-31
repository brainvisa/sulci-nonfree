#!/usr/bin/env python

from six.moves.urllib.request import urlopen
from optparse import OptionParser
import sys
import tempfile
import os
import zipfile

import six

if not six.PY2:
    long = int


class Context(object):
    '''simulate Axon context for code reusability'''

    def write(self, *args):
        nargs = [str(arg) for arg in args]
        print(' '.join(nargs))

    def progress(self, progress, num):
        sys.stdout.write('\r%02d ' % int(float(progress * 100) / num)
                         + '%')
        sys.stdout.flush()

    def temporary(self, filetype):
        tfile = tempfile.mkstemp(suffix='.zip')
        filename = tfile[1]
        os.close(tfile[0])
        return filename


download_url = 'https://brainvisa.info/download/data'
modelsversion = '4.2'

parser = OptionParser(
    'get and unzip SPAM-based sulci models in a given directory')
parser.add_option('-o', '--output',
                  help='output directory (should be something/share/brainvisa-share-<version>')
parser.add_option('-i', '--input',
                  help='input URL, default: %s' % download_url)
parser.add_option('-v', '--version',
                  help='models version, default: %s' % modelsversion)
parser.add_option('--no-update', action='store_true',
                  help='Only download/install if models do not exist locally')

options, args = parser.parse_args(sys.argv)
if options.output is None:
    print('option -o is mandatory')
    parser.parse_args(['-h'])
if options.input is not None:
    download_url = options.input
if options.version is not None:
    modelsversion = options.version

context = Context()

destdir = options.output
if not os.path.exists(destdir):
    os.makedirs(destdir)
context.write('install in dir:', destdir)

if download_url.startswith('ftp://') or download_url.startswith('https://') \
        or download_url.startswith('http://'):
    local_files = False
else:
    local_files = True

files = []
if not options.no_update or not os.path.exists(os.path.join(
        destdir, 'models', 'models_2008', 'descriptive_models', 'segments',
        'talairach_spam_right', 'spam_distribs.dat')):
    files.append('descriptive_models-talairach-' + modelsversion + '.zip')
if not options.no_update or not os.path.exists(os.path.join(
        destdir, 'models', 'models_2008', 'descriptive_models', 'segments',
        'global_registered_spam_left', 'spam_distribs.dat')):
    files.append('descriptive_models-global-' + modelsversion + '.zip')
if not options.no_update or not os.path.exists(os.path.join(
        destdir, 'models', 'models_2008', 'descriptive_models', 'segments',
        'locally_from_global_registred_spam_right',
        'gaussian_translation_trm_priors',
        'bayesian_gaussian_density_S.T.s.ter.asc.post._right.dat')):
    files.append('descriptive_models-local-' + modelsversion + '.zip')
if not options.no_update or not os.path.exists(os.path.join(
        destdir, 'models', 'models_2008', 'descriptive_models', 'segments',
        'global_registered_spam_left', 'meshes',
        'spam_F.Cal.ant.-Sc.Cal._left_2.mesh')):
    files.append('descriptive_models-additional_data-' + modelsversion +
                 '.zip')

# taken from spam_install_model process in morphologist-gpl

pnum = len(files) * 100 + 10
pgs = 0
for fname in files:
    context.write('downloading', fname, '...')
    context.progress(pgs, pnum)
    if local_files:
        tzf = os.path.join(download_url, fname)
        context.write('file %s is local' % tzf)
    else:
        ftp = urlopen(download_url + '/' + fname)
        tzf = context.temporary('zip file')
        f = open(tzf, 'wb')
        fsize = long(ftp.headers.get('content-length'))
        chunksize = 100000
        fread = 0
        while fread < fsize:
            pg = fread * 80 / fsize
            context.progress(pgs + pg, pnum)
            f.write(ftp.read(chunksize))
            fread += chunksize
        context.write('download done')
        f.close()
    pgs += 80
    context.progress(pgs, pnum)
    context.write('installing', fname, '...')
    f = open(tzf, 'rb')
    zf = zipfile.ZipFile(f, 'r')
    # extract zip files one by one
    # extractall() is not an option since on Mac at least it tries to
    # re-make directories even if they exist
    namelist = zf.namelist()
    fnlist = []
    for name in namelist:
        dname = os.path.join(destdir, name)
        if os.path.exists(dname):
            if os.path.isdir(dname):
                pass  # skip existing dirs
            else:  # existing file: remove it first
                os.unlink(dname)
                fnlist.append(name)
        else:
            fnlist.append(name)
    del namelist
    zf.extractall(destdir, fnlist)
    zf.close()
    zf = None
    f.close()
    f = None
    pgs += 20
    if not local_files:
        os.unlink(tzf)
context.progress(pgs, pnum)
context.progress(100, 100)
print
