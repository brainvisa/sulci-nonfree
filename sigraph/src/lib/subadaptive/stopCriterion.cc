
#include <si/subadaptive/stopCriterion.h>
#include <si/subadaptive/subAdaptive.h>

using namespace sigraph;


LearnStopCriterion *LearnStopCriterion::theCriterion = new LearnStopCriterion;



LearnStopCriterion::LearnStopCriterion() 
  : MaxAppError( 0.25 ), StopDelay( 2000 )
{
}


bool LearnStopCriterion::stops( const SubAdaptive & sa, unsigned ) const
{
  /*if( num < sa.stopDelay() 
      || sa.localMaxGErr() - sa.localMinGErr() > maxErrorDispersion )
    return( false );
  if( sa.genErrorRate() - sa.globalMinGErr() > maxErrorIncrease 
      || sa.localMaxGErr() - sa.globalMinGErr() < minDispersion )
      return( true );*/

  if( sa.stepsSinceGenMin() >= StopDelay )
    return( true );
  return( false );
}


bool LearnStopCriterion::stoppable( const SubAdaptive & sa, 
				    unsigned ) const
{
  if( sa.errorRate() < MaxAppError )	// pour le moment c'est simple.
    return( true );
  return( false );
}



