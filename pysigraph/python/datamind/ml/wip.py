# Copyright CEA (2000-2006)
#
#  This software and supporting documentation were developed by
#      CEA/DSV/SHFJ
#      4 place du General Leclerc
#      91401 Orsay cedex
#      France
#
# This software is governed by the CeCILL license version 2 under
# French law and abiding by the rules of distribution of free software.
# You can  use, modify and/or redistribute the software under the
# terms of the CeCILL license version 2 as circulated by CEA, CNRS
# and INRIA at the following URL "http://www.cecill.info".
#
# As a counterpart to the access to the source code and  rights to copy,
# modify and redistribute granted by the license, users are provided only
# with a limited warranty  and the software's author,  the holder of the
# economic rights,  and the successive licensors  have only  limited
# liability.
#
# In this respect, the user's attention is drawn to the risks associated
# with loading,  using,  modifying and/or developing or reproducing the
# software by the user in light of its specific status of free software,
# that may mean  that it is complicated to manipulate,  and  that  also
# therefore means  that it is reserved for developers  and  experienced
# professionals having in-depth computer knowledge. Users are therefore
# encouraged to load and test the software's suitability as regards their
# requirements in conditions enabling the security of their systems and/or
# data to be ensured and,  more generally, to use and operate it in the
# same conditions as regards security.
#
# The fact that you are presently reading this means that you have had
# knowledge of the CeCILL license version 2 and that you accept its terms.

from __future__ import print_function


from __future__ import absolute_import
class WipError(Exception):
    pass


def not_implemented(msg='', function=''):
    '''
Print a msg stressing that a function is not implemented yet.

msg :		additional message
function :	function name'''
    import datamind.tools as tools
    fixme = tools.msg.string('FIXME', 'bold_yellow')
    if function == '':
        raise WipError('%s : not implemented yet\n%s' % (fixme, msg))
    else:
        raise WipError("%s : function '%s' isn't "
                       "implemented yet\n%s" % (fixme, function, msg))


def fixme(msg=''):
    '''
Print 'FIXME' and an additional message specified by 'msg' parameter.'''
    import datamind.tools as tools
    fixme = tools.msg.string('FIXME', 'bold_yellow')
    if msg == '':

        print(fixme)
    else:
        print(fixme, ':', msg)


def warning(msg):
    '''
Print a warning message.

msg :     message'''
    import datamind.tools as tools
    tools.msg.warning(msg)


def todo(msg, priority='low', user='unknown'):
    '''
Print a todo message.

msg :         defition of the task.
priority :    priority of task (suggestions : low, high, medium).
user :        name of people who ask this task.'''
    import datamind.tools as tools
    todo = tools.msg.string('TODO', 'bold_yellow')
    print(todo, '(' + user + ' : ' + priority + '):', msg)
