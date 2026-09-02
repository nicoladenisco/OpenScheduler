/*
 * File:   File.cpp
 * Author: Nicola De Nisco
 *
 * Created on 29 aprile 2010, 8.52
 */

#include "File.hpp"
#include <stdio.h>
#include <string.h>

// sopprime warning su uso di readdir_r
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

File::File() {
  memset(&si, 0, sizeof(si));
  existFlg = false;
  fdLock = 0;
}

File::File(const String &path) {
  init(correctPath(path.c_str(), getPathSep()), "");
}

File::File(const String &pathDir, const String &fileName) {
  init(correctPath(pathDir.c_str(), getPathSep()), fileName);
}

File::File(const File &Dir, const String &fileName) {
  init(Dir.getAbsolutePath(), fileName);
}

File::File(const File &orig) {
  thePath = orig.thePath;
  si = orig.si;
  existFlg = orig.existFlg;
  absolutePath = orig.absolutePath;
  fdLock = orig.fdLock;
}

File::~File() {}

void File::init(const String &pathDir, const String &fileName) {
  fdLock = 0;
  thePath = pathDir;

  if (!fileName.empty()) {
    thePath += getPathSep();
    thePath += fileName;
  }

  readStat();
}

String File::getAbsolutePath() const {
  if (!absolutePath && thePath.length() > 0) {
    // path relativa: estrae la directory corrente
    String rv = getCurrentWorkingDir();
    rv += getPathSep();
    rv += thePath;

    return rv;
  }
  return thePath;
}

File File::getParent() const {
  auto idx = thePath.rfind(getPathSep());
  if (idx == String::npos)
    throw FileException(
        format("Impossibile ottenere il parent di %s", thePath.c_str()));

  String parent = thePath.substr(0, idx);
  if (parent.length() == 0)
    return File(String(1, getPathSep()));

  return File(parent);
}

String File::getName() const {
  auto idx = thePath.rfind(getPathSep());
  if (idx == String::npos)
    return thePath;

  return thePath.substr(idx + 1);
}

String File::getExtension() const {
  auto idx = thePath.rfind(".");
  if (idx == String::npos)
    return "";

  return thePath.substr(idx + 1);
}

bool File::startWith(const String &str) const {
  return strStartWith(thePath, str);
}

bool File::endWith(const String &str) const { return strEndWith(thePath, str); }

/**
 * Toglie la directory radice indicata alla path assoluta
 * del file ritorna la differenza sotto forma di stringa:
 * this=/usr/local/bin/fileuno directory=/usr/local differenza=bin/fileuno
 */
String File::dirDifference(const File &directory) const {
  int l1 = thePath.length();
  int l2 = directory.thePath.length();

  if (l2 >= l1)
    return thePath;

  String rv = thePath.substr(l2, l1 - l2);

  // elimina lo / iniziale se serve
  while (rv[0] == getPathSep())
    rv = rv.substr(1, rv.length() - 1);

  return rv;
}

#if defined(WINDOWS) && !defined(CYGWIN)

char File::getPathSep() { return '\\'; }

#else

char File::getPathSep() { return '/'; }

#endif

void File::readStat() {
  if (thePath.length() > 0) {
    absolutePath = (thePath[0] == getPathSep());
    existFlg = stat(thePath.c_str(), &si) == 0;
    if (!existFlg)
      memset(&si, 0, sizeof(si));
  } else {
    absolutePath = true;
    existFlg = false;
    memset(&si, 0, sizeof(si));
  }
}

void File::makeAbsolute() {
  if (!absolutePath) {
    thePath = getAbsolutePath();
    readStat();
  }
}

