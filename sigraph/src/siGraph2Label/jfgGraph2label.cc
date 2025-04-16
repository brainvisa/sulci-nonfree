
#include <si/fold/foldReader.h>
#include <si/fold/fgraph.h>
#include <si/global/global.h>
#include <si/fold/labelsTranslator.h>
#include <cartodata/volume/volume.h>
#include <aims/io/reader.h>
#include <aims/io/writer.h>
#include <aims/bucket/bucket.h>
#include <aims/getopt/getopt2.h>
#include <aims/transformation/affinetransformation3d.h>
#include <aims/graph/graphmanip.h>
#include <aims/resampling/standardreferentials.h>
#include <list>
#include <vector>
#include <cartobase/exception/assert.h>
#include <cartobase/smart/rcptr.h>

using namespace sigraph;
using namespace aims;
using namespace carto;
using namespace std;


void resizeVol( VolumeRef<int16_t> & vol, const map<int16_t, string> & rlabels,
                bool as4d )
{
  if( !as4d || rlabels.empty() )
    return;
  // cout << "resizeVol !\n";
  int dimt = rlabels.rbegin()->first + 1;
  if( vol->getSize()[3] >= dimt )
    return;
  // cout << "increase size: " << dimt << endl;
  vector<int> dims = vol->getSize();
  dims[3] = dimt;
  VolumeRef<int16_t> oldvol = vol;
  vol.reset( new Volume<int16_t>( dims ) );
  vol->copyHeaderFrom( oldvol->header() );
  vol->fill( 0 );
  VolumeRef view( vol, vector<int>(4, 0), oldvol->getSize() );
  *view = *oldvol;
}


