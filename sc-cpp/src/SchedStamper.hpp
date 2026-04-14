#ifndef __SHEDSTAMPER_HPP
#define __SHEDSTAMPER_HPP

#include "Properties.hpp"
#include "SchedResource.hpp"
#include "SchedStamperAlgo.hpp"
#include "common.hpp"

class SchedStamper {
public:
  SchedStamper();
  virtual ~SchedStamper();

  virtual void stampResource(SchedResource &resource, String nomeAlgoritmo,
                             Properties &properties);
  virtual void stampResource(SchedResource &resource, SchedStamperAlgo &algo,
                             Properties &properties);

  virtual void getAlgoNames(StringVector &names) const;

private:
  void __buildStampersTable();

protected:
  SchedStamperAlgoPtrMap stampers;
};

#define BEGIN_TABLE_STAMPER_ALGOS() void SchedStamper::__buildStampersTable() {

#define ADD_STAMPER_ALGO(nome, classe)                                         \
  stampers[nome] = std::make_shared<classe>();

#define END_TABLE_STAMPER_ALGOS() }

#endif
