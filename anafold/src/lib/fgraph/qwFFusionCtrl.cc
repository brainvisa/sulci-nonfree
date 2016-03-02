
#include <anafold/fgraph/qwFFusionCtrl.h>
#include <anafold/fgraph/afgraph.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/dialogs/colorDialog.h>
#include <anatomist/color/objectPalette.h>
#include <QButtonGroup>
#include <QGroupBox>

#include <iostream>


using namespace anatomist;
using namespace std;


QFFoldCtrl::QFFoldCtrl( QWidget* parent, const char* name, AFGraph* fusion )
  : QWidget( parent ), _fusion( fusion ), _updating( false )
{
  setAttribute( Qt::WA_DeleteOnClose );
  setObjectName( name );

  const unsigned	pixsize = 12;

  fusion->addObserver( this );
  setWindowTitle( ( string( name ) + " : " + fusion->name() ).c_str() );
  QHBoxLayout	*lay1 = new QHBoxLayout( this );
  lay1->setMargin( 10 );

  // left panel

  QFrame	*fr1 = new QFrame( this );
  fr1->setObjectName( "frame1" );
  fr1->setFrameStyle( QFrame::NoFrame );
  QVBoxLayout	*lay4 = new QVBoxLayout( fr1 );
  lay4->setMargin( 0 );
  setLayout( lay1 );
  QGroupBox *bg = new QGroupBox( tr( "Node mapping mode" ), fr1 );
  QGroupBox *groupbox = bg;
  bg->setObjectName( "btngroup" );
  QVBoxLayout	*lay2 = new QVBoxLayout( bg );
  lay2->setMargin( 10 );
  lay2->setSpacing( 10 );
  _groupBox = new QButtonGroup( bg );
  QRadioButton	*np = new QRadioButton( tr( "Node potentials" ), bg );
  QRadioButton	*tp = new QRadioButton( tr( "Total potentials" ), bg );
  QRadioButton	*la = new QRadioButton( tr( "Labels" ), bg );
  QRadioButton	*wt = new QRadioButton( tr( "Weights only (nodes)" ), bg );
  QRadioButton	*wtt = new QRadioButton( tr( "Weights only (total)" ), bg );
  _groupBox->addButton( np, 0 );
  _groupBox->addButton( tp, 1 );
  _groupBox->addButton( la, 2 );
  _groupBox->addButton( wt, 3 );
  _groupBox->addButton( wtt, 4 );

  np->setFixedSize( np->sizeHint() );
  tp->setFixedSize( tp->sizeHint() );
  la->setFixedSize( la->sizeHint() );
  wt->setFixedSize( wt->sizeHint() );
  wtt->setFixedSize( wtt->sizeHint() );

  bg = new QGroupBox( tr( "Edge mapping" ), fr1 );
  QGroupBox *relgb = bg;
  bg->setObjectName( "relgroup" );
  QVBoxLayout	*lay5 = new QVBoxLayout( bg );
  lay5->setMargin( 10 );
  lay5->setSpacing( 10 );
  _rpBtn = new QCheckBox( tr( "Show edge potentials" ), bg );
  _rpBtn->setObjectName( "relpot" );
  _rpBtn->setFixedSize( _rpBtn->sizeHint() );
  relgb->hide(); // not used up to now...

  // right panel

  QFrame	*frrgt = new QFrame( this );
  frrgt->setObjectName( "rightPan" );
  QVBoxLayout	*lfrrgt = new QVBoxLayout( frrgt );
  lfrrgt->setMargin( 0 );

  QGroupBox *group2 = new QGroupBox( tr( "Modifiers" ), frrgt );
  group2->setObjectName( "btngroup2" );
  QGroupBox *grgtbtm = new QGroupBox( tr( "Model" ), frrgt );
  grgtbtm->setObjectName( "grgtbtm" );
  lfrrgt->addWidget( group2 );
  lfrrgt->addWidget( grgtbtm );

  // right upper group : modifiers

  QVBoxLayout	*lay3 = new QVBoxLayout( group2 );
  lay3->setMargin( 10 );
  lay3->setSpacing( 10 );
  _wtBtn = new QCheckBox( tr( "Weighted potentials" ), group2 );
  _wtBtn->setObjectName( "wtpot" );
  _wtBtn->setFixedSize( _wtBtn->sizeHint() );
  _midBtn = new QCheckBox( tr( "0 potential at mid-colormap" ), group2 );
  _midBtn->setObjectName( "mid" );
  _midBtn->setFixedSize( _midBtn->sizeHint() );

  QFrame	*fr2 = new QFrame( group2 );
  fr2->setObjectName( "fr2" );
  fr2->setFrameStyle( QFrame::NoFrame );
  QHBoxLayout	*lay6 = new QHBoxLayout( fr2 );
  lay6->setMargin( 0 );
  _p0Btn = new QCheckBox( tr( "0 potential with different color : " ), fr2);
  _p0Btn->setObjectName( "pot0" );
  _p0Btn->setFixedSize( _p0Btn->sizeHint() );
  _p0col = new QPushButton( fr2 );
  fr2->setObjectName( "pot0col" );
  QPixmap	pix( pixsize, pixsize );
  pix.fill( QColor( 255, 255, 0 ) );
  _p0col->setIcon( pix );
  _p0col->setFixedSize( _p0col->sizeHint() );
  lay6->addWidget( _p0Btn );
  lay6->addWidget( _p0col, 0, Qt::AlignRight );

  QFrame	*fr3 = new QFrame( group2 );
  fr3->setObjectName( "fr3" );
  fr3->setFrameStyle( QFrame::NoFrame );
  QHBoxLayout	*lay7 = new QHBoxLayout( fr3 );
  lay7->setMargin( 0 );
  QLabel	*lab1 = new QLabel( tr( "No potential color : " ), fr3);
  lab1->setObjectName( "lab1" );
  lab1->setFixedSize( lab1->sizeHint() );
  _nopotCol = new QPushButton( fr3 );
  _nopotCol->setObjectName( "nopotcol" );
  QPixmap	pix2( pixsize, pixsize );
  pix2.fill( QColor( 0, 255, 0 ) );
  _nopotCol->setIcon( pix2 );
  _nopotCol->setFixedSize( _nopotCol->sizeHint() );
  lay7->addWidget( lab1 );
  lay7->addStretch();
  lay7->addWidget( _nopotCol, 0, Qt::AlignRight );

  lay2->addSpacing( 10 );
  lay2->addWidget( np, 0, Qt::AlignLeft );
  lay2->addWidget( tp, 0, Qt::AlignLeft );
  lay2->addWidget( la, 0, Qt::AlignLeft );
  lay2->addWidget( wt, 0, Qt::AlignLeft );
  lay2->addWidget( wtt, 0, Qt::AlignLeft );

  // right lower panel : model

  QVBoxLayout	*lmod = new QVBoxLayout( grgtbtm );
  lmod->setMargin( 10 );
  lmod->setSpacing( 10 );
  QFrame	*frmodwt = new QFrame( grgtbtm );
  frmodwt->setObjectName( "frmodwt" );

  lmod->addSpacing( 10 );
  lmod->addWidget( frmodwt );

  QHBoxLayout	*lmodwt = new QHBoxLayout( frmodwt );
  lmodwt->setMargin( 0 );
  lmodwt->setSpacing( 10 );

  QLabel	*lbmodwt = new QLabel( tr( "Contribution of edge numbers" ), 
                                       frmodwt );
  lbmodwt->setObjectName( "lbmodwt" );
  _modWeightEdit = new QLineEdit( "0", frmodwt );
  _modWeightEdit->setObjectName( "emodwt" );
  QPushButton	*bmodwt = new QPushButton( tr( "Update model" ), frmodwt );
  bmodwt->setObjectName( "bmodwt" );
  QDoubleValidator	*vmodwt = new QDoubleValidator( -1, 1000, 5, 
                                                        _modWeightEdit );

  _modWeightEdit->setValidator( vmodwt );

  lbmodwt->setFixedSize( lbmodwt->sizeHint() );
  _modWeightEdit->setMinimumSize( 40, _modWeightEdit->sizeHint().height() );
  bmodwt->setFixedSize( bmodwt->sizeHint() );

  lmodwt->addWidget( lbmodwt );
  lmodwt->addWidget( _modWeightEdit );
  lmodwt->addWidget( bmodwt );

  // general size setup

  lay5->addSpacing( 10 );
  lay5->addWidget( _rpBtn, 0, Qt::AlignLeft );

  lay3->addSpacing( 10 );
  lay3->addWidget( _wtBtn, 0, Qt::AlignLeft );
  lay3->addWidget( _midBtn, 0, Qt::AlignLeft );
  lay3->addWidget( fr2, 0, Qt::AlignLeft );
  lay3->addWidget( fr3, 0, Qt::AlignLeft );

  lay4->addWidget( groupbox );
  lay4->addWidget( relgb );

  lay1->addWidget( fr1 );
  lay1->addWidget( frrgt );

  update( _fusion );

  resize( minimumSizeHint() );
  connect( _groupBox, SIGNAL( buttonClicked( int ) ), this,
           SLOT( btnClick( int ) ) );
  connect( _rpBtn, SIGNAL( clicked() ), this, SLOT( relPotBtnClicked() ) );
  connect( _wtBtn, SIGNAL( clicked() ), this, SLOT( weightButtonClicked() ) );
  connect( _midBtn, SIGNAL( clicked() ), this, SLOT( midBtnClicked() ) );
  connect( _p0Btn, SIGNAL( clicked() ), this, SLOT( pot0BtnClicked() ) );
  connect( _p0col, SIGNAL( clicked() ), this, SLOT( setPot0Color() ) );
  connect( _nopotCol, SIGNAL( clicked() ), this, SLOT( setNoPotColor() ) );
  connect( bmodwt, SIGNAL( clicked() ), this, SLOT( updateModelWeights() ) );
}


