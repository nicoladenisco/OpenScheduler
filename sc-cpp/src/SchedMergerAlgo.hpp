#ifndef __SCHEDMERGERALGO_HPP
#define __SCHEDMERGERALGO_HPP

#include "SchedResource.hpp"
#include "common.hpp"
#include "dataStructure.hpp"
#include <memory>

class SchedMergerAlgo {
public:
  SchedMergerAlgo() {}
  virtual ~SchedMergerAlgo() {}

  virtual int checkCompatibilita(SlotFile *merged, SchedResource &tomerge,
                                 bool thexcpt = true);
  virtual SlotFile *inizializzaMerge(SlotFile *merged, SchedResource &tomerge);

  virtual SlotFile *apply(SlotFile *merged, SchedResource &tomerge,
                          SchedResourcePtrVector otherResources,
                          u_int64_t idUniqueLoock,
                          AnyStringMap &properties) = 0;

  virtual SlotFile *sbloccaAltri(SlotFile *merged, SchedResource &tomerge,
                                 SchedResourcePtrVector otherResources,
                                 u_int64_t idUniqueLoock,
                                 AnyStringMap &properties);
};

using SchedMergerAlgoPtr = std::shared_ptr<SchedMergerAlgo>;
using SchedMergerAlgoPtrVector = std::vector<SchedMergerAlgoPtr>;
using SchedMergerAlgoPtrMap = std::map<String, SchedMergerAlgoPtr>;

class DefaultMerger : public SchedMergerAlgo {
public:
  DefaultMerger() {}
  virtual ~DefaultMerger() {}

  virtual SlotFile *apply(SlotFile *merged, SchedResource &tomerge,
                          SchedResourcePtrVector otherResources,
                          u_int64_t idUniqueLoock, AnyStringMap &properties);
};

#endif // __SCHEDMERGERALGO_HPP
