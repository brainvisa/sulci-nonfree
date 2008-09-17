#ifndef NEUR_MLP_PAT_H
#define NEUR_MLP_PAT_H

#include <string.h>


/*	BASE D'EXEMPLES		*/

/**	Base d'exemples : classe pat. \\ \\
	template<class T> class pat; \\ \\
	Cette classe stocke en m�moire et manipule les exemples des bases
	d'apprentissage et de test d'un r�seau. Ses caract�ristiques
	sont: son nom (nm), son nombre d'exemples (ou patterns)
	(np), les nombres d'entr�es (ni) et de sorties (no)
	de ses exemples, et les valeurs de ces entr�es (vi) et sorties
	(vo). */
template<class T> class pat
{
  public:

  /**@name	Constructeurs - Destructeur */
  //@{
  /**	Cr�e une base. \\
	@param	nom	Nom de la base
	@param	np	Nombre d'exemples
	@param	ni	Nombre d'entr�es des exemples
	@param	no	Nombre de sorties des exemples
  */
  pat(const char *nom, int np, int ni, int no)	{ init(nom,np,ni,no); }
  /**	Charge une base. \\
	Lit une base � partir d'un fichier au format SNNS. */
  pat(const char *nom)				{ load(nom); }
  ///	Constructeur de copie
  pat( pat<T> & pa );

  ///	Destructeur
  ~pat();
  //@}

  /**@name	Op�rateurs */
  //@{
  ///	Copie
  pat<T> & operator = ( const pat<T> & pa );
  ///	Concat�nation de 2 bases
  pat<T> & operator += ( const pat<T> & pa );
  ///	Concat�nation de 2 bases
  inline pat<T> operator + ( const pat<T> & pa );
  //@}

  /**@name	Fonctions */
  //@{
  /**	Initialise les param�tres de la base. \\
	Les initialisations pr�c�dentes ne sont pas d�sallou�es ! */
  void init( const char *nom, int np, int ni, int no);

  ///	Charge une base d'exemples
  int	load( const char *nom );
  ///	Sauve une base d'exemples
  int	save( const char *nom ) const;
  ///	Affiche les exemples de la base
  void	aff() const;
  ///	Ajout d'un exemple
  void	add( T *vi, T *vo );
  //@}

  /**@name	Modification des param�tres */
  //@{
  ///	Fixe le nom
  void set_nm(const char *nm)	{ strcpy(_nm,nm); }
  /**	Change le nombe d'exemples. \\
	En cas de diminution du nombre d'exemples, les derniers sont 
	effac�s. */
  void set_np(int np);
    //    void set_ni(int ni)		{ _ni=ni; }
    //    void set_no(int no)		{ _no=no; }
    //  void set_vi(T **vi)		{ _vi=vi; }
    //  void set_vo(T **vo)		{ _vo=vo; }
  //@}

  /**@name	Acc�s aux champs */
  //@{
  ///	Nom de la base
  const char *nm() const	{ return _nm; }
  ///	Nombre d'exemples ("patterns")
  int	np() const	{ return _np; }
  ///	Nombre d'entr�es
  int	ni() const	{ return _ni; }
  ///	Nombre de sorties
  int	no() const	{ return _no; }
  ///	Tableau des entr�es
  T	**vi() const	{ return _vi; }
  ///	Tableau des sorties
  T	**vo() const	{ return _vo; }
  //@}


  protected:

  ///	Nom de la base
  char		_nm[20];
  ///	Nombre d'exemples (patterns)
  int		_np;
  ///	Nombre d'entr�es  (inputs)
  int		_ni;
  ///	Nombre de sorties (output)
  int		_no;
  ///	Tableau des valeurs d'entr�e
  T		**_vi;
  ///	Tableau des valeurs de sortie
  T		**_vo;
};



#endif
