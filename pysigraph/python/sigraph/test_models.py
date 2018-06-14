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


def resume_errors_info(data):
    import numpy
    import datamind.tools as T

    def print_errors(type, errors):
        if len(errors) == 0:
            print('\tN/D\tN/D\tN/D\tN/D')
            return
        list = (float(numpy.mean(errors)), float(numpy.std(errors)),
                float(max(errors)), float(min(errors)))
        T.msg.write_list([(type, 'green'),
                          '\t%.4f\t%.4f\t%.4f\t%.4f\n' % list])
    T.msg.write('    \tMean\tStd\tMax\tMin\n', 'red')
    print_errors('Raw', data['Raw'])
    print_errors('Mean', data['Mean'])
    print_errors('Good', data['Good'])
    print_errors('Bad', data['Bad'])


def make_errors_hist(filename, data, title='Model fitting errors'):
    import pylab
    import numpy

    def generic_hist(axes, data, subtitle):
        h = pylab.hist(data)
        ma = axes.get_ylim()[1]
        me = numpy.mean(data)
        pylab.plot([me, me], [0, ma], color='red', linewidth=2)
        pylab.title(subtitle)

    f = pylab.figure()
    ax1 = pylab.subplot(221)
    generic_hist(ax1, data['Raw'], 'Raw')
    ax2 = pylab.subplot(222)
    generic_hist(ax2, data['Mean'], 'Mean')
    ax3 = pylab.subplot(223)
    generic_hist(ax3, data['Good'], 'Good')
    ax4 = pylab.subplot(224)
    generic_hist(ax4, data['Bad'], 'Bad')
    pylab.subplots_adjust(top=0.8)
    f.text(0.5, 0.9, title, verticalalignment='bottom',
           horizontalalignment='center', fontsize=16)
    pylab.savefig(filename)
