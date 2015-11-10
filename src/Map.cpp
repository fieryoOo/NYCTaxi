#include "Map.h"
#include "DisAzi.h"
#include <cstdio>
#include <cmath>
#include <iostream>
#include <fstream>

struct Map::Mimpl {
	//int nlon = NaN, nlat = NaN;
	double grd_lon = 1., grd_lat = 1.;
	double lonmin = NaN, lonmax = NaN;
	double latmin = NaN, latmax = NaN;

	double dis_lat1D;
	std::vector<double> dis_lon1D;

	std::vector< DataPoint<double> > dataV;
	Array2D< std::vector< DataPoint<double> > > dataM;


	void ReadData( const std::string& fname ) {
		std::ifstream fin(fname.c_str());
		if( ! fin )
			throw ErrorM::BadFile(FuncName, "read from "+fname);
		double lon, lat, data;
		//sscanf(line.c_str(), "%f %f %f", &lon, &lat, &data);
		//fin.seekg(0); // rewind
		/* load in all data */
		dataV.clear();
		for(std::string line; std::getline(fin, line); ) {
			double lon, lat, data;
			if( sscanf(line.c_str(), "%lf %lf %lf", &lon, &lat, &data) < 2 ) {
				std::cerr<<"Warning(Map::Load): wrong format detected in file "<<fname<<std::endl;
				continue;
			}
			if( data != data ) continue;
			if( lon < 0. ) lon += 360.;
			//double dis = Path<double>(srcin, Point<double>(lon,lat)).Dist();
			dataV.push_back( DataPoint<double>(lon, lat, data) );
		}
		fin.close();
		if( dataV.size() == 0 )
			throw ErrorM::BadFile(FuncName, "empty file "+fname);

		/* /map boundaries */
		lonmin = lonmax = dataV[0].Lon();
		latmin = latmax = dataV[0].Lat();
		for(size_t i=0; i<dataV.size(); i++) {
			const auto& dpcur = dataV[i];
			if( lonmax < dpcur.Lon() ) {
				lonmax = dpcur.Lon();
			} else if( lonmin > dpcur.Lon() ) {
				lonmin = dpcur.Lon();
			}
			if( latmax < dpcur.Lat() ) {
				latmax = dpcur.Lat();
			} else if( latmin > dpcur.Lat() ) {
				latmin = dpcur.Lat();
			}
		}
		std::cout<<"### "<<dataV.size()<<" map points loaded. ###\n"
					<<"### "<<lonmin<<" "<<lonmax<<" "<<latmin<<" "<<latmax<<" ###"<<std::endl;
	}

	void Hash() {
		if( dataV.size() == 0 ) return;
		/* grid/hash size */
		//grd_lon = grd_lat = sqrt( (lonmax-lonmin) * (latmax-latmin) / sqrt((double)dataV.size()) );
		//grd_lon = grdlon; grd_lat = grdlat;
		int nlon = (int)ceil( (lonmax-lonmin) / grd_lon ) + 1;
		int nlat = (int)ceil( (latmax-latmin) / grd_lat ) + 1;
		dataM.clear(); dataM.resize(nlon, nlat);
		/* hash */
		for(size_t i=0; i<dataV.size(); i++) {
			DataPoint<double>& dpcur = dataV[i];
			int ilon = (int)floor((dpcur.Lon()-lonmin) / grd_lon + 0.5);
			int ilat = (int)floor((dpcur.Lat()-latmin) / grd_lat + 0.5);
			dataM(ilon, ilat).push_back(dpcur);
		}
		dataV.clear();
		/* compute distance of 1 degree in lon/lat */
		double clon = 0.5 * (lonmin + lonmax);
		dis_lat1D = Path<double>(clon, 0., clon, 1.).Dist();
		//dis_lon1D.resize( dataM.NumCols() );
		dis_lon1D.clear();
		double latcur = latmin;
		while( latcur<=90. ) {
			dis_lon1D.push_back( Path<double>(0., latcur, 1., latcur).Dist() );
			latcur += grd_lat;
		}
	}
};


