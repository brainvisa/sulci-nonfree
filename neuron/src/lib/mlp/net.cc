
#include <neur/mlp/net.h>
#include <neur/mlp/misc.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <iomanip>
#include <time.h>
#include <string>

using namespace std;

/*	RESEAU DE NEURONES	*/

template<class T,class U> 
void net<T,U>::init( const char *nom, int nu, int nl, 
		     int nac, int ni, int no, int fo,
		     int *tar, int *nli, int **ori)
{
    int		i;

    strcpy(_nm,nom);
    _nu=nu;
    _nl=nl;
    _nac=nac;
    _ni=ni;
    _no=no;
    _fo=fo;

    if(_nu>0) _u = new unit<T>[_nu];
    else _u = (unit<T>*) NULL;
    if(_nac>0) _l = new neur_link<U>[_nac];
    else _l = (neur_link<U>*) NULL;

    for(i=ni; i<fo; i++) _u[i].set_t(CACHE);
    for(i=fo; i<nu; i++) _u[i].set_t(SORTIE);

    for(i=0; i<nac; i++) _l[i].init(tar[i],nli[i],ori[i]);
}


template<class T, class U> net<T,U>::net()
{
  _nu = 0;
  _nl = 0;
  _nac = 0;
  _ni = 0;
  _no = 0;
  _fo = 0;
  _nm[0] = '\0';
  _u = (unit<T>*) NULL;
  _l = (neur_link<U>*) NULL;
}


template<class T, class U> int net<T,U>::add_le(int tar,int org, U w)
{
  int i=get_link(tar);
  if(i>=0) { _l[i].add(org,w); _nl++; return(0); }
  else return(1);
}


template<class T,class U> 
void net<T,U>::pmc( const char *nom, int nc, int *couch)
{
    int	i,j,c,ind,indo;
    char	com[100];

    _nl=0;
    _nac=0;
    for(i=1; i<nc; i++)
	{
	    _nac+=couch[i];
	    _nl+=couch[i]*couch[i-1];
	}
    _ni=couch[0];
    _no=couch[nc-1];
    _nu=_nac+_ni;
    _fo=_nu-_no;

    _u= new unit<T>[_nu];
    _l= new neur_link<U>[_nac];
    strcpy( _nm, nom );

    for(i=0; i<_ni; i++)
	{
	    sprintf(com,"Unite %d",i);
	    _u[i].set(com,0,0,i,0,0,0);
	}
    ind=_ni;
    indo=0;
    for(c=1; c<nc; c++)
	{
	    for(i=0; i<couch[c]; i++)
		{
		    sprintf(com,"Unite %d",ind);
		    if(c<nc-1)
			_u[ind].set(com,1,c,i,0,0,0);
		    else _u[ind].set(com,2,c,i,0,0,0);

		    _l[ind-_ni].init(ind,couch[c-1]);
		    for(j=0; j<couch[c-1]; j++)
			_l[ind-_ni].le()[j].set(indo+j,0);

		    ind++;
		}
	    indo+=couch[c-1];
	}
}


template<class T,class U> net<T,U>::net( const char *nom)
{
  _nu = 0;
  _nac = 0;
  _nm[0] ='\0';
  _ni = _no = _fo = _nl = 0;
  _u = (unit<T>*) NULL;
  _l = (neur_link<U>*) NULL;
  load(nom);
}


template<class T,class U> net<T,U>::net(const net<T,U> & n)
{
    int	i;

    strcpy(_nm,n.nm());
    _nu=n._nu;	// calling the func gets an internal compiler error
                // on mac/gcc-2.95.2
    _nl=n.nl();
    _nac=n.nac();
    _ni=n.ni();
    _no=n.no();
    _fo=n.fo();
    _u= new unit<T>[_nu];
    _l= new neur_link<U>[_nac];

    for(i=0; i<_nu; i++) _u[i]=n.u()[i];
    for(i=0; i<_nac; i++) _l[i]=n.l()[i];
}


