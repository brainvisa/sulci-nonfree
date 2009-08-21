
namespace sigraph
{

class TrainerIterator
{
%TypeHeaderCode
#include <si/learner/traineriterator.h>
%End

public:
  TrainerIterator( const sigraph::TrainerIterator & );
  ~TrainerIterator();

  void train(carto::Object &);
  void next();
  bool isValid() const;

  sigraph::Model* model();
  sigraph::Adaptive* adaptive();
  int cycles() const;
  int testCycles() const;
};


class Trainer
{
%TypeHeaderCode
#include <si/learner/trainer.h>
%End

public:
  typedef sigraph::TrainerIterator iterator;

  Trainer( sigraph::MGraph &, sigraph::Learner * = 0);
  virtual ~Trainer();

  enum TrainerMode
  {
        GenerateOnly,
        GenerateAndTrain,
        ReadAndTrain,
        TrainDomain,
        TrainStats,
  };

  virtual void init(TrainerMode, unsigned = 0 );
  virtual void resetDomains();
  sigraph::Trainer::TrainerMode mode() const;
  void train_py( SIP_PYOBJECT, SIP_PYOBJECT = 0, int = 1, int = 1 ) /PyName=train/;
%MethodCode
  std::cout << "train\n";
  if( !PySequence_Check( a0 ) )
  {
    sipIsErr = 1;
    PyErr_SetString( PyExc_TypeError, "argument 1 is not a sequence" );
  }
  else if( a1 && !PySequence_Check( a1 ) )
  {
    sipIsErr = 1;
    PyErr_SetString( PyExc_TypeError, "argument 2 is not a sequence" );
  }
  else
  {
    int i, n = PySequence_Length( a0 );
    PyObject *po;
    std::set<sigraph::CGraph *> lrn, tst;
    void *ptr;

    for( i=0; i<n; ++i )
    {
      po = PySequence_ITEM( a0, i );
      Py_DECREF( po );
      if( !PyObject_TypeCheck( po, (PyTypeObject *) sipClass_sigraph_CGraph ) )
      {
        sipIsErr = 1;
        std::ostringstream s;
        s << "in argument 1: element " << i 
          << " is not a sigraph.CGraph instance";
        PyErr_SetString( PyExc_TypeError, s.str().c_str() );
        break;
      }
      ptr = sipForceConvertToInstance( po, sipClass_sigraph_CGraph, 0, 0, 0,
                                       &sipIsErr );
      if( ptr )
      {
        lrn.insert( reinterpret_cast<sigraph::CGraph *>( ptr ) );
        // should call sipReleadeInstance() !
      }
      else
        break;
    }
    if( !sipIsErr && a1 )
    {
      n = PySequence_Length( a1 );
      for( i=0; i<n; ++i )
      {
        po = PySequence_ITEM( a1, i );
        Py_DECREF( po );
        if( !PyObject_TypeCheck( po, 
                                 (PyTypeObject *) sipClass_sigraph_CGraph ) )
        {
          sipIsErr = 1;
          std::ostringstream s;
          s << "in argument 1: element " << i 
            << " is not a sigraph.CGraph instance";
          PyErr_SetString( PyExc_TypeError, s.str().c_str() );
          break;
        }
        ptr = sipForceConvertToInstance( po, sipClass_sigraph_CGraph, 0, 0, 0,
                                         &sipIsErr );
        if( ptr )
        {
          tst.insert( reinterpret_cast<sigraph::CGraph *>( ptr ) );
          // should call sipReleadeInstance() !
        }
        else
          break;
      }
    }
    if( !sipIsErr )
    {//FIXME : tochange : euuu
      //sipCpp->train( &lrn, a1 ? &tst : 0, a2, a3 );
    }
  }
%End

