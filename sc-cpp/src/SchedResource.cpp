#include "SchedResource.hpp"
#include "dataStructure.hpp"
#include <cstring>
#include <fcntl.h>    // open
#include <sys/mman.h> // mmap, munmap
#include <sys/stat.h> // fstat
#include <unistd.h>   // close

SchedResource::SchedResource() : slotFile(nullptr) {}
SchedResource::SchedResource(const File &fileSlot) : slotFile(nullptr) {
  attachSlotFile(fileSlot);
}

void SchedResource::attachSlotFile(const File &fileSlot) {
  if (slotFile != nullptr)
    detachSlotFile();

  slotFile = nullptr;
  originFile = fileSlot;
  lockFile = File(originFile.getAbsolutePath() + ".lock");

  // apre file su disco in lettura/scrittura
  if ((fdSlotFile = open(fileSlot.c_str(), O_RDWR)) == -1) {
    throw StructureException(
        format("Errore nell'apertura del file %s", fileSlot.c_str()));
  }

  // ottieni la dimensione del file
  struct stat sb;
  if (fstat(fdSlotFile, &sb) == -1) {
    close(fdSlotFile);
    throw StructureException("Errore nell'ottenere la dimensione del file");
  }
  lengthSlotFile = sb.st_size;

  if (lengthSlotFile == 0) {
    close(fdSlotFile);
    throw StructureException("File slot vuoto.");
  }

  // mappa il file in memoria
  slotFile = (SlotFile *)::mmap(NULL, lengthSlotFile, PROT_READ | PROT_WRITE,
                                MAP_SHARED, fdSlotFile, 0);

  if (slotFile == MAP_FAILED) {
    close(fdSlotFile);
    slotFile = nullptr;
    throw StructureException("Errore durante mmap");
  }
}

SchedResource::~SchedResource() { detachSlotFile(); }

void SchedResource::detachSlotFile() {
  if (slotFile != nullptr) {
    // Pulizia (l'ordine è importante)
    if (::munmap(slotFile, lengthSlotFile) == -1) {
      perror("Errore durante munmap");
    }

    close(fdSlotFile);
    slotFile = nullptr;
  }

  // rimuove eventuale lock su risorsa
  if (isLocked())
    removeLockFile();
}

bool SchedResource::isValidVersion(String *error /* = nullptr*/) const {
  if (slotFile == nullptr) {
    if (error != nullptr)
      (*error) = "Nessun riferimento; oggetto non inizializzato.";
    return false;
  }

  if (strncmp(MAGIC, slotFile->magic, 2) != 0) {
    if (error != nullptr)
      (*error) = "Il file indicato non è un file slot.";
    return false;
  }

  if (strncmp(FIRMA, slotFile->firma, 16) != 0) {
    if (error != nullptr) {
      String ss(slotFile->firma);
      (*error) = format("Formato incompatibile: atteso '%s' letto '%s'; "
                        "versione non compatibile.",
                        FIRMA, trim(ss.substr(0, 16)).c_str());
    }
    return false;
  }

  return true;
}

bool SchedResource::flush() {
  if (::msync(slotFile, lengthSlotFile, MS_SYNC) == -1) {
    perror("Sincronizzazione slot file su disco in errore!");
    return false;
  }

  return true;
}

int SchedResource::createLockFile(bool wait /*= true*/,
                                  long timeoutMillis /*= 0*/) {
  return lockFile.createLockFile(wait, timeoutMillis);
}

int SchedResource::removeLockFile() { return lockFile.removeLockFile(); }

slotType *SchedResource::getSlot(int day) {
  if (day < 0 || day >= 365)
    throw StructureException("Valore giorno non valido.");

  int offset = day * slotFile->numSlotsGiorno;
  return &slotFile->arrySlot[offset];
}

void SchedResource::initializeSlotFile() {
  if (slotFile->initalized == 0) {
    slotType *ptSlots = slotFile->arrySlot;
    memset(ptSlots, 0, sizeof(slotType) * slotFile->numSlotsTotali);
    slotFile->initalized = 1;
  }
}

void SchedResource::clearAllSlots(int stato) {
  slotType *ptSlots = slotFile->arrySlot;
  for (int i = 0; i < slotFile->numSlotsTotali; i++, ptSlots++) {
    ptSlots->status = stato;
    ptSlots->info = 0;
  }
}

void SchedResource::populateHeaderProp(Properties &properties) {
  if (slotFile != nullptr)
    toProperties(*slotFile, properties);
}

