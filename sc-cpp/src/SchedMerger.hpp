#ifndef __SCHEDMERGER_HPP
#define __SCHEDMERGER_HPP

#include "Properties.hpp"
#include "SchedMergerAlgo.hpp"
#include "SchedResource.hpp"
#include "dataStructure.hpp"

class SchedMerger {
public:
  SchedMerger(u_int64_t __idUniqueLoock);
  virtual ~SchedMerger();

  virtual void addResource(SchedResourceMultiLock &multilock,
                           SchedResourcePtr resource, String nomeAlgoritmo,
                           Properties &properties);

  virtual int checkRisorsa(String codice);
  virtual void reserveSlot(SchedResourceMultiLock &multilock, int giorno,
                           int slotgiorno, u_int64_t uniqueid,
                           Properties &properties);

  virtual void clear();
  virtual void getAlgoNames(StringVector &names) const;
  virtual void getResourcesCode(StringVector &names) const;
  virtual const SlotFile *getMerged() const { return merged; }
  virtual String toString();
  virtual void populateHeaderProp(Properties &properties);

private:
  void __buildMergersTable();
  void reserveSlotWorker(SlotFile *sf, int giorno, int slotgiorno,
                         u_int64_t uniqueid, Properties &properties);

protected:
  SchedResourcePtrVector resources;
  SchedMergerAlgoPtrMap mergers;
  SlotFile *merged;
  u_int64_t idUniqueLoock;
};

#define BEGIN_TABLE_MERGER_ALGOS() void SchedMerger::__buildMergersTable() {

#define ADD_MERGER_ALGO(nome, classe)                                          \
  mergers[nome] = std::make_shared<classe>();

#define END_TABLE_MERGER_ALGOS() }

#endif // __SCHEDMERGER_HPP
