


namespace sigraph
{

class FoldLabelsTranslator 
{
%TypeHeaderCode
#include <si/fold/labelsTranslator.h>
%End

public:
  FoldLabelsTranslator();
  FoldLabelsTranslator( const std::string & );
  FoldLabelsTranslator( const sigraph::MGraph &mg, const std::string &filename="" );
  virtual ~FoldLabelsTranslator();

  void readLabels( const std::string &filename );
  void makeFromModel( const sigraph::MGraph &mg, const std::string &filename );

  void translate( sigraph::CGraph&, const std::string & ="name",
    const std::string & ="name", const std::string & = "" ) const;
  std::string lookupLabel( std::string label ) const;
};

};

