#include "SchedStamperAlgo.hpp"
#include "Properties.hpp"
#include "SchedResource.hpp"
#include "common.hpp"
#include "dataStructure.hpp"

void SchedStamperAlgo::applyCommon(int dayStart, int dayStop,
                                   SchedResource &resource,
                                   Properties &properties) {
  if (!resource.isInitialized())
    resource.initializeSlotFile();

  SlotFile *sfil = resource.getSlotFile();
  slotType model;
  model.status = properties.get("model", SLOT_SCHEDULABLE);
  model.info = 0;

  String hourmap = properties.get("hourmap", "");
  if (!hourmap.empty()) {
    IntVector iv;
    splitComma(hourmap, iv);

    if (debugOutput) {
      cout << DEBUGOUT << "Apply hourmap " << hourmap << std::endl;
    }

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
                                   Properties &properties) {
  if (!resource.isInitialized())
    resource.initializeSlotFile();

  SlotFile *sfil = resource.getSlotFile();
  slotType model;
  model.status = properties.get("model", SLOT_SCHEDULABLE);
  model.info = 0;

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
        daySlots[ds - 1] = model;
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

void DailyStamper::apply(SchedResource &resource, Properties &properties) {
  IntPair rd = properties.parseDays();
  applyCommon(rd.first, rd.second, resource, properties);
}

//////////////////////////////////////////////////////////////////////////////////////

FreeStamper::FreeStamper() {}
FreeStamper::~FreeStamper() {}

void FreeStamper::apply(SchedResource &resource, Properties &properties) {
  String daymap = properties.get("daymap", "");
  IntVector days;
  splitComma(daymap, days);

  // verifica valori nella daymap; devono essere 1 based
  for (auto day : days) {
    if (day <= 0 || day > 365)
      throw StructureException("Valori non corretti per daymap.");
  }

  applyCommon(days, resource, properties);
}
