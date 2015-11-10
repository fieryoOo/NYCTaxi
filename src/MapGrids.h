#ifndef MAPGRIDS_H
#define MAPGRIDS_H

#include "Array2D.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>


struct GaussianParams {
	GaussianParams() {}
	GaussianParams( const float init )
		: A(init), sigma(init), alpha(init) {}
	GaussianParams( const float A, const float sigma )
		: A(A), sigma(sigma) { setSigma(sigma); }

	bool operator== ( const GaussianParams& gp2 ) const { return (A==gp2.A && alpha==gp2.alpha); }
	bool operator!= ( const GaussianParams& gp2 ) const { return !(*this == gp2); }

	void setSigma( const float sigmain ) {
		sigma = sigmain;
		alpha = -0.5 / (sigma*sigma);
	}

	friend GaussianParams& operator +=(GaussianParams& gp1, const GaussianParams& gp2) {
		gp1.A += gp2.A;
		gp1.setSigma( gp1.sigma+gp2.sigma );
		return gp1;
	}

	friend GaussianParams& operator -=(GaussianParams& gp1, const GaussianParams& gp2) {
		gp1.A -= gp2.A;
		gp1.setSigma( gp1.sigma-gp2.sigma );
		return gp1;
	}

	friend GaussianParams& operator *=(GaussianParams& gp1, const double mul) {
		gp1.A *= mul; 
		gp1.setSigma( gp1.sigma * mul );
		return gp1;
	}

	friend GaussianParams& operator /=(GaussianParams& gp1, const double denom) {
		return ( gp1 *= (1./denom) );
	}

	friend std::ostream& operator<<( std::ostream& o, const GaussianParams& gp ) {
		o << gp.A << " " << gp.alpha;	return o;
	}
	friend std::istream& operator>>( std::istream& i, GaussianParams& gp ) {
		i >> gp.A >> gp.sigma;
		gp.setSigma( gp.sigma );
		return i;
	}

	float A = 0., alpha = -5.56, sigma = 0.3;	// ~ sigma = 0.3 km
};

template < typename Tloc, typename Tdat >
struct Node {
	Tloc x, y;
	Tdat data;

	Node( Tloc x = Tloc(), Tloc y = Tloc(), Tdat data = Tdat() )
		: x(x), y(y), data(data) {}

	bool isWithin( const Node<Tloc, Tdat>& BL, const Node<Tloc, Tdat>& TR ) const {
		return x>=BL.x&&x<TR.x && y>=BL.y&&y<TR.y;
	}

	friend std::ostream& operator<<( std::ostream& o, const Node<Tloc,Tdat>& node ) {
		o << node.x << " " << node.y << " " << node.data;	return o;
	}
};

//template < typename Tloc, typename Tdat >
template < typename Tloc, typename Tdat >
class MapGrids : public Array2D<Tdat> {
public:
	Tloc xmin, xmax, ymin, ymax;
	Tloc xgrd, ygrd;
	Tdat dataavg = Tdat(0);

public:
	MapGrids() {}

	MapGrids( const std::string& fname, const bool correctLon = false ) { Load(fname, correctLon); }

	void getNodes( Tloc cx, const Tloc cy, const Tloc xhlen, const Tloc yhlen, 
						std::vector< Node<Tloc,Tdat> >& nodesV, const bool correctLon = false ) const {
		// correct lon
		if( correctLon&&cx<0. ) cx+=360.; 

		// requested boundaries
		const Node<Tloc,Tdat> BL(cx-xhlen, cy-yhlen);
		const Node<Tloc,Tdat> TR(cx+xhlen, cy+yhlen);

		// define region
		int rowmin = std::max( 0, xindex_ceil(BL.x) );
		int rowmax = std::min( this->NumRows(), xindex_floor(TR.x)+1 );
		int colmin = std::max( 0, yindex_ceil(BL.y) );
		int colmax = std::min( this->NumCols(), yindex_floor(TR.y)+1 );

		nodesV.clear();
		if( rowmin>=rowmax || colmin>=colmax ) return;

		// grab all nodes in the given region
		nodesV.reserve((rowmax-rowmin)*(colmax-colmin));
		for(int irow=rowmin; irow<rowmax; irow++) {
			for(int icol=colmin; icol<colmax; icol++) {
				const auto& node = getNode(irow, icol);
				if( node.data != NaN ) nodesV.push_back( node );
			}
		}
	}

	void Load( const std::string& fname, const bool correctLon = false,
				  const size_t ix = 1, const size_t iy = 2, const size_t iz = 3 ) {
		// load file into a vector
		std::vector< Node<Tloc,Tdat> > dataV;
		ReadData( fname, correctLon, ix, iy, iz, dataV );
		if( dataV.size() == 0 )
			throw std::runtime_error("Error(Load): empty data input");

		// compute map grids and boundaries
		ComputeMapInfo( dataV );

		// hash points to grids
		Hash( dataV );

		std::cout<<"### "<<dataV.size()<<" / "<<(*this).Size()<<" grid points loaded. ###"<<std::endl;
	}

	void dump( const std::string& fname ) const {
		std::ofstream fout(fname);
		for(int irow=0; irow<this->NumRows(); irow++)
			for(int icol=0; icol<this->NumCols(); icol++)
				fout<<X(irow)<<" "<<Y(icol)<<" "<<(*this)(irow, icol)<<"\n";
	}

