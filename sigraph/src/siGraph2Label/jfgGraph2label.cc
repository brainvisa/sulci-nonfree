
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
#include <list>
#include <vector>
#include <cartobase/exception/assert.h>
#include <cartobase/smart/rcptr.h>

using namespace sigraph;
using namespace aims;
using namespace carto;
using namespace std;


int main( int argc, const char** argv )
{
  //
  // Parser of options
  //

  string		gname, tvname, vname, tname, otrans, itrans;
  vector<string>	bname, aname, lname1, synt1;
  set<string>		lname, synt;
  bool                  use_tal = false;
  Reader<AffineTransformation3d> trreader;

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
                 "transform is centered on 0 and will end up with most of the "
                 "brain outside of the output volume if used alone without an "
                 "additional translation.", true );
  app.addOption( trreader, "--transform",
                 "apply the given coordinates transformation (.trm file), "
                 "after Talairach transform if --talairach is also used",
                 true );

  try
    {
      app.initialize();

      VolumeRef<short> vol;

      Reader<Graph>        fr( gname );
      unique_ptr<Graph>	fg( fr.read() );

      lname.insert( lname1.begin(), lname1.end() );
      synt.insert( synt1.begin(), synt1.end() );

      cout << "graph read\n";
      if( !tname.empty() )
        si().setLabelsTranslPath( tname );

      // transformation handling
      AffineTransformation3d transform, talairach;
      talairach = GraphManip::talairach( *fg );
      if( use_tal )
        transform = talairach;

      if( !trreader.fileName().empty() )
      {
        AffineTransformation3d tr;
        trreader.read( tr );
        transform = tr * transform;
      }

      // graph voxel size
      vector<float>	vs;
      if( !fg->getProperty( "voxel_size", vs ) || vs.size() < 3 )
      {
        vs.push_back( 1 );
        vs.push_back( 1 );
        vs.push_back( 1 );
      }
      // volume voxel size
      vector<float> vvs( 1., 3 );

      if( !tvname.empty() )
      {
        Reader<Volume<short> > tvreader( tvname );
        vol.reset( tvreader.read() );
        vvs = vol->getVoxelSize();
      }
      else	// make vol from graph information
      {
        vector<int>	dims;
        ASSERT( fg->getProperty( "boundingbox_max", dims )
                && dims.size() >= 3 );
        vol = VolumeRef<short>( dims[0] + 1, dims[1] + 1, dims[2] + 1 );
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

        refs.push_back( "Talairach-AC/PC-Anatomist" );
        vtrans.push_back( ( talairach * invtrans ).toVector() );

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
            if( ref != "Talairach-AC/PC-Anatomist" )
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

      map<string, short>			labels;
      map<short, string>			rlabels;
      Graph::iterator				iv, fv=fg->end();
      string					name;
      vector<float>				vsz;
      set<string>				slab;
      set<string>::const_iterator		is, fs=slab.end();
      short					i, lastlabel;
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

      set<string>::const_iterator	el = lname.end();

      if( !tname.empty() )
        trans.readLabels( tname );

      for( il=trans.begin(); il!=fl; ++il )
        if( lname.empty() || lname.find( il->second ) != el )
          slab.insert( il->second );
      // cout << slab.size() << " labels with translation file\n";
      for( lastlabel = 1, is=slab.begin(); is!=fs; ++is, ++lastlabel )
      {
        if( labels.find( *is ) == labels.end() )
        {
          while( rlabels.find( lastlabel ) != rlabels.end() )
            ++lastlabel;
          labels[ *is ] = lastlabel;
          rlabels[ lastlabel ] = *is;
        }
      }
      // cout << labels.size() << " labels indices\n";
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
              i = labels[name];
              if( i == 0 && trans.empty() 
                  && ( lname.empty() || lname.find( name ) != el ) )
                {
                  while( rlabels.find( lastlabel ) != rlabels.end() )
                    ++lastlabel;
                  i = lastlabel;
                  labels[ name ] = i;
                  rlabels[ i ] = name;
                  ++lastlabel;
                }
              if( i != 0 )
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
                              vol->at( pos[0], pos[1], pos[2] ) = i;
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
                                  vol->at( pos[0], pos[1], pos[2] ) = i;
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