template<class T,class U> net<T,U>::~net()
{
  if( _l )
    {
      delete[] _l;
      _l = (neur_link<U>*)NULL;
    }
  if( _u ) delete[] _u;
}



//	Opérateurs


template<class T,class U> 
net<T,U> & net<T,U>::operator = ( const net<T,U> & n )
{
  if( &n == this ) return( *this );

  int	i;

  if(_nu!=n.nu())
    {
      if(_nu>0) delete[] _u;
      _u= new unit<T>[n.nu()];
      _nu=n.nu();
    }
  if(_nac!=n.nac())
    {
      if(_nac>0) delete[] _l;
      _l= new neur_link<U>[n.nac()];
      _nac=n.nac();
    }

  for(i=0; i<_nu; i++) _u[i]=n.u()[i];
  for(i=0; i<_nac; i++) _l[i]=n.l()[i];
  _nl=n.nl();
  _ni=n.ni();
  _no=n.no();
  _fo=n.fo();
  strcpy(_nm,n.nm());

  return( *this );
}


template<class T, class U> net<T,U> & 
net<T,U>::operator += ( const unit<T> & u )
{
  add_neur( u.nm(), u.t(), u.x(), u.y(), u.z(), u.bi(), u.o() );
  return( *this );
}


template<class T, class U> net<T,U> 
net<T,U>::operator + ( const unit<T> & u )
{
  net<T,U>	n( *this );
  n += u;
  return( n );
}


template<class T, class U> net<T,U> & 
net<T,U>::operator -= ( const unit<T> & u )
{
  del_neur( get_neur( u ) );
  return( *this );
}


template<class T, class U> net<T,U> 
net<T,U>::operator - ( const unit<T> & u )
{
  net<T,U>	n( *this );
  n -= u;
  return( n );
}


template<class T, class U> net<T,U> & 
net<T,U>::operator += ( const neur_link<U> & l )
{
  add_link( l.ta(), l.nl() );
  neur_link<U>	*lk = &_l[_nac-1];

  int	i;

  for( i=0; i<l.nl(); i++ )
    lk->le()[i] = l.le()[i];

  return( *this );
}


template<class T, class U> net<T,U> 
net<T,U>::operator + ( const neur_link<U> & l )
{
  net<T,U>	n( *this );
  n += l;
  return( n );
}


//	Fonctions


template<class T, class U> void net<T,U>::empty()
{
  if( _u ) delete[] _u;
  if( _l ) delete[] _l;
  _nu = 0;
  _nl = 0;
  _nac = 0;
  _ni = _no = _fo = 0;
  _u = (unit<T>*) NULL;
  _l = (neur_link<U>*)NULL;
  _nm[0] = '\0';
}


template<class T, class U> int net<T,U>::get_neur( const unit<T> & u )
{
  int	i;

  for( i=0; i<_nu && _u[i]!=u; i++ ) {}
  if( i == _nu ) return( -1 );
  return( i );
}


template<class T, class U> int net<T,U>::get_link(int tar)
{
    int	i;

    for(i=0; i<_nac; i++) if(_l[i].ta()==tar) break;
    if(i==_nac) return(-1);
    else return(i);
}


template<class T,class U> void net<T,U>::aff() const
{
    int		i;

    cout << "\nRESEAU : " << _nm << "\n\n";
    cout << "Nb de neurones   : " << _nu << "\n";
    cout << "Nb de connexions : " << _nl << "\n";
    cout << "Nb d'arrivees    : " << _nac << "\n";
    cout << "Nb d'entrees     : " << _ni << "\n";
    cout << "Nb de sorties    : " << _no << "\n";
    cout << "No 1ere sortie   : " << _fo << "\n\nNEURONES:\n\n";

    for(i=0; i<_nu; i++)
	{
	    cout << "Neurone " << i << ":\n";
	    _u[i].aff();
	}
    cout << "\nLIAISONS:\n\n";

    for(i=0; i<_nac; i++)
	{
	    cout << "Liaison " << i << ":\n";
	    _l[i].aff();
	}
}


