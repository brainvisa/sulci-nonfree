/*
 *  Copyright (C) 1998-2005 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */

#include <anatomist/application/fileDialog.h>
#include <anafold/fgraph/qwAnnealParams.h>
#include <si/graph/annealConfigurator.h>
#include <si/fold/frgraph.h>
#include <anafold/fgraph/afgraph.h>
#include <cartobase/thread/thread.h>
#include <cartobase/thread/semaphore.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qtooltip.h>
#include <qvalidator.h>
#include <qevent.h>
#include <qapplication.h>
#include <iostream>

using namespace anatomist;
using namespace sigraph;
using namespace aims;
using namespace carto;
using namespace std;


namespace
{

  class ThreadBridgeEvent : public QEvent
  {
  public:
    ThreadBridgeEvent() : QEvent( QEvent::Type( QEvent::User + 103 ) ) {}
    virtual ~ThreadBridgeEvent() {}
  };

}


class QAnnealParams::AnnealPThread : public Thread
{
public:
  AnnealPThread( QAnnealParams* ap ) : Thread(), _ap( ap ) {}
  virtual ~AnnealPThread() {}
  virtual void doRun()
  { _ap->annealThread(); }

private:
  QAnnealParams	*_ap;
};


struct QAnnealParams::PrivateData
{
  PrivateData( QAnnealParams* ap )
    : init( 0 ), mode( 0 ), iter( 0 ),
      transl( 0 ), temp( 0 ), rate( 0 ), tempicm( 0 ), stoprate( 0 ),
      verbose( 0 ), gibbschange( 0 ), removebrain( 0 ), setbrain( 0 ),
      setweight( 0 ), outplot( 0 ), initlabels( 0 ), voidlabel( 0 ),
      voidmode( 0 ), voidoccur( 0 ), extmode( 0 ), extoccur( 0 ),
      ddrawlots( 0 ), stoppass( 0 ), annealThread( ap ),
      annealThRunning( QAnnealParams::STOPPED ),
      triggerAnnealState( QAnnealParams::STOPPED ),
      threaded( true )
  {}
  ~PrivateData() {}

  QComboBox			*init;
  QComboBox			*mode;
  QComboBox			*iter;
  QComboBox			*transl;
  QLineEdit			*temp;
  QLineEdit			*rate;
  QLineEdit			*tempicm;
  QLineEdit			*stoprate;
  QComboBox			*verbose;
  QLineEdit			*gibbschange;
  QComboBox			*removebrain;
  QComboBox			*setbrain;
  QLineEdit			*setweight;
  QComboBox			*outplot;
  QComboBox			*initlabels;
  QLineEdit			*voidlabel;
  QComboBox			*voidmode;
  QLineEdit			*voidoccur;
  QComboBox			*extmode;
  QLineEdit			*extoccur;
  QComboBox			*ddrawlots;
  QLineEdit			*stoppass;
  QAnnealParams::AnnealPThread	annealThread;
  Semaphore			interfaceSem;
  QAnnealParams::State		annealThRunning;
  QAnnealParams::State		triggerAnnealState;
  bool				threaded;
};


