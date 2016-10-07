/*
 *  Copyright (C) 1998-2004 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */

#include <cstdlib>
#include <assert.h>
#include <graph/tree/tree.h>
#include <graph/tree/treader.h>
#include <si/global/global.h>
#include <cartobase/object/sreader.h>
#include <iomanip>
#include <vector>
#include <iostream>
#include <stdio.h>

using namespace sigraph;
using namespace carto;
using namespace std;


void usage( const char* name )
{
  cerr << "usage : \n" << name << " resultfile.lyx resultfile.tre [dir]\n";
  cerr << "dir: directory où seront écrites les images .eps\n\n";
  cerr << "Ecrit les tableaux d'erreurs dans un fichier au format LyX\n" 
       << "Ces erreurs doivent avoir été calculées avec siErrorStats\n\n";
  exit( 1 );
}


//void newTable( ostream & f, const vector<string> & lines, 
//	       const vector<string> & cols )
void newTable( ostream & f, unsigned nl, unsigned nc )
{
  unsigned	i; //, j, nl = lines.size(), nc = cols.size();

  //	table
  //  f << "\\layout Standard\n";

  f << "\\begin_inset  Tabular\n";
  f << "<lyxtabular version=\"2\" rows=\"" << nl << "\" columns=\"" 
    << nc << "\">\n";
  f << "<features rotate=\"false\" islongtable=\"false\" endhead=\"0\" "
    << "endfirsthead=\"0\" endfoot=\"0\" endlastfoot=\"0\">\n";

  for( i=0; i<nc; ++i )
    f << "<column alignment=\"center\" valignment=\"top\" leftline=\"true\" "
      << "rightline=\"true\" width=\"\" special=\"\">\n";
}


void newFoldsTable( ostream & f, unsigned nfl, unsigned nperline, 
		    unsigned lpf, unsigned cpf )
{
  vector<string>	lines, cols;
  unsigned		i, j;

  //  nl = nfl * lpf;	// nb lignes = nb lignes de sillon * nb lignes par sil.

  for( i=0; i<nperline; ++i )
    {
      if( cpf >= 2 )
	{
	  cols.push_back( "1 0" );
	  for( j=1; j<cpf-1; ++j )
	    cols.push_back( "0 0" );
	  cols.push_back( "0 1" );
	}
      else
	cols.push_back( "1 1" );
      }

  for( i=0; i<nfl; ++i )
    {
      lines.push_back( "1 0" );		// 1ere ligne de chaque sillon
      for( j=1; j<lpf-1; ++j )
	lines.push_back( "0 0" );	// lignes du milieu
      lines.push_back( "0 1" );		// dernière ligne du sillon
    }

  newTable( f, lines.size(), cols.size() );
}


void newGraphsTable( ostream & f, unsigned nb, unsigned nt, unsigned ng )
{
  unsigned	i, nl = nb + nt + ng + 4, ncol = 6;
  f << "\\begin_inset  Tabular\n";
  f << "<lyxtabular version=\"2\" rows=\"" << nl << "\" columns=\"" << ncol 
    << "\">\n";
  f << "<features rotate=\"false\" islongtable=\"false\" endhead=\"0\" "
    << "endfirsthead=\"0\" endfoot=\"0\" endlastfoot=\"0\">\n";
  for( i=0; i<ncol; ++i )
    f << "<column alignment=\"center\" valignment=\"top\" leftline=\"true\" "
      << "rightline=\"false\" width=\"\" special=\"\">\n";

  //	ligne de titres
  f << "<row topline=\"true\" bottomline=\"true\" newpage=\"false\">\n";
  f << "<cell>\n\\begin_inset Text\n";
  f << "\\series bold\n";
  f << "Graphe\n";
  f << "\\end_inset\n</cell>\n";
  f << "<cell>\n\\begin_inset Text\n";
  f << "\\series bold\n";
  f << "Noeuds\n";
  f << "\\end_inset\n</cell>\n";
  f << "<cell>\n\\begin_inset Text\n";
  f << "\\series bold\n";
  f << "Masse\n";
  f << "\\end_inset\n</cell>\n";
  //f << "Err n\n\\newline\n";
  f << "<cell>\n\\begin_inset Text\n";
  f << "\\series bold\n";
  f << "Taux\n";
  f << "\\end_inset\n</cell>\n";
  //f << "p(G)\n\\newline\n";
  f << "<cell>\n\\begin_inset Text\n";
  f << "\\series bold\n";
  f << "Ei\n";
  f << "\\end_inset\n</cell>\n";
  f << "<cell>\n\\begin_inset Text\n";
  f << "\\series bold\n";
  f << "Ef\n";
  f << "\\end_inset\n</cell>\n";
  f << "</row>\n";
}