template<class T,class U> void net<T,U>::add_neur( const char *nom, int typ,
				       int xx, int yy, int zz, T bia, T sor)
{
    unit<T>	*u;

    u=_u;
    _u= new unit<T>[_nu+1];

    for(int i=0; i<_nu; i++) _u[i]=u[i];

    _u[_nu].set(nom,typ,xx,yy,zz,bia,sor);

    _nu++;
    if( typ == ENTREE ) _ni++;
    if( typ == SORTIE ) _no++;
    if(_nu>1) delete[] u;
}


template<class T,class U> int net<T,U>::del_neur(int no)
{
  unit<T>	*u;
  int		i, j, org, lien_a_blaster = 0, lt_a_blaster;

  if( no>=0 && no<_nu)
    {
      u=_u;
      if(_nu>1) _u= new unit<T>[_nu-1];

      for(i=0; i<no; i++) _u[i]=u[i];
      for(i=no+1; i<_nu; i++) _u[i-1]=u[i];

      if( u[no].t() == ENTREE ) _ni--;
      if( u[no].t() == SORTIE ) _no--;
      _nu--;
      delete[] u;

      lt_a_blaster=-1;

      for(i=0; i<_nac; i++)
	if(_l[i].ta()==no) lt_a_blaster=i;
	else
	  {
	    if( _l[i].ta() > no ) _l[i].set_ta( _l[i].ta()-1 );
	    for(j=0; j<_l[i].nl(); j++)
	      {
		lien_a_blaster=-1;
		org=_l[i].le()[j].org();
		if(org>no)
		  _l[i].le()[j].set_or(org-1);
		else if(org==no) lien_a_blaster=j;
	      }
	    if(lien_a_blaster>=0)
	      {
		_nl--;
		_l[i].del(no);
	      }
	  }
      if(lt_a_blaster>=0) del_link(no);
      for(i=0; i<_nac; i++) if(_l[i].nl()==0)
	{
	  del_link(_l[i].ta());
	  i--;
	}
      return(0);
    }
  else
    return(1);
}


template<class T,class U> int net<T,U>::add_link(int tar,int nli)
{
    int		i,j,ta,cnt=0;
    neur_link<U>	*l;

    l=_l;
    _l= new neur_link<U>[_nac+1];

    j=-1;
    for(i=0; i<_nac; i++)
	{
	    if((ta=l[i].ta())<tar)
		_l[i]=l[i];
	    else if(ta==tar)
		{
		    cout << "Le lien vers " << tar << " existe deja.\n";
		    cnt=l[i].nl();
		    delete[] _l;
		    _l=l;
		    //l= new neur_link<U>[1];
		    _l[i] = neur_link<U>(tar,nli);
		    //_l[i]=l[0];
		    //delete[] l;
		    _nl+=nli-cnt;
		    return(1);
		}
	    else
		{
		    if(j<0) j=i;
		    _l[i+1]=l[i];
		}
	    cnt+=l[i].nl();
	}

    if(j<0) j=_nac;

    _l[j].init(tar,nli);
    _nl=cnt+nli;

    _nac++;
    if(_nac>1) delete[] l;
    return(0);
}


template<class T,class U> int net<T,U>::del_link(int no)
{
    int		i;
    neur_link<U>	*l;

    if(_nac>0)
	{
	    l=_l;
	    if(_nac>1) _l= new neur_link<U>[_nac-1];
	    for(i=0; (i<_nac-1) && (l[i].ta()<no); i++)
		_l[i]=l[i];
	    if((i==_nac-1) && (l[i].ta()!=no))
		{
		    cout << "Pas de lien vers " << no << ".\n";
		    if(_nac>1) delete[] _l;
		    _l=l;
		    return(1);
		}
	    _nl-=l[i].nl();
	    for(i++; i<_nac; i++)
		_l[i-1]=l[i];

	    delete[] l;
	    _nac--;
	    return(0);
	}
    else
	{
	    cout << "Il n'y a aucun lien a enlever.\n";
	    return(1);
	}
}


