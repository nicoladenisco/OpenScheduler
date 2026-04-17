#include "JNI_adapter.hpp"
#include "SchedResource.hpp"
#include "org_opensc_SchedResource.h"

/*
 * Class:     org_opensc_SchedResource
 * Method:    openNative
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_org_opensc_SchedResource_openNative(
    JNIEnv *env, jobject javaSchedResource, jstring javaPathFile) {
  try {
    const char *ptrPath = env->GetStringUTFChars(javaPathFile, NULL);
    File f(ptrPath);
    SchedResource *ptr = new SchedResource(f);
    setHandle<SchedResource>(env, javaSchedResource, ptr);
  } catch (Exception ex) {
    setError(env, javaSchedResource, ex.why());
  } catch (...) {
    setError(env, javaSchedResource, "Unknow internal error.");
  }
}

/*
 * Class:     org_opensc_SchedResource
 * Method:    closeNative
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_opensc_SchedResource_closeNative(
    JNIEnv *env, jobject javaSchedResource) {
  SchedResource *res = getHandle<SchedResource>(env, javaSchedResource);
  if (res != nullptr) {
    setHandle<SchedResource>(env, javaSchedResource, 0);
    delete res;
  }
}

/*
 * Class:     org_opensc_SchedResource
 * Method:    buildNative
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_org_opensc_SchedResource_buildNative(
    JNIEnv *env, jobject javaSchedResource, jstring javaPathFile) {
  try {
    const char *ptrPath = env->GetStringUTFChars(javaPathFile, NULL);
    File f(ptrPath);
    SchedResource *ptr = new SchedResource(f);
    setHandle<SchedResource>(env, javaSchedResource, ptr);
  } catch (Exception ex) {
    setError(env, javaSchedResource, ex.why());
  } catch (...) {
    setError(env, javaSchedResource, "Unknow internal error.");
  }
}
