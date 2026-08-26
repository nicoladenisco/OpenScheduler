#include "SchedMerger.hpp"
#include "SchedMergerAlgo.hpp"
#include "SchedResource.hpp"
#include "common.hpp"
#include "dataStructure.hpp"
#include <utility>

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

  // modifica la fusione
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

  int num = properties.get("numSlots", 1);
  if ((slotgiorno + num) < 0 || (slotgiorno + num) >= sf->numSlotsGiorno)
    throw StructureException(format(
        "il valore numSlots %d non è compatibile con la risorsa %s: "
        "il massimo ammesso per (slotgiorno + num) è %d",
        (slotgiorno + num), sf->codiceRisorsa, (int)sf->numSlotsGiorno - 1));

  bool force = properties.getBool("force", false);

  int offset = giorno * sf->numSlotsGiorno;
  slotType *slot = sf->arrySlot + offset + slotgiorno;

  while (num-- > 0) {
    // se non impostato force=true verifica che lo slot sia libero
    if (!force) {
      if (!(slot->status == SLOT_LOOKED || slot->status == SLOT_SCHEDULABLE))
        throw StructureException(
            format("lo slot non è disponibile (stato %d)", (int)slot->status));
    }

    // marca lo slot come prenotato; imposta id prenotazione
    slot->status = SLOT_BOOKED;
    slot->info = uniqueid;
    slot++;
  }
}

void SchedMerger::populateHeaderProp(Properties &properties) {
  if (merged != nullptr)
    toProperties(*merged, properties);
}

void SchedMerger::findFreeSlot(SchedResourceMultiLock &multilock,
                               Properties &properties,
                               IntPairVector &risultati) {
  IntPair days = properties.parseDays();
  int num = properties.get("numSlots", 1);

  if (num > merged->numSlotsGiorno)
    throw StructureException(format("il valore di numSlots non è compatibile "
                                    "con i limiti di questo merger"));

  IntPair slotgiorno(
      properties.get("slotgiornoInizio", 0),
      properties.get("slotgiornoFine", merged->numSlotsGiorno - num));

  if (slotgiorno.first < 0 || (slotgiorno.first + num) > merged->numSlotsGiorno)
    throw StructureException(
        format("il valore slotgiornoInizio %d non è compatibile: "
               "il massimo ammesso è %d",
               slotgiorno, (int)merged->numSlotsGiorno - num));
  if (slotgiorno.second < 0 ||
      (slotgiorno.second + num) > merged->numSlotsGiorno)
    throw StructureException(
        format("il valore slotgiornoFine %d non è compatibile: "
               "il massimo ammesso è %d",
               slotgiorno.second, (int)merged->numSlotsGiorno - num));

  if (num + slotgiorno.first > slotgiorno.second)
    throw StructureException(format("il valore di numSlots non è compatibile "
                                    "con slotgiornoInizio e slotgiornoFine"));

  for (int giorno = days.first; giorno < days.second; giorno++) {
    int offset = giorno * merged->numSlotsGiorno;
    for (int orario = slotgiorno.first; orario < slotgiorno.second; orario++) {
      bool good = true;
      // verifica che tutti gli slot richiesti siano liberi
      slotType *slot = merged->arrySlot + offset + orario;
      for (int n = 0; n < num; n++) {
        if (!(slot->status == SLOT_LOOKED ||
              slot->status == SLOT_SCHEDULABLE)) {
          good = false;
          break;
        }
        slot++;
      }

      if (good) {
        // trovato un risultato valido: lo salva per il ritorno
        risultati.push_back(IntPair(giorno, orario));
      }
    }
  }
}