QFFoldCtrl::~QFFoldCtrl()
{
  _fusion->deleteObserver( this );
}


void QFFoldCtrl::update( const Observable*, void* )
{
  //cout << "QFFoldCtrl::update 1\n";
  if( _updating )
    return;

  //cout << "QFFoldCtrl::update 2\n";
  if( theAnatomist->hasObject( _fusion ) )
    {
      const unsigned	pixsize = 12;

      _updating = true;
      _groupBox->button( _fusion->mapMode() )->setChecked( true );
      _rpBtn->setChecked( _fusion->isRelPotentials() );
      _wtBtn->setChecked( _fusion->isMapWeighted() );
      _midBtn->setChecked( _fusion->isPot0Centered() );
      _p0Btn->setChecked( _fusion->pot0HasColor() );
      QPixmap	pix;
      pix = QPixmap( pixsize, pixsize );
      pix.fill( QColor( (int) (_fusion->pot0Red() * 256),
                        (int) (_fusion->pot0Green() * 256),
                        (int) (_fusion->pot0Blue() * 256) ) );
      _p0col->setIcon( pix );
      pix = QPixmap( pixsize, pixsize );
      pix.fill( QColor( (int) (_fusion->noPotRed() * 256),
                        (int) (_fusion->noPotGreen() * 256),
                        (int) (_fusion->noPotBlue() * 256) ) );
      _nopotCol->setIcon( pix );
      _updating = false;
    }
  else
    close();
}