int File::listFiles(FileVector &fVect) const {
  if (!isDirectory())
    return 0;

  int count = 0;
  DIR *ptDir = opendir(thePath.c_str());
  if (ptDir) {
    struct dirent dt, *pdt = NULL;
    while (readdir_r(ptDir, &dt, &pdt) == 0 && pdt != NULL) {
      if (strcmp(pdt->d_name, ".") == 0 || strcmp(pdt->d_name, "..") == 0)
        continue;

      fVect.push_back(File(*this, dt.d_name));
      count++;
    }

    closedir(ptDir);
    return count;
  }
  return 0;
}

int File::deepScan(File::FuncScan f, void *args /*= NULL*/, int maxDept /*= 0*/,
                   FuncScanOrder order /*= File::SCAN_FILE_FIRST*/) const {
  if (!isDirectory())
    return 0;

  File::FuncScanRetVal retVal = File::SCAN_CONTINUE;
  return deepScanInternal(0, *this, f, args, maxDept, order, retVal);
}

int File::deepScanInternal(int dept, const File &parent, File::FuncScan f,
                           void *args, int maxDept, FuncScanOrder order,
                           File::FuncScanRetVal &retVal) const {
  int count = 0;

  switch (order) {
  case SCAN_DIR_FIRST:
    count += deepScanInternalDir(dept, parent, f, args, maxDept, order, retVal);
    if (retVal == File::SCAN_STOP)
      return count;
    count += deepScanInternalFile(dept, parent, f, args, retVal);
    break;

  case SCAN_FILE_FIRST:
    count += deepScanInternalFile(dept, parent, f, args, retVal);
    if (retVal == File::SCAN_STOP)
      return count;
    count += deepScanInternalDir(dept, parent, f, args, maxDept, order, retVal);
    break;
  }

  return count;
}

int File::deepScanInternalDir(int dept, const File &parent, File::FuncScan f,
                              void *args, int maxDept, FuncScanOrder order,
                              File::FuncScanRetVal &retVal) const {
  int count = 0;
  DIR *ptDir = opendir(parent.thePath.c_str());
  if (!ptDir)
    return 0;

  struct dirent dt, *pdt = NULL;
  while (readdir_r(ptDir, &dt, &pdt) == 0 && pdt != NULL) {
    if (strcmp(dt.d_name, ".") == 0 || strcmp(dt.d_name, "..") == 0)
      continue;

    File target(parent, dt.d_name);
    if (!target.exist())
      continue;

    if (target.isDirectory()) {
      // una directory
      retVal = (FuncScanRetVal)((*f)(args, dept, parent, target, dt.d_name));
      switch (retVal) {
      case SCAN_STOP:
        goto exitNow;
      case SCAN_NO_DEEP_CONTINUE:
        break;
      case SCAN_CONTINUE:
        if (maxDept == 0 || dept < maxDept) {
          // entra nella sottodirectory e continua la scansione
          count += deepScanInternal(dept + 1, target, f, args, maxDept, order,
                                    retVal);
        }
        break;
      }
      continue;
    }

    count++;
  }

exitNow:
  closedir(ptDir);
  return count;
}

int File::deepScanInternalFile(int dept, const File &parent, File::FuncScan f,
                               void *args, File::FuncScanRetVal &retVal) const {
  int count = 0;
  DIR *ptDir = opendir(parent.thePath.c_str());
  if (!ptDir)
    return 0;

  struct dirent dt, *pdt = NULL;
  while (readdir_r(ptDir, &dt, &pdt) == 0 && pdt != NULL) {
    if (strcmp(dt.d_name, ".") == 0 || strcmp(dt.d_name, "..") == 0)
      continue;

    File target(parent, dt.d_name);
    if (!target.exist())
      continue;

    if (target.isFile() || target.isLink()) {
      // un file o altro
      retVal = (FuncScanRetVal)((*f)(args, dept, parent, target, dt.d_name));
      switch (retVal) {
      case SCAN_STOP:
        goto exitNow;
      case SCAN_NO_DEEP_CONTINUE:
      case SCAN_CONTINUE:
        break;
      }
    }

    count++;
  }

exitNow:
  closedir(ptDir);
  return count;
}

