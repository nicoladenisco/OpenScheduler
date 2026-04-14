#ifndef __PROPERTIES_H
#define __PROPERTIES_H

#include "common.hpp"
#include <boost/lexical_cast.hpp>

class Properties : public StringMap {
public:
  Properties() {}
  Properties(const StringVector &vstr) { vector2Properties(vstr); }

  inline const String &get(const String &key, const String &defVal) const {
    auto it = find(key);
    return it == end() ? defVal : it->second;
  }

  inline int get(const String &key, int defVal) const {
    try {
      return boost::lexical_cast<int>(get(key, ""));
    } catch (boost::bad_lexical_cast &) {
      return defVal;
    }
  }

  inline float get(const String &key, float defVal) const {
    try {
      return boost::lexical_cast<float>(get(key, ""));
    } catch (boost::bad_lexical_cast &) {
      return defVal;
    }
  }

  inline double get(const String &key, double defVal) const {
    try {
      return boost::lexical_cast<double>(get(key, ""));
    } catch (boost::bad_lexical_cast &) {
      return defVal;
    }
  }

  int vector2Properties(const StringVector &vstr) {
    int count = 0;
    for (auto str : vstr) {
      std::string::size_type found = str.find("=");
      if (found != String::npos) {
        (*this)[str.substr(0, found)] = str.substr(found + 1);
        count++;
      }
    }
    return count;
  }

  String toString(String separator = ",") const {
    String rv;
    rv.reserve(1024);

    for (auto it : *this) {
      if (!rv.empty())
        rv.append(separator);
      rv.append(it.first + "=" + it.second);
    }

    return rv;
  }

  std::pair<int, int> parseDays() {
    int dayStart = get("daystart", 0);
    int dayStop = get("daystop", 365);

    if (dayStart < 0 || dayStart > 365 || dayStop < dayStart || dayStop > 365)
      throw GenericException("Valori non corretti per daystart/daystop.");

    return std::make_pair(dayStart, dayStop);
  }
};

#endif // __PROPERTIES_H
