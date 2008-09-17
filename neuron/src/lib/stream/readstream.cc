/*********************************
 * File : readstream.cc
 * Prog : fonctions pour lire des fichiers
 ***********************************************/


#include <stdio.h>
#include <string.h>
#include <fstream>
#include <neur/stream/readstream.h>

using namespace std;

char *ReadLine( istream & fich )
{
  if( !fich ) return( NULL );
  fich.unsetf(ios::skipws);

  unsigned	i, nch = 256;
  char		*buf, *buf2=NULL;

  buf = new char[nch];
  buf[0] = '\0';

  do
    {
      i = 0;
      while(i==0)
	{
	  fich >> buf[0];
	  if( !fich ) return( NULL );
	  if(buf[0]!=' ' && buf[0]!='\t' && buf[0]!='\n') i++;
	}
      if( buf[0]=='\0' ) return( NULL );

      do
	{
	  fich >> buf[i++];
	  if( i >= nch )
	    {
	      buf2 = buf;
	      nch *= 2;
	      buf = new char[nch];
	      strcpy( buf, buf2 );
	      delete[] buf2;
	      buf2 = NULL;
	    }
	  if( !fich ) return( NULL );
	} while(buf[i-1]!='\n' && buf[i-1]!='\0');
      if( buf[0]=='\0' ) return( NULL );
    } while(buf[0]=='#' || buf[0]=='\n');

  buf[--i] = '\0';

  if( i == 0 ) return( NULL );

  char	*bf;
  bf = new char[i+1];
  strcpy(bf,buf);
  delete[] buf;
  //  cout << bf << "\n";
  return(bf);
}



int ReadLabel( istream & fich, char *lbl )
{
  //cout << "ReadLabel : &fich=" << &fich << endl;
  //cout << "!fich : " << !fich << endl;
  if( !fich ) return( 0 );
  fich.unsetf(ios::skipws);

  int	i,comp = 1;
  int	j,lcf;
  char	buf[256];
  char	cfin[] = " :";

  lcf = strlen(cfin);

  for( i=j=0; j<lcf; i++ )
    {
      do
	{
	  fich >> buf[i];
	  if( !fich ) return( 0 );
	  while( i==0 && buf[0]=='#' ) 
	    {
	      while( buf[0]!='\n' && buf[0]!='\0' ) fich >> buf[0];
	      fich >> buf[0];
	      if( !fich ) return( 0 );
	    }
	}
      while( i==0 && (buf[0]==' ' || buf[0]=='\t' || buf[0]=='\n') );
      if( buf[i]=='\n')
	{
	  i = -1;
	  j = 0;
	}
      else if(buf[i] == cfin[j]) j++;
      else if(buf[i] == cfin[0]) j = 1;
      else j = 0;
    }

  i -= lcf;
  buf[i] = '\0';
  while( i>0 && ( buf[i-1] == ' ' || buf[i-1] == '\t' ) )
    {
      i--;
      buf[i] = '\0';
    }

  //  cout << "Lu : " << buf << endl;

  for( j=0; j<=i; j++ )
    {
      if( comp && buf[j] != lbl[j] ) comp = 0;
      lbl[j] = buf[j];
    }

  return(comp);
}


int ReadInt( istream & fich )
{
  if( !fich ) return( 0 );
  fich.unsetf(ios::skipws);

  char	buf[20] = "";
  int	i=1, num;

  do
    {
      fich >> buf[0];
    } while( buf[0]<'0' || buf[0]>'9' || buf[0]=='-' );

  do
    {
      fich >> buf[i];
    } while( buf[i]>='0' && buf[i]<='9' );

  buf[i] = '\0';

  sscanf(buf,"%d",&num);

  return(num);
}



//	Classe cchar

cchar::cchar( int len )
{
  if( len > 0 )
    {
      _ch = new char[ len ];
      _ch[0] = 0;
    }
  else _ch = NULL;

  _len = ( len > 0 ) ? len : 0;
}


cchar::cchar( char *ch )
{
  if( ch )
    {
      _len = strlen( ch );
      _ch = new char[ _len+1 ];
      strcpy( _ch, ch );
    }
  else
    {
      _len = 0;
      _ch = NULL;
    }
}


cchar::cchar( char *ch, int len )
{
  if( ch ) _len = ( len > 1 ) ? len : 1;
  else _len = 0;
  if( _len > 0 )
    {
      _ch = new char[ _len ];
      strcpy( _ch, ch );
    }
  else _ch = NULL;
}


cchar::cchar( const cchar & ch )
{
  if( ch._ch )
    {
      _len = strlen( ch._ch )+1;
      _ch = new char[ _len ];
      strcpy( _ch, ch._ch );
    }
  else
    {
      _len = 0;
      _ch = NULL;
    }
}


cchar::~cchar()
{
  if( _ch ) delete[] _ch;
}


cchar & cchar::operator = ( const cchar & ch )
{
  if( &ch == this ) return( *this );
  if( _ch ) delete[] _ch;
  if( ch._ch )
    {
      _ch = new char[ ch._len ];
      _len = ch._len;
      strcpy( _ch, ch._ch );
    }
  else
    {
      _ch = NULL;
      _len = 0;
    }
  return( *this );
}



//	Operateurs externes

ostream & operator << ( ostream & str, const cchar & ch )
{
  if( ch._ch ) str << ch._ch;
  return( str );
}


istream & operator >> ( istream & str, cchar & ch )
{
  if( ch._ch ) str >> ch._ch;
  return( str );
}