QAnnealParams::QAnnealParams( QWidget* parent, const char*,
			      AFGraph* fusion )
  : QWidget( parent ),
  _fusion( fusion ), pdat( new QAnnealParams::PrivateData( this ) )
{
  _conf = new AnnealConfigurator;
  fusion->addObserver( this );

  setAttribute( Qt::WA_DeleteOnClose );
  setObjectName( "QAnnealParams" );
  setWindowTitle( tr( "Annealing" ) + " : " + fusion->name().c_str() );

  QVBoxLayout	*lay1 = new QVBoxLayout; //( this, 10, -1, "QAnnealParamsLay1" );
  setLayout( lay1 );
  lay1->setMargin( 10 );
  lay1->setMargin( 5 );
  QWidget	*iobox = new QWidget( this );
  QHBoxLayout *iolay = new QHBoxLayout;
  iobox->setLayout( iolay );
  iolay->setSpacing( 10 );
  iolay->setMargin( 5 );
  QPushButton	*loadBtn = new QPushButton( tr( "Load config" ), iobox );
  QPushButton	*saveBtn = new QPushButton( tr( "Save config" ), iobox );
  iolay->addWidget( loadBtn );
  iolay->addWidget( saveBtn );

  QCheckBox	*thrBox = new QCheckBox( tr( "Threaded interface" ), this );
  thrBox->setToolTip( tr( "Using threads allows to keep Anatomist user\n"
                          "interface active during the several-hours\n"
                          "long annealing process, and allows to stop it\n"
                          "but it's more subject to bugs, so don't play\n"
                          "too much with Anatomist during the process,\n"
                          "it can easily crash.\n"
                          "If not threaded, you won't be able to use\n"
                          "this Anatomist until the relaxation is \n"
                          "finished (hope 2-4 hours...)\n"
                          "(expect ~400 passes for a complete "
                          "recognition)" ) );
  thrBox->setChecked( pdat->threaded );
  connect( thrBox, SIGNAL( toggled( bool ) ), this,
	   SLOT( setThreaded( bool ) ) );

  //	params
  QWidget *parbox = new QWidget( this );
  QGridLayout *parlay = new QGridLayout;
  parbox->setLayout( parlay );
  parlay->setSpacing( 3 );
  parlay->setMargin( 0 );
  int row = 0;
  QLabel	*l = new QLabel( tr( "Initialize annealing :" ), parbox );
  parlay->addWidget( l, row, 0 );
  l->setToolTip( tr( "If not set, annealing will not be initialized\n"
                     "(labels will be left unchanged, to continue "
                    "an interrupted relaxation for instance)" ) );
  pdat->init = new QComboBox( parbox );
  pdat->init->setObjectName( "init" );
  parlay->addWidget( pdat->init, row++, 1 );
  pdat->init->addItem( tr( "yes" ) );
  pdat->init->addItem( tr( "no" ) );
  l = new QLabel( tr( "Mode :" ), parbox );
  parlay->addWidget( l, row, 0 );
  l->setToolTip( tr( "Annealing transition accept/reject mode :\n"
                    "Gibbs sampler, Metropolis or ICM (deterministic)" ) );
  pdat->mode = new QComboBox( parbox );
  pdat->mode->setObjectName( "mode" );
  parlay->addWidget( pdat->mode, row++, 1 );
  pdat->mode->addItem( "gibbs" );
  pdat->mode->addItem( "metro" );
  pdat->mode->addItem( "icm" );
  l = new QLabel( tr( "Iteration mode :" ), parbox );
  parlay->addWidget( l, row, 0 );
  l->setToolTip( tr( "Transitions iterate mode\n"
                    "don't select CLIQUE - it's useless !" ) );
  pdat->iter = new QComboBox( parbox );
  parlay->addWidget( pdat->iter, row++, 1 );
  pdat->iter->setObjectName( "iter" );
  pdat->iter->addItem( "VERTEX" );
  pdat->iter->addItem( "CLIQUE" );
  l = new QLabel( tr( "Translation file :" ), parbox );
  parlay->addWidget( l, row, 0 );
  l->setToolTip( tr( "Labels translation file\n"
                     "Tells how to translate elements names\n"
                     "from one nomenclature to the model one.\n"
                     "If none, the default one is used\n"
                     "(see SiGraph library for details)" ) );
  QWidget	*tb = new QWidget( parbox );
  parlay->addWidget( tb, row++, 1 );
  QHBoxLayout *tblay = new QHBoxLayout;
  tb->setLayout( tblay );
  tblay->setSpacing( 5 );
  pdat->transl = new QComboBox( tb );
  pdat->transl->setObjectName( "transl" );
  pdat->transl->addItem( tr( "<default>" ) );
  QPushButton	*trbtn = new QPushButton( "...", tb );
  tblay->addWidget( pdat->transl );
  tblay->addWidget( trbtn );
  trbtn->setFixedHeight( pdat->transl->sizeHint().height() );
  trbtn->setFixedWidth( trbtn->sizeHint().width() );
  l = new QLabel( tr( "Temperature :" ), parbox );
  parlay->addWidget( l, row, 0 );
  l->setToolTip(tr( "Starting annealing temperature, will decrease "
                    "during the process" ) );
  pdat->temp = new QLineEdit( "50", parbox );
  parlay->addWidget( pdat->temp, row++, 1 );
  l = new QLabel( tr( "Rate :" ), parbox );
  parlay->addWidget( l, row, 0 );
  l->setToolTip( tr( "Temperature decreasing rate - at each pass\n"
                     "the temperature will be multiplied by "
                     "this factor" ) );
  pdat->rate = new QLineEdit( "0.98", parbox );
  parlay->addWidget( pdat->rate, row++, 1 );
  l = new QLabel( tr( "ICM switching temp :" ), parbox );
  parlay->addWidget( l, row, 0 );
  l->setToolTip( tr( "When the temperature gets lower than this\n"
                     "threshold, annealing automatically switches\n"
                     "to ICM deterministic mode.\n"
                     "If left to 0, ICM is used only after a whole pass\n"
                     "with no changes" ) );
  pdat->tempicm = new QLineEdit( "0", parbox );
  parlay->addWidget( pdat->tempicm, row++, 1 );
  l = new QLabel( tr( "Stop rate :" ), parbox );
  parlay->addWidget( l, row, 0 );
  l->setToolTip( tr( "% of accepted transitions below which the "
                     "annealing stops" ) );
  pdat->stoprate = new QLineEdit( "0.01", parbox );
  parlay->addWidget( pdat->stoprate, row++, 1 );
  l = new QLabel( tr( "Verbose :" ), parbox );
  parlay->addWidget( l, row, 0 );
  l->setToolTip( tr( "If set, verbose mode prints lots of counters\n"
                     "to keep you awaken during relaxation" ) );
  pdat->verbose = new QComboBox( parbox );
  pdat->verbose->setObjectName( "verbose" );
  parlay->addWidget( pdat->verbose, row++, 1 );
  pdat->verbose->addItem( tr( "yes" ) );
  pdat->verbose->addItem( tr( "no" ) );
  l = new QLabel( tr( "Gibbs nodes changes :" ), parbox );
  parlay->addWidget( l, row, 0 );
  l->setToolTip( tr( "Number of nodes allowed to change simultaneously "
                     "to form a Gibbs transition\n"
                     "LEAVE IT TO 1 FOR FOLDS RECOGNITION, or annealing "
                     "will last for months..." ) );
  pdat->gibbschange = new QLineEdit( "1", parbox );
  parlay->addWidget( pdat->gibbschange, row++, 1 );
  l = new QLabel( tr( "Remove brain :" ), parbox );
  parlay->addWidget( l, row, 0 );
  l->setToolTip( tr( "Removes possible 'brain' nodes in graph -\n"
                     "this shouldn't happen in newer graphs, but it's "
                     "recommended to leave 'yes'" ) );
  pdat->removebrain = new QComboBox( parbox );
  parlay->addWidget( pdat->removebrain, row++, 1 );
  pdat->removebrain->setObjectName( "removebrain" );
  pdat->removebrain->addItem( tr( "yes" ) );
  pdat->removebrain->addItem( tr( "no" ) );
  l = new QLabel( tr( "Set weights :" ), parbox );
  parlay->addWidget( l, row, 0 );
  l->setToolTip( tr( "Allows to set weights on each model element :\n"
                     "  0 : don't set anything (leaves it as in the "
                     "model file)\n"
                     " -1 : explicitly unsets the weights (RECOMMENDED)\n"
                     "t>0 : sets nodes weights to t x num of relations" ) );
  pdat->setweight = new QLineEdit( "-1", parbox );
  parlay->addWidget( pdat->setweight, row++, 1 );
  pdat->setweight->setObjectName( "setweight" );
  l = new QLabel( tr( "Output plot file :" ), parbox );
  parlay->addWidget( l, row, 0 );
  l->setToolTip( tr( "If a file is specified here, a 'plot file' will\n"
                     "be written during relaxation, with a line for \n"
                     "each pass, containing temperatures, numbers of\n"
                     "accepted transitions, energies, etc.\n"
                     "- Can be viewed with gnuplot for instance" ) );
  QWidget *opb = new QWidget( parbox );
  parlay->addWidget( opb, row++, 1 );
  QHBoxLayout *opblay = new QHBoxLayout;
  opb->setLayout( opblay );
  opblay->setSpacing( 5 );
  pdat->outplot = new QComboBox( opb );
  pdat->outplot->setObjectName( "outplot" );
  pdat->outplot->addItem( tr( "<none>" ) );
  QPushButton	*outbtn = new QPushButton( "...", opb );
  opblay->addWidget( pdat->outplot );
  opblay->addWidget( outbtn );
  outbtn->setFixedHeight( pdat->outplot->sizeHint().height() );
  outbtn->setFixedWidth( outbtn->sizeHint().width() );
  l= new QLabel( tr( "Initial labels :" ), parbox );
  parlay->addWidget( l, row, 0 );
  l->setToolTip( tr( "if VOID, all labels are initialized to the void "
                     "(unrecognized) value (see below)\n"
                     "If NONE, labels are not initialized "
                     "(left unchanged)\n"
                     "if RANDOM, labels are randomized: each node gets one "
                     "of its possible labels") );
  pdat->initlabels = new QComboBox( parbox );
  parlay->addWidget( pdat->initlabels, row++, 1 );
  pdat->initlabels->setObjectName( "initlabels" );
  pdat->initlabels->addItem( "VOID" );
  pdat->initlabels->addItem( "NONE" );
  pdat->initlabels->addItem( "RANDOM" );
  l = new QLabel( tr( "Void label :" ), parbox );
  parlay->addWidget( l, row, 0 );
  l->setToolTip( tr( "Label used for unrecognized nodes" ) );
  pdat->voidlabel = new QLineEdit( "unknown", parbox );
  parlay->addWidget( pdat->voidlabel, row++, 1 );
  l = new QLabel( tr( "Void pass mode :" ), parbox );
  parlay->addWidget( l, row, 0 );
  l->setToolTip( tr( "Void pass is a special relaxation pass wich can\n"
                     "occur from time to time to increase annealing "
                     "performance :\n"
                     "it consists in trying to 'remove' a whole fold\n"
                     "in a single transition to avoid aberrant labels "
                     "distributions to persist\n\n"
                     "NONE : don't perform such special passes\n"
                     "REGULAR : perform them regularly, with occurency "
                     "given below\n"
                     "STOCHASTIC : perform them irregularly on a "
                     "mean probability\n based on the occurency below" ) );
  pdat->voidmode = new QComboBox( parbox );
  parlay->addWidget( pdat->voidmode, row++, 1 );
  pdat->voidmode->setObjectName( "voidmode" );
  pdat->voidmode->addItem( "NONE" );
  pdat->voidmode->addItem( "REGULAR" );
  pdat->voidmode->addItem( "STOCHASTIC" );
  //pdat->voidmode->setCurrentIndex( 1 );
  l = new QLabel( tr( "Void pass occurency :" ), parbox );
  parlay->addWidget( l, row, 0 );
  l->setToolTip( tr( "Occurency (1 out of n) or mean inverse probability\n"
                     "of Void passes (if used)" ) );
  pdat->voidoccur = new QLineEdit( "20", parbox );
  parlay->addWidget( pdat->voidoccur, row++, 1 );
  l = new QLabel( tr( "Extension mode :" ), parbox );
  parlay->addWidget( l, row, 0 );
  l->setToolTip( tr( "List of extended passes used during relaxation\n"
                     "Extended passes are plug-ins that can be inserted\n"
                     "in the annealing process.\n"
                     "Up to now 2 extensions exist :\n\n"
                     "CONNECT_VOID is similar to Void passes, but only\n"
                     " tries to remove one connected component of\n"
                     " a fold at e time\n"
                     "CONNECT inversely tries to mute a connected \n"
                     " component of void label nodes to the same fold "
                     "label\n"
                     " - useful after VOID and/or CONNECT_VOID  passes\n\n"
                     "Both CONNECT_VOID and CONNECT passes seem to\n"
                     "significantly improve the recognition, so you\n"
                     "should certainly use them" ) );
  pdat->extmode = new QComboBox( parbox );
  parlay->addWidget( pdat->extmode, row++, 1 );
  pdat->extmode->setObjectName( "extmode" );
  pdat->extmode->addItem( "" );
  pdat->extmode->addItem( "CONNECT" );
  pdat->extmode->addItem( "CONNECT_VOID" );
  pdat->extmode->addItem( "CONNECT_VOID CONNECT" );
  pdat->extmode->setEditable( true );
  //pdat->extmode->setCurrentIndex( 3 );
  l = new QLabel( tr( "Extension pass occurency :" ), parbox );
  parlay->addWidget( l, row, 0 );
  l->setToolTip( tr( "Occurency (1 out of n) of extended passes\n"
                     "Up to now they happen regularly in their given\n"
                     "order; if occurency is the same as Void passes,\n"
                     "Void pass always happens first, then immediately\n"
                     "followed by extended passes" ) );
  pdat->extoccur = new QLineEdit( "10", parbox );
  parlay->addWidget( pdat->extoccur, row++, 1 );
  l = new QLabel( tr( "Double drawing lots :" ), parbox );
  parlay->addWidget( l, row, 0 );
  l->setToolTip( tr( "Performs 2 drawing lots before accepting a "
                     "transition\n(if my memory is good enough...)\n"
                     "This leads to accept only transitions with a high\n"
                     "probability, or no change at all.\n"
                     "NOT RECOMMENDED - it seems to give bad results\n"
                     "and it's theoretically an heresy..." ) );
  pdat->ddrawlots = new QComboBox( parbox );
  parlay->addWidget( pdat->ddrawlots, row++, 1 );
  pdat->ddrawlots->setObjectName( "ddrawlots" );
  pdat->ddrawlots->addItem( tr( "yes" ) );
  pdat->ddrawlots->addItem( tr( "no" ) );
  l = new QLabel( tr( "Number of stable passes before stopping :" ), parbox );
  parlay->addWidget( l, row, 0 );
  l->setToolTip( tr( "Stopping when the number of changes just drops "
                     "can sometimes be a bit too unstable,\n"
                     "so specifying a number of consecutive passes "
                     "below this level before stopping can avoid\n"
                     "too abrupt stops." ) );
  pdat->stoppass = new QLineEdit( "1", parbox );
  parlay->addWidget( pdat->stoppass, row++, 1 );
  pdat->stoppass->setValidator( new QIntValidator( 0, 10000, parbox ) );
  //pdat->ddrawlots->setCurrentIndex( 1 );

  QWidget	*btnbox = new QWidget( this );
  QHBoxLayout *btnboxlay = new QHBoxLayout;
  btnbox->setLayout( btnboxlay );
  btnboxlay->setSpacing( 10 );
  QPushButton	*startBtn = new QPushButton( tr( "Start relaxation" ),
					     btnbox );
  QPushButton	*stopBtn = new QPushButton( tr( "Stop" ), btnbox );
  btnboxlay->addWidget( startBtn );
  btnboxlay->addWidget( stopBtn );

  lay1->addWidget( iobox );
  lay1->addWidget( thrBox );
  lay1->addWidget( parbox );
  lay1->addWidget( btnbox );

  _conf->init();
  updateBoxes();

  connect( loadBtn, SIGNAL( clicked() ), this, SLOT( loadConfig() ) );
  connect( saveBtn, SIGNAL( clicked() ), this, SLOT( saveConfig() ) );
  connect( startBtn, SIGNAL( clicked() ), this, SLOT( start() ) );
  connect( stopBtn, SIGNAL( clicked() ), this, SLOT( stop() ) );

  connect( trbtn, SIGNAL( clicked() ), this, SLOT( selectTranslationFile() ) );
  connect( outbtn, SIGNAL( clicked() ), this, SLOT( selectPlotFile() ) );
}