/**
 * Cancella la entry su disco puntata da questo oggetto File.
 * @param deep cancella contenuto se è una directory
 * @param itself se vero cancella anche se stessa
 * @return numero di files/directory cancellati
 */
int File::deleteFiles(bool deep /*= true*/, bool itself /*= true*/) {
  if (isFile()) {
    return unlink(thePath.c_str()) == 0 ? 1 : 0;
  }

  int rv = 0;
  if (isDirectory()) {
    if (deep)
      rv = deleteDirectoryContent(thePath, deep);

    if (itself)
      rmdir(thePath.c_str());
  }

  readStat();
  return rv;
}

int File::deleteDirectoryContent(const String &dirPath, bool deep) const {
  int count = 0;
  DIR *ptDir = opendir(dirPath.c_str());
  if (!ptDir)
    return 0;

  struct dirent dt, *pdt = NULL;
  while (readdir_r(ptDir, &dt, &pdt) == 0 && pdt != NULL) {
    if (strcmp(dt.d_name, ".") == 0 || strcmp(dt.d_name, "..") == 0)
      continue;

    File target(dirPath, dt.d_name);
    if (!target.exist())
      continue;

    if (deep && target.isDirectory()) {
      count += deleteDirectoryContent(target.thePath, true);
      rmdir(target.thePath.c_str());
      continue;
    }

    if (!target.isDirectory())
      if (unlink(target.thePath.c_str()) == 0)
        count++;
  }

  closedir(ptDir);
  return count;
}

/**
 * Copia un file.
 * @param source path sorgente (deve esistere)
 * @param dest path destinazione (la directory deve esistere, il file verrà
 * sovrascritto se esiste)
 * @param blkSize dimensione blocco di lettura/scrittura
 * @return numero byte copiati o -1 per errore (vedi errno)
 */
int File::copyFile(const String &source, const String &dest,
                   int blkSize /*= 4096*/) {
  int hin, hout, nbr, nbw, total = -1;
  char *buffer = (char *)alloca(blkSize);

  if ((hin = open(source.c_str(), O_RDONLY)) != -1) {
    if ((hout = creat(dest.c_str(), 0644)) != -1) {
      total = 0;
      do {
        if ((nbr = read(hin, buffer, blkSize)) > 0) {
          if ((nbw = write(hout, buffer, nbr)) != nbr) {
            total = -1;
            break;
          }

          total += nbw;
        }
      } while (nbr > 0);
      close(hout);
    }
    close(hin);
  }

  return total;
}

/**
 * Funzione callback per la copia ricorsiva di una directory.
 */
int File::copyEntry(void *arg, int dept, const File &parent, const File &target,
                    const String &name) {
  const void **args = (const void **)arg;
  File *ptThis = (File *)args[0];
  File *ptDest = (File *)args[1];
  int blksize = *((int *)args[2]);
  String diffParent = parent.thePath.substr(ptThis->thePath.length());

  File out(ptDest->thePath + getPathSep() + diffParent, name);

  if (target.isDirectory()) {
    ::mkdir(out.thePath.c_str(), 0755);
  } else if (target.isFile() || target.isLink()) {
    if (copyFile(target.thePath.c_str(), out.thePath.c_str(), blksize) == -1)
      throw FileException(format("Errore di IO copiando %s in %s",
                                 target.thePath.c_str(), out.thePath.c_str()));
  }

  return SCAN_CONTINUE;
}

/**
 * Copia il file/directory puntato da questa path
 * in una nuova destinazione. Se destPath punta ad
 * una directory esistente il contenuto viene riversato
 * nella directory puntata.
 * @param destPath directory o nuovo file destinazione
 */
