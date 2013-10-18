#include <boost/math/special_functions.hpp>
#include <cmath>
#include <exception>

#include "camelot.h"
#include "utils.h"

static Camelot::Keys keys;
static Camelot::Keys ordering_min;
static Camelot::Keys ordering_maj;

Camelot::Keys const& Camelot::GetKeys()
{
  if (!keys.empty()) {
    return keys;
  }

  keys.push_back(Key(1, kMinor, "A-Flat Minor", "Abm", "G#m"));
  keys.push_back(Key(1, kMajor, "B Major", "B", ""));
  keys.push_back(Key(2, kMinor, "E-Flat Minor", "Ebm", "D#m"));
  keys.push_back(Key(2, kMajor, "F-Sharp Major", "F#", "Gb"));
  keys.push_back(Key(3, kMinor, "B-Flat Minor", "Bbm", "A#m"));
  keys.push_back(Key(3, kMajor, "D-Flat Major", "Db", "C#"));
  keys.push_back(Key(4, kMinor, "F Minor", "Fm", ""));
  keys.push_back(Key(4, kMajor, "A-Flat Major", "Ab", "G#"));
  keys.push_back(Key(5, kMinor, "C Minor", "Cm", ""));
  keys.push_back(Key(5, kMajor, "E-Flat Major", "Eb", "D#"));
  keys.push_back(Key(6, kMinor, "G Minor", "Gm", ""));
  keys.push_back(Key(6, kMajor, "B-Flat Major", "Bb", "A#"));
  keys.push_back(Key(7, kMinor, "D Minor", "Dm", ""));
  keys.push_back(Key(7, kMajor, "F Major", "F", ""));
  keys.push_back(Key(8, kMinor, "A Minor", "Am", ""));
  keys.push_back(Key(8, kMajor, "C Major", "C", ""));
  keys.push_back(Key(9, kMinor, "E Minor", "Em", ""));
  keys.push_back(Key(9, kMajor, "G Major", "G", ""));
  keys.push_back(Key(10, kMinor, "B Minor", "Bm", ""));
  keys.push_back(Key(10, kMajor, "D Major", "D", ""));
  keys.push_back(Key(11, kMinor, "F-Sharp Minor", "F#m", "Gbm"));
  keys.push_back(Key(11, kMajor, "A Major", "A", ""));
  keys.push_back(Key(12, kMinor, "D-Flat Minor", "Dbm", "C#m"));
  keys.push_back(Key(12, kMajor, "E Major", "E", ""));

  return keys;
}

Camelot::Keys const& Camelot::GetOrdering(Camelot::Type type)
{
  switch (type) {
  case kMinor:
    if (!ordering_min.empty()) {
      return ordering_min;
    }
    ordering_min.push_back(Camelot::GetKey(5,  kMinor));
    ordering_min.push_back(Camelot::GetKey(12, kMinor));
    ordering_min.push_back(Camelot::GetKey(7,  kMinor));
    ordering_min.push_back(Camelot::GetKey(2,  kMinor));
    ordering_min.push_back(Camelot::GetKey(9,  kMinor));
    ordering_min.push_back(Camelot::GetKey(4,  kMinor));
    ordering_min.push_back(Camelot::GetKey(11, kMinor));
    ordering_min.push_back(Camelot::GetKey(6,  kMinor));
    ordering_min.push_back(Camelot::GetKey(1,  kMinor));
    ordering_min.push_back(Camelot::GetKey(8,  kMinor));
    ordering_min.push_back(Camelot::GetKey(3,  kMinor));
    ordering_min.push_back(Camelot::GetKey(10, kMinor));
    return ordering_min;
  case kMajor:
    if (!ordering_maj.empty()) {
      return ordering_maj;
    }
    ordering_maj.push_back(Camelot::GetKey(8,  kMajor));
    ordering_maj.push_back(Camelot::GetKey(3,  kMajor));
    ordering_maj.push_back(Camelot::GetKey(10, kMajor));
    ordering_maj.push_back(Camelot::GetKey(5,  kMajor));
    ordering_maj.push_back(Camelot::GetKey(12, kMajor));
    ordering_maj.push_back(Camelot::GetKey(7,  kMajor));
    ordering_maj.push_back(Camelot::GetKey(2,  kMajor));
    ordering_maj.push_back(Camelot::GetKey(9,  kMajor));
    ordering_maj.push_back(Camelot::GetKey(4,  kMajor));
    ordering_maj.push_back(Camelot::GetKey(11, kMajor));
    ordering_maj.push_back(Camelot::GetKey(6,  kMajor));
    ordering_maj.push_back(Camelot::GetKey(1,  kMajor));
    return ordering_maj;
  }

  throw "Invalid key type";
}

