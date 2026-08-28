#include "SchedSlotBase.hpp"
#include "common.hpp"

SchedSlotBase::SchedSlotBase() {}
SchedSlotBase::~SchedSlotBase() {}

void SchedSlotBase::reserveSlotWorker(SlotFile *sf, int giorno, int slotgiorno,
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

void SchedSlotBase::findFreeSlotWorker(SlotFile *sf, Properties &properties,
                                       IntPairVector &risultati) {
  IntPair days = properties.parseDays();
  int num = properties.get("numSlots", 1);

  if (num > sf->numSlotsGiorno)
    throw StructureException(format("il valore di numSlots non è compatibile "
                                    "con i limiti di questo merger"));

  IntPair slotgiorno(
      properties.get("slotgiornoInizio", 0),
      properties.get("slotgiornoFine", sf->numSlotsGiorno - num));

  if (slotgiorno.first < 0 || (slotgiorno.first + num) > sf->numSlotsGiorno)
    throw StructureException(
        format("il valore slotgiornoInizio %d non è compatibile: "
               "il massimo ammesso è %d",
               slotgiorno, (int)sf->numSlotsGiorno - num));
  if (slotgiorno.second < 0 || (slotgiorno.second + num) > sf->numSlotsGiorno)
    throw StructureException(
        format("il valore slotgiornoFine %d non è compatibile: "
               "il massimo ammesso è %d",
               slotgiorno.second, (int)sf->numSlotsGiorno - num));

  if (num + slotgiorno.first > slotgiorno.second)
    throw StructureException(format("il valore di numSlots non è compatibile "
                                    "con slotgiornoInizio e slotgiornoFine"));

  for (int giorno = days.first; giorno < days.second; giorno++) {
    int offset = giorno * sf->numSlotsGiorno;
    for (int orario = slotgiorno.first; orario < slotgiorno.second; orario++) {
      bool good = true;
      // verifica che tutti gli slot richiesti siano liberi
      slotType *slot = sf->arrySlot + offset + orario;
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

void SchedSlotBase::clearSlotWorker(SlotFile *sf, int giorno, int slotgiorno,
                                    u_int64_t uniqueid, u_char statusClear,
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

    // se richiesto controlla che uniqueid sia coerente
    if (uniqueid == 0 || uniqueid == slot->info) {
      // se non impostato force=true verifica che lo slot sia occupato
      if (!force) {
        if (!(slot->status == SLOT_BOOKED))
          throw StructureException(
              format("lo slot non è prenotato (stato %d)", (int)slot->status));
      }

      // marca lo slot come non prenotato; azzera id prenotazione
      slot->status = statusClear;
      slot->info = 0;
    }

    slot++;
  }
}

void SchedSlotBase::clearSlotWorker(SlotFile *sf, u_int64_t uniqueid,
                                    u_char statusClear,
                                    Properties &properties) {
  bool force = properties.getBool("force", false);
  if (uniqueid == 0)
    throw StructureException("il valore uniqueid deve essere specifiato");

  slotType *slot = sf->arrySlot;

  for (int num = 0; num < sf->numSlotsTotali; num++) {
    // controlla che uniqueid sia coerente
    if (uniqueid == slot->info) {
      // se non impostato force=true verifica che lo slot sia occupato
      if (!force) {
        if (!(slot->status == SLOT_BOOKED))
          throw StructureException(
              format("lo slot non è prenotato (stato %d)", (int)slot->status));
      }

      // marca lo slot come non prenotato; azzera id prenotazione
      slot->status = statusClear;
      slot->info = 0;
    }

    slot++;
  }
}
