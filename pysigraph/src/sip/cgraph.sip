
namespace sigraph
{

class CGraph : Graph
{
%TypeHeaderCode
#include <si/graph/cgraph.h>
%End

public:
  CGraph( const std::string & );
  virtual ~CGraph();

  // const std::set<sigraph::Clique *> & cliques() const;
  set_rc_ptr_Clique & cliques();

  virtual void deleteCliques();
  virtual void randLabels();
  void setAllLabels( const std::string & );
  void ensureAllLabelsPossible();
  virtual void clearAll();
};

};

