/*
 *  Copyright (C) 1998-2008 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 */

#include <si/fold/labelsTranslator.h>
#include <si/graph/mgraph.h>
#include <si/global/global.h>
#include <si/fold/fattrib.h>
#include <aims/def/path.h>
#include <aims/selection/selection.h>
#include <aims/io/selectionr.h>
#include <graph/tree/tree.h>
#include <graph/tree/treader.h>
#include <cartobase/object/sreader.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>

using namespace sigraph;
using namespace aims;
using namespace carto;
using namespace std;


void FoldLabelsTranslator::readLabels( const string & filename )
{
  //cout << "readLabels " << filename << "...\n";
  if( readSelection( filename ) || readHierarchy( filename ) 
      || readJFM95( filename ) )
    return;
  cerr << "FoldLabelsTranslator::readLabels - FAILED\n";
}


bool FoldLabelsTranslator::readJFM95( const string & filename )
{
  //cout << "readJFM95 " << filename << endl;
  char		buf[500], sill[200], trans[200];
  ifstream	file( filename.c_str() );
  string	sillL, sillR, sillL2, sillR2;

  if( !file )
    return( false );
  erase( begin(), end() );

  while( !file.eof() )
    {
      do
	{
	  file.getline( buf, 500, '\n' );
	}
      while( buf[0] != '\0' && buf[0] != '%' );	// sauter commentaires

      if( !file.eof() && buf[0] != '\0' )
	{
	  sscanf( buf+1, "%s", sill );
	  trans[0] = '\0';
	  sscanf( buf+1+strlen( sill ), "%s", trans );
	  if( sill[0] != '\0' )
	    {
	      sillL = string( sill ) + "_left";		// "name_left" variant
	      sillR = string( sill ) + "_right";
	      sillL2 = string( sill ) + " left";	// "name left" variant
	      sillR2 = string( sill ) + " right";
	      if( find( sillL ) != end() )
		cout << "Sillon " << sill << " lu plusieurs fois !\n";

	      if( trans[0] == '\0' )
		{
		  (*this)[ sillL ] = sillL;
		  (*this)[ sillR ] = sillR;
		  (*this)[ sillL2 ] = sillL;
		  (*this)[ sillR2 ] = sillR;
		  //cout << sill << " inchangé\n";
		}
	      else if( string( trans ) != SIV_VOID_LABEL )
		{
		  (*this)[ sillL ] = string( trans ) + "_left";
		  (*this)[ sillR ] = string( trans ) + "_right";
		  (*this)[ sillL2 ] = string( trans ) + "_left";
		  (*this)[ sillR2 ] = string( trans ) + "_right";
		  //cout << sill << " -> " << trans << endl;
		}
	    }
	}
    }
  cout << size() << " labels de sillons lus.\n";
  return( true );
}


bool FoldLabelsTranslator::readHierarchy( const string & filename )
{
  //cout << "readHierarchy: " << filename << endl;
  SyntaxSet	ss;
  Tree		hier;
  try
    {
      SyntaxReader	sr( Path::singleton().syntax() + "/hierarchy.stx" );
      sr >> ss;
      TreeReader	tr( filename, ss );
      tr >> hier;
    }
  catch( exception & e )
    {
      //cout << "couldn't read " << filename << " as a hierarchy\n";
      return( false );
    }

  erase( begin(), end() );
  insertTree( hier );
  return( true );
}


bool FoldLabelsTranslator::readSelection( const string & filename )
{
  //cout << "readSelection: " << filename << endl;
  SelectionSet	ss;
  try
    {
      SelectionReader	sr( filename );
      sr.read( ss );
    }
  catch( exception & e )
    {
      //cout << "couldn't read " << filename << " as a selection\n";
      return( false );
    }

  erase( begin(), end() );
  insertSelection( ss );
  return( true );
}


void FoldLabelsTranslator::makeFromModel( const MGraph & mg, 
					  const string & filename )
{
  if( makeFromModelHierarchy( mg, filename ) )
    return;
  if( !makeFromModelJFM95( mg, filename ) )
    cerr << "FoldLabelsTranslator::makeFromModel - FAILED\n";
}


bool FoldLabelsTranslator::makeFromModelJFM95( const MGraph & mg, 
					       const string & filename )
{
  // cout << "makeFromModelJFM95 " << filename << endl;
  //	lire le fichier

  string	fname;
  if( filename.empty() )
    fname = si().labelsTranslPath();
  else
    fname = filename;
  if( !readJFM95( fname ) )
    {
      cerr << "Couldn't read JFM95 translation file " << fname << endl;
      return( false );
    }

  MGraph::const_iterator	im, fm=mg.end();
  set<string>			allowed;
  string			label;

  //	liste des labels du modèle

  for( im=mg.begin(); im!=fm; ++im )
    {
      assert( (*im)->getProperty( SIA_LABEL, label ) );
      allowed.insert( label );
    }

  iterator		il, fl=end();
  set<string>		toRemove;
  set<string>::iterator	is, fs=allowed.end();

  //	marquer ceux à enlever (qui n'ont pas de correspondance dans le MGraph)

  for( il=begin(); il!=fl; ++il )
    if( allowed.find( (*il).second ) == fs )
      {
	//cout << "pas de modèle pour " << (*il).second << endl;
	toRemove.insert( (*il).first );
      }

  //	enlever

  //cout << "\nnb : " << size() << endl;
  //cout << "à enlever : " << toRemove.size() << endl;
  for( is=toRemove.begin(), fs=toRemove.end(); is!=fs; ++is )
    erase( find( *is ) );
  //cout << "reste : " << size() << " labels\n\n";
  si().setLabelsTranslPath( fname );
  return( true );
}


