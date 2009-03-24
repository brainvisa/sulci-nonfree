
#include <cstdlib>
#include <neur/mlp/mlp.h>
#include <neur/rand/rand.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/types.h>

using namespace std;

//	RESEAUX DE PERCEPTRONS MULTI-COUCHES

template<class T,class U> 
void mlp<T,U>::init( const char *nom, int nc, int *couch)
{
    _nc=nc;
    _couch= new int[_nc];
    memcpy(_couch,couch,_nc*sizeof(int));

    this->pmc(nom,nc,couch);
}


template<class T, class U> 
mlp<T,U>::mlp(const mlp<T,U> & pmc): net<T,U>((net<T,U>) pmc)
{
  _nc=pmc.nc();
  _couch= new int[_nc];
  memcpy(_couch,pmc.couch(),_nc*sizeof(int));
}


template<class T, class U> 
mlp<T,U> & mlp<T,U>::operator = (const mlp<T,U> & pmc)
{
  if( &pmc == this ) return *this;
  *((net<T,U> *)this)=*((net<T,U>*) &pmc);
  _nc=pmc.nc();
  _couch= new int[_nc];
  memcpy(_couch,pmc.couch(),_nc*sizeof(int));
  return *this;
}


template<class T, class U> void mlp<T,U>::empty()
{
  net<T,U>::empty();
  if( _couch ) delete[] _couch;
  _nc = 0;
  _couch = (int*) NULL;
}


template<class T,class U> void mlp<T,U>::aff() const
{
    cout << "\nPerceptron Multi-couches : " << _nc << " couches :\n";
    cout << "--------------------------\n";
    net<T,U>::aff();
}


template <class T,class U> void mlp<T,U>::add_neur( const char *nom, int c,
						   T bia, T sor)
{
    unit<T>	*u;
    int		i,j,ind=0,indo=0,neur,t;

    if(c>=0 && c<=_nc)
	{
	    if(c==_nc)
		{
		    int *cou= new int[_nc+1];
		    memcpy(cou,_couch,_nc*sizeof(int));
		    cou[_nc]=0;
		    delete[] _couch;
		    _couch=cou;
		    _nc++;
		    if(c>=1)
			{
			    j=0;
			    for(i=0; i<c-1; i++) j+=_couch[i];
			    if(c>=2)
				for(i=0; i<_couch[c-1]; i++) this->_u[i+j].set_t(1);
			    else
				for(i=0; i<_couch[0]; i++) this->_u[i+j].set_t(0);
			}
		}
	    u= new unit<T>[this->_nu+1];
	    for(i=0; i<=c; i++) for(j=0; j<_couch[i]; j++)
		u[ind++]=this->_u[indo++];
	    neur=ind;
	    _couch[c]++;
	    ind++;
	    for(i=c+1; i<_nc; i++) for(j=0; j<_couch[i]; j++)
		u[ind++]=this->_u[indo++];

	    if(c==0) t=0;
	    else if(c==_nc-1) t=2;
	    else t=1;
	    u[neur].set(nom,t,c,_couch[c],0,bia,sor);

	    this->_nu++;
	    delete[] this->_u;
	    this->_u=u;

	// Decaler les liaisons

	    for(i=0; i<this->_nac; i++)
		{
		    if (this->_l[i].ta()>=neur) this->_l[i].set_ta(this->_l[i].ta()+1);
		    for (j=0; j<this->_l[i].nl(); ++j)
			if(this->_l[i].le()[j].org()>=neur)
			    this->_l[i].le()[j].set_or(this->_l[i].le()[j].org()+1);
		}
	}
    else cout << "Pas de couche " << c << " a creer." << endl;
}

template<class T,class U> int mlp<T,U>::del_neur(int c, int no)
{
    int	i,n=0;

    if(c>=_nc || c<0)
	{
	    cout << "Pas de couche " << c << "!\n";
	    return 1;
	}
    if(no<0 || _couch[c]<no+1)
	{
	    cout << "Pas de neurone " << no << " sur la couche "
		 << c << ".\n";
	    return 1;
	}

    for(i=0; i<c; i++) n+=_couch[i];
    n+=no;

    if(net<T,U>::del_neur(n)) return 1;

    _couch[c]--;
    if(_couch[c]==0)
	{
	    int	*ch;
	    for(i=c+1; i<_nc; i++) _couch[i-1]=_couch[i];
	    _nc--;
	    ch= new int[_nc];
	    memcpy(ch,_couch,_nc*sizeof(int));
	    delete[] _couch;
	    _couch=ch;
	    if( c == _nc )
	      {
		n = 0;
		if( _nc > 1 )
		  {
		    for( i=0; i<_nc-2; i++ ) n += _couch[i];
		    for( i=0; i<_couch[_nc-1]; i++ )
		      this->_u[i+n].set_t( SORTIE );
		  }
	      }
	    else if( c == 0 )
	      for( i=0; i<_couch[0]; i++ )
		this->_u[i].set_t( ENTREE );
	}

    return 0;
}


