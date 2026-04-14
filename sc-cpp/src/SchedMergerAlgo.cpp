#include "SchedMergerAlgo.hpp"
#include "common.hpp"
#include "dataStructure.hpp"

int SchedMergerAlgo::checkCompatibilita(SlotFile *m, SchedResource &tomerge,
                                        bool thexcpt /* = true */) {
  if (m == nullptr)
    return 0;

  SlotFile *c = tomerge.getSlotFile();
  if (m->slotOra != c->slotOra || m->oraIniziale != c->oraIniziale ||
      m->oraFinale != c->oraFinale) {
    if (thexcpt)
      throw new StructureException(
          "Orari non compatibili: la risorsa non può essere aggregata.");
    else
      return -1;
  }

  if (m->numSlotsGiorno != c->numSlotsGiorno ||
      m->numSlotsTotali != c->numSlotsTotali) {
    if (thexcpt)
      throw new StructureException("Numero di slot non compatibile: la risorsa "
                                   "non può essere aggregata.");
    else
      return -2;
  }

  return 0;
}

SlotFile *SchedMergerAlgo::inizializzaMerge(SlotFile *merged,
                                            SchedResource &tomerge) {
  if (merged == nullptr) {
    SlotFile *origin = tomerge.getSlotFile();
    merged = (SlotFile *)malloc(origin->dimensioneByte);
    memset(merged, 0, origin->dimensioneByte);
    memcpy(merged, origin, SIZE_SLOT_FILE);
  }

  return merged;
}

SlotFile *SchedMergerAlgo::sbloccaAltri(SlotFile *merged,
                                        SchedResource &tomerge,
                                        SchedResourcePtrVector otherResources,
                                        u_int64_t idUniqueLoock,
                                        Properties &properties) {

  for (auto othr : otherResources) {
    slotType *ptm = merged->arrySlot;
    slotType *pts = othr->getSlotFile()->arrySlot;
    for (int i = 0; i < merged->numSlotsTotali; i++, ptm++, pts++) {

      // se il merge non è disponibile rimuove eventuale lock sulla risorsa
      if (ptm->status != SLOT_SCHEDULABLE && pts->status == SLOT_LOOKED &&
          pts->info == idUniqueLoock) {

        pts->status = SLOT_SCHEDULABLE;
        pts->info = 0;
      }
    }
    othr->flush();
  }

  return merged;
}

/////////////////////////////////////////////////////////////////////////////

SlotFile *DefaultMerger::apply(SlotFile *merged, SchedResource &tomerge,
                               SchedResourcePtrVector otherResources,
                               u_int64_t idUniqueLoock,
                               Properties &properties) {
  IntPair days = properties.parseDays();

  if (merged == nullptr) {
    merged = inizializzaMerge(merged, tomerge);
    // inizializza gli slots nell'intervallo richiesto
    for (int day = days.first; day < days.second; day++) {
      int offset = day * merged->numSlotsGiorno;
      slotType *ptm = merged->arrySlot + offset;
      for (int i = 0; i < merged->numSlotsGiorno; i++, ptm++) {
        ptm->status = SLOT_SCHEDULABLE;
      }
    }
  } else {
    checkCompatibilita(merged, tomerge, true);
  }

  for (int day = days.first; day < days.second; day++) {

    int offset = day * merged->numSlotsGiorno;
    slotType *ptm = merged->arrySlot + offset;
    slotType *ptc = tomerge.getSlotFile()->arrySlot + offset;

    for (int i = 0; i < merged->numSlotsGiorno; i++, ptm++, ptc++) {
      // se uno dei due è non disponibile la destinazione diventa non
      // disponibile
      if (ptm->status == SLOT_UNAVAILABLE || ptc->status == SLOT_UNAVAILABLE) {
        ptm->status = SLOT_UNAVAILABLE;
        ptm->info = 0;
        continue;
      }

      // se il merge è disponibile ...
      if (ptm->status == SLOT_SCHEDULABLE && ptc->status == SLOT_SCHEDULABLE) {
        ptm->status = SLOT_SCHEDULABLE;
        ptm->info = 0;

        // ... blocca lo slot in tomerge
        ptc->status = SLOT_LOOKED;
        ptc->info = idUniqueLoock;

        continue;
      }

      // se il merge non è disponibile rimuove eventuale lock sulla risorsa
      if (ptm->status != SLOT_SCHEDULABLE && ptc->status == SLOT_LOOKED) {
        ptc->status = SLOT_SCHEDULABLE;
        ptc->info = 0;
      }

      // in tutti gli altri casi lo slot merge diventa non disponibile
      ptm->status = SLOT_UNAVAILABLE;
      ptm->info = 0;
    }
  }

  tomerge.flush();
  return sbloccaAltri(merged, tomerge, otherResources, idUniqueLoock,
                      properties);
}
