#include "ModelSpace.h"
#include "PDHandler.h"
#include <iostream>
#include <string>


int main(int argc, char* argv[]) {
	if( argc!=3 && argc!=4 ) {
		std::cerr<<"Usage: "<<argv[0]<<" [data_infile (x y)] [grid_infile (regular x y)] [total #days in data_infile (default=1)]"<<std::endl;
		return -1;
	}

	// take inputs
	std::string fdata(argv[1]), fgrid(argv[2]);

	// load grids
	//MapGrids<double, float> mapg(fgrid, true);
	ModelSpace ms(fgrid, 100., 0., true);

	// load data
	//Map mapd(fdata,ms.xgrd*2,ms.ygrd*2);
	float xspan = ms.xmax-ms.xmin, xmin = ms.xmin-0.1*xspan, xmax = ms.xmax+0.1*xspan;
	float yspan = ms.ymax-ms.ymin, ymin = ms.ymin-0.1*yspan, ymax = ms.ymax+0.1*yspan;
	PDHandler pd(fdata, ms.xgrd*0.5, ms.ygrd*0.5, xmin, xmax, ymin, ymax);
	// normalize by No. of days
	if( argc == 4 ) {
		float ndays = atof(argv[3]);
		if( ndays>0 && ndays!=1.0 ) pd.Mul( 1.0/ndays );
	}

	// 
	int nsearch = 200000, Tfactor = 0;
	float Tinit = 100.;
	float alpha = Searcher::Alpha(nsearch, Tfactor);
	//auto SIV = Searcher::MonteCarlo<ModelInfo>( ms, eka, nsearch, std::cout );
	int niter = 7; 
	float blockperc = 0.5, APstep = 300.;
	for(int iter=0; iter<niter; iter++) {
		//ms.dump( fdata + "_model_iter" + std::to_string(iter) );
		ms.SetBlockPerc(blockperc, APstep);
		// output accepted, do not save into vector
		Searcher::SimulatedAnnealing< MapGrids<double, GaussianParams> >( ms, pd, nsearch/niter, alpha, Tfactor, std::cout, 1, false );
		blockperc *= 0.5; APstep *= 0.5;
	}

	ms.dump( fdata + "_model" );

	return 0;
}