void SchedResource::reserveSlot(int giorno, int slotgiorno, u_int64_t uniqueid,
                                Properties &properties) {
  if (!isInitialized())
    throw StructureException("risorsa non inizializzata");

  if (giorno < 0 || giorno >= 365)
    throw StructureException(
        "Il valore giorno non è ammesso: deve essere compreso fra 0 e 364.");

  if (slotgiorno < 0 || slotgiorno >= slotFile->numSlotsGiorno)
    throw StructureException(
        format("il valore slotgiorno %d non è compatibile: "
               "il massimo ammesso è %d",
               slotgiorno, (int)slotFile->numSlotsGiorno - 1));

  // modifica la fusione
  reserveSlotWorker(slotFile, giorno, slotgiorno, uniqueid, properties);
}

void SchedResource::findFreeSlot(Properties &properties,
                                 IntPairVector &risultati) {
  findFreeSlotWorker(slotFile, properties, risultati);
}

void SchedResource::clearSlot(int giorno, int slotgiorno, u_int64_t uniqueid,
                              Properties &properties) {
  if (slotFile == nullptr)
    throw StructureException("Merger vuoto.");

  if (giorno < 0 || giorno >= 365)
    throw StructureException(
        "Il valore giorno non è ammesso: deve essere compreso fra 0 e 364.");

  if (slotgiorno < 0 || slotgiorno >= slotFile->numSlotsGiorno)
    throw StructureException(
        format("il valore slotgiorno %d non è compatibile: "
               "il massimo ammesso è %d",
               slotgiorno, (int)slotFile->numSlotsGiorno - 1));

  clearSlotWorker(slotFile, giorno, slotgiorno, uniqueid, SLOT_SCHEDULABLE,
                  properties);
}

void SchedResource::clearSlot(u_int64_t uniqueid, Properties &properties) {
  if (slotFile == nullptr)
    throw StructureException("Merger vuoto.");

  clearSlotWorker(slotFile, uniqueid, SLOT_SCHEDULABLE, properties);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

SchedResourceLock::SchedResourceLock(SchedResource &__tolock,
                                     const String &marker,
                                     bool __verbose /*= false*/,
                                     bool wait /*= true*/,
                                     long timeoutMillis /*= 0*/)
    : reslock(__tolock), verbose(__verbose) {
  if (verbose)
    cout << "Attempt to lock " << reslock.getSlotFile()->codiceRisorsa
         << std::endl;

  int fd = reslock.createLockFile(wait, timeoutMillis);
  if (fd > 0)
    ::write(fd, marker.c_str(), marker.length());
  else if (verbose)
    cout << "Failed lock " << reslock.getSlotFile()->codiceRisorsa << std::endl;
}

SchedResourceLock::~SchedResourceLock() {
  if (reslock.isLocked()) {
    reslock.removeLockFile();
    if (verbose)
      cout << "Unlock " << reslock.getSlotFile()->codiceRisorsa << std::endl;
  }
}

SchedResourceMultiLock::SchedResourceMultiLock(const String &__marker,
                                               bool __verbose /* = false*/,
                                               bool __wait /* = true */,
                                               long __timeoutMillis /* = 0 */)
    : marker(__marker), verbose(__verbose), wait(__wait),
      timeoutMillis(__timeoutMillis) {}
SchedResourceMultiLock::~SchedResourceMultiLock() { unlook(); }

bool SchedResourceMultiLock::addResource(SchedResourcePtr toLock) {
  resources.push_back(toLock);
  return true;
}

bool SchedResourceMultiLock::isLocked() const {
  for (auto p : resources) {
    if (!p->isLocked())
      return false;
  }
  return true;
}

bool SchedResourceMultiLock::look() {
  for (auto p : resources) {
    if (!p->isLocked()) {

      if (verbose)
        cout << "Attempt to lock " << p->getSlotFile()->codiceRisorsa
             << std::endl;

      int fd = p->createLockFile(wait, timeoutMillis);
      if (fd == -1) {
        unlook();
        return false;
      }

      ::write(fd, marker.c_str(), marker.length());
    }
  }
  return true;
}

bool SchedResourceMultiLock::unlook() {
  for (auto p : resources) {
    if (p->isLocked()) {
      p->removeLockFile();

      if (verbose)
        cout << "Unlock " << p->getSlotFile()->codiceRisorsa << std::endl;
    }
  }
  return true;
}