void QFFoldCtrl::btnClick( int btn )
{
  if( _updating )
    return;
  if( theAnatomist->hasObject( _fusion ) )
  {
    if( _fusion->mapMode() != btn )
    {
      _updating = true;
      _fusion->setMapMode( (AFGraph::Mode) btn );
      _fusion->getOrCreatePalette();
      AObjectPalette	*pal = _fusion->palette();
      pal->setMin1( 0 );
      pal->setMax1( 1 );
      _fusion->setColors();
      _fusion->notifyObservers( this );
      _updating = false;
    }
  }
  else
    close();
}


void QFFoldCtrl::weightButtonClicked()
{
  if( _updating )
    return;
  if( theAnatomist->hasObject( _fusion ) )
    {
      if( _fusion->isMapWeighted() != _wtBtn->isChecked() )
	{
	  _updating = true;
	  _fusion->setMapWeighted( _wtBtn->isChecked() );
	  _fusion->getOrCreatePalette();
	  AObjectPalette	*pal = _fusion->palette();
	  pal->setMin1( 0 );
	  pal->setMax1( 1 );
	  _fusion->setColors();
	  _fusion->notifyObservers( this );
	  _updating = false;
	}
    }
  else
    close();
}


void QFFoldCtrl::relPotBtnClicked()
{
  if( _updating )
    return;
  if( theAnatomist->hasObject( _fusion ) )
  {
    if( _fusion->isRelPotentials() != _rpBtn->isChecked() )
    {
      _updating = true;
      _fusion->setRelPotentials( _rpBtn->isChecked() );
      _fusion->getOrCreatePalette();
      AObjectPalette	*pal = _fusion->palette();
      pal->setMin1( 0 );
      pal->setMax1( 1 );
      _fusion->setColors();
      _fusion->notifyObservers( this );
      _updating = false;
    }
  }
  else
    close();
}


