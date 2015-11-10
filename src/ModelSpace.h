#ifndef MODELSPACE_H
#define MODELSPACE_H

#include "MapGrids.h"
#include "Searcher.h"
#include "Rand.h"
#include <iostream>
#include <string>

class ModelSpace : public MapGrids<double, GaussianParams>, public Searcher::IModelSpace< MapGrids<double, GaussianParams> > {
public:
	ModelSpace( const std::string& fname, const float APstepin = 1000., const float sPstepin = 0., const bool correctLon = false ) 
		: MapGrids(fname, correctLon) {
		APstep = APstepin; sPstep = sPstepin;
		std::cout<<"### model (source) region: "<<xmin<<" "<<xmax<<" "<<xgrd
					<<"   "<<ymin<<" "<<ymax<<" "<<ygrd<<" ###"<<std::endl;
	}

	void SetBlockPerc( const float blockperc, const float APstepin ) {
		Block_len_perc = blockperc; APstep = APstepin;
	}

	void Perturb( MapGrids<double, GaussianParams>& mgnew ) const {
		mgnew = *this;
		// randomly pick a sub region to perturb
		int nr = mgnew.NumRows(), nc = mgnew.NumCols();
		int icx = (int)(randO.Uniform() * nr);
		int icy = (int)(randO.Uniform() * nc);
		int ihr = (int)floor(0.5*Block_len_perc*nr + 0.5);
		int ihc = (int)floor(0.5*Block_len_perc*nc + 0.5);
		int rowmin = std::max(0, icx - ihr);
		int rowmax = std::min(nr, icx + ihr + 1);
		int colmin = std::max(0, icy - ihc);
		int colmax = std::min(nc, icy + ihc + 1);

		// perturb the block in the same direction
		// while updating the average
		mgnew.dataavg *= mgnew.Size();
		float Aadd = randO.Normal()*APstep;
		float sMul = exp( randO.Normal()*sPstep );
		for(int ix=rowmin; ix<rowmax; ix++)
			for(int iy=colmin; iy<colmax; iy++) {
			auto& gp = mgnew(ix, iy);
			// substract from sum
			mgnew.dataavg -= gp;
			// perturb A
			gp.A += Aadd;
			if( gp.A < 0. ) gp.A = -gp.A;
			// perturb sigma
			if( sPstep != 0 ) {
				gp.sigma *= sMul;
				gp.setSigma(gp.sigma);
			}
			// add back to sum
			mgnew.dataavg += gp;
		}
		mgnew.dataavg /= mgnew.Size();
	}

	void SetMState( const MapGrids<double, GaussianParams>& mg ) {
		static_cast< MapGrids<double, GaussianParams>& >(*this) = mg;
	}

	friend std::ostream& operator<<( std::ostream& o, const ModelSpace& ms ) {
		o << "ModelSpace with APstep = "<<ms.APstep<<" sPstep = "<<ms.sPstep<<" Block_len_perc = "<<ms.Block_len_perc; 
		return o;
	}

private:
	float APstep, sPstep;	// A += rand*APstep; sigma *= exp(rand*sPstep);
	mutable Rand randO;
	float Block_len_perc = 0.2;
};

#endif
