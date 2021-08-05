/*
 *  Copyright (C) 2001-2004 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 */


#include <cstdlib>
#include <aims/getopt/getopt2.h>
#include <aims/vector/vector.h>
#include <si/global/global.h>
#include <si/graph/attrib.h>
#include <aims/def/path.h>
#include <graph/tree/tree.h>
#include <aims/io/writer.h>
#include <aims/io/reader.h>
#include <cartobase/object/sreader.h>
#include <graph/tree/twriter.h>
#include <cartobase/stream/sstream.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdlib.h>
#include <float.h>
#include <aims/rgb/rgb.h> 
#include <aims/data/data_g.h>

#include <stdlib.h> 
#include <stdio.h>
#include <aims/io/io_g.h>

using namespace aims;
using namespace sigraph;
using namespace carto;
using namespace std;
 
typedef float float3[3];

void my_itoa(int value, std::string& buf, int base)
{
  
  int i = 30;
  
  buf = "";
  
  if (value == 0)
  buf = "0";
  for(;  value && i   ; --i, value /= base) buf = "0123456789abcdef"[value % base] + buf;
  
}

int main(int argc, const char **argv)
{
  try
    {
      string	 palettefile, outfile,infile,suffix, syntax = "CorticalFoldArg";
      float min = FLT_MAX, max = -FLT_MAX;
      int transparancyMin = 300, transparancyMax = 300 ;
      bool  biggest = false ,keepline = false, invert = false;


      AimsApplication     app( argc, argv, "Build colored hierarchy from p-value table.");

      app.addOption(palettefile , "-p", "Input RGV palette image.");
      app.alias( "--palette", "-p" );

      app.addOption(infile , "-i", "Input statistic file.");
      app.alias( "--input", "-i" );

      app.addOption(outfile, "-o", "Output hierarchy");
      app.alias( "--output", "-o" );

      app.addOption(suffix , "-s", "Suffix (eg. _left or _right) (default = none)",true);
      app.alias( "--suffix", "-s" );

      app.addOption(transparancyMin , "-t", "Transparancy for values less than min (default = none)",true);
      app.alias( "--transparancy", "-t" );

      app.addOption(transparancyMax , "-T", "Transparancy for values greater than max (default = none)",true);
      app.alias( "--Transparancy", "-T" );

      app.addOption( biggest, "-b", 
                     "Choose the biggest value (instead of the smallest value) in case of multiple possibility (default: false)", true );
      app.alias( "--biggest", "-b" );

      app.addOption( keepline, "-k", 
                     "Keep the first line (i.e. header) of the file (default: false)", true );
      app.alias( "--keepline", "-k" );

      app.addOption( invert, "-inv", 
                     "Invert label/name  (default: false)", true );
      app.alias( "--invert", "-inv" );

	

      app.addOption(syntax , "-g", "Hierarchy graph syntax (default = CorticalFoldArg)",true);
      app.alias( "--graph", "-g" );

      app.addOption(max, "-M", "Maximum palette value (default = auto).",true);
      app.alias( "--Max", "-M" );

      app.addOption(min, "-m", "Minimum palette value (default = auto).",true);
      app.alias( "--min", "-m" );

      app.initialize();


      // read stat file 
      cout << "Reading statistic file : " <<  infile << endl ;
      map<string,set<float> > raw_stat;
      map<string,set<float> >::iterator irs,ers;
      map<string,float> stat;
      map<string,float >::iterator  is,es ;

      ifstream 	tf(infile.c_str());
      if (!tf)
        {
          cout << "File " << infile << " missing.\n" ;
          assert(0);
        }
      string 	name;
      float		pvalue;
      char temp[256];
      
      if (!keepline)
        {
          tf.getline(temp,256) ;
          cout << "Remove the first line of the line (header) : " << temp << endl;
        }	
	
      while ( tf && !tf.eof() )
        {
          tf >> name >> pvalue ;
          name += string (suffix);
          if (!tf.eof() && !name.empty() )	
            raw_stat[name].insert(pvalue);
        }

      if (biggest)
        for ( irs=raw_stat.begin(), ers = raw_stat.end(); irs != ers; ++irs )
        {
          name = irs->first ;
          pvalue = *((irs->second).rbegin());
          stat[name] = pvalue;
        }
      else
        for ( irs=raw_stat.begin(), ers = raw_stat.end(); irs != ers; ++irs )
        {
          name = irs->first ;
          pvalue = *((irs->second).begin());
          stat[name] = pvalue;
        }

      if (max == - FLT_MAX)
        {
          for ( is=stat.begin(), es = stat.end(); is != es; ++is )
            if ( is->second > max) max = is->second ;
        }
      if (min ==  FLT_MAX)
        {
          for ( is=stat.begin(), es = stat.end(); is != es; ++is )
            if ( is->second < min) min = is->second ;
        }

      cout << "Max : " << max << endl;
      cout << "Min : " << min << endl;
 
      // read palette
      cout << "Reading palette   : " << palettefile << "..." ;
      VolumeRef<AimsRGB> palette;
      Reader<Volume<AimsRGB> > pal( palettefile );
      palette.reset( pal.read() );
      cout << "done \n";
  
      vector<AimsRGB> colors;
      unsigned lengthPalette = palette.getSizeX();
      for (unsigned i = 0 ; i < lengthPalette ; ++i )
        colors.push_back( palette.at(i,0,0) );

      //Build hierarchy
      int r,g,b;
      AimsRGB rgb;
      unsigned ic;
      float d; 

      Tree H(true,"hierarchy");
      H.setProperty("graph_syntax",string(syntax));
      Tree *A =  new Tree(true,"fold_name") ;
      A->setProperty("name", string("All"));

      std::string buf;
      
      for ( is=stat.begin(); is != es; ++is )
        {
          Tree *t =  new Tree(true,"fold_name");
          vector<int> vec;
          name = is->first  ;
          pvalue = is->second ;
          d = ( pvalue - min  )/( max - min ) ; // cf anatomist/src/lib/color/objectPalette
          if( d < 0 )
            d = 0;
          else if( d >= 0.9999 )
            d = 0.9999;
          ic = int (d * lengthPalette);
          
          rgb = colors[ ic ];  
          r=rgb.red() ;
          g=rgb.green() ;
          b= rgb.blue() ;
          vec.push_back(r);
          vec.push_back(g);
          vec.push_back(b);
          if (transparancyMin != 300 && pvalue < min  ) 
            vec.push_back(transparancyMin);
          if (transparancyMax != 300 && pvalue > max  ) 
            vec.push_back(transparancyMax);
          t->setProperty("color", vec);
          my_itoa((int)pvalue,buf,10);
          if (invert)
            {
              t->setProperty("label",name  );
              t->setProperty("name", buf );
            }
          else
            {
              t->setProperty("name", name );
              t->setProperty("label", buf );
            }
          A->insert(t);
        }
      H.insert(A);

      SyntaxSet	ss;
      SyntaxReader	sr( Path::singleton().syntax() + "/hierarchy.stx" );
      sr >> ss;
      TreeWriter tw(outfile,ss);
      tw << H;
    }
  catch( user_interruption & )
    {
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