vector<float> 
gnuplotErrGraph( const vector<float> & adds, const vector<float> & rejs, 
		 unsigned nb, unsigned nt, unsigned ng, 
		 const string & psfile )
{
  vector<float>	res;
  unsigned	n = nb + nt + ng, i, j=0, c, ind;
  unsigned	gr[3];
  string	datfile = "/tmp/gnuplot.dat";
  string	datfile2 = "/tmp/gnuplot2.dat";
  string	comfile = "/tmp/gnucom.txt";
  float		err, serr;

  assert( n == adds.size() );
  assert( n == rejs.size() );

  gr[0] = nb;
  gr[1] = nt;
  gr[2] = ng;

  res.push_back( 0 );
  res.push_back( 0 );
  res.push_back( 0 );

  //	fichiers de données pour gnuplot

  ofstream	datf( datfile.c_str() );

  if( !datf )
    {
      cerr << "cannot write " << datfile << endl;
      return ( res );
    }

  ofstream	datf2( datfile2.c_str() );

  if( !datf2 )
    {
      cerr << "cannot write " << datfile2 << endl;
    }

  c = 0;
  ind = 0;
  for( j=0; j<3; ++j )	// séries de graphes
    {
      serr = 0;
      for( i=0; i<gr[j]; ++i )
	serr += adds[ind+i] + rejs[ind+i];
      if( gr[j] == 0 )
	err = serr;
      else
	err = serr / gr[j];
      datf2 << ((float)c)-1 << "\t" << err * 100 << endl;
      res[j] = err * 100;

      for( i=0; i<gr[j]; ++i, ++c, ++ind )
	{
	  datf << c;
	  datf << "\t" << (adds[ind] + rejs[ind]) * 100 
	       << "\t" << adds[ind] * 100;
	  datf << endl;
	}
      ++c;
      datf2 << ((float)c)-1 << "\t" << err * 100 << endl;
    }
  datf2.close();
  datf.close();

  //	fichier de commandes gnuplot

  ofstream	comf( comfile.c_str() );
  if( comf )
    {
      comf << "set size 0.5\n";
      comf << "set boxwidth 1\n";
      comf << "set term postscript eps color\n";
      comf << "set output \"" << psfile << "\"\n";
      comf << "plot [-1:" << c-1 << "][0:100] \"" 
	   << datfile << "\" u 1:2 t \"\" w boxes 1, ";
      comf << "\"" << datfile << "\" u 1:3 t \"\" w boxes 3, ";
      comf << "\"" << datfile2 << "\" u 1:2 t \"\" w l 2\n";
      comf.close();

      //	lancer gnuplot
      string	command = "gnuplot \"";

      command += comfile;
      command += "\"";
      if( system( command.c_str() ) != 0 )
        cerr << "Warning: command '" << command << "' failed.\n";

      //	on efface les commandes
      remove( comfile.c_str() );
    }
  else
    cerr << "cannot write " << comfile << endl;

  //	on efface les données
  remove( datfile.c_str() );
  remove( datfile2.c_str() );

  return res;
}


void gnuplotSizeGraph( const vector<float> & sizes, 
		       unsigned nb, unsigned nt, unsigned ng, 
		       const string & psfile )
{
  unsigned	n = nb + nt + ng, i, j=0, c, ind;
  unsigned	gr[3];
  string	datfile = "/tmp/gnuplot.dat";
  string	comfile = "/tmp/gnucom.txt";

  assert( n == sizes.size() );

  gr[0] = nb;
  gr[1] = nt;
  gr[2] = ng;

  //	fichiers de données pour gnuplot

  ofstream	datf( datfile.c_str() );

  if( !datf )
    {
      cerr << "cannot write " << datfile << endl;
      return;
    }

  c = 0;
  ind = 0;
  for( j=0; j<3; ++j )	// séries de graphes
    {
      for( i=0; i<gr[j]; ++i, ++c, ++ind )
	{
	  datf << c;
	  datf << "\t" << sizes[ind];
	  datf << endl;
	}
      ++c;
    }
  datf.close();

  //	fichier de commandes gnuplot

  ofstream	comf( comfile.c_str() );
  if( comf )
    {
      comf << "set size 0.5\n";
      comf << "set boxwidth 1\n";
      comf << "set term postscript eps color\n";
      comf << "set output \"" << psfile << "\"\n";
      comf << "plot [-1:" << c-1 << "][0:] \"" 
	   << datfile << "\" u 1:2 t \"\" w boxes 3\n";
      comf.close();

      //	lancer gnuplot
      string	command = "gnuplot \"";

      command += comfile;
      command += "\"";
      if( system( command.c_str() ) != 0 )
        cerr << "Warning: command '" << command << "' failed.\n";

      //	on efface les commandes
      remove( comfile.c_str() );
    }
  else
    cerr << "cannot write " << comfile << endl;

  //	on efface les données
  remove( datfile.c_str() );
}


