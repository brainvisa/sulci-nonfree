
#include <cartobase/stream/sstream.h>
#include <cartobase/type/string_conversion.h>
#include <si/subadaptive/subAdMixGauss.h>
#include <graph/tree/tree.h>
#include <assert.h>
#include <iomanip>

using namespace sigraph;
using namespace std;



SubAdMixGauss::SubAdMixGauss() {}

SubAdMixGauss::SubAdMixGauss(const string & name,
			const std::vector<float> &sqrtdets)
  : NonIncrementalSubAdaptive( name ), _sqrtdets(sqrtdets)
{
}


SubAdMixGauss::SubAdMixGauss(const SubAdMixGauss &g)
	: NonIncrementalSubAdaptive(g),
	_metrics(g._metrics),
	_sqrtdets(g._sqrtdets)
{
}


SubAdMixGauss::~SubAdMixGauss()
{
}


void SubAdMixGauss::init()
{
	SubAdaptive::init();
}


void SubAdMixGauss::addMatrix(const std::vector<float> &matrix)
{
	_metrics.push_back(matrix);
}


void SubAdMixGauss::addSqrtDet(double sqrtdet)
{
	_sqrtdets.push_back(sqrtdet);	
}

void  SubAdMixGauss::reset()
{
	_sqrtdets.clear();
	_metrics.clear();
}

double	gaussian_density(const vector<float> &metric, double sqrtdet,
			const vector<double> &v)
{
	unsigned int	i, j, dim = v.size();
	double		dist = 0., norm;
	vector<double>	v2(dim);
	assert(dim * dim == metric.size());

	//v2 = metric * v
	for (i = 0; i < dim; ++i)
	{
		v2[i] = 0.;
		//v2_i = m_ij * v_j
		for (j = 0; j < dim; ++j)
			v2[i] += metric[i * dim + j] * v[j];
  	}
	//dist = v.t * metric * v = ||v||^2_metric
	for (i = 0; i < dim; ++i)
		dist += v[i] * v2[i];
	norm = pow(2 * M_PI, dim / 2.) * sqrtdet;
	return exp(-0.5 * dist) / norm;
}

double SubAdMixGauss::prop( const vector<double> & vec )
{
	vector<double>	*nrmv = NULL;
	std::list<vector<float> >::const_iterator mb, me;
	double		res, p0, sum_pi = 0., xi2 = 0., norm, sigma2 = 10000.;
	unsigned int	dim = vec.size();
	unsigned int	i;

	nrmv = normalizeSelected(&(vec[0]), vec.size());
	// p0 = p(X | Y = 0)
	p0 = gaussian_density(*(_metrics.begin()), _sqrtdets[0], vec);
	// sum_pi = sum_i p(X | Y = i)
	for (i = 0, mb = _metrics.begin(), me = _metrics.end();
		mb != me; ++mb, ++i)
		sum_pi += gaussian_density(*mb, _sqrtdets[i], vec);
	// sum_pi += rejection class (huge gaussian)
	for (i = 0; i < dim; ++i)
		xi2 = vec[i] * vec[i];
	norm = powl(2 * M_PI * sigma2, dim / 2.);
	sum_pi += exp(-0.5 * xi2 / sigma2) / norm;
	res = 1. - 2. * (p0 / sum_pi);
	delete nrmv;
	return res;
}


double SubAdMixGauss::learn(const SiDBLearnable &)
{
	//do nothing : all is done in Python
	return 0.;
}

void SubAdMixGauss::buildTree( Tree & tr ) const
{
	std::list<vector<float> >::const_iterator mb, me;
	unsigned int i;

	tr.setSyntax("sub_ad_mixgauss");
	SubAdaptive::buildTree( tr );
	tr.setProperty("sqrtdets", (std::vector<float>) _sqrtdets);
	Tree *tr2 = new Tree;
	tr.insert(tr2);
	tr2->setSyntax("matrices_list");
	for (i = 0, mb = _metrics.begin(), me = _metrics.end();
					mb != me; ++mb, ++i)
	{
		std::ostringstream	stream;
		stream << "class" << std::setfill('0') << std::setw(3) << i;
		//std::string	str = "class" +carto::toString<unsigned int>(i);
		tr2->setProperty(stream.str(), (std::vector<float>) *mb);
	}
}
