#include <jni.h>
#include "UnsafeDyckCFL.h"
#include "dyckcfl.h"
#include "nonspec.h"

union dn {
  dyck_node node;
  jlong node_id;
};

static dyck_node node_id_to_node(jlong nodeID) {
  return ((union dn)nodeID).node;
}

static jlong node_to_node_id(dyck_node n) {
  return ((union dn)n).node_id;
}

JNIEXPORT void JNICALL Java_UnsafeDyckCFL_initialize
(JNIEnv *env, jclass clazz) {
  nonspec_init();
  dyck_init(TRUE);
}

/*
 * Class:     UnsafeDyckCFL
 * Method:    print_dyck_constraints
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL Java_UnsafeDyckCFL_printDyckConstraints
  (JNIEnv *env, jclass clazz, jboolean enablePrint) {
  if (enablePrint) flag_dyck_print_constraints = TRUE;
  else flag_dyck_print_constraints = FALSE;
}

/*
 * Class:     UnsafeDyckCFL
 * Method:    makeTaggedNode
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_UnsafeDyckCFL_makeTaggedNode
  (JNIEnv *env, jobject clazz, jstring name) {

  const char *str = (*env)->GetStringUTFChars(env, name, 0);
  dyck_node result = make_tagged_dyck_node(str);
  (*env)->ReleaseStringUTFChars(env, name, str);

  return node_to_node_id(result);
}

/*
 * Class:     UnsafeDyckCFL
 * Method:    makeUntaggedNode
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_UnsafeDyckCFL_makeUntaggedNode
  (JNIEnv *env, jobject clazz, jstring name) { 

  const char *str = (*env)->GetStringUTFChars(env, name, 0);
  dyck_node result = make_untagged_dyck_node(str);
  (*env)->ReleaseStringUTFChars(env, name, str);

  return node_to_node_id(result);
}

/*
 * Class:     UnsafeDyckCFL
 * Method:    markNodeGlobal
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_UnsafeDyckCFL_markNodeGlobal
  (JNIEnv *env, jobject clazz, jlong nodeID) { 

  dyck_node n = node_id_to_node(nodeID);
  mark_dyck_node_global(n);
}

/*
 * Class:     UnsafeDyckCFL
 * Method:    makeSubtypeEdge
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_UnsafeDyckCFL_makeSubtypeEdge
  (JNIEnv *env, jobject clazz, jlong nodeID1, jlong nodeID2) { 

  dyck_node n1, n2;

  n1 = node_id_to_node(nodeID1);
  n2 = node_id_to_node(nodeID2);

  make_dyck_subtype_edge(n1, n2);
}

/*
 * Class:     UnsafeDyckCFL
 * Method:    makeOpenEdge
 * Signature: (JJI)V
 */
JNIEXPORT void JNICALL Java_UnsafeDyckCFL_makeOpenEdge
  (JNIEnv *env, jobject clazz, jlong nodeID1, jlong nodeID2 , jint index) { 

  dyck_node n1, n2;
  n1 = node_id_to_node(nodeID1);
  n2 = node_id_to_node(nodeID2);
  make_dyck_open_edge(n1, n2, index);


}

/*
 * Class:     UnsafeDyckCFL
 * Method:    makeCloseEdge
 * Signature: (JJI)V
 */
JNIEXPORT void JNICALL Java_UnsafeDyckCFL_makeCloseEdge
  (JNIEnv *env, jobject clazz, jlong nodeID1, jlong nodeID2, jint index) { 

  dyck_node n1, n2;
  n1 = node_id_to_node(nodeID1);
  n2 = node_id_to_node(nodeID2);
  make_dyck_close_edge(n1, n2, index);

}

/*
 * Class:     UnsafeDyckCFL
 * Method:    finishedAddingEdges
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_UnsafeDyckCFL_finishedAddingEdges
  (JNIEnv *env, jobject clazz) { 

  dyck_finished_adding();
}

/*
 * Class:     UnsafeDyckCFL
 * Method:    checkReaches
 * Signature: (JJ)Z
 */
JNIEXPORT jboolean JNICALL Java_UnsafeDyckCFL_checkReaches
  (JNIEnv *env, jobject clazz, jlong nodeID1, jlong nodeID2) { 
  
  dyck_node n1, n2;

  n1 = node_id_to_node(nodeID1);
  n2 = node_id_to_node(nodeID2);

  return dyck_check_reaches(n1,n2);
}

/*
 * Class:     UnsafeDyckCFL
 * Method:    checkPNReaches
 * Signature: (JJ)Z
 */
JNIEXPORT jboolean JNICALL Java_UnsafeDyckCFL_checkPNReaches
  (JNIEnv *env, jobject clazz, jlong nodeID1, jlong nodeID2) { 

  dyck_node n1, n2;

  n1 = node_id_to_node(nodeID1);
  n2 = node_id_to_node(nodeID2);

  return dyck_check_pn_reaches(n1,n2);
}