/* -------------- con/destructors and assignment operators ----------------- */
Map::Map( const std::string& inname, const double grdlon, const double grdlat )
	: pimplM( new Mimpl() ), fname(inname) {
   pimplM->grd_lon = grdlon;
   pimplM->grd_lat = grdlat;
	Load( fname );
}

Map::Map( const Map& mp_other ) : pimplM( new Mimpl(*(mp_other.pimplM)) ) {}

Map::Map( Map&& mp_other ) : pimplM( std::move(mp_other.pimplM) ) {}

Map& Map::operator= ( const Map& mp_other ) {
	pimplM.reset( new Mimpl(*(mp_other.pimplM)) );
	fname = mp_other.fname;
	src = mp_other.src;
	return *this;
}

Map& Map::operator= ( Map&& mp_other ) {
	pimplM = std::move(mp_other.pimplM);
	fname = std::move(mp_other.fname);
	src = std::move(mp_other.src);
	return *this;
}

Map::~Map() {}



/* ----- map boundaries ----- */
double Map::LonMin() const { return pimplM->lonmin; }
double Map::LonMax() const { return pimplM->lonmax; }
double Map::LatMin() const { return pimplM->latmin; }
double Map::LatMax() const { return pimplM->latmax; }

/* ------------ IO and resets ------------ */
void Map::Load( const std::string& fnamein ) {
	fname = fnamein;
	// open/check the file
	pimplM->ReadData( fname );
	pimplM->Hash();
}

/* --- clip the map around the source location (to speed up the average methods) --- */
void Map::Clip( const double lonmin, const double lonmax, const double latmin, const double latmax ) {
	// store new bounds
	pimplM->lonmin = lonmin; pimplM->lonmax = lonmax;
	pimplM->latmin = latmin; pimplM->latmax = latmax;
	// move data matrix
	auto& dataM = pimplM->dataM;
	auto dataMold = std::move(dataM);
	// resize data matrix
	double grd_lon = pimplM->grd_lon, grd_lat = pimplM->grd_lat;
	int nlon = (int)ceil( (lonmax-lonmin) / grd_lon ) + 1;
	int nlat = (int)ceil( (latmax-latmin) / grd_lat ) + 1;
	dataM.clear(); dataM.resize(nlon, nlat);
	/* re-hash */
	for( size_t i=0; i<dataMold.NumRows(); i++ ) {
		for( size_t j=0; j<dataMold.NumCols(); j++ ) {
			for( auto& dp : dataMold(i, j) ) {
				int ilon = (int)floor((dp.Lon()-lonmin) / grd_lon + 0.5);
				int ilat = (int)floor((dp.Lat()-latmin) / grd_lat + 0.5);
				try {
					dataM.at(ilon, ilat).push_back( std::move(dp) );
				} catch(...) {}
			}
		}
	}
	//dis_lon1D.resize( dataM.NumCols() );
	pimplM->dis_lon1D.clear();
	double latcur = latmin;
	while( latcur<=90. ) {
		pimplM->dis_lon1D.push_back( Path<double>(0., latcur, 1., latcur).Dist() );
		latcur += grd_lat;
	}
}


/* ------------ compute number of points near the given location ------------ */
int Map::NumberOfPoints(Point<double> rec, const double xhdis, const double yhdis, double& loneff, double& lateff) const {
	/* references */
	const Array2D< std::vector< DataPoint<double> > >& dataM = pimplM->dataM;
	double lonmin = pimplM->lonmin, latmin = pimplM->latmin;
	double grdlon = pimplM->grd_lon, grdlat = pimplM->grd_lat;;

	// correct lon
	rec.correctLon();

	// requested boundaries
	Point<double> BL(rec.lon-xhdis, rec.lat-yhdis);
	Point<double> TR(rec.lon+xhdis, rec.lat+yhdis);

	// define computation area
	int rowmin = std::max( 0, (int)floor((BL.lon - lonmin) / grdlon + 0.5) );
	int rowmax = std::min( dataM.NumRows(), (int)floor((TR.lon - lonmin) / grdlon + 0.5) + 1 );
	int colmin = std::max( 0, (int)floor((BL.lat - latmin) / grdlat + 0.5) );
	int colmax = std::min( dataM.NumCols(), (int)floor((TR.lat - latmin) / grdlat + 0.5) + 1 );

	// compute total number of points in given area
	int Npoints = 0;
	loneff = lateff = 0;
	for(int irow=rowmin; irow<rowmax; irow++)
		for(int icol=colmin; icol<colmax; icol++)
			for( const auto& dpcur : dataM(irow, icol) ) {
				if( ! dpcur.isWithin(BL, TR) ) continue;
				loneff += dpcur.Lon(); lateff += dpcur.Lat();
				Npoints++;
			}

	if( Npoints > 0 ) {
		loneff /= Npoints;
		lateff /= Npoints;
	} else {
		loneff = lateff = NaN;
	}

	return Npoints;
}

