# Copyright CEA and IFR 49 (2000-2005)
#
#  This software and supporting documentation were developed by
#      CEA/DSV/SHFJ and IFR 49
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

include( 'base' )

#local helper
templ_model1 = ( '*', SetType( 'Template model' ),
                 '*', SetType( 'Template model domain' ) )
templ_model2 = apply( SetContent, 
                      ( 'model', apply( SetContent, templ_model1 + \
                        ( '{model_element_type}', apply( SetContent,
                                                         templ_model1 ) ) ),
                        'hull_junction', SetType( 'Template model' ),
                        SetWeakAttr( 'model_element_type', 'hull_junction' ), 
                        'fakerel', SetType( 'Template model' ),
                        SetWeakAttr( 'model_element_type', 'fake_relation' ),
                        '*', SetType( 'Sigraph Learner' ), 
                        ) )

insertFirst( 'nomenclature/hierarchy',
  'sulcal_root_colors', SetPriorityOffset( +10 ), SetType( 'Nomenclature' ),
)

insertLast('nomenclature/translation',
  '*', SetType( "Label Translation" ),
)

insert( '',
  'models', SetContent( 
    "model_templates", templ_model2,
    '{graph_version}', SetContent( 
      "L{model}", SetContent(
        "L{model}", SetType( 'Model graph' ), SetWeakAttr( 'side', 'left' ),
        "*", SetType( 'Model graph' ),
      ),
      "R{model}", SetContent(
        "R{model}", SetType( 'Model graph' ), SetWeakAttr( 'side', 'right' ),
        "*", SetType( 'Model graph' ),
      ),
      "{model}", SetContent( 
        "{model}", SetType( 'Model graph' ),  
        "R*", SetWeakAttr( 'side', 'right' ), SetType( 'Model graph' ),
        "L*", SetWeakAttr( 'side', 'left' ), SetType( 'Model graph' ),
        "*", SetType( 'Model graph' ),
      ),
    ), 
  ),
)