int File::copyTo(const File &destPath) const {
  if (destPath.isDirectory()) {
    if (isDirectory()) {
      const void *args[] = {this, &destPath, &si.st_blksize};
      deepScan(copyEntry, args);
    } else if (isFile() || isLink()) {
      File out(destPath, getName().c_str());
      if (copyFile(thePath.c_str(), out.thePath.c_str(), si.st_blksize) == -1)
        throw FileException(format("Errore di IO copiando %s in %s",
                                   thePath.c_str(), destPath.thePath.c_str()));
    } else
      throw FileException("Tipo di oggetto non copiabile.");
  } else {
    if (isFile() || isLink()) {
      if (copyFile(thePath.c_str(), destPath.thePath.c_str(), si.st_blksize) ==
          -1)
        throw FileException(format("Errore di IO copiando %s in %s",
                                   thePath.c_str(), destPath.thePath.c_str()));
    } else
      throw FileException("Tipo di oggetto non copiabile.");
  }
  return 0;
}

int File::moveTo(File &destPath) {
  destPath.deleteFiles(true, true);

  if (renameTo(destPath) != 0 && errno == EXDEV) {
    copyTo(destPath);
    deleteFiles(true, true);
  }

  return 0;
}

int File::renameTo(const File &destPath) { return renameTo(destPath.thePath); }

int File::renameTo(const String &destPath) {
  int rv = ::rename(thePath.c_str(), destPath.c_str());
  thePath = destPath;
  readStat();
  return rv;
}

int File::renameName(const String &newName) {
  auto pos = thePath.rfind(getPathSep());
  String radice = pos == String::npos ? thePath : thePath.substr(0, pos);
  String newname = radice + newName;
  return renameTo(newname);
}

int File::renameExtension(const String &newExtension) {
  auto pos = thePath.rfind(".");
  String radice = pos == String::npos ? thePath : thePath.substr(0, pos);
  String newname = radice + newExtension;
  return renameTo(newname);
}

int File::mkdir(int mode /*= 0755*/) {
  if (exist()) {
    if (isDirectory())
      return 0; // esiste ed e' una directory

    throw FileException(
        format("La path %s esiste ma non è una directory.", thePath.c_str()));
  }

  int rv = ::mkdir(thePath.c_str(), mode);
  readStat();
  return rv;
}

int File::mkdirs(int mode /*= 0755*/) {
  if (exist()) {
    if (isDirectory())
      return 0; // esiste ed e' una directory

    throw FileException(
        format("La path %s esiste ma non è una directory.", thePath.c_str()));
  }

  int rv = recurseMkdir(*this, mode);
  readStat();
  return rv;
}

int File::recurseMkdir(File &d, int mode) {
  int rv;

  if ((rv = d.mkdir(mode)) == -1 && errno == ENOENT) {
    // manca uno dei componenti
    File parent = d.getParent();
    if ((rv = recurseMkdir(parent, mode)) != -1) {
      // riprova la creazione
      rv = d.mkdir(mode);
    }
  }

  return rv;
}

File File::createTempFile(const String &prefix, const String &suffix,
                          const File &dir) {
#if defined(_BSD_SOURCE) || defined(CYGWIN) || defined(MAC_OS_X) ||            \
    _POSIX_C_SOURCE >= 200112L
  String tmpName = dir.getAbsolutePath() + getPathSep() + prefix + "XXXXXX";
  char *ptTmpName = strdupa(tmpName.c_str());

  int handle = mkstemp(ptTmpName);
  if (handle == -1)
    throw FileException("Impossibile creare il file temporaneo.");

  close(handle);
  unlink(ptTmpName);

  tmpName = ptTmpName;
  if (!suffix.empty())
    tmpName += suffix;

  return File(tmpName);
#else
  const char *c_path = tempnam(dir.c_str(), prefix);
  String path(c_path);
  free((void *)c_path);
  path += suffix;
  return path;
#endif
}

File File::createTempFile(const String &prefix, const String &suffix) {
  return createTempFile(prefix, suffix, getTmpDir());
}

#if defined(_BSD_SOURCE) || defined(CYGWIN) || defined(MAC_OS_X) ||            \
    _POSIX_C_SOURCE >= 200112L

