#include <cassert>
#include <cmath>
#include <exception>

#include "key.h"
#include "utils.h"

static Keys keys;
static Keys ordering_min;
static Keys ordering_maj;

Keys const& Key::GetKeys()
{
  if (!keys.empty()) {
    return keys;
  }

  keys.push_back(Key(1,  Key::kMinor, "A-Flat Minor", "Abm", "G#m"));
  keys.push_back(Key(1,  Key::kMajor, "B Major", "B", ""));
  keys.push_back(Key(2,  Key::kMinor, "E-Flat Minor", "Ebm", "D#m"));
  keys.push_back(Key(2,  Key::kMajor, "F-Sharp Major", "F#", "Gb"));
  keys.push_back(Key(3,  Key::kMinor, "B-Flat Minor", "Bbm", "A#m"));
  keys.push_back(Key(3,  Key::kMajor, "D-Flat Major", "Db", "C#"));
  keys.push_back(Key(4,  Key::kMinor, "F Minor", "Fm", ""));
  keys.push_back(Key(4,  Key::kMajor, "A-Flat Major", "Ab", "G#"));
  keys.push_back(Key(5,  Key::kMinor, "C Minor", "Cm", ""));
  keys.push_back(Key(5,  Key::kMajor, "E-Flat Major", "Eb", "D#"));
  keys.push_back(Key(6,  Key::kMinor, "G Minor", "Gm", ""));
  keys.push_back(Key(6,  Key::kMajor, "B-Flat Major", "Bb", "A#"));
  keys.push_back(Key(7,  Key::kMinor, "D Minor", "Dm", ""));
  keys.push_back(Key(7,  Key::kMajor, "F Major", "F", ""));
  keys.push_back(Key(8,  Key::kMinor, "A Minor", "Am", ""));
  keys.push_back(Key(8,  Key::kMajor, "C Major", "C", ""));
  keys.push_back(Key(9,  Key::kMinor, "E Minor", "Em", ""));
  keys.push_back(Key(9,  Key::kMajor, "G Major", "G", ""));
  keys.push_back(Key(10, Key::kMinor, "B Minor", "Bm", ""));
  keys.push_back(Key(10, Key::kMajor, "D Major", "D", ""));
  keys.push_back(Key(11, Key::kMinor, "F-Sharp Minor", "F#m", "Gbm"));
  keys.push_back(Key(11, Key::kMajor, "A Major", "A", ""));
  keys.push_back(Key(12, Key::kMinor, "D-Flat Minor", "Dbm", "C#m"));
  keys.push_back(Key(12, Key::kMajor, "E Major", "E", ""));

  return keys;
}

Keys const& Key::GetOrdering(Key::Type type)
{
  switch (type) {
  case Key::kMinor:
    if (!ordering_min.empty()) {
      return ordering_min;
    }
    ordering_min.push_back(Key::GetKey(5,  Key::kMinor));
    ordering_min.push_back(Key::GetKey(12, Key::kMinor));
    ordering_min.push_back(Key::GetKey(7,  Key::kMinor));
    ordering_min.push_back(Key::GetKey(2,  Key::kMinor));
    ordering_min.push_back(Key::GetKey(9,  Key::kMinor));
    ordering_min.push_back(Key::GetKey(4,  Key::kMinor));
    ordering_min.push_back(Key::GetKey(11, Key::kMinor));
    ordering_min.push_back(Key::GetKey(6,  Key::kMinor));
    ordering_min.push_back(Key::GetKey(1,  Key::kMinor));
    ordering_min.push_back(Key::GetKey(8,  Key::kMinor));
    ordering_min.push_back(Key::GetKey(3,  Key::kMinor));
    ordering_min.push_back(Key::GetKey(10, Key::kMinor));
    return ordering_min;
  case Key::kMajor:
    if (!ordering_maj.empty()) {
      return ordering_maj;
    }
    ordering_maj.push_back(Key::GetKey(8,  Key::kMajor));
    ordering_maj.push_back(Key::GetKey(3,  Key::kMajor));
    ordering_maj.push_back(Key::GetKey(10, Key::kMajor));
    ordering_maj.push_back(Key::GetKey(5,  Key::kMajor));
    ordering_maj.push_back(Key::GetKey(12, Key::kMajor));
    ordering_maj.push_back(Key::GetKey(7,  Key::kMajor));
    ordering_maj.push_back(Key::GetKey(2,  Key::kMajor));
    ordering_maj.push_back(Key::GetKey(9,  Key::kMajor));
    ordering_maj.push_back(Key::GetKey(4,  Key::kMajor));
    ordering_maj.push_back(Key::GetKey(11, Key::kMajor));
    ordering_maj.push_back(Key::GetKey(6,  Key::kMajor));
    ordering_maj.push_back(Key::GetKey(1,  Key::kMajor));
    return ordering_maj;
  }

  throw "Invalid key type";
}

