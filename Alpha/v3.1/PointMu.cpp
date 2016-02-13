#ifndef HEADER_HPP
#include "Header.hpp"
#endif

PointMu::PointMu()
{
  alpha = 'A';
  pt.X = 0;
  pt.Y = 0;
  ticks = 0;
}

PointMu& PointMu::operator=(const PointMu& rhs)
{
  if(this != &rhs){
    alpha = rhs.alpha;
    ticks = rhs.ticks;
    pt.X = rhs.pt.X;
    pt.Y = rhs.pt.Y;
  }
  return *this;
}

PointMu::PointMu(const PointMu& rhs)
{
  alpha = rhs.alpha;
  ticks = rhs.ticks;
  pt.X = rhs.pt.X;
  pt.Y = rhs.pt.Y;
}