int File::createUnixTempFile(File &fTemp, const String &prefix,
                             const File &dir) {
  String tmpName = dir.getAbsolutePath() + getPathSep() + prefix + "XXXXXX";
  char *ptTmpName = strdupa(tmpName.c_str());

  int handle = mkstemp(ptTmpName);
  if (handle == -1)
    throw FileException("Impossibile creare il file temporaneo.");

  fTemp.init(ptTmpName, "");
  return handle;
}

int File::createUnixTempFile(File &fTemp, const String &prefix /*= "temp"*/) {
  return createUnixTempFile(fTemp, prefix, getTmpDir());
}

#endif

File File::getTmpDir() {
  char *p = getenv("TEMP");
  return File(correctPath(p == NULL ? "/tmp" : p, getPathSep()));
}

int File::touch() {
  int fd;

  if (isDirectory())
    throw FileException("touch() non consentito su una directory");

  if ((fd = open(thePath.c_str(), O_RDWR | O_CREAT, 0664)) == -1)
    return -1;

  close(fd);
  readStat();
  return 0;
}

int File::createLockFile(bool wait /*= true*/, long timeoutMillis /*= 0*/) {
  if (isDirectory())
    throw FileException("createLockFile() non consentito su una directory");

  if (fdLock > 0)
    return fdLock;

  boost::chrono::system_clock::time_point startTime =
      boost::chrono::system_clock::now();

tryagain:
  if ((fdLock = open(thePath.c_str(), O_RDWR | O_CREAT | O_EXCL, 0664)) == -1) {
    if (wait && errno == EEXIST) {
      if (timeoutMillis > 0) {
        // misura il tempo trascorso dal primo tentativo di lock
        boost::chrono::duration<double> sec =
            boost::chrono::system_clock::now() - startTime;

        if (timeoutMillis > ((long)(sec.count() * 1000)))
          return -1;
      }

      sched_yield();
      goto tryagain;
    }
    return -1;
  }

  readStat();
  return fdLock;
}

int File::removeLockFile() {
  if (isDirectory())
    throw FileException("removeLockFile() non consentito su una directory");

  if (fdLock <= 0)
    throw FileException("removeLockFile() va chiamata dopo createLockFile() "
                        "sullo stesso oggetto");

  close(fdLock);
  unlink(thePath.c_str());
  fdLock = 0;

  readStat();
  return 0;
}

bool File::isChanged() const {
  struct stat st;

  if (thePath.length() == 0)
    return false;

  if (existFlg != (stat(thePath.c_str(), &st) == 0))
    return true;

#ifdef MAC_OS_X
  if (tsCompare(si.st_mtimespec, st.st_mtimespec))
    return true;
  if (tsCompare(si.st_ctimespec, st.st_ctimespec))
    return true;
#else
  if (tsCompare(si.st_mtim, st.st_mtim))
    return true;
  if (tsCompare(si.st_ctim, st.st_ctim))
    return true;
#endif

  return false;
}

/**
 * Scrive un file binario.
 * Eventuali errori sono riportati in errno.
 * @param data buffer da scrivere su disco
 * @param len numero di byte da trasferire
 * @return numero di byte scritti
 */
int File::writeData(const void *data, int len) {
  if (thePath.length() == 0)
    return -1;

  FILE *fp = fopen(thePath.c_str(), "wb");
  if (fp == NULL)
    return -1;

  int nb = fwrite(data, 1, len, fp);

  fclose(fp);
  readStat();
  return nb;
}

void *File::readData(void *data /*= NULL*/, int len /*= 0*/) const {
  if (thePath.length() == 0 || !existFlg)
    return NULL;

  if (len == 0)
    len = length();

  if (data == NULL)
    if ((data = malloc(len)) == NULL)
      return NULL;

  FILE *fp = fopen(thePath.c_str(), "rb");
  if (fp == NULL) {
    free(data);
    return NULL;
  }

  int i = 0, nb = 0;
  do {
    nb = fread(((unsigned char *)data) + i, 1, len, fp);
    i += nb;
    len -= nb;
  } while (nb > 0);

  fclose(fp);
  return data;
}