Camelot::Key Camelot::KeyFromString(std::string const& str)
{
  Keys keys = Camelot::GetKeys();
  
  for (auto i : keys) {
    if (!i.name.compare(str)) {
      return i;
    } else if (!i.short_name.compare(str)) {
      return i;
    } else if (!i.alternate.compare(str)) {
      return i;
    }
  }

  throw "Invalid key: " + str;
}

bool Camelot::Key::operator==(Camelot::Key const& key) const
{
  return (this->num == key.num && this->type == key.type);
}

Camelot::Key Camelot::Key::operator+(int semitones) const
{
  // Convert to a positive value (since we move in a circle)
  if (semitones < 0) {
    semitones = 12 - (abs(semitones) % 12);
  }

  int key_index = Camelot::GetKeyIndex(*this);
  int key_shifted = (key_index + 2 * semitones) % GetKeys().size();
  return GetKeys()[key_shifted];
}

Camelot::Key Camelot::Key::operator-(int semitones) const
{
  return operator+(-semitones);
}

bool Camelot::Key::operator<(Camelot::Key const& k) const
{
  if (this->num > k.num) {
    return false;
  } else if (this->num == k.num) {
    return this->type < k.type;
  } else {
    return true;
  }
}

int Camelot::GetKeyIndex(Camelot::Key const& key)
{
  for (size_t i = 0; i < GetKeys().size(); ++i) {
    if (key == GetKeys()[i]) {
      return i;
    }
  }
  return 0;
}

Camelot::Key Camelot::GetShiftedKey(Camelot::Key const& original_key, double old_bpm, double new_bpm)
{
  double bpm_ratio = new_bpm / old_bpm;
  double bpm_ratio_st = log(bpm_ratio) / log(Utils::GetSemitoneRatio());

  // Cast to an integer to get how many keys to shift...
  int shift = static_cast<int>(boost::math::iround(bpm_ratio_st));
  return original_key + shift;
}

int Camelot::GetCamelotDistance(Camelot::Key const& k1, Camelot::Key const& k2)
{
  int diff = abs(k2.num - k1.num);
  diff = diff > 6 ? 12 - abs(diff) : diff;
  return diff + abs(k1.type - k2.type);
}

// Use a lookup: http://community.mixedinkey.com/Topics/6718
int Camelot::GetTransposeDistance(Camelot::Key const& k1, Camelot::Key const& k2)
{
  // Must be of the same type!
  assert(k1.type == k2.type);

  // Get the ordering
  Keys ordering = Camelot::GetOrdering(k1.type);

  // Find the two indices for each key, then calculate modular distance
  int i1, i2;
  int idx = 0;
  for (auto k : ordering) {
    if (k == k1) {
      i1 = idx;
    }
    if (k == k2) {
      i2 = idx;
    }
    idx++;
  }

  int diff = i2 - i1;
  return abs(diff) > 6 ? abs(diff) - 12 : diff;
}

Camelot::Keys Camelot::GetCompatibleKeys(Camelot::Key const& key)
{
  Keys keys;

  int  root_num  = key.num;
  Type root_type = key.type;

  // Trivial compatible one is the same key
  keys.push_back(Camelot::GetKey(root_num, root_type));

  // Next compatible one is the min-maj opposite of this key
  keys.push_back(Camelot::GetKey(root_num, static_cast<Type>(!root_type)));

  // Now shift it + and - 1 semiton from its existing type
  keys.push_back(key + 1);
  keys.push_back(key - 1);

  return keys;
}

bool Camelot::AreCompatibleKeys(Camelot::Key const& k1, Camelot::Key const& k2)
{
  return Camelot::GetCamelotDistance(k1, k2) <= 1;
}

Camelot::Key Camelot::GetKey(int num, Type type)
{
  for (auto k : GetKeys()) {
    if (k.num == num && k.type == type) {
      return k;
    }
  }

  throw "Invalid key: " + num + type;
}
