
#ifndef SI_FOLD_FOLDFAKEREL_H
#define SI_FOLD_FOLDFAKEREL_H



#include <si/model/model.h>
#include <map>


namespace sigraph
{
  class CGraph;
  class MGraph;

  /**	Evalue les instances de relations qui n'existent pas dans le modèle. 
	Ce modèle existe une seule fois dans un graphe modèle et évalue une 
	"fausse" clique à laquelle appartiennent tous les noeuds du graphe 
	exemple, ou en tout cas tous ceux qui ont des relations (en temps 
	normal, ils en ont tous). Tout ceci pour répondre "faux" quand on 
	trouve une instance de relation qui n'existe pas dans le modèle. 
	Comme ce modèle existe une seule fois et est une relation, par 
	convention il appartient à .........
	La clique correspondante a l'attribut syntaxique "FAKE" et doit 
	contenir un pointeur sur son graphe parent, de type CGraph* sous 
	l'attribut "cgraph".
  */
  class FoldFakeRel : public Model
  {
  public:
    ///	Description d'une relation
    struct Reldescr
    {
      Reldescr() : hasModel( false ), num( 0 ) {}
      bool	hasModel;
      unsigned	num;
    };
    ///	tableau indices -> Reldescr
    typedef Reldescr**			Relmap;
    ///	tableau graphe -> Relmap
    typedef std::map<const CGraph*, Relmap>	FakeRels;

    FoldFakeRel( Model* parent = 0 );
    FoldFakeRel( const FoldFakeRel & fr ) : Model( fr ), _mgraph( fr._mgraph ), 
					    _rels( fr._rels ) {}
    virtual ~FoldFakeRel();

    void setMGraph( MGraph* mg ) { _mgraph = mg; }
    virtual Model* clone() const { return( new FoldFakeRel( *this ) ); }
    virtual void buildTree( Tree & tr ) const;

    virtual double prop( const Clique* cl );
    /**	Ce prop() est utilisé pour tester un changement: on ne fait pas 
	d'update() avant, et il n'est pas effectué par prop() non plus: si le 
	changement est accepté, il faut absolument appeler update() ensuite. */
    virtual double prop( const Clique* cl, 
			 const std::map<Vertex*, std::string> & changes );
    virtual double update( const Clique* cl );
    /**	Change les états internes pour les noeuds de la liste changes, chacun 
	avec son ancien nom
	@return	la différence de potentiel entraînée par le changement */
    virtual double update( const Clique* cl, 
			   const std::map<Vertex*, std::string> & changes );

    virtual Relmap init( const CGraph & cg );
    virtual void clear();
    Reldescr* relDescr( const CGraph* cg, const std::string & label1, 
			const std::string & label2 );

  protected:
    Relmap allocGraph( const CGraph* cg );
    void deleteGraph( Relmap rmap );
    virtual double update( const CGraph & cg, Relmap & relm );

    ///	Graphe modèle
    MGraph	*_mgraph;
    ///	Conversion label -> indice
    std::map<std::string, int>	_ltoi;
    ///	Table graphe :-> tableau des relations
    FakeRels	_rels;

  private:
  };

}


namespace carto
{
  DECLARE_GENERIC_OBJECT_TYPE( sigraph::FoldFakeRel * )
}

#endif