template<class T,class U> int net<T,U>::del_le(int tar, int org)
{
    int i=get_link(tar);
    if(i>=0)
	{
	    if(_l[i].del(org)) return(1);
	    _nl--;
	    if(nlink(tar)==0) del_link(tar);
	    return(0);
	}
    else return(1);
}


template <class T, class U> int net<T,U>::load( const char *nom )
{
  cout << "Chargement du reseau " << nom << "\n";

  ifstream fich(nom,ios::in);
  if(!fich)
    {
      cout << "Fichier " << nom << " introuvable.\n";
      return(1);
    }

  int r = load( fich );

  if( fich ) fich.close();

  return( r );
}


#include <sstream>
template <class T, class U> int net<T,U>::load( istream & fich )
{
  // cout << "loading net from stream, begin\n";
  int		i,j,k,bid,typ,org,nu,nl,xx,yy,zz;
  T		bi;
  char	nm[50],chaine[200],tt[20];
  char	t;
  U		w;

  if(!fich)
    {
      cout << "Stream mal ouvert : lecture impossible.\n";
      return(1);
    }

  fich.unsetf(ios::skipws);

  if(cherche(&fich,"network name : ")) return(1) ;
  fich.get(nm,12,'\n');
  //    cout << "nom : " << nm << "\n\n";
  nm[11]='\0';

  if(cherche(&fich,"no. of units :")) return(1);
  fich.setf( ios::skipws );
  fich >> nu;
  // cout << "nu: " << nu << endl;

  fich.unsetf( ios::skipws );
  if(cherche(&fich,"no. of connections :")) return(1);
  fich.setf( ios::skipws );
  fich  >> nl;
  // cout << "nl: " << nl << endl;

  fich.unsetf( ios::skipws );
  if(cherche(&fich,"unit definition section :")) return(1);
  if(cherche(&fich,"sites")) return(1);
  if(cherche(&fich,"\n")) return(1);
  if(cherche(&fich,"\n")) return(1);

  init( nm, nu, nl, 0, 0, 0, 0, (int*) NULL, (int*) NULL, (int**) NULL);

  for(i=0; i<_nu; i++)
    {
      fich.setf( ios::skipws );
      fich >> bid;
      //	cout << "N " << bid << " : ";
      if(bid!=i+1) return(1);
      fich.unsetf( ios::skipws );
      if(cherche(&fich,"|")) return(1);
      if(cherche(&fich,"|")) return(1);
      fich.get(tt,10,'\n');
      tt[9]='\0';
      //	cout << ">" << chaine << "< : >" << tt << "<\n";

      if(cherche(&fich,"|")) return(1);
      if(cherche(&fich,"|")) return(1);
      fich.setf( ios::skipws );
      fich >> bi;
      //	cout << "B=" << bi << " , ";
	
      fich.unsetf( ios::skipws );
      if(cherche(&fich,"| ")) return(1);
      fich >> t;
      if(t=='i')
	{
	  typ=0;
	  _ni++;
	}
      else if(t=='h') typ=1;
      else if(t=='o')
	{
	  typ=2;
	  _no++;
	  if(_fo==0) _fo=i;
	}
      else return(1);
      //	cout << "T=" << typ << " , ";

      if(cherche(&fich,"|")) return(1);
      fich.setf( ios::skipws );
      fich >> xx >> t >> yy >> t >> zz;
      //	cout << "C=" << xx << "," << yy << "," << zz << " , ";
      fich.unsetf( ios::skipws );
      if(cherche(&fich,"\n")) return(1);

      _u[i].set(tt,typ,xx,yy,zz,bi,(T) 0);
      //	cout << "\n";
    }

  //cout << "Unites lues.\n";

  if(cherche(&fich,"connection definition section :")) return(1);
  if(cherche(&fich,"weight")) return(1);
  cherche(&fich,"\n");
  cherche(&fich,"\n");

  k=0;
  while(k<nl)
    {
      fich.setf( ios::skipws );
      fich >> bid;	// no unite cible
      fich.unsetf( ios::skipws );
      if(cherche(&fich,"|")) return(1);
      if(cherche(&fich,"|")) return(1);
      bid--;
      //	cout << "L -> " << bid << "\n";
      if(net<T,U>::add_link(bid,0)) return(1);

      j=0;
      fich.setf( ios::skipws );
      do
	{
          fich.setf( ios::skipws );
	  fich >> org;
          fich.unsetf( ios::skipws );
	  if(cherche(&fich,":")) return(1);
          fich.setf( ios::skipws );
	  fich >> w;
	  //cout << "origin: " << org << ", w: " << w << endl;
	  if(net<T,U>::add_le(bid,org-1,w)) return(1);
	  j++;
          fich.unsetf( ios::skipws );
	  fich >> t;	// separateur
	} while(t==',');

      k+=j;
    }

  //cout << "Liens lus.\n";
  fich >> chaine;		// vider la derniere ligne de ----

  // cout << "load mlp from stream, end\n";
  return(0);
}


