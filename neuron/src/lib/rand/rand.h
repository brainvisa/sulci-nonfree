/***********************************************
 * File : rand.hh
 * Prog : Fonctions de tirage au sort
 ***********************************************/


///	Loi uniforme: retourne un nombre entre 0 et 1
double ran1();
///	Loi Gaussienne
double GaussRand( double mean = 0., double stdev = 1. );
/**	Loi quelconque. \\
	OwnRand tire un nombre au hasard selon la loi donnée par la fonction 
	P( x ). On passe à OwnRand un pointeur sur la fonction, et des 
	paramètres de recherche le long de la courbe: valeur de départ, 
	minimum et maximum. La méthode est la suivante: on tire un nombre 
	entre 0 et 1 par la loi uniforme (ran1()) et on recherche l'abscisse 
	du point de P ayant cette ordonnée. On fait cette recherche en suivant 
	le gradient de la fonction P (qui doit être croissante), en commençant 
	en st, et en s'interdisant de sortir des bornes [mi, ma]. Cette 
	fonction n'est pas rapide puisqu'elle doit effectuer une recherche de 
	zéro de fonction. */
double OwnRand( double P( double x ), double st=0., 
		double mi=-1.e5, double ma=1.e5 );
///	Tirage au sort de l'ordre d'une liste
void randOrder( int *list, int n );

/**	Fonction d'énergie courante normalisée. \\
	Formule : \\
	E( x ) = x^4 / ( 1 + 15/16 * x^4 ) \\
	où on prend en général comme variable ( x - m ) / s \\
	m : moyenne \\
	s : écart-type */
float	EnergyFunc( float x );
/**	Fonction d'énergie courante. \\
	Même fonction que EnergyFunc( float x ) mais on précise ici la moyenne 
	et l'écart-type de la variable. */
inline float	EnergyFunc( float x, float m, float s )
{ return( EnergyFunc((x-m)/s) ); }
///	Calcule l'argument d'un complexe
float angle( float x, float y );

///	Fonction carre (math)
template<class T> T sqr( T t ) { return( t * t ); }

