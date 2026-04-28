#include "SchedMerger.hpp"
#include "SchedMergerAlgo.hpp"
#include "SchedResource.hpp"
#include "dataStructure.hpp"

BEGIN_TABLE_MERGER_ALGOS()
ADD_MERGER_ALGO("default", DefaultMerger)
END_TABLE_MERGER_ALGOS()

SchedMerger::SchedMerger(u_int64_t __idUniqueLoock)
    : merged(nullptr), idUniqueLoock(__idUniqueLoock) {
  __buildMergersTable();
}

SchedMerger::~SchedMerger() {
  clear();
  if (merged != nullptr)
    free(merged);
}

void SchedMerger::getAlgoNames(StringVector &names) const {
  std::transform(std::begin(mergers), std::end(mergers),
                 std::back_inserter(names),
                 [](auto const &pair) { return pair.first; });
}

void SchedMerger::getResourcesCode(StringVector &names) const {
  for (auto r : resources) {
    names.push_back(r->getSlotFile()->codiceRisorsa);
  }
}

void SchedMerger::addResource(SchedResourceMultiLock &multilock,
                              SchedResourcePtr resource, String nomeAlgoritmo,
                              Properties &properties) {

  auto it = mergers.find(nomeAlgoritmo);
  if (it == mergers.end())
    throw StructureException("Algoritmo specificato non esiste.");

  if (!resource->isInitialized())
    throw StructureException("la risorsa indicata non è stata inizializzata.");

  // blocca tutte le risorse
  multilock.addResource(resource);
  for (auto prev : resources)
    multilock.addResource(prev);

  if (!multilock.look())
    throw StructureException("Non riesco a bloccare le risorse indicate.");

  SchedMergerAlgoPtr algo = it->second;
  merged = algo->apply(merged, *resource, resources, idUniqueLoock, properties);
  resources.push_back(resource);
}

void SchedMerger::clear() {
  for (auto res : resources) {
    slotType *ptc = res->getSlotFile()->arrySlot;

    for (int i = 0; i < merged->numSlotsTotali; i++, ptc++) {
      if (ptc->status == SLOT_LOOKED && ptc->info == idUniqueLoock) {
        ptc->status = SLOT_SCHEDULABLE;
        ptc->info = 0;
      }
    }
  }
  resources.clear();
}

int SchedMerger::checkRisorsa(String codice) {
  for (auto r : resources) {
    if (codice == r->getSlotFile()->codiceRisorsa)
      return 1;
  }
  return 0;
}

String SchedMerger::toString() {
  if (resources.empty())
    return "Nessuna risorsa accorpata.";

  String rv;
  for (auto r : resources) {
    SlotFile *sf = r->getSlotFile();
    rv.append(sf->codiceRisorsa);
    rv.append(" todo: slots ");
    rv.append("\n");
  }
  return rv;
}

void SchedMerger::reserveSlot(SchedResourceMultiLock &multilock, int giorno,
                              int slotgiorno, u_int64_t uniqueid,
                              Properties &properties) {
  if (merged == nullptr)
    throw StructureException("Merger vuoto.");

  if (giorno < 0 || giorno >= 365)
    throw StructureException(
        "Il valore giorno non è ammesso: deve essere compreso fra 0 e 364.");

  if (slotgiorno < 0 || slotgiorno >= merged->numSlotsGiorno)
    throw StructureException(
        format("il valore slotgiorno %d non è compatibile: "
               "il massimo ammesso è %d",
               slotgiorno, (int)merged->numSlotsGiorno - 1));

  // blocca tutte le risorse
  for (auto r : resources)
    multilock.addResource(r);

  if (!multilock.look())
    throw StructureException("Non riesco a bloccare le risorse indicate.");

  // modifica tutte le risorse
  for (auto r : resources) {
    reserveSlotWorker(r->getSlotFile(), giorno, slotgiorno, uniqueid,
                      properties);
    r->flush();
  }

  reserveSlotWorker(merged, giorno, slotgiorno, uniqueid, properties);
}

void SchedMerger::reserveSlotWorker(SlotFile *sf, int giorno, int slotgiorno,
                                    u_int64_t uniqueid,
                                    Properties &properties) {
  if (slotgiorno < 0 || slotgiorno >= sf->numSlotsGiorno)
    throw StructureException(
        format("il valore slotgiorno %d non è compatibile con la risorsa %s: "
               "il massimo ammesso è %d",
               slotgiorno, sf->codiceRisorsa, (int)sf->numSlotsGiorno - 1));

  int offset = giorno * sf->numSlotsGiorno;
  slotType *slot = sf->arrySlot + offset + slotgiorno;
  slot->status = SLOT_BOOKED;
  slot->info = uniqueid;
}

void SchedMerger::populateHeaderProp(Properties &properties) {
  if (merged != nullptr)
    toProperties(*merged, properties);
}