template<class T,class U> int mlp<T,U>::add_link(int c, int tar,int nli)
{
    int	i,n=0;

    if(c>=_nc || c<=0)
	{
	    cout << "Pas de couche " << c
		 << " accessible en arrivee de lien!\n";
	    return 1;
	}
    if(tar<0 || _couch[c]<tar+1)
	{
	    cout << "Pas de neurone " << tar << " sur la couche "
		 << c << ".\n";
	    return 1;
	}

    for(i=0; i<c; i++) n+=_couch[i];
    n+=tar;

    if(net<T,U>::add_link(n,nli)) return 1;
    return 0;
}


template<class T,class U> int mlp<T,U>::add_link(int c, int tar)
{
    int	i,l,org;

    //cout << "add_link : " << _couch[c-1] << " liens.\n";
    if(add_link(c,tar,_couch[c-1])) return 1;
    org=get_neur(c-1,0);
    l=get_link(c,tar); //cout << "(no " << l << ")\n";
    for(i=0; i<this->_l[l].nl(); i++)
      this->_l[l].le()[i].set(org+i,0);

    return 0;
}


template<class T,class U> int mlp<T,U>::del_link(int c, int no)
{
    if(c>=_nc || c<=0)
	{
	    cout << "Pas de couche " << c << "!\n";
	    return 1;
	}
    if(no<0 || _couch[c]<no+1)
	{
	    cout << "Pas de neurone " << no << " sur la couche "
		 << c << ".\n";
	    return 1;
	}

    int n=get_neur(c,no);
    if(n<0) return 1;

    return net<T,U>::del_link(n);
}

template<class T,class U> int mlp<T,U>::add_le(int c, int tar, int co,
					       int org, U w)
{
    if(c>=_nc || co>=_nc || c<0 || co<0)
	{
	    cout << "Pas de couche " << c << " ou " << co << "!\n";
	    return 1;
	}
    if(tar<0 || org<0 || _couch[c]<tar+1 || _couch[co]<org+1)
	{
	    cout << "Pas de neurone " << tar << " ou " << org
		 << " sur la couche " << c << " ou " << co << ".\n";
	    return 1;
	}

    int n=get_neur(c,tar);
    if(n<0) return 1;
    int no=get_neur(co,org);
    if(no<0) return 1;

    return net<T,U>::add_le(n,no,w);
}


template<class T,class U> int mlp<T,U>::del_le(int c, int tar, int co, int org)
{
    if(c>=_nc || co>=_nc || c<=0 || co<=0)
	{
	    cout << "Pas de couche " << c << " ou " << co << "!\n";
	    return 1;
	}
    if(tar<0 || org<0 || _couch[c]<tar+1 || _couch[co]<org+1)
	{
	    cout << "Pas de neurone " << tar << " ou " << org
		 << " sur la couche " << c << " ou " << co << ".\n";
	    return 1;
	}

    int n=get_neur(c,tar);
    if(n<0) return 1;
    int no=get_neur(co,org);
    if(no<0) return 1;

    return net<T,U>::del_le(n,no);
}


