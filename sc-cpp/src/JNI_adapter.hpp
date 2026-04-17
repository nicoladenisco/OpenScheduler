#ifndef _HANDLE_H_INCLUDED_
#define _HANDLE_H_INCLUDED_

#include "common.hpp"
#include <jni.h>

jfieldID getHandleField(JNIEnv *env, jobject obj) {
  jclass c = env->GetObjectClass(obj);
  // J is the type signature for long:
  return env->GetFieldID(c, "nativeAddress", "J");
}

template <typename T> T *getHandle(JNIEnv *env, jobject obj) {
  jlong handle = env->GetLongField(obj, getHandleField(env, obj));
  return reinterpret_cast<T *>(handle);
}

template <typename T> void setHandle(JNIEnv *env, jobject obj, T *t) {
  jlong handle = reinterpret_cast<jlong>(t);
  env->SetLongField(obj, getHandleField(env, obj), handle);
}

void setError(JNIEnv *env, jobject obj, String errMsg) {
  jclass c = env->GetObjectClass(obj);
  jfieldID errorField = env->GetFieldID(c, "nativeError", "Ljava/lang/String;");
  env->SetObjectField(obj, errorField, errMsg);
}

#endif
