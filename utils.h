#ifndef UTILS_H
#define UTILS_H

#include <vector>

class Utils
{
public:

  static double GetSemitoneRatio();
  static double GetCentRatio();
  
  template <typename T> static int sgn(T val) 
  { 
    return (T(0) < val) - (val < T(0));
  }
};

#endif