/*********************************
 * File : readstream.hh
 * Prog : fonctions pour lire des fichiers
 ***********************************************/


#ifndef _READSTREAM_H_
#define _READSTREAM_H_

/**	Lit une ligne de fichier texte, 
	en sautant les lignes vides 
	et les lignes de commentaires commen�ant par '\#' */
char *ReadLine( std::istream & fich );

/**	Lit un label termin� par ' :', 
	le compare � lbl et remplace 
	lbl par ce qu'il a lu. Le retour est le r�sultat de 
	la comparaison (0 si diff�rent). */
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

/**@name	Op�rateurs externes */
//@{
///
std::ostream & operator << ( std::ostream & str, const cchar & ch );
///
std::istream & operator >> ( std::istream & str, cchar & ch );
//@}


#endif

