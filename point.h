#ifndef _POINT_H_
#define _POINT_H_

#include <stdint.h>

class point
{
public:
	point() : x_(0), y_(0) { };
	point(int32_t x, int32_t y): x_(x), y_(y) {};
	
	int32_t x() {return x_;}
	int32_t y() {return y_;}
	
	bool operator==(const point &p) { return (x_==p.x_) && (y_==p.y_); }
	
private:
	int32_t x_;
	int32_t y_;
};

#endif  /* _POINT_H_ */