QAnnealParams::~QAnnealParams()
{
  if( pdat->annealThRunning != STOPPED )
    {
      stop();
      pdat->annealThread.join();
    }
  _fusion->deleteObserver( this );
  delete _conf;
  delete pdat;
}


bool QAnnealParams::event( QEvent* e )
{
  if( e->type() == QEvent::User + 103 )
  {
    annealStepFinished();
    e->accept();
    return true;
  }
  else
    return QWidget::event( e );
}


void QAnnealParams::update( const anatomist::Observable*, void* arg )
{
  if( arg == 0 )
    delete this;

  cout << "QAnnealParams::update\n";
}


void QAnnealParams::loadConfig()
{
  QString filter = tr( "Annealing configuration" );
  filter += " (*.cfg)";
  QFileDialog	& fd = fileDialog();
  fd.selectFilter( filter );
  fd.setWindowTitle( tr( "Open annealing configuration" ) );
  fd.setFileMode( QFileDialog::ExistingFile );
  if( !fd.exec() )
    return;

  QString	fname = fd.selectedFiles()[0];
  if( !fname.isEmpty() )
    {
      cout << "load config" << fname.toStdString() << "\n";
      _conf->loadConfig( string( fname.toStdString() ) );
      updateBoxes();
    }
}


void QAnnealParams::start()
{
  cout << "QAnnealParams::start\n";

  if( pdat->annealThRunning != STOPPED )
    {
      cerr << "Ça tourne déjà !\n";
      return;
    }

  pdat->triggerAnnealState = RUNNING;
  pdat->annealThRunning = RUNNING;

  updateConfig();

  //	version multi-threadée
  if( pdat->threaded )
    {
      pdat->annealThread.launch();
    }
  else
    //	versionn mono-thread
    {
      // dans ce cas on appelle la fonction à la main
      annealThread();
    }
}


