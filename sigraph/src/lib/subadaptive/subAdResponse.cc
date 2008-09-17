#include <si/subadaptive/subAdaptive.h>
#include <si/subadaptive/subAdResponse.h>
#include <assert.h>
#include <iostream>

using namespace sigraph;
using namespace std;

SubAdResponse::SubAdResponse(std::vector<double> *true_values,
			std::vector<double> *predict_values)
{
	this->true_values = true_values;
	this->predict_values = predict_values;
}

SubAdResponse::~SubAdResponse()
{
	delete true_values;
	delete predict_values;
};
