#ifndef __SCHEDRESOURCE_HPP
#define __SCHEDRESOURCE_HPP

#include "File.hpp"
#include "common.hpp"
#include "dataStructure.hpp"
#include "properties.h"
#include <cstddef>
#include <memory>

class SchedResource {
  friend class SchedResourceLock;

public:
  SchedResource();
  SchedResource(const File &fileSlot);
  virtual ~SchedResource();

  virtual void attachSlotFile(const File &fileSlot);
  virtual void detachSlotFile();

  inline SlotFile *getSlotFile() { return slotFile; }
  virtual slotType *getSlot(int day);

  inline bool isInitialized() const { return slotFile->initalized != 0; }

  inline bool isValidLenght() const {
    return slotFile != nullptr && lengthSlotFile == slotFile->dimensioneFile;
  }

  virtual bool isValidVersion(String *error = nullptr) const;
  virtual bool flush();

  virtual int createLockFile(bool wait = true, long timeoutMillis = 0);
  virtual int removeLockFile();
  inline bool isLocked() const { return lockFile.isLocked(); }

  virtual void initializeSlotFile();
  virtual void clearAllSlots(int stato);
  virtual void populateHeaderProp(Properties &properties);

  // Impediamo la copia della classe per evitare double-free della memoria
  SchedResource(const SchedResource &) = delete;
  SchedResource &operator=(const SchedResource &) = delete;

protected:
  File originFile, lockFile;
  int fdSlotFile;
  size_t lengthSlotFile;
  SlotFile *slotFile;
};

using SchedResourcePtr = std::shared_ptr<SchedResource>;
using SchedResourcePtrVector = std::vector<SchedResourcePtr>;

inline SchedResourcePtr buildResource(const File &fileSlot) {
  return std::make_shared<SchedResource>(fileSlot);
}

class SchedResourceLock {
public:
  SchedResourceLock(SchedResource &tolock, const String &marker,
                    bool verbose = false, bool wait = true,
                    long timeoutMillis = 0);
  virtual ~SchedResourceLock();
  inline bool isLocked() const { return reslock.isLocked(); }

  SchedResource &reslock;
  bool verbose;
};

class SchedResourceMultiLock {
public:
  SchedResourceMultiLock(const String &marker, bool verbose = false,
                         bool wait = true, long timeoutMillis = 0);
  virtual ~SchedResourceMultiLock();

  virtual bool addResource(SchedResourcePtr toLock);
  virtual bool isLocked() const;

  virtual bool look();
  virtual bool unlook();

protected:
  SchedResourcePtrVector resources;
  bool wait;
  long timeoutMillis;
  String marker;
  bool verbose;
};

#endif