void QAnnealParams::annealThread()
{
  MGraph	*model = (MGraph *) _fusion->model()->graph();
  FRGraph	*fmodel = dynamic_cast<FRGraph *>( model );
  Anneal	an( *(CGraph *) _fusion->folds()->graph(), *model );
  ofstream	*plotf = 0;

  if( !_conf->plotFile.empty() )
    plotf = new ofstream( _conf->plotFile.c_str() );
  if( plotf && !*plotf )
    {
      cerr << "Cannot open plot file " << _conf->plotFile << endl;
      delete plotf;
      plotf = 0;
    }
  _conf->initAnneal( an, plotf );

  if( fmodel )
  {
    if( _conf->setWeights > 0 )
      fmodel->setWeights( _conf->setWeights );
    else if( _conf->setWeights < 0 )
      {
        cout << "remove weights\n";
        fmodel->removeWeights();
      }
  }

  an.reset();

  while( pdat->triggerAnnealState != STOPPED && !an.isFinished() )
  {
    an.fitStep();

    //	version multi-thread
    if( pdat->threaded )
    {
      // envoie le signal au thread de l'interface pour faire un refresh
      QApplication::postEvent( this, new ThreadBridgeEvent );
      // attendre que ce soit fait pour continuer
      pdat->interfaceSem.wait();
    }
    else	// mono-thread
    {
      updateInterface();
    }
  }

  if( plotf )
    plotf->close();
  delete plotf;

  pdat->triggerAnnealState = STOPPED;
  pdat->annealThRunning = STOPPED;
  if( pdat->threaded )
    {
      /* déclencher un dernier signal pour arrêter tout du côté du thread
      interface */
      QApplication::postEvent( this, new ThreadBridgeEvent );
    }
}


