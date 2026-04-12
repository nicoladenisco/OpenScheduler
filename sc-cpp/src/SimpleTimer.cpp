#include "SimpleTimer.hpp"
#include <boost/chrono.hpp>
#include <boost/chrono/duration.hpp>
#include <boost/thread.hpp>

SimpleTimer::SimpleTimer() { reset(); }

SimpleTimer::~SimpleTimer() {}

void SimpleTimer::reset() {
  startTime = parziale = boost::chrono::system_clock::now();
}

long SimpleTimer::getElapsedMillis() const {
  boost::chrono::duration<double> sec = getElapsed();
  double secondi = sec.count();
  return secondi * 1000.0;
}

long SimpleTimer::getParzialeMillis() const {
  boost::chrono::duration<double> sec = getParziale();
  double secondi = sec.count();
  return secondi * 1000.0;
}

long SimpleTimer::getParzialeMillisAndReset() {
  boost::chrono::duration<double> sec = getParzialeAndReset();
  double secondi = sec.count();
  return secondi * 1000.0;
}

long SimpleTimer::showElapsed(String prompt) {
  long tempo = getElapsedMillis();
  if (tempo == 0)
    cout << prompt << " eseguito in meno di un millisecondo.\n";
  else
    cout << prompt << " eseguito in " << tempo << " millisecondi.\n";
  return tempo;
}
