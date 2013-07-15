#ifndef MIX_H
#define MIX_H

#include <iostream>
#include <vector>

typedef std::string MixStep;
typedef std::vector<MixStep> MixSteps;

struct Mix
{
  MixSteps steps;

  friend std::ostream& operator<<(std::ostream& out, const Mix& mix);
};

#endif