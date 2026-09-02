/*
 * File:   File.h
 * Author: Nicola De Nisco
 *
 * Created on 29 aprile 2010, 8.52
 */

#ifndef _FILE_H
#define _FILE_H

#include <fcntl.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <boost/chrono.hpp>
#include <dirent.h>

#include "common.hpp"

class File;
typedef std::vector<File> FileVector;

///////////////////////////////////////////////////////////////////////

class FileException : public std::exception {
public:
  inline FileException(const String &cause) { this->cause = cause; }

  virtual ~FileException() throw() {}

  inline virtual const char *what() const throw() { return cause.c_str(); }

  String cause;
};

//////////////////////////////////////////////////////////////////////

class File {
public:
  File();
  File(const String &path);
  File(const String &pathDir, const String &fileName);
  File(const File &Dir, const String &fileName);
  File(const File &orig);
  virtual ~File();

  enum FuncScanRetVal { SCAN_STOP, SCAN_NO_DEEP_CONTINUE, SCAN_CONTINUE };

  enum FuncScanOrder { SCAN_FILE_FIRST, SCAN_DIR_FIRST };

  typedef int(FuncScan)(void *args, int dept, const File &parent,
                        const File &target, const String &name);

  virtual int access(int mode = 0) const;

  virtual inline bool exist() const { return existFlg; }

  virtual inline bool isFifo() const { return si.st_mode & S_IFIFO; }

  virtual inline bool isCharSpecial() const { return si.st_mode & S_IFCHR; }

  virtual inline bool isDirectory() const { return si.st_mode & S_IFDIR; }

  virtual inline bool isBlockSpecial() const { return si.st_mode & S_IFBLK; }

  virtual inline bool isFile() const { return si.st_mode & S_IFREG; }

  virtual inline bool isLink() const { return si.st_mode & S_IFLNK; }

  virtual inline bool isSocket() const { return si.st_mode & S_IFSOCK; }

  virtual inline struct stat getStat() const { return si; }

  virtual inline long length() const { return si.st_size; }

  virtual inline bool isAbsolute() const { return absolutePath; }

  virtual inline const char *c_str() const { return thePath.c_str(); }

  virtual inline const String str() const { return thePath; }

  virtual inline void refresh() { readStat(); }

  virtual String getAbsolutePath() const;

  virtual bool isChanged() const;
  virtual File getParent() const;
  virtual String getName() const;
  virtual String getExtension() const;
  virtual bool startWith(const String &str) const;
  virtual bool endWith(const String &str) const;
  virtual String dirDifference(const File &directory) const;

  virtual void makeAbsolute();
  virtual int listFiles(FileVector &fVect) const;
  virtual int deepScan(FuncScan f, void *args = NULL, int maxDept = 0,
                       FuncScanOrder order = File::SCAN_FILE_FIRST) const;
  virtual int deleteFiles(bool deep = true, bool itself = true);
  virtual int copyTo(const File &destPath) const;
  virtual int moveTo(File &destPath);
  virtual int renameTo(const File &destPath);
  virtual int renameTo(const String &destPath);
  virtual int renameName(const String &newName);
  virtual int renameExtension(const String &newExtension);

  virtual int mkdir(int mode = 0755);
  virtual int mkdirs(int mode = 0755);

  virtual int touch();
  virtual int createLockFile(bool wait = true, long timeoutMillis = 0);
  virtual int removeLockFile();

  virtual inline bool isLocked() const { return fdLock > 0; }

  virtual int writeData(const void *data, int len);
  virtual void *readData(void *data = NULL, int len = 0) const;

  virtual int readTextFile(StringVector &linesRead,
                           bool removeBlank = true) const;
  virtual int writeTextFile(const StringVector &linesWrite,
                            bool append = false);

  virtual String readString() const;
  virtual void writeString(const String &stringWrite, bool append = false);

  // funzioni statiche globali
  static char getPathSep();
  static int copyFile(const String &source, const String &dest,
                      int blkSize = 4096);
  static File createTempFile(const String &prefix, const String &suffix,
                             const File &dir);
  static File createTempFile(const String &prefix, const String &suffix);
#if defined(_BSD_SOURCE) || defined(CYGWIN) || defined(MAC_OS_X) ||            \
    _POSIX_C_SOURCE >= 200112L
  static int createUnixTempFile(File &fTemp, const String &prefix,
                                const File &dir);
  static int createUnixTempFile(File &fTemp, const String &prefix = "temp");
#endif
  static File getTmpDir();
  static String correctPath(const String &path, char sep);
  static String getCurrentWorkingDir();

protected:
  virtual void init(const String &pathDir, const String &fileName);
  virtual void readStat();
  virtual int deepScanInternal(int dept, const File &parent, File::FuncScan f,
                               void *args, int maxDept, FuncScanOrder order,
                               FuncScanRetVal &retVal) const;
  virtual int deepScanInternalDir(int dept, const File &parent,
                                  File::FuncScan f, void *args, int maxDept,
                                  FuncScanOrder order,
                                  FuncScanRetVal &retVal) const;
  virtual int deepScanInternalFile(int dept, const File &parent,
                                   File::FuncScan f, void *args,
                                   FuncScanRetVal &retVal) const;
  virtual int deleteDirectoryContent(const String &dirPath, bool deep) const;

  static int copyEntry(void *args, int dept, const File &parent,
                       const File &target, const String &name);

  static int recurseMkdir(File &, int mode);

protected:
  String thePath;
  struct stat si;
  bool existFlg, absolutePath;
  int fdLock;
};

///////////////////////////////////////////////////////////////////////

class LockFileHolder {
public:
  inline LockFileHolder(const String &__lockFile, long timeoutMillis = 0)
      : lockFile(__lockFile) {
    int fd = lockFile.createLockFile(true, timeoutMillis);
    writePid(fd);
  }

  inline LockFileHolder(const File &__lockFile, long timeoutMillis = 0)
      : lockFile(__lockFile) {
    int fd = lockFile.createLockFile(true, timeoutMillis);
    writePid(fd);
  }

  inline virtual ~LockFileHolder() { lockFile.removeLockFile(); }

  inline bool isLocked() const { return lockFile.isLocked(); }

  inline int writePid(int fd) const {
    if (fd == -1)
      throw FileException("Lock non possibile.");

    String tmp = itoa(::getpid());
    return ::write(fd, tmp.c_str(), tmp.length());
  }

  inline String readPid(int fd) const {
    char buffer[64];
    ::read(fd, buffer, sizeof(buffer));
    return buffer;
  }

  File lockFile;
};

#endif /* _FILE_H */