void QFFoldCtrl::midBtnClicked()
{
  if( _updating )
    return;
  if( theAnatomist->hasObject( _fusion ) )
    {
      if( _fusion->isPot0Centered() != _midBtn->isChecked() )
	{
	  _updating = true;
	  _fusion->setPot0Centered( _midBtn->isChecked() );
	  _fusion->getOrCreatePalette();
	  AObjectPalette	*pal = _fusion->palette();
	  pal->setMin1( 0 );
	  pal->setMax1( 1 );
	  _fusion->setColors();
	  _fusion->notifyObservers( this );
	  _updating = false;
	}
    }
  else
    close();
}


void QFFoldCtrl::pot0BtnClicked()
{
  if( _updating )
    return;
  if( theAnatomist->hasObject( _fusion ) )
    {
      if( _fusion->pot0HasColor() != _p0Btn->isChecked() )
	{
	  _updating = true;
	  _fusion->setPot0HasCol( _p0Btn->isChecked() );
	  _fusion->setColors();
	  _fusion->notifyObservers( this );
	  _updating = false;
	}
    }
  else
    close();
}


void QFFoldCtrl::setPot0Color()
{
  if( _updating )
    return;
  if( theAnatomist->hasObject( _fusion ) )
    {
      int alpha = int( _fusion->pot0Alpha() * 256 );
      bool neutr = !_fusion->pot0AlphaUsed();
      QColor col
        = QAColorDialog::getColor(
          QColor( (int) ( _fusion->pot0Red() * 256 ),
                  (int) ( _fusion->pot0Green() * 256 ),
                  (int) ( _fusion->pot0Blue() * 256 ) ),
          0, tr( "Potential 0 color" ).toStdString().c_str(), &alpha, &neutr );
      cout << "getColor done\n";
      if( col.isValid() )
	{
          cout << "color valid\n";
	  _updating = true;
	  _fusion->setPot0Color( ( (float) col.red() ) / 256, 
				 ( (float) col.green() ) / 256, 
                                 ( (float) col.blue() ) / 256,
                                 float(alpha) / 256, !neutr );
	  QPixmap	pix( 12, 12 );
	  pix.fill( QColor( (int) ( _fusion->pot0Red() * 256 ), 
			    (int) ( _fusion->pot0Green() * 256 ), 
			    (int) ( _fusion->pot0Blue() * 256 ) ) );
	  _p0col->setIcon( pix );
	  _fusion->setColors();
	  _fusion->notifyObservers( this );
	  _updating = false;
	}
    }
  else
    close();
}


void QFFoldCtrl::setNoPotColor()
{
  if( _updating )
    return;
  if( theAnatomist->hasObject( _fusion ) )
    {
      int alpha = int( _fusion->noPotAlpha() * 256 );
      bool neutr = !_fusion->noPotAlphaUsed();
      QColor col
        = QAColorDialog::getColor(
          QColor( (int) ( _fusion->noPotRed() * 256 ),
                  (int) (_fusion->noPotGreen() * 256 ),
                  (int) (_fusion->noPotBlue() * 256 ) ),
          0, tr( "Color for 'no potential'" ).toStdString().c_str(), &alpha,
          &neutr );
      if( col.isValid() )
	{
	  _updating = true;
	  _fusion->setNoPotColor( ( (float) col.red() ) / 256, 
				  ( (float) col.green() ) / 256, 
				  ( (float) col.blue() ) / 256,
                                  float(alpha) / 256, !neutr);
	  QPixmap	pix( 12, 12 );
	  pix.fill( QColor( (int) ( _fusion->noPotRed() * 256 ), 
			    (int) ( _fusion->noPotGreen() * 256 ), 
			    (int) ( _fusion->noPotBlue() * 256 ) ) );
	  _nopotCol->setIcon( pix );
	  _fusion->setColors();
	  _fusion->notifyObservers( this );
	  _updating = false;
	}
    }
  else
    close();
}


void QFFoldCtrl::updateModelWeights()
{
  if( _updating )
    return;
  if( theAnatomist->hasObject( _fusion ) )
    {
      _updating = true;
      double	factor = _modWeightEdit->text().toDouble();

      _fusion->getOrCreatePalette();
      AObjectPalette	*pal = _fusion->palette();
      pal->setMin1( 0 );
      pal->setMax1( 1 );
      _fusion->setModelWeights( factor );
      _updating = false;
    }
  else
    close();
}