template<class T,class U> int net<T,U>::save( const char *nom ) const 
{
  ofstream	fich(nom,ios::out);
  if(!fich)
    {
      cout << "Ouverture de " << nom << " impossible.\n";
      return(1);
    }

  int r = save( fich );

  if( fich ) fich.close();

  return( r );
}


template<class T,class U> int net<T,U>::save( ostream & fich ) const
{
    int		i,j;
    if(!fich)
      {
	cout << "Stream non ouvert : ecriture impossible.\n";
	return(1);
      }

    fich << "SNNS network definition file V1.4-3D\n";
    fich << "generated at Thu May 09 10:11:49 1996\n\n";
    fich << "network name : " << _nm << "\n";
    fich << "source files :\n";
    fich << "no. of units : " << _nu << "\n";
    fich << "no. of connections : " << _nl << "\n";
    fich << "no. of unit types : 0\nno. of site types : 0\n\n\n";
    fich << "learning function : Std_Backpropagation\n";
    fich << "update function : Topological_Order\n\n\n";
    fich << "unit default section :\n\n";
    fich <<
     "act      | bias     | st | subnet | layer | act func     | out func\n";
    fich <<
  "---------|----------|----|--------|-------|--------------|-------------\n";
    fich <<
  " 0.00000 |  0.00000 | h  |      0 |     1 | Act_Logistic | Out_Identity\n";
    fich <<
  "---------|----------|----|--------|-------|--------------|-------------\n";
    fich << "\n\nunit definition section :\n\n";
    fich <<
  "no. | typeName | unitName | act      | bias     | st | position | act func | out func | sites\n";
    fich <<
  "----|----------|----------|----------|----------|----|----------|----------|----------|-------\n";

// Ecriture des neurones

    for(i=0; i<_nu; i++)
	{
	    fich << setw(3) << i+1 << " |          | " << setw(8)
		 << _u[i].nm() << " | " << setw(8) << setprecision(5)
		 << _u[i].o()  << " | " << setw(8) << setprecision(5)
		 << _u[i].bi() << " | ";
	    switch(_u[i].t())
		{
		case 0:
		    fich << "i  | ";
		    break;
		case 2:
		    fich << "o  | ";
		    break;
		default:
		    fich << "h  | ";
		}
	    fich << setw(2) << _u[i].x() << "," << setw(2) << _u[i].y() << ","
		 << setw(2) << _u[i].z() << " |||\n";
	}

    fich <<
"----|----------|----------|----------|----------|----|----------|----------|----------|-------\n\n\n";

// Ecriture des connexions

    fich << "connection definition section :\n\n";
    fich << "target | site | source:weight\n";
    fich <<
"-------|------|------------------------------------------------------------------------------------------------------------\n";

    for(i=0; i<_nac; i++)
	{
	    fich << " " << setw(5) << _l[i].ta()+1 << " |      | ";
	    for(j=0; j<_l[i].nl(); j++)
		{
		    if(j>0 && (j & 7)==0)
			fich << "\n                ";
		    fich << setw(3) << _l[i].le()[j].org()+1 << ":" << setw(8)
			 << setprecision(5) << _l[i].le()[j].w();
		    if(j<_l[i].nl()-1) fich << ", ";
		}
	    fich << "\n";
	}
    fich <<
"-------|------|----------------------------------------------------------------------------------------------------------------\n";

    return(0);
}


