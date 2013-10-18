#include <cmath>

#include "utils.h"

double Utils::GetSemitoneRatio()
{ 
  return pow(2, 1.0 / 12); 
}

double Utils::GetCentRatio()
{ 
  return pow(2, 1.0 / 1200); 
}