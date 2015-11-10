#ifndef PDHANDLER_H
#define PDHANDLER_H

#include "Map.h"
#include "MapGrids.h"
#include "DisAzi.h"
#include "Searcher.h"
#include <fstream>
#include <string>

class PDHandler : public Searcher::IDataHandler< MapGrids<double, GaussianParams> > {
public:
	PDHandler( const std::string& fname, const float xgrd, const float ygrd,
				  const float xmin, const float xmax, const float ymin, const float ymax ) 
		: deg2dis_lat( Path<double>(0., 0., 0., 1.).DistF() ) {
		Load(fname, xgrd, ygrd, xmin, xmax, ymin, ymax);
	}

	void Energy( const MapGrids<double, GaussianParams>& mg, float& E, int& Ndata ) const {
		// loop over all data points
		Ndata = 0; E = 0.;	// the rms misfit
		for( const auto& dp : mapdV ) {
			// for each data point
			const double deg2dis = dp.dis;
			// assume zero effects from anything >1.0km (for now)
			const double deglon = 1.0 / deg2dis;
			const double deglat = 1.0 / deg2dis_lat;
			// grab nearby model grids
			std::vector< Node<double, GaussianParams> > nodesV;
			mg.getNodes(dp.lon, dp.lat, deglon, deglat, nodesV, true);
			// should not skip: density_pred = 0 when no source is nearby
			//	if( nodesV.size() == 0 ) continue;	
			// predict service demand density at this point
			float density = 0.;
			for( const auto& node : nodesV ) {
				double dis1 = (node.x - dp.lon) * deg2dis;
				double dis2 = (node.y - dp.lat) * deg2dis_lat;
				double disS = dis1*dis1 + dis2*dis2;
				density += node.data.A * exp( node.data.alpha*disS );
			}
			// compare prediction with data
			density -= dp.data;
			E += density * density;
			Ndata++;
		}
		E = sqrt( E / (Ndata - 1) );
	}

	void Load( const std::string& fname, const float xgrd, const float ygrd,
				  const float xmin, const float xmax, const float ymin, const float ymax ) {
		// initialize map
		Map mapd(fname, xgrd*4, ygrd*4);
		// compute density and deg2dis ratio
		for( double x=xmin; x<=xmax; x+=xgrd )
			for( double y=ymin; y<=ymax; y+=ygrd ) {
				double xeff, yeff;
				int Npts = mapd.NumberOfPoints( Point<double>(x, y), xgrd, ygrd, xeff, yeff );
				if( Npts==0 ) { xeff = x; yeff = y; }
				mapdV.push_back( DataPoint<double>(xeff, yeff, Npts, deg2dis(yeff)) );
			}
		std::cout<<"### "<<mapdV.size()<<" data points generated ###"<<std::endl;
		std::cout<<"### data region: "<<xmin<<" "<<xmax<<" "<<xgrd
					<<"   "<<ymin<<" "<<ymax<<" "<<ygrd<<" ###"<<std::endl;
		#if defined DEBUG
		std::ofstream fout("debug_data.txt");
		for(const auto& dp : mapdV) fout<<dp<<"\n";
		#endif
	}

	void Mul( const float mul ) { for( auto& dp : mapdV ) dp.data *= mul; }

protected:
	const double deg2dis_lat;

private:
	std::vector< DataPoint<double> > mapdV;
	double deg2dis( const double lat ) { return Path<double>(0., lat, 1., lat).DistF(); }
};

#endif
