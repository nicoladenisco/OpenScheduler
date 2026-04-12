#ifndef __SIMPLE_TIMER_HPP
#define __SIMPLE_TIMER_HPP

#include "common.hpp"
#include <boost/chrono.hpp>

class SimpleTimer {
public:
  SimpleTimer();
  virtual ~SimpleTimer();

  void reset();
  boost::chrono::duration<double> getElapsed() const {
    return boost::chrono::system_clock::now() - startTime;
  }
  boost::chrono::duration<double> getParziale() const {
    return boost::chrono::system_clock::now() - parziale;
  }
  boost::chrono::duration<double> getParzialeAndReset() {
    boost::chrono::duration<double> rv = getParziale();
    parziale = boost::chrono::system_clock::now();
    return rv;
  }

  bool isElapsed(long millis) const { return getElapsedMillis() > millis; }

  long getElapsedMillis() const;
  long getParzialeMillis() const;
  long getParzialeMillisAndReset();

  long showElapsed(String prompt);

  boost::chrono::system_clock::time_point startTime, parziale;
};

#endif // __SIMPLE_TIMER_HPP
