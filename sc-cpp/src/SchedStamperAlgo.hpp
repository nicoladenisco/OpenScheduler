#ifndef __SCHEDSTAMPERALGO_HPP
#define __SCHEDSTAMPERALGO_HPP

#include "SchedResource.hpp"
#include "common.hpp"
#include <memory>

class SchedStamperAlgo {
public:
  SchedStamperAlgo() {}
  virtual ~SchedStamperAlgo() {}

  virtual void apply(SchedResource &resource, AnyStringMap &properties) = 0;

protected:
  virtual void applyCommon(int dayStart, int dayStop, SchedResource &resource,
                           AnyStringMap &properties);
  virtual void applyCommon(IntVector days, SchedResource &resource,
                           AnyStringMap &properties);
};

using SchedStamperAlgoPtr = std::shared_ptr<SchedStamperAlgo>;
using SchedStamperAlgoPtrVector = std::vector<SchedStamperAlgoPtr>;
using SchedStamperAlgoPtrMap = std::map<String, SchedStamperAlgoPtr>;

class DailyStamper : public SchedStamperAlgo {
public:
  DailyStamper();
  ~DailyStamper();

  virtual void apply(SchedResource &resource, AnyStringMap &properties);
};

class FreeStamper : public SchedStamperAlgo {
public:
  FreeStamper();
  ~FreeStamper();

  virtual void apply(SchedResource &resource, AnyStringMap &properties);
};

#endif
