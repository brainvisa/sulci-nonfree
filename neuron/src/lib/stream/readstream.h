/*********************************
 * File : readstream.hh
 * Prog : fonctions pour lire des fichiers
 ***********************************************/


#ifndef _READSTREAM_H_
#define _READSTREAM_H_

/**	Lit une ligne de fichier texte, 
	en sautant les lignes vides 
	et les lignes de commentaires commençant par '\#' */
char *ReadLine( std::istream & fich );

/**	Lit un label terminé par ' :', 
	le compare à lbl et remplace 
	lbl par ce qu'il a lu. Le retour est le résultat de 
	la comparaison (0 si différent). */
int ReadLabel( std::istream & fich, char * lbl );
///
int ReadInt( std::istream & fich );

/**	Classe chaine de caracteres. \\ \\
	\#include <readstream.hh> \\
	class cchar; 
*/
class cchar
{
  public:
  ///
  cchar( int len );
  ///
  cchar( char *ch );
  ///
  cchar( char *ch, int len );
  ///
  cchar( const cchar & ch );
  ///
  virtual ~cchar();

  ///
  inline virtual cchar & operator = ( const cchar & ch );

  //protected:
  ///
  char	*_ch;
  ///
  int	_len;
};

/**@name	Opérateurs externes */
//@{
///
std::ostream & operator << ( std::ostream & str, const cchar & ch );
///
std::istream & operator >> ( std::istream & str, cchar & ch );
//@}


#endif