template<class T,class U> int mlp<T,U>::trie()
{
  int		par[this->_nu][this->_nu];
  int		lis[this->_nac+1][this->_nu+1];
  int		i,j,k,t,o,nuu,c,pn,dn,pnn,dnn;
  int		cou[this->_nac+1],ut[this->_nu];
  neur_link<U>	ltmp;

  // cout << "Tri du reseau en couches ..." << endl;

  if(_couch)
    {
      cout << "Des couches avaient deja ete faites." << endl;
    }

  for(i=0; i<this->_nu; i++)
    {
      ut[i]=this->_nu;
      for(j=0; j<this->_nu; j++)
	par[i][j]=0;
    }
  for(i=0; i<=this->_nac; i++)
    lis[i][0]=-1;

  //	Creation du tableau de parente

  for(i=0; i<this->_nac; i++)
    {
      t=this->_l[i].ta();
      for(j=0; j<this->_l[i].nl(); ++j)
	{
	  par[t][o=this->_l[i].le()[j].org()]=1;
	  if(par[o][t])
	    {
	      cout << "Reseau boucle." << endl;
	      return 1;
	    }
	  for(k=0; k<this->_nu; k++) if(par[o][k] && !par[t][k])
	    {
	      par[t][k]=2;
	      if(par[k][t])
		{
		  cout << "Reseau boucle." << endl;
		  return 1;
		}
	    }
	}
    }

  //	Classement en couches

  c=0;
  nuu=0;
  pnn=0;
  dnn=this->_nu;
  do
    {
      // cout << "Couche " << c << " : ";
      cou[c]=0;
      pn=pnn;
      dn=dnn;
      for(i=pn; i<dn; i++) if(c<ut[i])
	{
	  for(j=pn; j<dn; j++)
	    if(par[i][j] && ut[j]>=c) break;
	  if(j==dn)
	    {
	      lis[c][cou[c]++]=i;
	      ut[i]=c;
	      if(i==pnn) pnn=i+1;
	      if(i==dnn-1) dnn=i;
	      nuu++;
	      // cout << i << ", ";
	    }
	}
      // cout << ".\n";
      c++;
    } while(nuu<this->_nu && cou[c-1]!=0);
  if(nuu<this->_nu)
    {
      cout << "J'arrive pas a classer tous les neurones (bouclage?)" << endl;
      return 1;
    }

  //	Mise a jour de la table des couches

  if(_couch) delete _couch;
  _nc=c;
  _couch= new int[_nc];
  memcpy(_couch,cou,_nc*sizeof(int));

  //	Reordonnancement des neurones

  c=0;
  j=0;
  for(i=0; i<this->_nu; i++)
    {
      ut[i]=lis[c][j];
      j++;
      if(j==_couch[c])
	{
	  j=0;
	  c++;
	}
    }

  for(i=0; i<this->_nu; i++)
    {
      if(ut[i]!=-1 && ut[i]!=i)
	{
	  this->renum(ut[i],this->_nu);
	  j=ut[i];
	  while(j!=i)
	    {
	      this->renum(ut[j],j);
	      k=ut[j];
	      ut[j]=-1;
	      j=k;
	    }
	  this->renum(this->_nu,i);
	  // ut[i]=-1;
	}
    }

  //	Reperage des sorties

  this->_fo=this->_nu;
  for(i=0; i<this->_nu && this->_fo==this->_nu; i++)
    if(this->_u[i].t()==2)
      this->_fo=i;

  //	Reordonnamcement des liaisons

  for(i=0; i<this->_nac; i++) lis[i][0]=0;
  for(i=0; i<this->_nac; i++)
    {
      k=-1;
      for(j=0; j<this->_nac; j++)
	if(lis[j][0]==0 && (k<0 || this->_l[j].ta()<this->_l[k].ta()))
	  k=j;
      ut[i]=k;
      // cout << i << ":" << ut[i] << "(" << this->_l[k].ta() << ") ; ";
      lis[k][0]=1;
    }
  // cout << ".\n";

  for(i=0; i<this->_nac; i++)
    {
      if(ut[i]!=-1 && ut[i]!=i)
	{
	  // cout << "L" << ut[i] << " -> Ltmp\n";
	  ltmp=this->_l[ut[i]];
	  j=ut[i];
	  while(j!=i)
	    {
	      // cout << "L" << ut[j] << " -> L" << j << "\n";
	      this->_l[j]=this->_l[ut[j]];
	      k=ut[j];
	      ut[j]=-1;
	      j=k;
	    }
	  // cout << "Ltmp -> L" << i << "\n";
	  this->_l[i]=ltmp;
	}
    }

  return 0;
}


template<class T,class U> int mlp<T,U>::load( const char *nom )
{
  ifstream fich( nom, ios::in );

  if( !fich )
    {
      cout << "Ouverture de " << nom << " impossible.\n";
      return 1;
    }

  int r = load( fich );

  fich.close();

  return r;
}


template<class T,class U> int mlp<T,U>::load( istream & fich )
{
    if( net<T,U>::load( fich ) )
      {
	cout << "Erreur dans la lecture..." << endl;
	return 1;
      }

    if(_couch)
	{
	    cout << "Heu... il y avait deja qqch dans la table des couches" << endl;
	    delete _couch;
	}
    _nc=0;
    if(this->_nu==0)
	{
	    cout << "Reseau sans neurones..." << endl;
	    return 0;
	}

    if(trie()) return 1;

    //cout << this->_nc << " couches reperees." << endl;
    return 0;
}


