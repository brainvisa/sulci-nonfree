

#ifndef	_KOHONEN_HH_
#define _KOHONEN_HH_


#include <vector>
#include <iostream>


class Kohonen;


/**@name	Op�rateurs externes */
//@{
///
std::ostream & operator << ( std::ostream & fich, const Kohonen & koh );
///
std::istream & operator >> ( std::istream & fich, Kohonen & koh );
//@}



///	type Vecteur-code
class VectCode
{
  public:
  ///	Constructeur
  VectCode() { cla = 0; }
  ///	Coordonn�es dans la carte
  std::vector<double>	coord;
  ///	Vecteur lui-m�me
  std::vector<double> vect;
  ///	Classe repr�sent�e par ce vecteur
  int		cla;
};


/**	Classe Kohonen. \\ \\
	Cartes topologiques de kohonen */
class Kohonen
{
  /**@name	Friends */
  //@{
  ///
  friend std::ostream & operator << ( std::ostream &, const Kohonen & );
  ///
  friend std::istream & operator >> ( std::istream &, Kohonen & );
  //@}

  public:
  /**@name	Donn�es publiques */
  //@{
  /**	Donn�es utilisateur. \\
	Cette zone de donn�es peut �tre utilis�e par exemple pour conserver 
	des param�tres statistiques sur les entr�es. */
  std::vector<double>	UserData;
  //@}

  /**@name	Constructeurs - Destructeur */
  //@{
  ///
  Kohonen( int ndim=1, int ncdim=2 );
  ///
  Kohonen( const Kohonen & koh );
  ///
  virtual ~Kohonen();
  //@}

  /**@name	Op�rateurs */
  //@{
  ///
  Kohonen & operator = ( const Kohonen & koh );
  //@}

  /**@name	Acc�s aux membres */
  //@{
  ///	Indice du vecteur-code �lu
  int WinIndex() const { return( _winIndex ); }
  ///	Carr� de la distance au vecteur-code gagnant
  double	SqDistWinner() const { return( _sqDistWinner ); }
  ///	Nombre de dimensions de la carte (1 ou 2 normalement)
  int NDim() const { return( _nDim ); }
  ///	Nombre de dimensions des vecteurs-code
  int NCDim() const { return( _nCDim ); }
  ///	Nombre de vecteurs-code
  int	NVCode() const { return( _vCode.size() ); }
  ///	Vecteur code num�ro i
  VectCode* VCode( unsigned i ) const { return( _vCode[i] ); }
  ///	Vecteur-code �lu
  VectCode VCWinner() const { return(	*_vCWinner ); }
  ///	Rayon d'apprentisasge
  double	LearnRay() const { return( _learnRay ); }
  ///
  void SetLearnRay( double lr ) { _learnRay = lr; }
  ///	Facteur d'apprentissage
  double	LearnFactor() const { return( _learnFactor ); }
  ///
  void SetLearnFactor( double lf ) { _learnFactor = lf; }
  ///	Rayon de recherche pour un comptage de densit�
  double	DensityRay() const { return( _densityRay ); }
  ///
  void SetDensityRay( double dr ) { _densityRay = dr; }
  ///
  void ChangeDims( unsigned ndim, unsigned ncdim );
  //@}

  /**@name	Manipulation des vecteurs-code */
  //@{
  ///	Recopie tous les vecteurs-code � partir de vcl
  void CopyVC( const std::vector<VectCode*> &vcl );
  ///
  int AddVectCode( VectCode *vc );
  ///
  int RemoveVectCode( const VectCode *vc );
  ///
  int DelVectCode( VectCode *vc );
  ///
  void Empty();
  /**	Cr�e des vecteurs-code uniform�ment r�partis. \\
	nvd est le nombre de VC par dimension de la carte. Les VC ont 
	des positions enti�res positives. \\
	cmin et cmax sont les bornes de tirage des composantes des VC.*/
  void CreateUniformVC( unsigned nvd, double cmin=-1., double cmax=1. );
  ///	Ajoute uniform�ment des VC sur les composantes � partir de la c-�me
  void CreateUniformVCFromComp( unsigned c, double *comp, unsigned nvd, 
				double cmin, double cmax );
  ///
  void RandVC( double cmin=-1., double cmax=1. );
  //@}

  /**@name	Fonctions statiques */
  //@{
  ///
  static void PrintVC( const VectCode *vc );
  ///
  static void PrintVect( const std::vector<double> *vect );
  ///
  static double	SqVectDist( const std::vector<double> *v1, 
			    const std::vector<double> *v2 );
  //@}

  /**@name	Entr�es / Sorties */
  //@{
  ///
  int Save( const char *nom ) const;
  ///
  int Load( const char *nom );
  ///
  int ByteSize() const;
  //@}

  /**@name	Utilisation / Apprentissage */
  //@{
  ///
  VectCode *FindWinner( const std::vector<double> *vec );
  //VectCode *FindWinner( void *toto );
  ///
  int Learn( const std::vector<double> *vect );
  ///
  void PrintMap() const;
  ///
  double MapDist( int i1, int i2 ) const;
  ///
  double MapDist( const VectCode *vc1, const VectCode *vc2 ) const;
  ///
  double DistFact( double d ) const;
  ///
  int Density( const std::vector<double> *vect ) const;
  //@}


  protected:
  ///	Nombre de dimensions de la carte (1 ou 2 normalement)
  unsigned		_nDim;
  ///	Nombre de dimensions des vecteurs-code
  unsigned		_nCDim;
  ///	Vecteurs-code
  std::vector<VectCode*>	_vCode;
  ///	Vecteur-code �lu
  VectCode		*_vCWinner;
  ///	Index du vecteur-code �lu
  unsigned		_winIndex;
  ///	Distance au vecteur-code �lu
  double		_sqDistWinner;
  ///	Rayon d'apprentisasge
  double		_learnRay;
  ///	Facteur d'apprentissage
  double		_learnFactor;
  ///	Rayon de recherche pour un comptage de densit�
  double		_densityRay;


  private:

};


#endif
