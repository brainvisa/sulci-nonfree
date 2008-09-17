/***********************************************
 * File : rand.hh
 * Prog : Fonctions de tirage au sort
 ***********************************************/


///	Loi uniforme: retourne un nombre entre 0 et 1
double ran1();
///	Loi Gaussienne
double GaussRand( double mean = 0., double stdev = 1. );
/**	Loi quelconque. \\
	OwnRand tire un nombre au hasard selon la loi donn�e par la fonction 
	P( x ). On passe � OwnRand un pointeur sur la fonction, et des 
	param�tres de recherche le long de la courbe: valeur de d�part, 
	minimum et maximum. La m�thode est la suivante: on tire un nombre 
	entre 0 et 1 par la loi uniforme (ran1()) et on recherche l'abscisse 
	du point de P ayant cette ordonn�e. On fait cette recherche en suivant 
	le gradient de la fonction P (qui doit �tre croissante), en commen�ant 
	en st, et en s'interdisant de sortir des bornes [mi, ma]. Cette 
	fonction n'est pas rapide puisqu'elle doit effectuer une recherche de 
	z�ro de fonction. */
double OwnRand( double P( double x ), double st=0., 
		double mi=-1.e5, double ma=1.e5 );
///	Tirage au sort de l'ordre d'une liste
void randOrder( int *list, int n );

/**	Fonction d'�nergie courante normalis�e. \\
	Formule : \\
	E( x ) = x^4 / ( 1 + 15/16 * x^4 ) \\
	o� on prend en g�n�ral comme variable ( x - m ) / s \\
	m : moyenne \\
	s : �cart-type */
float	EnergyFunc( float x );
/**	Fonction d'�nergie courante. \\
	M�me fonction que EnergyFunc( float x ) mais on pr�cise ici la moyenne 
	et l'�cart-type de la variable. */
inline float	EnergyFunc( float x, float m, float s )
{ return( EnergyFunc((x-m)/s) ); }
///	Calcule l'argument d'un complexe
float angle( float x, float y );

///	Fonction carre (math)
template<class T> T sqr( T t ) { return( t * t ); }

