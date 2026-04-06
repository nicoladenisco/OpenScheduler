#include "SchedStamperAlgo.hpp"
#include "SchedResource.hpp"
#include "common.hpp"

void SchedStamperAlgo::applyCommon(int dayStart, int dayStop,
                                   SchedResource &resource,
                                   AnyStringMap &properties) {
  if (!resource.isInitialized())
    resource.initializeSlotFile();

  SlotFile *sfil = resource.getSlotFile();
  slotType model = anyCastSlot(properties["model"]);

  if (is_string(properties["hourmap"])) {
    String hourmap = anyCastString(properties["hourmap"]);
    IntVector iv;
    splitComma(hourmap, iv);

    // check per slot validi nella hourmap; devono essere 1 based
    for (auto ds : iv)
      if (ds <= 0 || ds > sfil->numSlotsGiorno)
        throw StructureException("Un valore nella hourmap non è corretto.");

    slotType *daySlots = resource.getSlot(dayStart);
    for (int day = dayStart; day < dayStop; day++) {
      for (auto ds : iv)
        daySlots[ds - 1] = model;

      daySlots += sfil->numSlotsGiorno;
    }
  } else {
    slotType *daySlots = resource.getSlot(dayStart);
    for (int day = dayStart; day < dayStop; day++) {
      for (int sd = 0; sd < sfil->numSlotsGiorno; sd++)
        *daySlots++ = model;
    }
  }
}

void SchedStamperAlgo::applyCommon(IntVector days, SchedResource &resource,
                                   AnyStringMap &properties) {
  if (!resource.isInitialized())
    resource.initializeSlotFile();

  SlotFile *sfil = resource.getSlotFile();
  slotType model = anyCastSlot(properties["model"]);

  if (is_string(properties["hourmap"])) {
    String hourmap = anyCastString(properties["hourmap"]);
    IntVector hours;
    splitComma(hourmap, hours);

    // check per slot validi nella hourmap; devono essere 1 based
    for (auto ds : hours)
      if (ds <= 0 || ds > sfil->numSlotsGiorno)
        throw StructureException("Un valore nella hourmap non è corretto.");

    for (auto day : days) {
      slotType *daySlots = resource.getSlot(day - 1);
      for (auto ds : hours)
        daySlots[ds] = model;
    }
  } else {
    for (auto day : days) {
      slotType *daySlots = resource.getSlot(day - 1);
      for (int sd = 0; sd < sfil->numSlotsGiorno; sd++)
        *daySlots++ = model;
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////

DailyStamper::DailyStamper() {}
DailyStamper::~DailyStamper() {}

void DailyStamper::apply(SchedResource &resource, AnyStringMap &properties) {
  int dayStart = 0;
  int dayStop = 365;
  if (is_int(properties["daystart"])) {
    anyCastInt(properties["daystart"]);
    if (dayStart == 0)
      dayStart = 365;
  }
  if (is_int(properties["daystop"])) {
    anyCastInt(properties["daystop"]);
    if (dayStop == 0)
      dayStop = 365;
  }

  if (dayStart < 0 || dayStart > 365 || dayStop < dayStart || dayStop > 365)
    throw StructureException("Valori non corretti per daystart/daystop.");

  applyCommon(dayStart, dayStop, resource, properties);
}

//////////////////////////////////////////////////////////////////////////////////////

FreeStamper::FreeStamper() {}
FreeStamper::~FreeStamper() {}

void FreeStamper::apply(SchedResource &resource, AnyStringMap &properties) {
  String daymap = anyCastString(properties["daymap"]);
  IntVector days;
  splitComma(daymap, days);

  // verifica valori nella daymap; devono essere 1 based
  for (auto day : days) {
    if (day <= 0 || day > 365)
      throw StructureException("Valori non corretti per daymap.");
  }

  applyCommon(days, resource, properties);
}