	friend std::ostream& operator<<( std::ostream& o, const MapGrids<Tloc, Tdat>& mg ) {
		o << "MapGrid with data avg = " << mg.dataavg; return o;
	}


protected:
	//const int NaN = -12345;
	static constexpr Tloc NaN = -12345.;
	static constexpr Tloc toler = 1.0e-6;

private:

	// load data file into a vector
	void ReadData( const std::string& fname, const bool correctLon,
						const size_t ix, const size_t iy, const size_t iz, std::vector< Node<Tloc,Tdat> >& dataV ) {
		std::ifstream fin(fname);
		dataV.clear();
		for(std::string line; std::getline(fin, line);) {
			std::stringstream ss(line);
			bool suc = true;
			Node<Tloc,Tdat> node;
			for(int i=1; (i<=iz)&&suc; i++) {
				if(i == ix) suc = (ss >> node.x);
				else if(i == iy) suc = (ss >> node.y);
				else if(i == iz) suc = (ss >> node.data);	// dis-allow data to be empty!
				else {
					std::string stmp;
					suc = (ss >> stmp);
				}
			}
			if( !suc ) continue;
			if( correctLon&&node.x<0. ) node.x+=360.; 
			dataV.push_back(node);
		}
	}

	// compute map grids and boundaries
	void ComputeMapInfo( const std::vector< Node<Tloc,Tdat> >& dataV ) {
		// compute x grid
		auto dataVtmp = dataV;
		std::sort( dataVtmp.begin(), dataVtmp.end(), [](const Node<Tloc,Tdat>& n1, const Node<Tloc,Tdat>& n2) {
				return n1.x < n2.x;
		} );
		auto last = std::unique(dataVtmp.begin(), dataVtmp.end(), [&](const Node<Tloc,Tdat>& n1, const Node<Tloc,Tdat>& n2) {
			return isEqual(n1.x, n2.x);
		} );
		dataVtmp.erase(last, dataVtmp.end());
		xmin = dataVtmp.front().x;
		xmax = dataVtmp.back().x;
		xgrd = dataVtmp[1].x - dataVtmp[0].x;
		for(int i=2; i<dataVtmp.size(); i++) {
			Tloc xgrdc = dataVtmp[i].x - dataVtmp[i-1].x;
			if( xgrd > xgrdc ) xgrd = xgrdc;
		}
		// compute y grid
		dataVtmp = dataV;
		std::sort( dataVtmp.begin(), dataVtmp.end(), [](const Node<Tloc,Tdat>& n1, const Node<Tloc,Tdat>& n2) {
				return n1.y < n2.y;
		} );
		last = std::unique(dataVtmp.begin(), dataVtmp.end(), [&](const Node<Tloc,Tdat>& n1, const Node<Tloc,Tdat>& n2) {
			return isEqual(n1.y, n2.y);
		} );
		dataVtmp.erase(last, dataVtmp.end());
		ymin = dataVtmp.front().y;
		ymax = dataVtmp.back().y;
		ygrd = dataVtmp[1].y - dataVtmp[0].y;
		for(int i=2; i<dataVtmp.size(); i++) {
			Tloc ygrdc = dataVtmp[i].y - dataVtmp[i-1].y;
			if( ygrd > ygrdc ) ygrd = ygrdc;
		}
	}

	// hash points to grids
	void Hash( const std::vector< Node<Tloc,Tdat> >& dataV ) {
		int nx = (int)ceil( (xmax-xmin) / xgrd );
		int ny = (int)ceil( (ymax-ymin) / ygrd );
		this->clear(); this->resize(nx, ny, NaN);
		dataavg = Tdat(0);
		for( const auto& node : dataV ) {
			Tloc ix = (node.x-xmin) / xgrd;
			Tloc iy = (node.y-ymin) / ygrd;
			(*this)(toSize(ix), toSize(iy)) = node.data;
			dataavg += node.data;
		}
		dataavg /= dataV.size();
	}

	// indexing
	inline bool isEqual(Tloc v1, Tloc v2) { 
		Tloc diff = fabs(v1 - v2);
		return diff<toler || diff<fabs(v1)*toler;
	}

	inline size_t toSize(Tloc val) {
		size_t i = (size_t)floor(val+0.5);
		if( ! isEqual(val, i) )
			throw std::runtime_error("irregular grid");
		return i;
	}

	inline int xindex( Tloc xloc ) const { return (int)floor((xloc-xmin) / xgrd + 0.5); }
	inline int yindex( Tloc yloc ) const { return (int)floor((yloc-ymin) / ygrd + 0.5); }
	inline int xindex_floor( Tloc xloc ) const { return (int)floor((xloc+toler-xmin) / xgrd); }
	inline int yindex_floor( Tloc yloc ) const { return (int)floor((yloc+toler-ymin) / ygrd); }
	inline int xindex_ceil( Tloc xloc ) const { return (int)ceil((xloc-toler-xmin) / xgrd); }
	inline int yindex_ceil( Tloc yloc ) const { return (int)ceil((yloc-toler-ymin) / ygrd); }

	inline Tloc X( const size_t ix ) const { return xmin + ix*xgrd; }
	inline Tloc Y( const size_t iy ) const { return ymin + iy*ygrd; }
	inline Node<Tloc,Tdat> getNode( const size_t ix, const size_t iy ) const {
		return Node<Tloc,Tdat>( X(ix), Y(iy), (*this)(ix, iy) );
	}
};

#endif