void QAnnealParams::annealStepFinished()
{
  if( pdat->annealThRunning == STOPPED )
    {
      // stoppé ou fini: plus de thread
      cout << "Anneal Thread fini\n";
      //pthread_mutex_unlock( &_interfaceLock );
      return;
    }

  cout << "Anneal refresh\n";
  updateInterface();

  // dire au recuit qu'on s'en occupe
  pdat->interfaceSem.post();
}


void QAnnealParams::updateInterface()
{
  _fusion->folds()->setChanged();
  /*	si l'interface est multi-threadée, il faut faire un update plus
	poussé pcq on peut aller regarder l'intérieur des graphes pendant que
	ça tourne */
  if( pdat->threaded )
    _fusion->internalUpdate();
  else	// si c'est mono-thread, on s'emmerde pas
    _fusion->setColors();
  _fusion->notifyObservers( this );
}


void QAnnealParams::stop()
{
  cout << "QAnnealParams::stop\n";

  if( pdat->threaded )
    {
      if( pdat->annealThRunning == STOPPED )
	{
	  cerr << "Déjà arrêté.\n";
	  return;
	}
      pdat->triggerAnnealState = STOPPED;
    }
  else
    cout << "Ce bouton ne sert qu'en mode multi-thread, puisque en mono-"
	 << "thread, on ne peut pas cliquer dessus pendant le recuit...\n";
}