/**
 * Legge un file di testo in memoria.
 * Viene applicato l'encoding di default del sistema.
 * @param linesRead vettore di stringhe da popolare con il contenuto
 * @param removeBlank se vero ingnora le linee vuote
 * @return numero di linee lette dal file (può essere maggiore degli elementi in
 * linesRead)
 */
int File::readTextFile(StringVector &linesRead,
                       bool removeBlank /*= true*/) const {
  FILE *fp = fopen(thePath.c_str(), "r");
  if (fp == NULL)
    throw FileException("File inesistente.");

  int count;
  char buffer[1024], *s;
  for (count = 0; (s = fgets(buffer, sizeof(buffer), fp)) != NULL; count++) {
    String st = trim(s);
    if (removeBlank && st.length() == 0)
      continue;

    linesRead.push_back(st);
  }

  fclose(fp);
  return count;
}

/**
 * Scrive un file di testo.
 * Viene applicato l'encoding di default del sistema.
 * Eventuali errori sono riportati in errno.
 * @param linesWrite array con le linee da scrivere
 * @param append se vero aggiunge in coda al file altrimenti sovrascrive
 * @return numero di linee scritte
 */
int File::writeTextFile(const StringVector &linesWrite,
                        bool append /*= false*/) {
  FILE *fp = fopen(thePath.c_str(), append ? "a" : "w");
  if (fp == NULL)
    throw FileException("Scrittura non possibile.");

  int n;
  int size = linesWrite.size();
  for (int i = 0; i < size; i++) {
    if ((n = fputs(linesWrite[i].c_str(), fp)) == EOF)
      break;
    fputc('\n', fp);
  }

  fclose(fp);
  readStat();

  if (n == EOF)
    throw FileException("Errore durante la scrittura.");

  return size;
}

String File::readString() const {
  FILE *fp = fopen(thePath.c_str(), "r");
  if (fp == NULL)
    throw FileException("File inesistente.");

  int nb = length();
  char *buffer = (char *)alloca(nb + 10);
  char *rv = fgets(buffer, nb, fp);

  fclose(fp);
  buffer[nb] = 0;

  if (rv == NULL)
    throw FileException("Errore durante la lettura.");

  return buffer;
}

void File::writeString(const String &stringWrite, bool append /*= false*/) {
  FILE *fp = fopen(thePath.c_str(), append ? "a" : "w");
  if (fp == NULL)
    throw FileException("Scrittura non possibile.");

  int n = fputs(stringWrite.c_str(), fp);
  fclose(fp);
  readStat();

  if (n == EOF)
    throw FileException("Errore durante la scrittura.");
}

int File::access(int mode /*= 0*/) const {
  return ::access(thePath.c_str(), mode);
}

/**
 * Corregge una path eliminando i possibili errori
 * che contiene da un punto di vista formale: rimuove
 * doppi slash (consentito solo all'inizio della path),
 * elimina eventuale slash finale.
 */
String File::correctPath(const String &path, char sep) {
  if (path.length() == 0)
    return "";

  if (path[0] == sep && path[1] == 0)
    return path;

  char *str = strdupa(path.c_str());
  char *p = str + 1;
  if (*p == 0)
    return path;

  // elimina doppi slash per caratteri successivi ai primi due
  for (; *p; p++) {
    if (*p == sep && *(p + 1) == sep) {
      for (char *k = p + 1; *k; k++)
        *k = *(k + 1);
    }
  }

  // elimina slash in coda
  if (*--p == sep)
    *p = 0;

  return str;
}

String File::getCurrentWorkingDir() {
  char cwd[FILENAME_MAX + 1];
  return getcwd(cwd, sizeof(cwd));
}