void newFigure( ostream & f )
{
  f << "\\begin_float fig\n";
  f << "\\layout Standard\n";
}


void endFigure( ostream & f, const string & caption, const string & label )
{
  f << "</lyxtabular>\n";
  f << "\\end_inset\n";
  f << "\\layout Standard\n";
  f << label << endl;
  f << "\\layout Caption\n";
  f << caption << endl;
  f << "\\end_float\n";

  /*f << "\\layout Caption\n\n";
  f << caption << endl;
  f << "\\begin_inset LatexCommand \\label{" << label << "}\n";
  f << endl;
  f << "\\end_inset\n";
  f << "\n\n";
  f << "\\end_float\n";*/
}


string figure( const string & psfile, unsigned w )
{
  string	of;
  char		num[10];

  of = "\\begin_inset Figure size 180 80\n";
  of += "  file ";
  of += psfile + "\n";
  of += "  width 3 ";
  sprintf( num, "%d", w );
  of += num;
  of += "\n  flags 11\n";
  of += "\\end_inset\n";

  return of;
}


string formula( const string & form )
{
  string	res = "\\begin_inset Formula \\( ";

  res += form + " \\)\n";
  res += "\\end_inset\n\n";
  return res;
}


int main( int argc, char** argv )
{
  if( argc < 3 || argc > 4 )
    usage( argv[0] );

  Tree			errTr( true, "siErrorStats" );
  string		dirname;

  if( argc >= 4 )
    {
      dirname = argv[3];
      dirname += "/";
    }

  try
    {
      SyntaxReader	sr( si().basePath() + "/config/siErrorStats.stx" );
      SyntaxSet	ss;

      sr >> ss;

      TreeReader	tr( argv[2], ss );
      tr >> errTr;
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
      throw;
    }

  ofstream	f( argv[1] );
  assert( f );

  unsigned		nl;
  int			i, j, nbase = 0, ntest = 0, ngen = 0;
  Tree::const_iterator	it, ft;
  Tree			*t, *folds = 0, *graphs = 0;
  string		label;

  errTr.getProperty( "baseGraphs", nbase );
  errTr.getProperty( "testGraphs", ntest );
  errTr.getProperty( "genGraphs", ngen );

  for( it = errTr.begin(), ft=errTr.end(); it!=ft; ++it )
    {
      t = (Tree *) *it;
      if( t->getSyntax() == "folds" )
	folds = t;
      else
	graphs = t;
    }

  assert( folds && graphs );

  //	en-tête LyX
  f << "#This file was created by <siLyxErrors> Thu Jan 27 18:11:07 2000\n";
  f << "#LyX 1.1 http://www.lyx.org/\n";
  f << "\\lyxformat 218\n";
  f << "\\textclass article\n";
  f << "\\language french\n";
  f << "\\inputencoding latin1\n";
  f << "\\fontscheme default\n";
  f << "\\graphics default\n";
  f << "\\paperfontsize default\n";
  f << "\\spacing single\n";
  f << "\\papersize Default\n";
  f << "\\paperpackage widemarginsa4\n";
  f << "\\use_geometry 0\n";
  f << "\\use_amsmath 0\n";
  f << "\\paperorientation portrait\n";
  f << "\\secnumdepth 3\n";
  f << "\\tocdepth 3\n";
  f << "\\paragraph_separation indent\n";
  f << "\\defskip medskip\n";
  f << "\\quotes_language english\n";
  f << "\\quotes_times 2\n";
  f << "\\papercolumns 1\n";
  f << "\\papersides 1\n";
  f << "\\paperpagestyle default\n";
  f << "\n";

  f << "\\layout Standard\n\n";
  f << "Commencer le copier/coller ici.\n";
  f << "\\layout Standard\n";


  //	Tables de graphes

  string				name;
  int					nc;
  float					mass, merr, Ei, Ef;
  vector<float>				vadd, vrej, vsize;
  int					grlist[3], grnodes[3];
  float					grmass[3], grmerr[3], grEi[3], grEf[3];
  string				grtitle[3] 
    = { "Base :", "Test :", "Gén. :" };

  //	tri par ordre alphabétique

  /*for( it=graphs->begin(), ft=graphs->end(); it!=ft; ++it )
    {
      t = (Tree *) *it;
      t->getProperty( "name", name );
      mt[ name ] = t;
      }*/

  nl = graphs->childrenSize();
  grlist[0] = nbase;
  grlist[1] = ntest;
  grlist[2] = ngen;
  grnodes[0] = grnodes[1] = grnodes[2] = 0;
  grmerr[0] = grmerr[1] = grmerr[2] = 0;
  grEi[0] = grEi[1] = grEi[2] = 0;
  grEf[0] = grEf[1] = grEf[2] = 0;

  newFigure( f );
  newGraphsTable( f, nbase, ntest, ngen );

  for( it=graphs->begin(), ft=graphs->end(), i=0; i<3; ++i )
    {
      grmass[i] = 0;
      for( j=0; j<grlist[i]; ++j, ++it )
	{
	  t = (Tree *) *it;
	  t->getProperty( "name", name );
	  t->getProperty( "nodes_count", nc );
	  t->getProperty( "mass", mass );
	  t->getProperty( "pmass_error", merr );
	  //t->getProperty( "node_errors_count", nerr );
	  //t->getProperty( "folds_count", nfolds );
	  //t->getProperty( "partly_recogn_folds", npG );
	  t->getProperty( "initial_energy", Ei );
	  t->getProperty( "final_energy", Ef );
	  f << "<row topline=\"true\" bottomline=\"true\" "
	    << "newpage=\"false\">\n";
	  f << "<cell multicolumn=\"0\" alignment=\"center\" "
	    << "valignment=\"top\" topline=\"true\" bottomline=\"false\" "
	    << "leftline=\"true\" rightline=\"true\" rotate=\"false\" " 
	    << "usebox=\"none\" width=\"\" special=\"\">\n";
	  f << "\\begin_inset Text\n";
	  f << name << "\n\\end_inset\n</cell>\n";
	  f << "<cell>\n\\begin_inset Text\n";
	  f << nc << "\n\\end_inset\n</cell>\n";
	  grnodes[i] += nc;
	  f << "<cell>\n\\begin_inset Text\n";
	  f << (int) mass << "\n";
	  f << "\\end_inset\n</cell>\n";
	  grmass[i] += mass;
	  merr = 100 - merr * 100;
	  f << "<cell>\n\\begin_inset Text\n";
	  f << setprecision( 3 ) << merr << "\n";
	  f << "\\end_inset\n</cell>\n";
	  grmerr[i] += merr;
	  f << "<cell>\n\\begin_inset Text\n";
	  f << setprecision( 4 ) << Ei << "\n";
	  f << "\\end_inset\n</cell>\n";
	  grEi[i] += Ei;
	  f << "<cell>\n\\begin_inset Text\n";
	  f << Ef << "\n";
	  f << "\\end_inset\n</cell>\n";
	  f << "</row>\n";
	  grEf[i] += Ef;
	}

      // sous-moyennes
      f << "<row topline=\"true\" bottomline=\"true\" newpage=\"false\">\n";
      f << "<cell>\n\\begin_inset Text\n";
      f << "\\series bold\n";
      f << grtitle[i] << "\n";
      f << "\\end_inset\n</cell>\n";
      if( grlist[i] )
	{
	  f << "<cell>\n\\begin_inset Text\n";
	  f << "\\series bold\n";
	  f << (int) ( ((float) grnodes[i]) / grlist[i] + 0.5 ) << endl;
	  f << "\\end_inset\n</cell>\n";
	  f << "<cell>\n\\begin_inset Text\n";
	  f << "\\series bold\n";
	  f << (int) ( ((float) grmass[i]) / grlist[i] + 0.5 ) << endl;
	  f << "\\end_inset\n</cell>\n";
	  f << "<cell>\n\\begin_inset Text\n";
	  f << "\\series bold\n";
	  f << setprecision( 3 ) << grmerr[i] / grlist[i] << endl;
	  f << "\\end_inset\n</cell>\n";
	  f << "<cell>\n\\begin_inset Text\n";
	  f << "\\series bold\n";
	  f << setprecision( 4 ) << grEi[i] / grlist[i] << endl;
	  f << "\\end_inset\n</cell>\n";
	  f << "<cell>\n\\begin_inset Text\n";
	  f << "\\series bold\n";
	  f << grEf[i] / grlist[i] << endl;
	  f << "\\end_inset\n</cell>\n";
	}
      else
	{
	  f << "<cell>\n\\begin_inset Text\n";
	  f << "\\end_inset\n</cell>\n";
	  f << "<cell>\n\\begin_inset Text\n";
	  f << "\\end_inset\n</cell>\n";
	  f << "<cell>\n\\begin_inset Text\n";
	  f << "\\end_inset\n</cell>\n";
	  f << "<cell>\n\\begin_inset Text\n";
	  f << "\\end_inset\n</cell>\n";
	  f << "<cell>\n\\begin_inset Text\n";
	  f << "\\end_inset\n</cell>\n";
	}
      f << "</row>\n";
      //      f << "\\series default\n";
    }

  // moyennes générales
  mass = grmass[0] + grmass[1] + grmass[2];
  f << "<row topline=\"true\" bottomline=\"true\" newpage=\"false\">\n";
  f << "<cell>\n\\begin_inset Text\n";
  f << "\\series bold\n";
  f << "Moyenne:\n";
  f << "\\end_inset\n</cell>\n";
  f << "<cell>\n\\begin_inset Text\n";
  f << "\\series bold\n";
  f << (int) (((float) grnodes[0] + grnodes[1] + grnodes[2] ) / nl +0.5) 
    << "\n";
  f << "\\end_inset\n</cell>\n";
  f << "<cell>\n\\begin_inset Text\n";
  f << "\\series bold\n";
  f << (int) (mass / nl + 0.5) << "\n";
  f << "\\end_inset\n</cell>\n";
  f << "<cell>\n\\begin_inset Text\n";
  f << "\\series bold\n";
  f << setprecision( 3 ) << (grmerr[0] + grmerr[1] + grmerr[2]) / nl 
    << "\n";
  f << "\\end_inset\n</cell>\n";
  f << "<cell>\n\\begin_inset Text\n";
  f << "\\series bold\n";
  f << setprecision( 4 ) << (grEi[0] + grEi[1] + grEi[2]) / nl 
    << "\n";
  f << "\\end_inset\n</cell>\n";
  f << "<cell>\n\\begin_inset Text\n";
  f << "\\series bold\n";
  f << (grEf[0] + grEf[1] + grEf[2]) / nl << "\n";
  f << "\\end_inset\n</cell>\n";
  f << "</row>\n";

  endFigure( f, "Taux de reconnaissance sur les sillons", 
	     "fig.sillons-erreur-graphes" );

  f << "\\layout standard\n\n";
  f << "Commencer le copier/coller ici.\n";


  //	Tables de sillons

  int				linespercell = 5, colspercell = 2;
  int				ncols = 2;
  int				maxperpage = 5;

  map<string, Tree *>			mt;
  map<string, Tree *>::const_iterator	im, fm=mt.end();
  string			psfile;
  vector<int>			tables;
  int				ng, tbl, npertable, ntables;
  int				nlines, k;
  int				c, ind;
  vector< vector<string> >	text;
  string			caption, figlabel;
  unsigned			nfolds;
  vector<float>			err;
  char				num[20];
  float				errs[4], errs2[4];
  float				wt_fld, wt_glob, smass = 0, smass2 = 0;
  set<string>			excludeLabels;
  set<string>::const_iterator	fex = excludeLabels.end();

  //	tri par ordre alphabétique

  errTr.getProperty( "graphs_count", ng );
  errs[0] = errs[1] = errs[2] = errs[3] = 0;
  errs2[0] = errs2[1] = errs2[2] = errs2[3] = 0;
  excludeLabels.insert( "unknown" );
  //excludeLabels.insert( "INSULA_right" );
  //excludeLabels.insert( "INSULA_left" );
  excludeLabels.insert( "OCCIPITAL_right" );
  excludeLabels.insert( "OCCIPITAL_left" );

  for( it=folds->begin(), ft=folds->end(); it!=ft; ++it )
    {
      t = (Tree *) *it;
      t->getProperty( "label", label );
      mt[label] = t;
    }

  nfolds = mt.size();
  nlines = nfolds / ncols;
  if( nlines * ncols < (int) nfolds )
    ++nlines;
  ntables = nlines / maxperpage;
  if( ntables * maxperpage < nlines )
    ++ntables;
  npertable = (int) ( ((float) nlines) / ntables + 0.5 );

  for( i=0; i<ntables-1; ++i )
    tables.push_back( npertable );
  tables.push_back( nlines - npertable * (ntables - 1) );

  for( tbl=0, im=mt.begin(); tbl<ntables; ++tbl )	// tables
    {
      newFigure( f );
      newFoldsTable( f, tables[tbl], ncols, linespercell, colspercell );

      for( i=0; i<tables[tbl]; ++i )			// lignes
	{
	  text.erase( text.begin(), text.end() );

	  for( j=0; j<ncols && im!=fm; ++j, ++im )	// colonnes
	    {
	      t = (*im).second;
	      t->getProperty( "label", label );
	      t->getProperty( "mass", mass );
	      t->getProperty( "adds_vector", vadd );
	      t->getProperty( "rejects_vector", vrej );
	      t->getProperty( "sizes_vector", vsize );
	      wt_fld = wt_glob = 0;
	      //t->getProperty( "weight_size", wt_sz );
	      t->getProperty( "weight_fold", wt_fld );
	      t->getProperty( "weight_global", wt_glob );

	      text.push_back( vector<string>() );

	      vector<string>	& txt = text[j];

	      psfile = dirname + "sillons_size-par-sillon-" + label + ".eps";
	      gnuplotSizeGraph( vsize, nbase, ntest, ngen, psfile );
	      txt.push_back( figure( psfile, 20 ) );

	      psfile = dirname + "sillons_err-par-sillon-" + label + ".eps";
	      err = gnuplotErrGraph( vadd, vrej, nbase, ntest, ngen, psfile );
	      txt.push_back( figure( psfile, 20 ) );

	      caption = "\\series bold\n";
	      txt.push_back( caption + label + "\n\\series default" );

	      merr = ( err[0] * nbase + err[1] * ntest + err[2] * ngen ) / ng;
	      errs[0] += err[0] * mass;
	      errs[1] += err[1] * mass;
	      errs[2] += err[2] * mass;
	      errs[3] += merr * mass;
	      smass += mass;

	      if( excludeLabels.find( label ) == fex )
		{
		  errs2[0] += err[0] * mass;
		  errs2[1] += err[1] * mass;
		  errs2[2] += err[2] * mass;
		  errs2[3] += merr * mass;
		  smass2 += mass;
		}

	      sprintf( num, "%4.1f", err[0] );
	      txt.push_back( formula( "E_{b}" ) + ": " + num );

	      //sprintf( num, "%4.1f", wt_sz );
	      //txt.push_back( formula( "P_{t}" ) + ": " + num );
	      txt.push_back( "" );

	      sprintf( num, "%4.1f", err[1] );
	      txt.push_back( formula( "E_{t}" ) + ": " + num );

	      sprintf( num, "%4.2f", wt_fld );
	      txt.push_back( formula( "P_{s}" ) + ": " + num );

	      sprintf( num, "%4.1f", err[2] );
	      txt.push_back( formula( "E_{g}" ) + ": " + num );

	      sprintf( num, "%4.2f", wt_glob );
	      txt.push_back( formula( "P" ) + ": " + num );

	      sprintf( num, "%4.1f", merr );
	      txt.push_back( formula( "E" ) + ": " + num );
	    }

	  //	Ecrire en lignes
	  for( k=0; k<linespercell; ++k )
	    {
	      f << "<row>\n";
	      for( j=0; j<(int)text.size(); ++j )
		{
		  for( c=0; c<colspercell; ++c )
		    {
		      f << "<cell>\n\\begin_inset Text\n";
		      ind = k*colspercell + c;
		      if( ind < (int) text[j].size() )
			f << text[j][ind] << endl;
		      if( c+1 < colspercell || k+1 < linespercell 
			  || j+1 < ncols || i+1 < tables[tbl] )
			f << "\n";
		      f << "\\end_inset\n</cell>\n";
		    }
		}
	      for( ; j<ncols; ++j )	// compléter les lignes vides
		{
		  for( c=0; c<colspercell; ++c )
		    {
		      f << "<cell>\n\\begin_inset Text\n";
		      if( c+1 < colspercell || k+1 < linespercell 
			  || j+1 < ncols || i+1 < tables[tbl] )
			f << "\n";
		      f << "\\end_inset\n</cell>\n";
		    }
		}
	      f << "</row>\n";
	    }
	}
      caption = "Taux d'erreur sillon par sillon, ";
      figlabel = "fig.sillons-erreur-par-sillon-";
      sprintf( num, "%d", tbl+1 );
      caption += num;
      figlabel += num;
      caption += " / ";
      sprintf( num, "%d", ntables );
      caption += num;
      endFigure( f, caption, figlabel );
      f << "\n\\layout Standard\n\n";
      f << "Commencer le copier/coller ici.\n";
    }

  // moyennes générales

  vector<string>	lines, cols;

  newFigure( f );

  lines.push_back( "1 1" );
  lines.push_back( "0 1" );
  lines.push_back( "0 1" );
  cols.push_back( "1 1" );
  for( i=0; i<4; ++i )
    cols.push_back( "0 1" );

  newTable( f, lines.size(), cols.size() );

  f << "<row>\n";
  f << "<cell>\n\\begin_inset Text\n";
  f << "\\series bold\n";
  f << "Moyennes\n";
  f << "\\end_inset\n</cell>\n";
  f << "<cell>\n\\begin_inset Text\n";
  f << "Base:\n";
  f << "\\end_inset\n</cell>\n";
  f << "<cell>\n\\begin_inset Text\n";
  f << "Test:\n";
  f << "\\end_inset\n</cell>\n";
  f << "<cell>\n\\begin_inset Text\n";
  f << "Généralisation:\n";
  f << "\\end_inset\n</cell>\n";
  f << "<cell>\n\\begin_inset Text\n";
  f << "Général:\n";
  f << "\\end_inset\n</cell>\n";
  f << "</row>\n";

  f << "<row>\n";
  f << "<cell>\n\\begin_inset Text\n";
  f << "Tout compris\n";
  f << "\\end_inset\n</cell>\n";
  sprintf( num, "%4.1f", errs[0] / smass );
  f << "<cell>\n\\begin_inset Text\n";
  f << num << "\n";
  f << "\\end_inset\n</cell>\n";
  sprintf( num, "%4.1f", errs[1] / smass );
  f << "<cell>\n\\begin_inset Text\n";
  f << num << "\n";
  f << "\\end_inset\n</cell>\n";
  sprintf( num, "%4.1f", errs[2] / smass );
  f << "<cell>\n\\begin_inset Text\n";
  f << num << "\n";
  f << "\\end_inset\n</cell>\n";
  sprintf( num, "%4.1f", errs[3] / smass );
  f << "<cell>\n\\begin_inset Text\n";
  f << num << "\n";
  f << "\\end_inset\n</cell>\n";
  f << "</row>\n";

  f << "<row>\n";
  f << "<cell>\n\\begin_inset Text\n";
  f << "Sillons seulement\n";
  f << "\\end_inset\n</cell>\n";
  sprintf( num, "%4.1f", errs2[0] / smass2 );
  f << "<cell>\n\\begin_inset Text\n";
  f << num << "\n";
  f << "\\end_inset\n</cell>\n";
  sprintf( num, "%4.1f", errs2[1] / smass2 );
  f << "<cell>\n\\begin_inset Text\n";
  f << num << "\n";
  f << "\\end_inset\n</cell>\n";
  sprintf( num, "%4.1f", errs2[2] / smass2 );
  f << "<cell>\n\\begin_inset Text\n";
  f << num << "\n";
  f << "\\end_inset\n</cell>\n";
  sprintf( num, "%4.1f", errs2[3] / smass2 );
  f << "<cell>\n\\begin_inset Text\n";
  f << num << "\n";
  f << "\\end_inset\n</cell>\n";
  f << "</row>\n";

  endFigure( f, "Taux d'erreurs globaux sur les sillons", 
	     "fig.sillons-erreurs-glob" );
  f << endl;

  f << "\\layout Standard\n";
  f << "<- Finir le copier/coller ici.\n";

  // fin LyX
  f << "\n\\the_end\n";

  return 0;
}