template<class T,class U> int mlp<T,U>::prop(const pat<T> & pa, int n)
{
  int	i;

  if(n>=pa.np())
    {
      cout << "No d'exemple superieur au nombre contenu dans la base." << endl;
      return 1;
    }
  if(pa.ni()!=this->_ni)
    {
      cout << "La base d'exemple ne correspond pas au reseau." << endl;
      return 1;
    }

  for(i=0; i<this->_ni; i++)
    this->_u[i].set_o(pa.vi()[n][i]);

  return prop();
}


template<class T,class U> int mlp<T,U>::prop()
{
  int	i,j;
  U	s, t;

  for(i=0; i<this->_nac; i++)
    {
      s=0;
      for(j=0; j<this->_l[i].nl(); j++)
	s += this->_l[i].le()[j].w() * (U) this->_u[this->_l[i].le()[j].org()].o();
      t = -s-this->_u[this->_l[i].ta()].bi();
      if( t > 700 )
	t = 700;
      this->_u[this->_l[i].ta()].set_o( (T) ( (U)1/( (U)1+exp( t ) ) ) );
    }

  return 0;
}


template<class T,class U> int mlp<T,U>::backprop(const pat<T> & pa, int nc,
						 double eta, int flg, 
						 T *err )
{
  int		i,j,pass,ex;
  time_t	t;
  T		erreur=0;

  if(pa.ni()!=this->_ni || pa.no()!=this->_no)
    {
      cout << "Reseau et base d'exemples incompatibles." << endl;
      cout << "Entrees: reseau : " << this->_ni << ", base : " << pa.ni() << endl;
      cout << "Sorties: reseau : " << this->_no << ", base : " << pa.no() << endl;
      return 1;
    }

  int		ord[pa.np()],cla[pa.np()+1];

  if( flg ) 
    cout << "Apprentissage du perceptron multi-couches " << this->_nm << "..." << endl;

  t=time( (time_t*) NULL );
  srand(t);

  int car = 1;
  unsigned ncar = nc;
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

      //	Pour chaque exemple dans le nouvel ordre

      for(ex=0; ex<pa.np(); ex++)
	{
	  // Propager l'exemple en cours
	  if(prop(pa,ord[ex])) return 1;
	  erreur += backprop( pa.vo()[ord[ex]], eta );
	}
    }
  if( flg ) cout << "\n";

  if( err ) *err = erreur/2;

  return 0;
}


template<class T, class U> 
T mlp<T,U>::backprop( T *teachO, double eta )
{
  T		dlt[this->_nu];
  int		ido, i, cel, k;
  T		erreur = 0;

  // Preparation

  for( i=0; i<this->_nu; i++ ) dlt[i] = 0;

  // Apprentissage cellule par cellule

  ido = this->_no-1;
  for( i=this->_nac-1; i>=0; i-- )
    {
      cel = this->_l[i].ta();
      if( this->_u[cel].t() == 2 )
	{
	  dlt[cel] = ( teachO[ido--]
		       - this->_u[cel].o() ) * 2;
	  erreur += fabs( dlt[cel] );
	}
      dlt[cel] *= this->learn_p( i, dlt[cel], eta );

      for( k=0; k<this->_l[i].nl(); k++ )
	dlt[this->_l[i].le()[k].org()] += this->_l[i].le()[k].w() * dlt[cel];
    }
  return erreur;
}


template<class T, class U> void mlp<T,U>::Reduction()
{
  if( _nc != 3 )
    {
      cerr << "mlp<T,U>::Reduction : ne peut se faire QUE sur "
	   << "un PMC 3 couches" << endl;
      return;
    }

  int	j;

  for( j=0; j<_couch[1]; j++ ) ReductionIndex( j );
}


template<class T, class U> void mlp<T,U>::ReductionIndex( int j )
{
  int	i, k;
  U	w=0, s=0;

  for( i=0; i<_couch[2]; i++ )
    // On suppose que les liens élémentaires sont rangés dans le bon ordre!!
    w += sqr( this->_l[ this->_ni+i ].le()[ j ].w() );

  //	Mettre à jour les entrées avant!...
  // ...

  for( k=0; k<this->_ni; k++ )
    s += this->_l[j].le()[k].w() * this->_u[k].o();

  s = 1. / ( 1. + exp( -s -this->_u[this->_ni+j].bi() ) );
  // dQ ??
}



//	Opérateurs externes


template<class T, class U>
inline ostream & operator << ( ostream & fich, const mlp<T,U> & res )
{
  res.save( fich );
  return fich;
}


template<class T, class U>
inline istream & operator >> ( istream & fich, mlp<T,U> & res )
{
  res.load( fich );
  return fich;
}



//	Compilation


template class mlp<double,double>;
template ostream & operator << ( ostream & , const mlp<double,double> & );
template istream & operator >> ( istream & , mlp<double,double> & );