void QAnnealParams::setThreaded( bool t )
{
  if( pdat->annealThRunning != STOPPED )
    {
      cout << "Ce mode ne peut pas être changé en cours de recuit\n";
      if( !pdat->threaded )
	{
	  cerr << "d'ailleurs on ne devrait aps pouvoir cliquer en ce "
	       << "moment... (BUG)\n";
	}
      return;
    }
  pdat->threaded = t;
  cout << "Interface ";
  if( !pdat->threaded )
    cout << "not ";
  cout << "threaded.\n";
}


void QAnnealParams::selectTranslationFile()
{
  QString	init;
  if( pdat->transl->currentIndex() != 0 )
    init = pdat->transl->currentText();

  QString filter = tr( "Translation file" );
  filter += " (*.def)";
  QFileDialog	& fd = fileDialog();
  fd.selectFilter( filter );
  fd.setWindowTitle( tr( "Open translation file" ) );
  fd.setFileMode( QFileDialog::ExistingFile );
  if( !fd.exec() )
    return;

  QString	fname = fd.selectedFiles()[0];
  if( !fname.isNull() && !fname.isEmpty() )
    {
      unsigned	i, n = pdat->transl->count();
      for( i=1; i<n && pdat->transl->itemText( i )!=fname; ++i ) {}
      if( i == n )
        pdat->transl->addItem( fname );
      pdat->transl->setCurrentIndex( i );
    }
}


