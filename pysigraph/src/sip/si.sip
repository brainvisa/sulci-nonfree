
namespace sigraph
{

class Si
{
%TypeHeaderCode
#include <si/global/global.h>
%End

public:
  Si();
  ~Si();
  std::string basePath() const;
  std::string labelsTranslPath() const;
  void setLabelsTranslPath( const std::string & );

  static const std::string & version();
  static const std::string & shortVersion();
};

sigraph::Si & si();

};

