/*
 * Definizione strutture dati principali.
 */

#ifndef __DATA_STRUCTURE_HPP
#define __DATA_STRUCTURE_HPP

#include "File.hpp"
#include "Properties.hpp"
#include <sys/types.h>

#define SLOT_UNAVAILABLE 0
#define SLOT_SCHEDULABLE 1
#define SLOT_LOOKED 2
#define SLOT_BOOKED 3

struct slotType {
  u_char status;
  u_int64_t info;
};

struct SlotFile {
  char magic[2];
  char firma[16];
  char sanno[4];
  char zero;

  char codiceRisorsa[64];
  u_int16_t anno;
  u_char slotOra;
  u_char oraIniziale;
  u_char oraFinale;

  u_int16_t numSlotsGiorno;
  u_int32_t numSlotsTotali;
  u_int32_t dimensioneByte;
  u_int32_t dimensionePagine;
  u_int16_t pageSize;
  u_int32_t dimensioneFile;
  u_char initalized;

  char riservatoFutureEspansioni[64];

  slotType arrySlot[1];
};

#define MAGIC "KK"
#define FIRMA "Slots file v1"

void initSlotFile(int anno, int slotOra, int oraIniziale, int oraFinale,
                  String codiceRisorsa, SlotFile &sf);
void initSlotFile(int anno, int slotOra, int oraIniziale, int oraFinale,
                  String codiceRisorsa, SlotFile &sf, File &tosave);

String toString(const SlotFile &sf, String separator = "\n");
String dump(const SlotFile &sf, int dayStart = 0, int dayStop = 365,
            String separator = "\n");
void toProperties(const SlotFile &sf, Properties &prop);

#define SIZE_SLOT_FILE (sizeof(SlotFile) - sizeof(slotType))

//////////////////////////////////////////////////////////////////////

class StructureException : public std::exception {
public:
  inline StructureException(const String &cause) {
    this->cause = cause;
    this->errorStore = errno;
  }

  inline StructureException(const String &cause, int errore) {
    this->cause = cause;
    this->errorStore = errore;
  }

  virtual ~StructureException() throw() {}

  inline virtual const char *what() const throw() { return cause.c_str(); }

  String cause;
  int errorStore;
};

#endif //  __DATA_STRUCTURE_HPP
