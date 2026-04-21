#include "JNI_adapter.hpp"
#include "Properties.hpp"
#include "SchedMerger.hpp"
#include "SchedResource.hpp"
#include "SchedStamper.hpp"
#include "common.hpp"
#include "org_opensc_SchedResource.h"

#define PROLOG(env, othis) try {

#define EPILOG(env, othis)                                                     \
  return 0;                                                                    \
  }                                                                            \
  catch (std::exception & ex) {                                                \
    setError(env, othis, ex.what());                                           \
  }                                                                            \
  catch (...) {                                                                \
    setError(env, othis, "Unknow internal error.");                            \
  }                                                                            \
  return -1;

#define PROLOG_STR(env, othis)                                                 \
  String retVal;                                                               \
  try {

#define EPILOG_STR(env, othis)                                                 \
  }                                                                            \
  catch (std::exception & ex) {                                                \
    setError(env, othis, ex.what());                                           \
    retVal = "ERROR";                                                          \
  }                                                                            \
  catch (...) {                                                                \
    setError(env, othis, "Unknow internal error.");                            \
    retVal = "ERROR";                                                          \
  }                                                                            \
  return env->NewStringUTF(retVal.c_str());

/*
 * Class:     org_opensc_SchedResource
 * Method:    setDebugMode
 * Signature: (I)V
 */
JNIEXPORT void JNICALL
Java_org_opensc_SchedResource_setDebugMode(JNIEnv *, jclass, jint jdebugMode) {
  debugOutput = jdebugMode;
}

/*
 * Class:     org_opensc_SchedResource
 * Method:    openNative
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT jint JNICALL Java_org_opensc_SchedResource_openNative(
    JNIEnv *env, jobject othis, jstring javaPathFile) {
  PROLOG(env, othis)
  const char *ptrPath = env->GetStringUTFChars(javaPathFile, NULL);
  File f(ptrPath);
  SchedResource *ptr = new SchedResource(f);
  setHandle<SchedResource>(env, othis, ptr);
  EPILOG(env, othis)
}

/*
 * Class:     org_opensc_SchedResource
 * Method:    closeNative
 * Signature: ()V
 */
JNIEXPORT jint JNICALL
Java_org_opensc_SchedResource_closeNative(JNIEnv *env, jobject othis) {
  PROLOG(env, othis)
  SchedResource *res = getHandle<SchedResource>(env, othis);
  if (res != nullptr) {
    setHandle<SchedResource>(env, othis, 0);
    delete res;
  }
  EPILOG(env, othis)
}