/* ------------ compute average value on the point rec ------------ */
double Map::PointAverage(Point<double> rec, double hdis, double& weit) {
	/* references */
	Array2D< std::vector< DataPoint<double> > >& dataM = pimplM->dataM;
	double lonmin = pimplM->lonmin, latmin = pimplM->latmin;
	double grdlon = pimplM->grd_lon, grdlat = pimplM->grd_lat;;

	// correct lon
	rec.correctLon();

	// define computation area
	double dismax = hdis * 2.5, dismax_s = dismax*dismax;
	size_t idlmax = pimplM->dis_lon1D.size() - 1;
	int ilatrec = (int)floor((rec.Lat()-latmin) / grdlat + 0.5);
	if( ilatrec > idlmax ) ilatrec = idlmax;
	else if( ilatrec < 0 ) ilatrec = 0;
	double dis_lon1D = pimplM->dis_lon1D[ilatrec], dis_lat1D = pimplM->dis_lat1D;
	//calc_dist( rec.Lat(), rec.Lon(), rec.Lat(), rec.Lon()+1., &dis_lon1D );
	//calc_dist( rec.Lat(), rec.Lon(), rec.Lat()+1., rec.Lon(), &dis_lat1D );
	double Rlon = dismax / dis_lon1D, Rlat = dismax / dis_lat1D;
	int rowmin = (int)floor((rec.Lon()-Rlon - lonmin) / grdlon + 0.5);
	if( rowmin < 0 ) rowmin = 0;
	int rowmax = (int)floor((rec.Lon()+Rlon - lonmin) / grdlon + 0.5) + 1;
	if( rowmax > dataM.NumRows() ) rowmax = dataM.NumRows();
	int colmin = (int)floor((rec.Lat()-Rlat - latmin) / grdlat + 0.5);
	if( colmin < 0 ) colmin = 0;
	int colmax = (int)floor((rec.Lat()+Rlat - latmin) / grdlat + 0.5) + 1;
	if( colmax > dataM.NumCols() ) colmax = dataM.NumCols();

	weit = 0.;
	double alpha = -0.5 / (hdis*hdis), datasum = 0.;
	for(int irow=rowmin; irow<rowmax; irow++) {
		for(int icol=colmin; icol<colmax; icol++) {
			for(size_t idata=0; idata<dataM(irow, icol).size(); idata++) {
				DataPoint<double> dpcur = dataM(irow, icol)[idata];
				if( dpcur.Data() == NaN ) continue;
				//distance from dataM(irow, icol).at(idata) to DPrec.
				double xdis = ( rec.Lon() - dpcur.Lon() ) * dis_lon1D;
				double ydis = ( rec.Lat() - dpcur.Lat() ) * dis_lat1D;
				double disc_s = xdis*xdis + ydis*ydis;
				//std::cerr<<dpcur.Data()<<" "<<disc_s<<"  "<<dpcur.Lon()<<" "<<dpcur.Lat()<<"  "<<rec.Lon()<<" "<<rec.Lat()<<std::endl;
				if( disc_s > dismax_s ) continue;
				double weight = exp( alpha * disc_s );
				weit += weight;
				datasum += (dpcur.Data() * weight);
			}
		}
	}
	if(weit==0.) datasum = NaN;
	else datasum /= weit;

	return datasum;

}

