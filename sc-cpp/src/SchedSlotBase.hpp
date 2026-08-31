#ifndef __SCHEDSLOTBASE_HPP
#define __SCHEDSLOTBASE_HPP

#include "Properties.hpp"
#include "dataStructure.hpp"

class SchedSlotBase {
public:
  SchedSlotBase();
  virtual ~SchedSlotBase();

protected:
  virtual void reserveSlotWorker(SlotFile *sf, int giorno, int slotgiorno,
                                 u_int64_t uniqueid, Properties &properties);

  virtual void findFreeSlotWorker(SlotFile *sf, Properties &properties,
                                  IntPairVector &risultati);

  virtual void clearSlotWorker(SlotFile *sf, int giorno, int slotgiorno,
                               u_int64_t uniqueid, u_char statusClear,
                               Properties &properties);

  virtual void clearSlotWorker(SlotFile *sf, u_int64_t uniqueid,
                               u_char statusClear, Properties &properties);

  virtual String dumpXmlWorker(SlotFile *sf, String rootName,
                               Properties &properties);
};

#endif // __SCHEDSLOTBASE_HPP