int main( int argc, const char** argv )
{
  //
  // Parser of options
  //

  string		gname, tvname, vname, tname, otrans, itrans;
  vector<string>	bname, aname, lname1, synt1, lforbid1;
  set<string>		lname, synt, lforbid;
  bool                  use_tal = false;
  bool                  use_icbm = false;
  Reader<AffineTransformation3d> trreader;
  bool                  as4d = false;

  AimsApplication	app( argc, argv, "Create a volume of label from a " 
                             "graph and a file translation.txt " 
                             " (bucket: aims_bottom, aims_ss, or " 
                             "aims_junction + syntax hull_junction) " );

  app.addOption( gname, "-g", "input graph" );
  app.alias( "-graph", "-g" );
  app.alias( "--graph", "-g" );

  app.addOption( tname, "-tr", "tranlslation file, labels are translated " 
                 "according to this dictionary if it is provided", true );
  app.alias( "-translation", "-tr" );
  app.alias( "--translation", "-tr" );

  app.addOption( tvname, "-tv",
                 "template volume: used to set label volume dimensions and "
                 "voxel size. Note that a resampling takes place if voxel "
                 "sizes differ between graph and output volume.", true );
  app.alias( "-template", "-tv" );
  app.alias( "--template", "-tv" );

  app.addOption( vname, "-o", "output volume" );
  app.alias( "-output", "-o" );
  app.alias( "--output", "-o" );

  app.addOptionSeries( bname, "-b", "bucket name[s]" );
  app.alias( "-bucket", "-b" );
  app.alias( "--bucket", "-b" );

  app.addOptionSeries( synt1, "-s", "syntax[es] of bucket", 0 );
  app.alias( "-syntax", "-s" );
  app.alias( "--syntax", "-s" );

  app.addOptionSeries( aname, "-a", "attributes list that contain a label " 
                       "[default: 'label', 'name' ]", 0 );
  app.alias( "--attribute", "-a" );

  app.addOptionSeries( lname1, "-l", "label values to be kept (filter) " 
                       "[default: all]", 0 );
  app.alias( "--label", "-l" );
  app.addOptionSeries( lforbid1, "-f", "label values to be forbidden (filter) "
                       "[default: none]", 0 );
  app.alias( "--forbid", "-f" );
  app.addOption( itrans, "-it", "input int/label translation file. If " 
                 "provided, it will be used for the label mapping, and " 
                 "completed if needed", true );
  app.addOption( otrans, "-ot", "output int/label translation file [default: "
                 "not saved]", true );
  app.alias( "--outtrans", "-ot" );
  app.alias( "--intrans", "-it" );
  app.addOption( use_tal, "--talairach",
                 "apply internal Talairach transform (prior to other "
                 "coordinates transform if specified). Note that Talairach "
                 "space is centered on 0 and will end up with most of the "
                 "brain outside of the output volume if used alone without an "
                 "additional translation.", true );
  app.addOption( use_icbm, "--icbm",
                 "apply internal transform to ICBM space, plus axes inversion "
                 "from AIMS world (prior to other coordinates transform if "
                 "specified). Note that ICBM "
                 "space is centered on 0, but if no other transform is given, "
                 "we are using the standard template image space instead "
                 "here. Note that this option and --talairach are mutually "
                 "exclusive.",
                 true );
  app.addOption( trreader, "--transform",
                 "apply the given coordinates transformation (.trm file), "
                 "after Talairach transform if --talairach is also used",
                 true );
  app.addOption( as4d, "--4d", "use 4th dimension as label", true );

  try
    {
      app.initialize();

      VolumeRef<short> vol;

      Reader<Graph>        fr( gname );
      unique_ptr<Graph>	fg( fr.read() );

      lname.insert( lname1.begin(), lname1.end() );
      lforbid.insert( lforbid1.begin(), lforbid1.end() );
      synt.insert( synt1.begin(), synt1.end() );

      cout << "graph read\n";
      if( !tname.empty() )
        si().setLabelsTranslPath( tname );

      // transformation handling
      if( use_tal && use_icbm )
        throw runtime_error( "--talairach and --icbm cannot be used at the "
                             "same time." );
      AffineTransformation3d transform, talairach, icbm;
      talairach = GraphManip::talairach( *fg );
      icbm = GraphManip::getICBMTransform( *fg );
      if( use_tal )
        transform = talairach;
      if( use_icbm )
      {
        AffineTransformation3d neg;
        neg.matrix()( 0, 0 ) = -1;
        neg.matrix()( 1, 1 ) = -1;
        neg.matrix()( 2, 2 ) = -1;
        transform = neg * icbm;
      }

      bool user_trans = false;
      if( !trreader.fileName().empty() )
      {
        AffineTransformation3d tr;
        trreader.read( tr );
        transform = tr * transform;
        user_trans = true;
      }
      else if( use_icbm )
      {
        Object mnihdr = StandardReferentials::icbm2009cTemplateHeader();
        Object mnitr = mnihdr->getProperty( "transformations" );
        Object tit = mnitr->objectIterator();
        AffineTransformation3d tpl2icbm( tit->currentValue() );
        transform = *tpl2icbm.inverse() * icbm;
        // a translation will be added if tempalte volume is specified
      }

      // graph voxel size
      vector<float>	vs;
      if( !fg->getProperty( "voxel_size", vs ) || vs.size() < 3 )
      {
        vs.push_back( 1 );
        vs.push_back( 1 );
        vs.push_back( 1 );
      }

      // input labels
      map<string, short>			labels;
      map<short, string>			rlabels;
      int16_t                                   i;

      if( !itrans.empty() )
        {
          ifstream inames( itrans.c_str() );
          if( inames )
            {
              cout << "reading existing labels\n";
              string	line;
              int	pos;
              char	buffer[ 10000 ];

              while( !inames.eof() )
                {
                  inames.getline( buffer, 10000 );
                  line = buffer;
                  for( pos=line.length()-1; pos >=0; --pos )
                    if( line[pos] == ' ' || line[pos] == '\t' )
                      break;
                  if( pos >= 0 )
                    {
                      string lnum = line.substr( pos+1, line.length() - pos );
                      istringstream	ss( lnum );
                      ss >> i;
                      while( pos >= 0
                             && ( line[pos] == ' ' || line[pos] == '\t' ) )
                        --pos;
                      ++pos;
                      string label = line.substr( 0, pos );
                      labels[ label ] = i;
                      rlabels[ i ] = label;
                    }
                }
              inames.close();
              cout << "read " << labels.size() << " labels from " << itrans
                   << endl;
            }
        }

      // volume voxel size
      vector<float> vvs( 1., 3 );

      if( !tvname.empty() )
      {
        Reader<Volume<short> > tvreader( tvname );
        vol.reset( tvreader.read() );
        vvs = vol->getVoxelSize();
        resizeVol( vol, rlabels, as4d );
        if( !user_trans && use_icbm )
        {
          // add half the size difference between ICBM template and given
          // template
          Object mnihdr = StandardReferentials::icbm2009cTemplateHeader();
          Object icbmdim = mnihdr->getProperty( "volume_dimension" );
          Object dit = icbmdim->objectIterator();
          vector<int> tdim = vol->getSize();
          // add also a half voxel size difference translation
          AffineTransformation3d tr;
          tr.matrix()(0, 3)
            = ( ( tdim[0] - 1 ) * vvs[0]
                - ( dit->currentValue()->getScalar() - 1 ) ) / 2;
          dit->next();
          tr.matrix()(1, 3)
            = ( ( tdim[1] - 1 ) * vvs[1]
                - ( dit->currentValue()->getScalar() - 1 ) ) / 2;
          dit->next();
          tr.matrix()(2, 3)
            = ( ( tdim[2] - 1 ) * vvs[2]
                - ( dit->currentValue()->getScalar() - 1 ) ) / 2;
          transform = tr * transform;
        }
      }
      else	// make vol from graph information
      {
        vector<int> vdims( 4, 1 );
        vector<int>	dims;

        if( as4d && !rlabels.empty() )
          vdims[3] = rlabels.rbegin()->first + 1;

        ASSERT( fg->getProperty( "boundingbox_max", dims )
                && dims.size() >= 3 );
        vdims[0] = dims[0] + 1;
        vdims[1] = dims[1] + 1;
        vdims[2] = dims[2] + 1;
        vol = VolumeRef<short>( vdims );
        vvs = vs;
        vol->header().setProperty( "voxel_size", vvs );
      }

      if( vol->header().hasProperty( "uuid" ) )
        // the output is a new file, prevent "duplicate UUID" messages in Axon
        vol->header().removeProperty( "uuid" );

      // adapt volume transforms
      {
        vector<string> refs;
        vector<vector<float> > vtrans;
        AffineTransformation3d invtrans = *transform.inverse();
        if( vol->header().hasProperty( "referential" ) )
          vol->header().removeProperty( "referential" );
        if( !transform.isIdentity() )
        {
          refs.push_back( "subject space" );
          if( fg->hasProperty( "referential" ) )
            refs[0] = fg->getProperty( "referential" )->getString();
          vtrans.push_back( invtrans.toVector() );
        }
        else if( fg->hasProperty( "referential" ) )
          vol->header().setProperty(
            "referential", fg->getProperty( "referential" )->getString() );

        refs.push_back( StandardReferentials::acPcReferentialID() );
        vtrans.push_back( ( talairach * invtrans ).toVector() );
        refs.push_back( StandardReferentials::mniTemplateReferentialID() );
        AffineTransformation3d icbmtr = icbm * invtrans;
        vtrans.push_back( icbmtr.toVector() );

        if( fg->hasProperty( "transformations" ) )
        {
          Object tlist = fg->getProperty( "transformations" );
          Object rlist = fg->getProperty( "referentials" );
          int i;
          Object it = tlist->objectIterator();
          Object ir = rlist->objectIterator();
          for( i=0; it->isValid() && ir->isValid();
                it->next(), ir->next(), ++i )
          {
            string ref = ir->currentValue()->getString();
            if( ref != StandardReferentials::acPcReferential()
                && ref != StandardReferentials::acPcReferentialID()
                && ref != StandardReferentials::mniTemplateReferential()
                && ref != StandardReferentials::mniTemplateReferentialID() )
            {
              AffineTransformation3d t
                = AffineTransformation3d( it->currentValue() );
              t *= invtrans;
              vtrans.push_back( t.toVector() );
              refs.push_back( ref );
            }
          }
        }

        if( !refs.empty() )
        {
          vol->header().setProperty( "referentials", refs );
          vol->header().setProperty( "transformations", vtrans );
        }
      }

      Graph::iterator				iv, fv=fg->end();
      string					name;
      vector<float>				vsz;
      set<string>				slab;
      set<string>::const_iterator		is, fs=slab.end();
      short					lastlabel;
      rc_ptr<BucketMap<Void> >			bck;
      BucketMap<Void>::Bucket::const_iterator	ib, fb;
      AimsVector<short, 3>			pos;
      Point3df                                  fpos;
      FoldLabelsTranslator			trans;
      map<string, string>::const_iterator	il, fl = trans.end();

      *vol = 0;
      if( aname.empty() )
        {
          aname.push_back( "label" );
          aname.push_back( "name" );
        }

      set<string>::const_iterator	el = lname.end();

      if( !tname.empty() )
        trans.readLabels( tname );

      for( il=trans.begin(); il!=fl; ++il )
        if( lname.empty() || lname.find( il->second ) != el )
          slab.insert( il->second );
      // cout << slab.size() << " labels with translation file\n";
      lastlabel = 1;
      for( is=slab.begin(); is!=fs; ++is )
      {
        if( labels.find( *is ) == labels.end()
            && lforbid.find( *is ) == lforbid.end() )
        {
          while( rlabels.find( lastlabel ) != rlabels.end() )
            ++lastlabel;
          labels[ *is ] = lastlabel;
          // cout << "new label index: " << lastlabel << ": " << *is << endl;
          rlabels[ lastlabel ] = *is;
        }
      }
      cout << labels.size() << " labels indices\n";
      // cout << "lastlabel: " << lastlabel << endl;

      resizeVol( vol, rlabels, as4d );

      unsigned	ial, nal = aname.size();

      Vertex::const_iterator ie, fe;
      int count=0;
      unsigned	ibk, nbk = bname.size();
      set<string>::const_iterator	esnt = synt.end();

      cout << "begin processing...\n";
      for( iv=fg->begin(); iv!=fv; ++iv )
	{
          for( ial=0; ial<nal && !(*iv)->getProperty( aname[ial], name ); 
               ++ial ) {}
          if( ial < nal )
            {
              if( !trans.empty() )
                name = trans.lookupLabel( name );
              bool found = ( labels.find( name ) != labels.end() );

              if( !found && lforbid.find( name ) != lforbid.end() )
                continue;

              i = labels[name];
              if( !found // && trans.empty()
                  && ( lname.empty() || lname.find( name ) != el ) )
                {
                  while( rlabels.find( lastlabel ) != rlabels.end() )
                    ++lastlabel;
                  i = lastlabel;
                  labels[ name ] = i;
                  rlabels[ i ] = name;
                  ++lastlabel;
                  resizeVol( vol, rlabels, as4d );
                  // cout << "add label " << i << ": " << name << endl;
                }
              if( i != 0 || as4d )
                {
                  for( ibk=0; ibk<nbk; ++ibk )
                    if( (*iv)->getProperty( bname[ibk], bck ) 
                        && ( synt.empty() 
                             || synt.find( (*iv)->getSyntax() ) != esnt ) )
                      {
                        BucketMap<Void>::Bucket	& bl = (*bck)[0];
                        ++count;
                        for( ib=bl.begin(), fb=bl.end(); ib!=fb; ++ib )
                          {
                            pos = ib->first;
                            // transform voxel size from bucket vs to volume vs
                            fpos = transform.transform(
                              Point3df( pos[0] * vs[0], pos[1] * vs[1],
                                        pos[2] * vs[2] ) );
                            pos[0] = int( rint( fpos[0] / vvs[0] ) );
                            pos[1] = int( rint( fpos[1] / vvs[1] ) );
                            pos[2] = int( rint( fpos[2] / vvs[2] ) );
                            if( pos[0] >= 0 && pos[0] < vol->getSizeX()
                                && pos[1] >= 0 && pos[1] < vol->getSizeY()
                                && pos[2] >= 0 && pos[2] < vol->getSizeZ() )
                            {
                              if( as4d )
                                vol->at( pos[0], pos[1], pos[2], i ) = 1;
                              else
                                vol->at( pos[0], pos[1], pos[2] ) = i;
                            }
                          }
                      }
                  for( ie=(*iv)->begin(), fe=(*iv)->end(); ie!=fe; ie++)
                    if( synt.empty() 
                        || synt.find( (*ie)->getSyntax() ) != esnt )
                      for( ibk=0; ibk<nbk; ++ibk )
                        if( (*ie)->getProperty( bname[ibk], bck ) )
                          {
                            BucketMap<Void>::Bucket & bl = (*bck)[0];
                            ++count;
                            for( ib=bl.begin(), fb=bl.end(); ib!=fb; 
                                 ++ib )
                              {
                                pos = ib->first;
                                // transform voxel size from bucket vs
                                // to volume vs
                                fpos = transform.transform(
                                  Point3df( pos[0] * vs[0], pos[1] * vs[1],
                                            pos[2] * vs[2] ) );
                                pos[0] = int( rint( fpos[0] / vvs[0] ) );
                                pos[1] = int( rint( fpos[1] / vvs[1] ) );
                                pos[2] = int( rint( fpos[2] / vvs[2] ) );
                                if( pos[0] >= 0 && pos[0] < vol->getSizeX()
                                    && pos[1] >= 0 && pos[1] < vol->getSizeY()
                                    && pos[2] >= 0 && pos[2] < vol->getSizeZ()
                                  )
                                {
                                  if( as4d )
                                    vol->at( pos[0], pos[1], pos[2], i ) = 1;
                                  else
                                    vol->at( pos[0], pos[1], pos[2] ) = i;
                                }
                              }
                          }
                }
            }
	}
      cout << "now: " << labels.size() << " labels indices\n";

      if( !otrans.empty() )
        {
          ofstream	namefile( otrans.c_str() );
          map<string, short>::const_iterator	il, el = labels.end();
          for( il=labels.begin(); il!=el; ++il )
            namefile << il->first << "\t" << il->second << endl;
          namefile.close();
        }
      cout << count << " graph nodes written\n";
      Writer<Volume<short> >	vw( vname );

      vw.write( *vol );

    }
  catch( user_interruption & )
    {
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
      return 1;
    }
}