  sigraph::Trainer::iterator trainIterator_py( SIP_PYOBJECT, SIP_PYOBJECT = 0, 
    int = 1, int = 1 ) /PyName=trainIterator/;
%MethodCode
  if( !PySequence_Check( a0 ) )
  {
    sipIsErr = 1;
    PyErr_SetString( PyExc_TypeError, "argument 1 is not a sequence" );
  }
  else if( a1 && !PySequence_Check( a1 ) )
  {
    sipIsErr = 1;
    PyErr_SetString( PyExc_TypeError, "argument 2 is not a sequence" );
  }
  else
  {
    int i, n = PySequence_Length( a0 );
    PyObject *po;
    std::set<sigraph::CGraph *> lrn, tst;
    void *ptr;

    for( i=0; i<n; ++i )
    {
      po = PySequence_ITEM( a0, i );
      Py_DECREF( po );
      if( !PyObject_TypeCheck( po, (PyTypeObject *) sipClass_sigraph_CGraph ) )
      {
        sipIsErr = 1;
        std::ostringstream s;
        s << "in argument 1: element " << i 
          << " is not a sigraph.CGraph instance";
        PyErr_SetString( PyExc_TypeError, s.str().c_str() );
        break;
      }
      ptr = sipForceConvertToInstance( po, sipClass_sigraph_CGraph, 0, 0, 0,
                                       &sipIsErr );
      if( ptr )
      {
        lrn.insert( reinterpret_cast<sigraph::CGraph *>( ptr ) );
        // should call sipReleadeInstance() !
      }
      else
        break;
    }
    if( !sipIsErr && a1 )
    {
      n = PySequence_Length( a1 );
      for( i=0; i<n; ++i )
      {
        po = PySequence_ITEM( a1, i );
        Py_DECREF( po );
        if( !PyObject_TypeCheck( po, 
                                 (PyTypeObject *) sipClass_sigraph_CGraph ) )
        {
          sipIsErr = 1;
          std::ostringstream s;
          s << "in argument 1: element " << i 
            << " is not a sigraph.CGraph instance";
          PyErr_SetString( PyExc_TypeError, s.str().c_str() );
          break;
        }
        ptr = sipForceConvertToInstance( po, sipClass_sigraph_CGraph, 0, 0, 0,
                                         &sipIsErr );
        if( ptr )
        {
          tst.insert( reinterpret_cast<sigraph::CGraph *>( ptr ) );
          // should call sipReleadeInstance() !
        }
        else
          break;
      }
    }
    if( !sipIsErr )
    {
      sipRes = new sigraph::Trainer::iterator( sipCpp->trainIterator( &lrn, 
        a1 ? &tst : 0, a2, a3 ) );
    }
  }
%End

  virtual void trainOne( sigraph::Trainer::iterator &, carto::Object &);
  void generateDataBase( sigraph::Trainer::iterator &,
			const std::string &);
%MethodCode
  try
  {
    sipCpp->generateDataBase(*a0, *a1);
  }
  catch( std::exception & e )
  {
    sipIsErr = 1;
    PyErr_SetString( PyExc_RuntimeError, e.what() );
  }
%End


  void trainDomain( sigraph::Trainer::iterator & );
  void trainStats( sigraph::Trainer::iterator & );

  const sigraph::MGraph &getGraphModel() const;
};


class SelectiveTrainer : sigraph::Trainer
{
%TypeHeaderCode
#include <si/learner/selectiveTrainer.h>
#ifndef PYAIMSSIP_SET_STRING_DEFINED
#define PYAIMSSIP_SET_STRING_DEFINED
typedef std::set<std::string> set_STRING;
#endif
%End

public:
  SelectiveTrainer( sigraph::MGraph &, sigraph::Learner * = 0, 
                    const std::string & =".*" );
  virtual ~SelectiveTrainer();

  virtual void setPattern( const std::string & );
  virtual void setFiltAttributes( const set_STRING & );
  virtual void save( sigraph::MGWriter & );
  virtual const set_AdaptivePtr &usedAdap() const;
};

};

