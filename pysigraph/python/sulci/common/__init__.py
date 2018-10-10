from __future__ import print_function
import os
import numpy
from soma import aims
import socket
import six


def add_translation_option_to_parser(parser):
    transfile = os.path.join(aims.carto.Paths.shfjShared(), 'nomenclature',
                             'translation', 'sulci_model_noroots.trl')

    parser.add_option('-t', '--translation', dest='transfile',
                      metavar='FILE', action='store', default=transfile,
                      help='translation file (default : %default)')


def vertex2voxels(motion, vertex, data_type):
    if data_type == 'simple_surface':
        map = vertex['aims_ss'].get()
    elif data_type == 'bottom':
        map = vertex['aims_bottom'].get()
    s = numpy.array([map.sizeX(), map.sizeY(), map.sizeZ()])
    vox = [motion.transform(aims.Point3df(p * s)) for p in map[0].keys()]
    return vox

import os
import distutils.spawn
import importlib
import time
import getpass
import re
StringIO = six.moves


def import_from(filename):
    modulename = distutils.spawn.find_executable(filename)
    if modulename is None:
        raise ImportError(filename)
    path, file = os.path.split(modulename)
    name, ext = os.path.splitext(file)
    spec = importlib.util.spec_from_file_locarion(name, modulename)
    if not spec:
        raise ImportError(name)
    return importlib.util.module_from_spec(spec)


class NoMessage(object):

    def haveColor(self):
        return False

    def info(self, msg): pass

    def error(self, msg): pass

    def warning(self, msg): pass

    def string(self, msg, color='back'): pass

    def write(self, msg, color='back'): pass

    def write_list(self, msg_list): pass

try:
    grid = import_from('grid.py')
except ImportError as e:
    # print("warning: can't load grid module '%s'" %e)
    pass
else:
    grid.msg = NoMessage()


def unlock(filename):
    try:
        os.unlink(filename)  # atomic on NFS
    except OSError:
        pass  # allready unlock


def lock(filename, timeout=60):
    '''
When lock is supposed to be own by another process

filename : lock filename
timeout:   in seconds
    '''
    host, pid = socket.gethostname(), os.getpid()
    # make sure to create the temp file in the same directory
    # (on the same filesystem)
    tmp = "%s.lock.%s.%s" % (filename, host, pid)
    fd = open(tmp, 'w')
    fd.write("%s\n%s\n" % (host, pid))
    fd.close()
    while 1:
        try:
            os.link(tmp, filename)  # atomic on NFS
        except OSError:
            pass  # ignore errors (known bug on NFS)
        tmp_stats = os.stat(tmp)
        if tmp_stats[3] == 2:
            os.unlink(tmp)
            return
        try:
            lock_stats = os.stat(filename)
        except OSError as e:
            time.sleep(0.5)
            continue
        last_modification = lock_stats[8]
        now = time.time()
        if now - last_modification > timeout:
            try:
                fd = open(filename)
                host, pid = [l.strip() for l in fd.readlines()]
            except OSError:
                time.sleep(0.1)
                continue
            except IOError:
                time.sleep(0.1)
                continue
            finally:
                fd.close()
            if not remote_process_is_active(host, pid):
                unlock(filename)
                continue
    time.sleep(0.1)


def remote_process_is_active(hostname, pid):
    dir = '/proc/%s' % pid
    localhost = os.getenv('HOSTNAME')
    # local connexion
    if hostname in [localhost, 'localhost', '127.0.0.1']:
        try:
            os.listdir(dir)
            return True
        except OSError:
            return False
    # remote connexion
    timeslot = grid.read_timeslot('-')
    passwd = None
    keytype = 'dsa'
    user = grid.User(getpass.getuser(), passwd, keytype)
    tasks = grid.Task('ls -d %s > /dev/null' % dir)
    hosts = [hostname]
    log = StringIO.StringIO()
    brokenfd = StringIO.StringIO()
    hm = grid.HostsManager(hosts)
    if len(hm._available_list) == 0:
        return False
    # run test
    tm = grid.OneTaskManager(timeslot, user, tasks, hm, log, brokenfd)
    try:
        tm.start()
    except:
        return False
    # read result
    log.seek(0)
    brokenfd.seek(0)
    lines = log.readlines()
    brokenfd.close()
    log.close()
    if len(lines) < 3:
        return False
    m = re.match("# exit status = (?P<status>\d+)", lines[2])
    status = int(m.group('status'))
    if status == 0:
        return True
    return False
