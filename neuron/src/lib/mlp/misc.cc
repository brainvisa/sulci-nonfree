
#include <neur/mlp/misc.h>
#include <string>
#include <string.h>

using namespace std;

//	FONCTION(S) DIVERSE(S)

int cherche( istream *fich, const char *ch )
{
  unsigned	i;
  char	c,d;

  //cout << "cherche '" << ch << "' ... \n" << flush;
  do
    {
      if(fich->eof()) return(1);
      *fich >> c;
      //cout << c << flush;
      if(c==ch[0])
	{
	  //cout << " --> ";
	  for(i=1; i<strlen(ch); i++)
	    {
	      if(fich->eof()) return(1);
	      *fich >> d;
	      //cout << d << flush;
	      if(d!=ch[i]) break;
	    }
	  //cout << "\n";
	  if(i==strlen(ch)) break;
	  //cout << "seekg " << -(int)i << endl;
	  fich->seekg(-(int)i,ios::cur);
	  //cout << "pos : " << fich->tellg() << endl;
	}
    } while(1);

  //cout << "fini.\n";
  return(0);
}