bool FoldLabelsTranslator::makeFromModelHierarchy( const MGraph & mg, 
						   const string & filename )
{
  // cout << "makeFromModelHierarchy " << filename << endl;
  MGraph::const_iterator	im, fm=mg.end();
  set<string>			allowed;
  string			label;

  //	liste des labels du modèle

  for( im=mg.begin(); im!=fm; ++im )
    {
      assert( (*im)->getProperty( SIA_LABEL, label ) );
      allowed.insert( label );
    }

  string	fname;
  if( filename.empty() )
    {
      fname = si().labelsTranslPath();
      if( fname.empty() ) // WHY THIS?? || fname.rfind( ".def" ) == fname.length() - 4 )
	fname = Path::singleton().hierarchy() + "/sulcal_root_colors.hie";
    }
  else
    fname = filename;

  // cout << "filename : " << fname << endl;

  SyntaxSet	ss;
  Tree		hier;
  try
    {
      SyntaxReader	sr( Path::singleton().syntax() + "/hierarchy.stx" );
      sr >> ss;
      TreeReader	tr( fname, ss );
      tr >> hier;
    }
  catch( exception & e )
    {
      // cout << "makeFromModelHierarchy failed\n";
      return false;
    }

  si().setLabelsTranslPath( fname );
  erase( begin(), end() );
  // cout << "allowed names : " << allowed.size() << endl;
  insertTree( hier, allowed );
  // cout << size() << " names read as hierarchy\n";
  return( true );
}


/*
bool FoldLabelsTranslator::makeFromModelSelection( const MGraph & mg, 
						   const string & filename )
{
  //cout << "readSelection: " << filename << endl;
  SelectionSet	ss;
  try
    {
      SelectionReader	sr( filename );
      sr.read( ss );
    }
  catch( exception & e )
    {
      //cout << "couldn't read " << filename << " as a selection\n";
      return( false );
    }

  SelectionExpander	se;
  se.query( mg, ss );
  erase( begin(), end() );
  insertSelection( ss );
  return( true );
}
*/


void FoldLabelsTranslator::translate( CGraph & gr, const string & ilabel, 
				      const string & olabel, 
				      const string & altlabel ) const
{
  CGraph::iterator	ig, fg=gr.end();
  string		label, trans;

  for( ig=gr.begin(); ig!=fg; ++ig )
    if( (*ig)->getProperty( ilabel, label ) 
	|| ( !altlabel.empty() && (*ig)->getProperty( altlabel, label ) ) )
      {
	trans = lookupLabel( label );
	(*ig)->setProperty( olabel, trans );
      }
    else
      (*ig)->setProperty( olabel, string( SIV_VOID_LABEL ) );
}


string FoldLabelsTranslator::lookupLabel( string label ) const
{
  string		trans;
  string		label2;
  const_iterator	im, fm = end();

  //cout << "translate " << label << endl;
  do
    {
      string::size_type pos = label.find( '+' );
      if( pos != string::npos )
	{
	  label2 = label.substr( 0, pos );
	  //cout << "translate nom composé --- " << label2 << endl;
	  im = find( label2 );
	  label.erase( 0, pos+1 );
	}
      else
	{
	  im = find( label );
	  label.erase( 0, label.size() );
	}

      if( im == fm )
	{
	  trans = SIV_VOID_LABEL;
	  break;
	}
      else
	{
	  if( trans.empty() )
	    trans = (*im).second;
	  else if( trans != (*im).second )
	    {
	      trans = SIV_VOID_LABEL;
	      break;
	    }
	}
    }
  while( !label.empty() );
  //cout << "     --> " << trans << endl;

  return trans;
}


void FoldLabelsTranslator::insertTree( const Tree & t )
{
  Tree::const_iterator	it, et = t.end();
  string		name;
  const Tree		*t2;

  if( t.getProperty( "name", name ) )
    (*this)[ name ] = name;

  for( it=t.begin(); it!=et; ++it )
    if( ( t2 = dynamic_cast<const Tree *>( *it ) ) )
      insertTree( t2 );
}


void FoldLabelsTranslator::insertTree( const Tree & t, 
				       const set<string> & mnames )
{
  Tree::const_iterator	it, et = t.end();
  string		name;
  const Tree		*t2;

  //cout << "FoldLabelsTranslator::insertTree, tree size : " << t.size() 
  //<< "\n";
  if( t.getProperty( "name", name ) )
    {
      //cout << "trying name " << name << "...\n";
      string				name2( name );
      set<string>::const_iterator	in, en = mnames.end();
      t2 = &t;
      while( ( in = mnames.find( name2 ) ) == en && t2 )
	{
	  t2 = (const Tree *) t2->getParent();
	  if( t2 )
	    t2->getProperty( "name", name2 );
	}
      if( in == en )
	name2 = SIV_VOID_LABEL;
      //cout << "insert name " << name << " : " << name2 << endl;
      (*this)[ name ] = name2;
    }

  for( it=t.begin(); it!=et; ++it )
    if( ( t2 = dynamic_cast<const Tree *>( *it ) ) )
      insertTree( *t2, mnames );
}


void FoldLabelsTranslator::insertSelection( const SelectionSet & sel )
{
  SelectionSet::const_iterator	iss, ess = sel.end();
  Selection::const_iterator	is, es;
  string			label;

  for( iss=sel.begin(); iss!=ess; ++iss )
    {
      label = iss->name();
      for( is=iss->begin(), es=iss->end(); is!=es; ++is )
        (*this)[ *is ] = label;
    }
}



