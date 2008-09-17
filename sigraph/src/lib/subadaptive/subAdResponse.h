
#ifndef SI_SUBADAPTIVE_SUBADARESPONSE_H
#define SI_SUBADAPTIVE_SUBADARESPONSE_H

#include <vector>


namespace sigraph
{
class SubAdaptive;

class SubAdResponse
{
        public:
	SubAdResponse(std::vector<double> *true_values = NULL,
		std::vector<double> *predict_values = NULL);
	virtual ~SubAdResponse();

        std::vector<double>     	*true_values;
        std::vector<double>     	*predict_values;
};

}

#endif