template<class T,class U> int net<T,U>::prop(const pat<T> & pa, int n)
{
  int	i, j;

  if(pa.ni()!=_ni || n>=pa.np()) return(1);

  // Initialisation des entrees
  j=0;
  for(i=0; i<_nu; i++)
    {
      if(_u[i].t()==0)
	{
	  _u[i].set_o(pa.vi()[n][j]);
	  // naj[i]=1;
	  j++;
	}
      // else naj[i]=0;
      // nen[i]=0;
    }
  return( prop() );
}


template<class T,class U> int net<T,U>::prop()
{
    int		i,j,nnaj,nnaj_old,noaj,ta;
    T		s;
    int		naj[_nu],nen[_nu];

// Initialisation des entrees

    j=0;
//    cout << "Entrees : ";
    for(i=0; i<_nu; i++)
	{
	    if(_u[i].t()==0)
		{
		  // _u[i].set_o(pa.vi()[n][j]);
		    naj[i]=1;
//		    cout << i << " ";
		    j++;
		}
	    else naj[i]=0;
	    nen[i]=0;
	}
//    cout << ".\n";

    nnaj=0;
    noaj=0;
    nnaj_old=-1;

// Calcul de l'etat des neurones

    while(noaj<_no && nnaj!=nnaj_old)
	{
//	    cout << "NNAJ: " << nnaj << "\n";
	    nnaj_old=nnaj;
	    for(i=0; i<_nac; i++)
		{
		    ta=_l[i].ta();
//		    cout << "L-> " << ta << " : ";
		    if(!naj[ta])
			{
			    nen[ta]=0;
			    for(j=0; j<_l[i].nl(); j++)
				if(!naj[_l[i].le()[j].org()]) nen[ta]++;
//			    cout << "nen: " << nen[ta] << " ";
			    if(nen[ta]==0)
				{
//				    cout << ta << "; ";
				    s=0;
				    for(j=0; j<_l[i].nl(); j++)
					s+=_l[i].le()[j].w()
					    * _u[_l[i].le()[j].org()].o();
				    _u[ta].set_o( (T)(1/(1+exp(-s-_u[ta].bi()))
					) );
				    nnaj++;
				    if(_u[ta].t()==2) noaj++;
				    naj[ta]=1;
				}
			}
//		    cout << ".\n";
		}
//	    cout << "\n";
	}
    if(noaj<_no)
	{
	    cout <<
	      "Le reseau est boucle: je ne sais pas calculer les sorties.\n";
	    return(1);
	}

    return(0);
}


template<class T, class U> void net<T,U>::rand_w(U min, U max)
{
    int	i,j;
    U	w;
    time_t	t;

    t = time( (time_t*) NULL );
    srand(t);
    for(i=0; i<_nac; i++)
      {
	for(j=0; j<_l[i].nl(); j++)
	  {
	    w=(U) ((double)(rand() & 0x7FFF)*(max-min)/0x7FFF) + min;
	    _l[i].le()[j].set_w(w);
	  }
	_u[_l[i].ta()].set_bi( (T) ((double)(rand() & 0x7FFF)*(max-min)
				     /0x7FFF) + min );
      }
}


