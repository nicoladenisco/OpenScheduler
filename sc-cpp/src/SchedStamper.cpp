#include "SchedStamper.hpp"
#include "SchedStamperAlgo.hpp"

BEGIN_TABLE_STAMPER_ALGOS()
ADD_STAMPER_ALGO("daily", DailyStamper)
ADD_STAMPER_ALGO("free", FreeStamper)
END_TABLE_STAMPER_ALGOS()

SchedStamper::SchedStamper() { __buildStampersTable(); }

SchedStamper::~SchedStamper() {}

void SchedStamper::stampResource(SchedResource &resource,
                                 SchedStamperAlgo &algo,
                                 AnyStringMap &properties) {
  algo.apply(resource, properties);
}

void SchedStamper::stampResource(SchedResource &resource, String nomeAlgoritmo,
                                 AnyStringMap &properties) {

  auto it = stampers.find(nomeAlgoritmo);
  if (it == stampers.end())
    throw StructureException("Algoritmo specificato non esiste.");

  SchedStamperAlgoPtr algo = it->second;
  stampResource(resource, *algo, properties);
}

void SchedStamper::getAlgoNames(StringVector &names) const {
  std::transform(std::begin(stampers), std::end(stampers),
                 std::back_inserter(names),
                 [](auto const &pair) { return pair.first; });
}