void QAnnealParams::selectPlotFile()
{
  QString	init;
  if( pdat->outplot->currentIndex() != 0 )
    init = pdat->outplot->currentText();

  QString filter = tr( "Output plot file" );
  filter += " (*.dat)";
  QFileDialog	& fd = fileDialog();
  fd.selectFilter( filter );
  fd.setWindowTitle( tr( "Select output plot file" ) );
  fd.setFileMode( QFileDialog::AnyFile );
  if( !fd.exec() )
    return;

  QString	fname = fd.selectedFiles()[0];
  if( !fname.isNull() && !fname.isEmpty() )
    {
      unsigned	i, n = pdat->outplot->count();
      for( i=1; i<n && pdat->outplot->itemText( i )!=fname; ++i ) {}
      if( i == n )
        pdat->outplot->addItem( fname );
      pdat->outplot->setCurrentIndex( i );
    }
}


void QAnnealParams::updateConfig()
{
  //	init
  _conf->initMode = 1 - pdat->init->currentIndex();

  //	mode
  _conf->mode = pdat->mode->currentText().toStdString();

  //	iteration mode
  _conf->iterType = pdat->iter->currentText().toStdString();

  //	translation
  if( pdat->transl->currentIndex() == 0 )
    _conf->labelsMapFile = "";
  else
    _conf->labelsMapFile = pdat->transl->currentText().toStdString();

  //	temp
  _conf->temp = pdat->temp->text().toFloat();

  //	rate
  _conf->rate = pdat->rate->text().toFloat();

  //	ICM temp
  _conf->tempICM = pdat->tempicm->text().toFloat();

  //	stop rate
  _conf->stopRate = pdat->stoprate->text().toFloat();

  //	verbose
  _conf->verbose = 1 - pdat->verbose->currentIndex();

  //	gibbs changes
  _conf->gibbsChange = pdat->gibbschange->text().toInt();

  //	remove brain
  _conf->removeVoid = 1 - pdat->removebrain->currentIndex();

  //	set weights
  _conf->setWeights = pdat->setweight->text().toFloat();

  //	ouput plot file
  if( pdat->outplot->currentIndex() == 0 )
    _conf->plotFile = "";
  else
    _conf->plotFile = pdat->outplot->currentText().toStdString();

  //	init labels
  _conf->initLabelTypeString = pdat->initlabels->currentText().toStdString();

  //	void label
  _conf->voidLabel = pdat->voidlabel->text().toStdString();

  //	void pass mode
  _conf->voidMode = pdat->voidmode->currentText().toStdString();

  //	void pass occurency
  _conf->voidOccurency = pdat->voidoccur->text().toInt();

  //	extension mode
  _conf->extensionMode = pdat->extmode->currentText().toStdString();

  //	extension pass occurency
  _conf->extPassOccurency = pdat->extoccur->text().toInt();

  //	double drawing lots
  _conf->doubleDrawingLots = 1 - pdat->ddrawlots->currentIndex();

  //	stop steps
  _conf->niterBelowStopProp = pdat->stoppass->text().toInt();

  _conf->processParams();
}