Key Key::KeyFromString(std::string const& str)
{
  Keys keys = GetKeys();
  
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

bool Key::operator==(Key const& key) const
{
  return (this->num == key.num && this->type == key.type);
}

Key Key::operator+(int semitones) const
{
  // Convert to a positive value (since we move in a circle)
  if (semitones < 0) {
    semitones = 12 - (abs(semitones) % 12);
  }

  int key_index = GetKeyIndex(*this);
  int key_shifted = (key_index + 2 * semitones) % GetKeys().size();
  return GetKeys()[key_shifted];
}

Key Key::operator-(int semitones) const
{
  return operator+(-semitones);
}

bool Key::operator<(Key const& k) const
{
  if (this->num > k.num) {
    return false;
  } else if (this->num == k.num) {
    return this->type < k.type;
  } else {
    return true;
  }
}

int Key::GetKeyIndex(Key const& key)
{
  for (size_t i = 0; i < GetKeys().size(); ++i) {
    if (key == GetKeys()[i]) {
      return i;
    }
  }
  return 0;
}

Key Key::GetShiftedKey(Key const& original_key, double old_bpm, double new_bpm)
{
  double bpm_ratio = new_bpm / old_bpm;
  double bpm_ratio_st = log(bpm_ratio) / log(Utils::GetSemitoneRatio());

  // Cast to an integer to get how many keys to shift...
  int shift = static_cast<int>(rint(bpm_ratio_st));
  return original_key + shift;
}

// An UNSIGNED distance between two keys
int Key::GetCamelotDistance(Key const& k1, Key const& k2)
{
  int diff = abs(k2.num - k1.num);
  diff = diff > 6 ? 12 - abs(diff) : diff;
  return diff + abs(k1.type - k2.type);
}

int Key::GetTransposeDistance(Key const& k1, Key const& k2)
{
  // Must be of the same type!
  assert(k1.type == k2.type);

  // Find the two indices for each key, then calculate modular distance
  int i1, i2;
  int idx = 0;
  for (auto k : GetOrdering(k1.type)) {
    if (k == k1) {
      i1 = idx;
    }
    if (k == k2) {
      i2 = idx;
    }
    idx++;
  }

  int diff = i2 - i1;
  return abs(diff) > 6 ? diff - Utils::sgn(diff) * 12 : diff;
}

void Key::GetCompatibleKeys(Key const& key, Keys& compatible)
{
  compatible.resize(4);

  int  root_num  = key.num;
  Key::Type root_type = key.type;

  // Trivial compatible one is the same key
  compatible[0] = key;

  // Next compatible one is the min-maj opposite of this key
  compatible[1] = Key::GetKey(root_num, static_cast<Key::Type>(!root_type));

  // Now shift it + and - 1 semiton from its existing type
  compatible[2] = key + 1;
  compatible[3] = key - 1;
}

bool Key::AreCompatibleKeys(Key const& k1, Key const& k2)
{
  return GetCamelotDistance(k1, k2) <= 1;
}

Key Key::GetKey(int num, Key::Type type)
{
  for (auto k : GetKeys()) {
    if (k.num == num && k.type == type) {
      return k;
    }
  }

  throw "Invalid key: " + num + type;
}