template<class T,class U> T net<T,U>::learn_p( int ta, T dlt, double eta )
{
  int		i;
  T		netj=0;
  lelm<T>	*le;
  double	fprim;

  le = _l[ta].le();

  netj = _u[_l[ta].ta()].o();

  // pour fonction de cout entropie, prendre fprim = 1.
  fprim = netj * ( 1. - netj );
  //fprim = 1.;

  for( i=0; i<_l[ta].nl(); i++ )
    {
      le[i].set_w( (U)( le[i].w() + eta*fprim*dlt*_u[le[i].org()].o() ) );
    }
  _u[_l[ta].ta()].set_bi( (T)( _u[_l[ta].ta()].bi() + eta*fprim*dlt ) );

  return( fprim );
}


template<class T, class U>
void net<T,U>::learn( int n, T dlt, double eta )
{
  int i; for(i=0; i<_nac; i++) if(_l[i].ta()==n) break;
  if(i<_nac) learn_p(i,dlt,eta);
}


template<class T,class U> 
int net<T,U>::backprop( const pat<U> & pa,int nc, double eta, int flg, T *err )
{
  int		i,j,pass,cel,k,ex;
  int		ord[pa.np()],cla[pa.np()+1];
  time_t	t;
  T		dlt[_nu];
  int		naj[_nac],nsa[_nu],ns[_nu];;
  int		nnaj,nnaj_old;
  T		erreur = 0;

  if(_ni!=pa.ni() || _no!=pa.no())
    {
      cout << "La base d'exemple est incompatible avec le reseau.\n";
      return(1);
    }

  if( flg ) cout << "Apprentissage du reseau " << _nm << "...\n";

  t = time( (time_t*) NULL);
  srand(t);

  //	Compter les sorties de chaque neurone

  for(i=0; i<_nac; i++)
    {
      if(_u[cel=_l[i].ta()].t()==2) ns[cel]=1;
      else ns[cel]=0;
    }
  for(i=0; i<_nac; i++)
    {
      for(j=0; j<_l[i].nl(); j++)
	ns[_l[i].le()[j].org()]++;
    }

  int car = 0;
  unsigned ncar;
  if( flg )
    {
      for( car=0, ncar=nc; ncar; car++, ncar/=10 ) {}
      if( car == 0 ) car = 1;
      cout << "Apprentissage cycle ";
    }

  for(pass=0; pass<nc; pass++)
    {
      if(flg && ((pass-1)*10/nc!=pass*10/nc || pass==0) )
	{
	  cout << setw(car) << pass << "...\b\b\b" << flush;
	  for( i=0; i<car; i++ ) cout << "\b";
	}

      //	Tirage au sort de l'ordre d'apprentissage des exemples

      for(i=0; i<pa.np(); i++) ord[i]=rand();

      //	Tri

      cla[0]=-1;
      for(i=0; i<pa.np(); i++)
	{
	  for(j=0; cla[j]!=-1; j=cla[j]+1)
	    if(ord[i]<ord[cla[j]]) break;
	  cla[i+1]=cla[j];
	  cla[j]=i;
	}

      //	Reecriture de la chaine d'indices dans l'ordre

      i=0;
      for(j=0; cla[j]!=-1; j=cla[j]+1) ord[i++]=cla[j];

      //	    cout << "Ordre d'apprentissage :\n";
      //	    for(i=0; i<pa.np(); i++) cout << ord[i] << ", ";
      //	    cout << "\n";

      //	Pour chaque exemple dans le nouvel ordre

      for(ex=0; ex<pa.np(); ex++)
	{
	  // Propager l'exemple en cours
	  if(prop(pa,ord[ex])) return(1);

	  // Initialisation des sorties

	  nnaj=0;
	  nnaj_old=0;

	  j=0;
	  //		    cout << "Sorties : ";
	  for(i=0; i<_nu; i++)
	    {
	      dlt[i]=0;
	      nsa[i]=0;
	    }

	  for(i=0; i<_nac; i++)
	    {
	      cel=_l[i].ta();
	      if(_u[cel].t()==2)
		{
		  dlt[cel] = pa.vo()[ord[ex]][j]-_u[cel].o();
		  erreur += fabs( dlt[cel] );
		  dlt[cel] *= learn_p(i,dlt[cel],eta);
		  naj[i]=1;
		  //				    cout << cel << " ";
		  for(k=0; k<_l[i].nl(); k++)
		    {
		      dlt[_l[i].le()[k].org()]+=
			dlt[cel]*_l[i].le()[k].w();
		      nsa[_l[i].le()[k].org()]++;
		    }
		  j++;
		  nnaj++;
		}
	      else naj[i]=0;
	    }
	  //		    cout << ".\n";

	  // Apprentissage cellule par cellule

	  while(nnaj<_nac && nnaj!=nnaj_old)
	    {
	      // cout << "NNAJ: " << nnaj << "\n";
	      nnaj_old=nnaj;
	      for(i=_nac-1; i>=0; i--)
		{
		  cel=_l[i].ta();
		  // cout << "L-> " << cel << " : ";
		  if(!naj[i])
		    {
		      if(nsa[cel]==ns[cel])
			{
			  learn_p(i,dlt[cel],eta);
			  naj[i]=1;
			  for(k=0; k<_l[i].nl(); k++)
			    {
			      dlt[_l[i].le()[k].org()]+=
				dlt[cel]*_l[i].le()[k].w();
			      nsa[_l[i].le()[k].org()]++;
			    }
			  // cout << "A ;";
			}
		      // else cout << "- ;";
		    }
		}
	      // cout << "\n";
	    }
	}
    }
  if( car ) cout << "\n";

  if( err ) *err = erreur;

  return(0);
}


