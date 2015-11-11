#include "ModelSpace.h"
#include "PDHandler.h"
#include <iostream>
#include <string>

template < class T >
void OutputV( const std::vector<T>& dataV, const std::string& fname, const bool app = false ) {
	std::ofstream fout;
	if( app ) fout.open( fname, std::ofstream::app );
	else fout.open( fname );
	if( ! fout )
		throw std::runtime_error("Badfile: open "+fname);

	if( app )fout<<"\n\n";
	for( const auto& t : dataV )
		fout << t << "\n";
}


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

	// SA params
	std::string name_mod = fdata+"_model";
	int niter = 7, nout = 10;
	int ns_out = 3000, ns_iter = ns_out*nout, ns_tot = ns_iter*niter;
	float Tinit = 100., Tfactor = 0;
	float alpha = Searcher::Alpha(ns_out, Tfactor);
	float blockperc = 0.5, APstep = 300.;
	for(int iter=0; iter<niter; iter++) {
		ms.SetBlockPerc(blockperc, APstep);
		for(int iout=0; iout<nout; iout++) {
			ms.dump( name_mod + "_iter" + std::to_string(iter) + "_" + std::to_string(iout) );
			// output accepted, save into vector, istart = nsearch*iout
			auto SIV = Searcher::SimulatedAnnealing< MapGrids<double, GaussianParams> >( 
						  ms, pd, ns_out, alpha, Tfactor, std::cout, 1, true, ns_iter*iter+ns_out*iout );
			OutputV( SIV, name_mod+".SAlog", true );	// append to file
		}
		blockperc *= 0.5; APstep *= 0.5;
	}

	ms.dump( name_mod );

	return 0;
}