void QAnnealParams::updateBoxes()
{
  unsigned	i, n;

  //	init
  pdat->init->setCurrentIndex( 1 - _conf->initMode );

  //	mode
  n = pdat->mode->count();
  for( i=0; i<n && pdat->mode->itemText( i )!=_conf->mode.c_str(); ++i ) {}
  if( i == n )
    pdat->mode->addItem( _conf->mode.c_str() );
  pdat->mode->setCurrentIndex( i );

  //	iteration mode
  n = pdat->iter->count();
  for( i=0; i<n && pdat->iter->itemText( i )!=_conf->iterType.c_str(); ++i ) {}
  if( i == n )
    pdat->iter->addItem( _conf->iterType.c_str() );
  pdat->iter->setCurrentIndex( i );

  //	translation
  if( !_conf->labelsMapFile.empty() )
    {
      n = pdat->transl->count();
      for( i=1; i<n && pdat->transl->itemText( i )!=_conf->labelsMapFile.c_str();
           ++i ) {}
      if( i == n )
        pdat->transl->addItem( _conf->labelsMapFile.c_str() );
      pdat->transl->setCurrentIndex( i );
    }
  else
    pdat->transl->setCurrentIndex( 0 );

  //	temp
  pdat->temp->setText( QString::number( _conf->temp ) );

  //	rate
  pdat->rate->setText( QString::number( _conf->rate ) );

  //	ICM temp
  pdat->tempicm->setText( QString::number( _conf->tempICM ) );

  //	stop rate
  pdat->stoprate->setText( QString::number( _conf->stopRate ) );

  //	verbose
  pdat->verbose->setCurrentIndex( 1 - _conf->verbose );

  //	gibbs changes
  pdat->gibbschange->setText( QString::number( _conf->gibbsChange ) );

  //	remove brain
  pdat->removebrain->setCurrentIndex( 1 - _conf->removeVoid );

  //	set weights
  pdat->setweight->setText( QString::number( _conf->setWeights ) );

  //	output plot file
  if( !_conf->plotFile.empty() )
    {
      n = pdat->outplot->count();
      for( i=1; i<n && pdat->outplot->itemText( i )!=_conf->plotFile.c_str();
           ++i ) {}
      if( i == n )
        pdat->outplot->addItem( _conf->plotFile.c_str() );
      pdat->outplot->setCurrentIndex( i );
    }
  else
    pdat->outplot->setCurrentIndex( 0 );

  //	init labels
  n = pdat->initlabels->count();
  for( i=0; i<n
       && pdat->initlabels->itemText( i )!=_conf->initLabelTypeString.c_str();
       ++i ) {}
  if( i == n )
    pdat->initlabels->addItem( _conf->initLabelTypeString.c_str() );
  pdat->initlabels->setCurrentIndex( i );

  //	void label
  pdat->voidlabel->setText( _conf->voidLabel.c_str() );

  //	void pass mode
  n = pdat->voidmode->count();
  for( i=0; i<n && pdat->voidmode->itemText( i )!=_conf->voidMode.c_str(); ++i ) {}
  if( i == n )
    pdat->voidmode->addItem( _conf->voidMode.c_str() );
  pdat->voidmode->setCurrentIndex( i );

  //	void pass occurency
  pdat->voidoccur->setText( QString::number( _conf->voidOccurency ) );

  //	extension mode
  n = pdat->extmode->count();
  for( i=0; i<n && pdat->extmode->itemText( i )!=_conf->extensionMode.c_str();
       ++i ) {}
  if( i == n )
    pdat->extmode->addItem( _conf->extensionMode.c_str() );
  pdat->extmode->setCurrentIndex( i );

  //	extension pass occurency
  pdat->extoccur->setText( QString::number( _conf->extPassOccurency ) );

  //	double drawing lots
  pdat->ddrawlots->setCurrentIndex( 1 - _conf->doubleDrawingLots );

  //	stop steps
  pdat->stoppass->setText( QString::number( _conf->niterBelowStopProp ) );
}


void QAnnealParams::saveConfig()
{
  QString filter = tr( "Annealing configuration" );
  filter += " (*.cfg)";
  QFileDialog	& fd = fileDialog();
  fd.setNameFilter( filter );
  fd.setWindowTitle( tr( "Save annealing config file" ) );
  fd.setFileMode( QFileDialog::AnyFile );
  if( !fd.exec() )
    return;

  QString	fname = fd.selectedFiles()[0];
  if( !fname.isNull() && !fname.isEmpty() )
    {
      updateConfig();
      _conf->modelFile = _fusion->model()->fileName();
      _conf->graphFile = _fusion->folds()->fileName();
      _conf->save = 1;
      _conf->saveConfig( fname.toStdString() );
    }
}


