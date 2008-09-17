
#include <si/mixer/mixerParser.h>
#include <si/mixer/meanMixer.h>
#include <si/model/adaptiveTree.h>
#include <graph/tree/tree.h>
#include <iostream>

using namespace sigraph;
using namespace carto;
using namespace std;


TreePostParser::FactorySet MixerParser::factories()
{
  TreePostParser::FactorySet	fs;

  fs[ "mean_mixer" ] = buildMean;

  return( fs );
}


void MixerParser::buildMean( AttributedObject* parent, Tree* t, 
			     const string & )
{
  MeanMixer	*mm = new MeanMixer;

  t->setProperty( "pointer", (Mixer*) mm );
  parseMix( parent, t, mm );
}


void MixerParser::parseMix( AttributedObject* parent, Tree*, Mixer* mix )
{
  Model	*mod;
  if( parent && parent->getProperty( "pointer", mod ) )
    {
      AdaptiveTree	*at = dynamic_cast<AdaptiveTree *>( mod );

      if( at )
	at->setMixer( mix );
      else
	cerr << "Mixer parent is not an AdTree\n";
    }
  else
    cerr << "Mixer without parent\n";
}



