#include <jni.h>
#include "JDyckCFL.h"
#include "dyckcfl.h"
#include "nonspec.h"

static dyck_node node_id_to_node(jlong nodeID) {
  return (dyck_node)nodeID;
}

satic jlong node_to_node_id(dyck_node n) {
  return (jlong)n;
}

JNIEXPORT void JNICALL Java_JDyckCFL_initialize
(JNIEnv *env, jclass clazz) {
  nonspec_init();
  dyck_init(TRUE);
}

/*
 * Class:     JDyckCFL
 * Method:    print_dyck_constraints
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL Java_JDyckCFL_printDyckConstraints
  (JNIEnv *env, jclass clazz, jboolean enablePrint) {
  if (enablePrint) flag_dyck_print_constraints = TRUE;
  else flag_dyck_print_constraints = FALSE;
}

/*
 * Class:     JDyckCFL
 * Method:    makeTaggedNode
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_JDyckCFL_makeTaggedNode
  (JNIEnv *env, jobject clazz, jstring name) {

  const char *str = (*env)->GetStringUTFChars(env, name, 0);
  dyck_node result = make_tagged_dyck_node(str);
  (*env)->ReleaseStringUTFChars(env, name, str);

  return node_to_node_id(result);
}

/*
 * Class:     JDyckCFL
 * Method:    makeUntaggedNode
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_JDyckCFL_makeUntaggedNode
  (JNIEnv *env, jobject clazz, jstring name) { 

  const char *str = (*env)->GetStringUTFChars(env, name, 0);
  dyck_node result = make_untagged_dyck_node(str);
  (*env)->ReleaseStringUTFChars(env, name, str);

  return (jlong)result;
}

/*
 * Class:     JDyckCFL
 * Method:    markNodeGlobal
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_JDyckCFL_markNodeGlobal
  (JNIEnv *env, jobject clazz, jlong nodeID) { 

  dyck_node n = node_id_to_node(nodeID);
  mark_dyck_node_global(n);
}

/*
 * Class:     JDyckCFL
 * Method:    makeSubtypeEdge
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_JDyckCFL_makeSubtypeEdge
  (JNIEnv *env, jobject clazz, jlong nodeID1, jlong nodeID2) { 

  dyck_node n1, n2;

  n1 = node_id_to_node(nodeID1);
  n2 = node_id_to_node(nodeID2);

  make_dyck_subtype_edge(n1, n2);
}

/*
 * Class:     JDyckCFL
 * Method:    makeOpenEdge
 * Signature: (JJI)V
 */
JNIEXPORT void JNICALL Java_JDyckCFL_makeOpenEdge
  (JNIEnv *env, jobject clazz, jlong nodeID1, jlong nodeID2 , jint index) { 

  dyck_node n1, n2;
  n1 = node_id_to_node(nodeID1);
  n2 = node_id_to_node(nodeID2);
  make_dyck_open_edge(n1, n2, index);


}

/*
 * Class:     JDyckCFL
 * Method:    makeCloseEdge
 * Signature: (JJI)V
 */
JNIEXPORT void JNICALL Java_JDyckCFL_makeCloseEdge
  (JNIEnv *env, jobject clazz, jlong nodeID1, jlong nodeID2, jint index) { 

  dyck_node n1, n2;
  n1 = node_id_to_node(nodeID1);
  n2 = node_id_to_node(nodeID2);
  make_dyck_close_edge(n1, n2, index);

}

/*
 * Class:     JDyckCFL
 * Method:    finishedAddingEdges
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_JDyckCFL_finishedAddingEdges
  (JNIEnv *env, jobject clazz) { 

  dyck_finished_adding();
}

/*
 * Class:     JDyckCFL
 * Method:    checkReaches
 * Signature: (JJ)Z
 */
JNIEXPORT jboolean JNICALL Java_JDyckCFL_checkReaches
  (JNIEnv *env, jobject clazz, jlong nodeID1, jlong nodeID2) { 
  
  dyck_node n1, n2;

  n1 = node_id_to_node(nodeID1);
  n2 = node_id_to_node(nodeID2);

  return dyck_check_reaches(n1,n2);
}

/*
 * Class:     JDyckCFL
 * Method:    checkPNReaches
 * Signature: (JJ)Z
 */
JNIEXPORT jboolean JNICALL Java_JDyckCFL_checkPNReaches
  (JNIEnv *env, jobject clazz, jlong nodeID1, jlong nodeID2) { 

  dyck_node n1, n2;

  n1 = node_id_to_node(nodeID1);
  n2 = node_id_to_node(nodeID2);

  return dyck_check_pn_reaches(n1,n2);
}


