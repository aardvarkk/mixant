#include "mix.h"

#include <string>

std::ostream& operator<<(std::ostream& out, const Mix& mix)
{
  for (auto s : mix.steps) {
    out << s << std::endl;
  }
  return out;
}