/*
 * Class:     org_opensc_SchedResource
 * Method:    buildNative
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT jint JNICALL Java_org_opensc_SchedResource_buildNative(
    JNIEnv *env, jobject othis, jstring jproperties) {
  PROLOG(env, othis)
  const char *ptrMapPipe = env->GetStringUTFChars(jproperties, NULL);
  Properties prop(ptrMapPipe);

  String codice = prop.getNotNull("codice");
  String nomeFile = prop.getNotNull("nomefile");
  int anno = prop.get("anno", 2026);
  int slotOra = prop.get("slotOra", 4);
  int oraIniziale = prop.get("oraIniziale", 8);
  int oraFinale = prop.get("oraFinale", 19);

  SlotFile generato;
  File genfile(nomeFile);
  if (genfile.isFile())
    throw NativeException("Il file indicato già esiste.");

  initSlotFile(anno, slotOra, oraIniziale, oraFinale, codice, generato,
               genfile);

  SchedResource *ptr = new SchedResource(genfile);
  setHandle<SchedResource>(env, othis, ptr);
  EPILOG(env, othis)
}

/*
 * Class:     org_opensc_SchedResource
 * Method:    clearAllSlotsNative
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_org_opensc_SchedResource_clearAllSlotsNative(
    JNIEnv *env, jobject othis, jint stato) {
  PROLOG(env, othis)
  SchedResource *res = getHandle<SchedResource>(env, othis);
  res->clearAllSlots(stato);
  EPILOG(env, othis)
}

/*
 * Class:     org_opensc_SchedResource
 * Method:    stampResourcesNative
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_org_opensc_SchedResource_stampResourcesNative(
    JNIEnv *env, jobject othis, jstring jalgo, jstring jproperties) {
  PROLOG(env, othis)
  SchedResource *res = getHandle<SchedResource>(env, othis);
  const char *ptrAlgo = env->GetStringUTFChars(jalgo, NULL);
  const char *ptrMapPipe = env->GetStringUTFChars(jproperties, NULL);
  Properties prop(ptrMapPipe);
  SchedStamper stamper;

  if (debugOutput) {
    cout << DEBUGOUT << "Algo: " << ptrAlgo << std::endl;
    cout << DEBUGOUT << prop.toString() << std::endl;
  }

  // verifica per algoritmo esistente
  StringVector names;
  stamper.getAlgoNames(names);
  if (!contains(ptrAlgo, names))
    throw NativeException(
        format("Algoritmo %s inesistente: deve essere uno di %s", ptrAlgo,
               join(names, ",", "'").c_str()));

  // lock della risorsa
  long timeout = prop.get("lockDelayMillis", 3000);
  SchedResourceLock reslock(*res, "stamp", false, true, timeout);
  if (!reslock.isLocked())
    throw NativeException(
        "Non riesco a bloccare la risorsa; operazione abortita.");

  stamper.stampResource(*res, ptrAlgo, prop);
  EPILOG(env, othis)
}

/*
 * Class:     org_opensc_SchedResource
 * Method:    getStamperAlgosNative
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_org_opensc_SchedResource_getStamperAlgosNative(
    JNIEnv *env, jobject othis) {
  PROLOG_STR(env, othis)
  SchedStamper stamper;
  StringVector names;
  stamper.getAlgoNames(names);
  retVal = join(names, "|", "");
  EPILOG_STR(env, othis)
}

/*
 * Class:     org_opensc_SchedResource
 * Method:    dumpHeaderNative
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_org_opensc_SchedResource_dumpHeaderNative(
    JNIEnv *env, jobject othis, jstring jproperties) {
  PROLOG_STR(env, othis)
  SchedResource *res = getHandle<SchedResource>(env, othis);
  retVal = toString(*res->getSlotFile());
  EPILOG_STR(env, othis)
}

/*
 * Class:     org_opensc_SchedResource
 * Method:    dumpSlotsNative
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_org_opensc_SchedResource_dumpSlotsNative(
    JNIEnv *env, jobject othis, jstring jproperties) {
  PROLOG_STR(env, othis)
  SchedResource *res = getHandle<SchedResource>(env, othis);
  const char *ptrMapPipe = env->GetStringUTFChars(jproperties, NULL);
  Properties prop(ptrMapPipe);
  IntPair days = prop.parseDays();
  retVal = dump(*res->getSlotFile(), days.first, days.second);
  EPILOG_STR(env, othis)
}

////////////////////////////////////////////////////////////////////////////////////

/*
 * Class:     org_opensc_SchedMerger
 * Method:    openNative
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_org_opensc_SchedMerger_openNative(JNIEnv *env,
                                                              jobject othis,
                                                              jlong unique) {
  PROLOG(env, othis)
  SchedMerger *ptr = new SchedMerger(unique);
  setHandle<SchedMerger>(env, othis, ptr);
  EPILOG(env, othis)
}

/*
 * Class:     org_opensc_SchedMerger
 * Method:    closeNative
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_org_opensc_SchedMerger_closeNative(JNIEnv *env,
                                                               jobject othis) {
  PROLOG(env, othis)
  SchedMerger *res = getHandle<SchedMerger>(env, othis);
  if (res != nullptr) {
    setHandle<SchedMerger>(env, othis, 0);
    delete res;
  }
  EPILOG(env, othis)
}

/*
 * Class:     org_opensc_SchedMerger
 * Method:    dumpSlotsNative
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_org_opensc_SchedMerger_dumpSlotsNative(
    JNIEnv *env, jobject othis, jstring jproperties) {
  PROLOG_STR(env, othis)
  SchedMerger *merger = getHandle<SchedMerger>(env, othis);
  const char *ptrMapPipe = env->GetStringUTFChars(jproperties, NULL);
  Properties prop(ptrMapPipe);
  IntPair days = prop.parseDays();

  const SlotFile *ptSlot = merger->getMerged();

  if (ptSlot == nullptr)
    throw NativeException("Il merger è vuoto.");

  retVal = dump(*ptSlot, days.first, days.second);
  EPILOG_STR(env, othis)
}

/*
 * Class:     org_opensc_SchedMerger
 * Method:    getMergerAlgosNative
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_org_opensc_SchedMerger_getMergerAlgosNative(JNIEnv *env, jobject othis) {
  PROLOG_STR(env, othis)
  SchedMerger *merger = getHandle<SchedMerger>(env, othis);
  StringVector names;
  merger->getAlgoNames(names);
  retVal = join(names, "|", "");
  EPILOG_STR(env, othis)
}

/*
 * Class:     org_opensc_SchedMerger
 * Method:    mergeResourcesNative
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_org_opensc_SchedMerger_mergeResourcesNative(
    JNIEnv *env, jobject othis, jstring jalgo, jstring jproperties) {
  PROLOG(env, othis)
  SchedMerger *merger = getHandle<SchedMerger>(env, othis);
  const char *ptrAlgo = env->GetStringUTFChars(jalgo, NULL);
  const char *ptrMapPipe = env->GetStringUTFChars(jproperties, NULL);
  Properties prop(ptrMapPipe);

  String codice = prop.getNotNull("codice");
  String nomeFile = prop.getNotNull("nomefile");

  // verifica per risorsa gia presente
  if (merger->checkRisorsa(codice))
    throw NativeException(format(
        "La risorsa con codice %s è stata già inclusa.", codice.c_str()));

  // verifica per algoritmo esistente
  StringVector names;
  merger->getAlgoNames(names);
  if (!contains(ptrAlgo, names))
    throw NativeException(
        format("Algoritmo %s inesistente: deve essere uno di %s", ptrAlgo,
               join(names, ",", "'").c_str()));

  // verifica per file risorsa
  File genfile(nomeFile);
  if (!genfile.isFile())
    throw NativeException(
        format("File di risorsa %s inesistente.", nomeFile.c_str()));

  // carica risorsa
  SchedResourcePtr res = buildResource(genfile);
  if (!res->isInitialized())
    throw NativeException(
        "La risorsa non è stata inizializzata; usare uno stamper per "
        "poterla usare.");

  // fonde risorsa
  long timeout = prop.get("lockDelayMillis", 5000);
  SchedResourceMultiLock multilock("merge", false, true, timeout);
  merger->addResource(multilock, res, ptrAlgo, prop);
  EPILOG(env, othis)
}

/*
 * Class:     org_opensc_SchedMerger
 * Method:    getResourcesListNative
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_org_opensc_SchedMerger_getResourcesListNative(JNIEnv *env, jobject othis) {
  PROLOG_STR(env, othis)
  SchedMerger *merger = getHandle<SchedMerger>(env, othis);
  StringVector names;
  merger->getResourcesCode(names);
  retVal = join(names, "|", "");
  EPILOG_STR(env, othis)
}

/*
 * Class:     org_opensc_SchedMerger
 * Method:    clearResourcesNative
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
Java_org_opensc_SchedMerger_clearResourcesNative(JNIEnv *env, jobject othis) {
  PROLOG(env, othis)
  SchedMerger *merger = getHandle<SchedMerger>(env, othis);
  merger->clear();
  EPILOG(env, othis)
}

/*
 * Class:     org_opensc_SchedMerger
 * Method:    reserveSlotNative
 * Signature: (IIJLjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_org_opensc_SchedMerger_reserveSlotNative(
    JNIEnv *env, jobject othis, jint giorno, jint slotgiorno, jlong uniqueid,
    jstring jproperties) {
  PROLOG(env, othis)
  SchedMerger *merger = getHandle<SchedMerger>(env, othis);
  const char *ptrMapPipe = env->GetStringUTFChars(jproperties, NULL);
  Properties prop(ptrMapPipe);

  long timeout = prop.get("lockDelayMillis", 5000);
  SchedResourceMultiLock multilock("merge", false, true, timeout);
  merger->reserveSlot(multilock, giorno, slotgiorno, uniqueid, prop);
  EPILOG(env, othis)
}
