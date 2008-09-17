


namespace sigraph
{

class Learner : Tree /Abstract/
{
%TypeHeaderCode
#include <si/learner/learner.h>
%End

public:
  virtual ~Learner();
};


/* not needed yet
class LearnReader
{
%TypeHeaderCode
#include <si/learner/learnReader.h>
%End

public:
  LearnReader( const std::string & );
  virtual ~LearnReader();
};
*/


class FoldLearnReader
{
%TypeHeaderCode
#include <si/fold/foldLearnReader.h>
%End

public:
  FoldLearnReader( const std::string & );
  virtual ~FoldLearnReader();
  virtual Tree *read();
%MethodCode
  try
  {
    sipRes = sipCpp->read();
  }
  catch( std::exception & e )
  {
    sipIsErr = 1;
    PyErr_SetString( PyExc_RuntimeError, e.what() );
    sipRes = 0;
  }
%End

private:
  FoldLearnReader( const sigraph::FoldLearnReader & );
};

};

