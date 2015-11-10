#ifndef MAP_H
#define MAP_H

#include "Point.h"
#include "Array2D.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

#ifndef FuncName
#define FuncName __FUNCTION__
#endif


template < class T >
class DataPoint : public Point<T> {
public:
   T dis, data;

public:
   DataPoint(const Point<T> ptin, const T datain = -12345., const T disin = -12345. )
      : Point<T>(ptin)
      , dis(disin), data(datain) {}

   DataPoint(const T lonin = -12345., const T latin = -12345., const T datain = -12345., const T disin = -12345. )
      : Point<T>(lonin, latin)
      , dis(disin), data(datain) {}

   inline const T& Dis() const { return dis; }
   inline	T& Dis() { return dis; }
   inline const T& Data() const { return data; }
   inline	T& Data() { return data; }

   friend std::ostream& operator << (std::ostream& o, DataPoint a) { 
      o << a.lon << " " << a.lat << " " <<a.data<<" "<<a.dis; 
      return o; 
   }

};


class Map {
public:
   //Map( const std::string& inname );
   Map( const std::string& inname, const double grdlon = 1.0, const double grdlat = 1.0 );
   Map( const Map& );
   Map( Map&& );
   Map& operator= ( const Map& );
   Map& operator= ( Map&& );
   ~Map();

	/* ----- map boundaries ----- */
	double LonMin() const;
	double LonMax() const;
	double LatMin() const;
	double LatMax() const;

	/* ------------ IO and resets ------------ */
	void Load( const std::string& fnamein );

	/* --- clip the map around the source location (to speed up the average methods) --- */
	void Clip( const double lonmin, const double lonmax, const double latmin, const double latmax );

	/* ------------ compute number of points near the given location ------------ */
	int NumberOfPoints(Point<double> rec, const double xhdis, const double yhdis) const {
		double loneff, lateff;
		NumberOfPoints( rec, xhdis, yhdis, loneff, lateff );
	}
	int NumberOfPoints(Point<double> rec, const double xhdis, const double yhdis, double& loneff, double& lateff) const;

   /* ------------ compute average value on the point rec ------------ */
   double PointAverage(Point<double> Prec, double hdis ) {
      double weit;
      return PointAverage(Prec, hdis, weit);
   }
   double PointAverage(Point<double> rec, double hdis, double& weit);
  
protected:
	static constexpr double NaN = -12345.;

private:
	std::string fname;
	Point<double> src;
   struct Mimpl;
   std::unique_ptr<Mimpl> pimplM;
};

namespace ErrorM {
   class Base : public std::runtime_error {
   public:
      Base(const std::string message)
         : runtime_error(message) {
            //PrintStacktrace();
      }
   };

	class BadFile : public Base {
   public:
      BadFile(const std::string funcname, const std::string info = "")
         : Base("Error("+funcname+"): Cannot access file ("+info+").") {}
   };

	class BadParam : public Base {
   public:
      BadParam(const std::string funcname, const std::string info = "")
         : Base("Error("+funcname+"): Bad parameters ("+info+").") {}
   };

}

#endif
