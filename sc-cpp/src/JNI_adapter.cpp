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
  catch (std::exception ex) {                                                  \
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
  catch (std::exception ex) {                                                  \
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
  int slotOra = prop.get("slotOra", 2026);
  int oraIniziale = prop.get("oraIniziale", 2026);
  int oraFinale = prop.get("oraFinale", 2026);

  SlotFile generato;
  File genfile(nomeFile);
  if (genfile.isFile())
    throw GenericException("Il file indicato già esiste.");

  initSlotFile(anno, slotOra, oraIniziale, oraFinale, codice, generato,
               genfile);

  SchedResource *ptr = new SchedResource(genfile);
  setHandle<SchedResource>(env, othis, ptr);
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

  // lock della risorsa
  long timeout = prop.get("lockDelayMillis", 3000);
  SchedResourceLock reslock(*res, "stamp", false, true, timeout);
  if (!reslock.isLocked())
    throw StructureException(
        "Non riesco a bloccare la risorsa; operazione abortita.");

  SchedStamper stamper;
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
    throw StructureException("Il merger è vuoto.");

  retVal = dump(*ptSlot, days.first, days.second);
  EPILOG_STR(env, othis)
}
