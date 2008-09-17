


namespace sigraph
{

class FoldWriter //: sigraph::GraphWriter 
{
%TypeHeaderCode
#include <si/fold/foldWriter.h>
%End

public:
	FoldWriter(const std::string &filename);
	virtual ~FoldWriter();
	virtual void write(Graph &ao);
private:
	FoldWriter(const sigraph::FoldWriter&);
};

};

