#ifndef __SHEDSTAMPER_HPP
#define __SHEDSTAMPER_HPP

#include "SchedResource.hpp"
#include "SchedStamperAlgo.hpp"
#include "common.hpp"

class SchedStamper {
public:
  SchedStamper();
  virtual ~SchedStamper();

  virtual void stampResource(SchedResource &resource, String nomeAlgoritmo,
                             AnyStringMap &properties);
  virtual void stampResource(SchedResource &resource, SchedStamperAlgo &algo,
                             AnyStringMap &properties);

  virtual void getAlgoNames(StringVector &names) const;

private:
  void __buildStampersTable();

protected:
  SchedStamperAlgoPtrMap stampers;
};

#define BEGIN_TABLE_ALGOS() void SchedStamper::__buildStampersTable() {

#define ADD_ALGO(nome, classe) stampers[nome] = std::make_shared<classe>();

#define END_TABLE_ALGOS() }

#endif