template<class T,class U> int net<T,U>::renum(int org, int ta)
{
    int		i,j;
    unit<T>	utmp;

//     cout << "Renum : " << org << " -> " << ta << ".\n";
    if(ta==_nu && org<_nu) utmp=_u[org];
    else if(ta<_nu && org==_nu) _u[ta]=utmp;
    else if(ta<_nu && org<_nu) _u[ta]=_u[org];
    else
	{
	    cout << "Renum: mauvais nos de neurones : " << org << " vers "
		 << ta << ".\n";
	    return(1);
	}

    for(i=0; i<_nac; i++)
	{
	    if(_l[i].ta()==org) _l[i].set_ta(ta);
	    else for(j=0; j<_l[i].nl(); j++)
		if(_l[i].le()[j].org()==org) _l[i].le()[j].set_or(ta);
	}

    return(0);
}


template<class T,class U> void net<T,U>::af_out(int fl) const
{
    int	i;

    if(fl)
	{
	    for(i=_fo; i<_nu; i++) if(_u[i].t()==2)
		cout << setw(3) << i << " : " << setw(7) << setprecision(5)
		     << _u[i].o() << " (" << _u[i].nm() << ")\n";
	    cout << "\n\n";
	}
    else
	{
	    for(i=_fo; i<_nu; i++) if(_u[i].t()==2)
		cout << setw(7) << setprecision(5) << _u[i].o() << " ";
	    cout << "\n";
	}
}


template<class T, class U> int net<T,U>::bytesize() const
{
  int	c, nb=0, nb2=0;

  for( c = 0; c<_nac; c++ )
    {
      nb += _l[c].nl();
      nb2 += _l[c].nl() >> 3;
    }

  return( 1272 + +72*_nu + 18*_nac + 14*nb + 18*nb2 );
}




//	Opérateurs externes


template<class T, class U>
ostream & operator << ( ostream & fich, const net<T,U> & res )
{
  res.save( fich );
  return( fich );
}


template<class T, class U>
istream & operator >> ( istream & fich, net<T,U> & res )
{
  res.load( fich );
  return( fich );
}




//	Forcer la compilation des templates

template class net<double,double>;
// template class net<int,int>;
template ostream & operator << ( ostream & , const net<double,double> & );
template istream & operator >> ( istream & , net<double,double> & );


