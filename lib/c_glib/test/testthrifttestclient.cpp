/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/* test a C client with a C++ server */

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/protocol/TDebugProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include "ThriftTest.h"
#include "ThriftTest_types.h"

#include <iostream>

using namespace std;
using namespace boost;

using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

using namespace thrift::test;

#define TEST_PORT 9980

// Extra functions required for ThriftTest_types to work
namespace thrift { namespace test {

bool Insanity::operator<(thrift::test::Insanity const& other) const {
  using apache::thrift::ThriftDebugString;
  return ThriftDebugString(*this) < ThriftDebugString(other);
}

}}

class TestHandler : public ThriftTestIf {
  public:
  TestHandler() {}

  void testVoid() {
    printf("[C -> C++] testVoid()\n");
  }

  void testString(string& out, const string &thing) {
    printf("[C -> C++] testString(\"%s\")\n", thing.c_str());
    out = thing;
  }

  int8_t testByte(const int8_t thing) {
    printf("[C -> C++] testByte(%d)\n", (int)thing);
    return thing;
  }
  int32_t testI32(const int32_t thing) {
    printf("[C -> C++] testI32(%d)\n", thing);
    return thing;
  }

  int64_t testI64(const int64_t thing) {
    printf("[C -> C++] testI64(%ld)\n", thing);
    return thing;
  }

  double testDouble(const double thing) {
    printf("[C -> C++] testDouble(%lf)\n", thing);
    return thing;
  }

  void testBinary(string& out, const string &thing) {
    printf("[C -> C++] testBinary(\"%s\")\n", thing.c_str());
    out = thing;
  }

  void testStruct(Xtruct& out, const Xtruct &thing) {
    printf("[C -> C++] testStruct({\"%s\", %d, %d, %ld})\n", thing.string_thing.c_str(), (int)thing.byte_thing, thing.i32_thing, thing.i64_thing);
    out = thing;
  }

  void testNest(Xtruct2& out, const Xtruct2& nest) {
    const Xtruct &thing = nest.struct_thing;
    printf("[C -> C++] testNest({%d, {\"%s\", %d, %d, %ld}, %d})\n", (int)nest.byte_thing, thing.string_thing.c_str(), (int)thing.byte_thing, thing.i32_thing, thing.i64_thing, nest.i32_thing);
    out = nest;
  }

  void testMap(map<int32_t, int32_t> &out, const map<int32_t, int32_t> &thing) {
    printf("[C -> C++] testMap({");
    map<int32_t, int32_t>::const_iterator m_iter;
    bool first = true;
    for (m_iter = thing.begin(); m_iter != thing.end(); ++m_iter) {
      if (first) {
        first = false;
      } else {
        printf(", ");
      }
      printf("%d => %d", m_iter->first, m_iter->second);
    }
    printf("})\n");
    out = thing;
  }

  void testStringMap(map<std::string, std::string> &out, const map<std::string, std::string> &thing) {
    printf("[C -> C++] testStringMap({");
    map<std::string, std::string>::const_iterator m_iter;
    bool first = true;
    for (m_iter = thing.begin(); m_iter != thing.end(); ++m_iter) {
      if (first) {
        first = false;
      } else {
        printf(", ");
      }
      printf("\"%s\" => \"%s\"", (m_iter->first).c_str(), (m_iter->second).c_str());
    }
    printf("})\n");
    out = thing;
  }


  void testSet(set<int32_t> &out, const set<int32_t> &thing) {
    printf("[C -> C++] testSet({");
    set<int32_t>::const_iterator s_iter;
    bool first = true;
    for (s_iter = thing.begin(); s_iter != thing.end(); ++s_iter) {
      if (first) {
        first = false;
      } else {
        printf(", ");
      }
      printf("%d", *s_iter);
    }
    printf("})\n");
    out = thing;
  }

  void testList(vector<int32_t> &out, const vector<int32_t> &thing) {
    printf("[C -> C++] testList({");
    vector<int32_t>::const_iterator l_iter;
    bool first = true;
    for (l_iter = thing.begin(); l_iter != thing.end(); ++l_iter) {
      if (first) {
        first = false;
      } else {        printf(", ");
      }
      printf("%d", *l_iter);
    }
    printf("})\n");
    out = thing;
  }

  Numberz::type testEnum(const Numberz::type thing) {
    printf("[C -> C++] testEnum(%d)\n", thing);
    return thing;
  }

  UserId testTypedef(const UserId thing) {
    printf("[C -> C++] testTypedef(%ld)\n", thing);
    return thing;  }

  void testMapMap(map<int32_t, map<int32_t,int32_t> > &mapmap, const int32_t hello) {
    printf("[C -> C++] testMapMap(%d)\n", hello);

    map<int32_t,int32_t> pos;
    map<int32_t,int32_t> neg;
    for (int i = 1; i < 5; i++) {
      pos.insert(make_pair(i,i));
      neg.insert(make_pair(-i,-i));
    }

    mapmap.insert(make_pair(4, pos));
    mapmap.insert(make_pair(-4, neg));

  }

  void testInsanity(map<UserId, map<Numberz::type,Insanity> > &insane, const Insanity &argument) {
    printf("[C -> C++] testInsanity()\n");

    Xtruct hello;
    hello.string_thing = "Hello2";
    hello.byte_thing = 2;
    hello.i32_thing = 2;
    hello.i64_thing = 2;

    Xtruct goodbye;
    goodbye.string_thing = "Goodbye4";
    goodbye.byte_thing = 4;
    goodbye.i32_thing = 4;
    goodbye.i64_thing = 4;

    Insanity crazy;
    crazy.userMap.insert(make_pair(Numberz::EIGHT, 8));
    crazy.xtructs.push_back(goodbye);

    Insanity looney;
    crazy.userMap.insert(make_pair(Numberz::FIVE, 5));
    crazy.xtructs.push_back(hello);

    map<Numberz::type, Insanity> first_map;
    map<Numberz::type, Insanity> second_map;

    first_map.insert(make_pair(Numberz::TWO, crazy));
    first_map.insert(make_pair(Numberz::THREE, crazy));

    second_map.insert(make_pair(Numberz::SIX, looney));

    insane.insert(make_pair(1, first_map));
    insane.insert(make_pair(2, second_map));

    printf("return");
    printf(" = {");
    map<UserId, map<Numberz::type,Insanity> >::const_iterator i_iter;
    for (i_iter = insane.begin(); i_iter != insane.end(); ++i_iter) {
      printf("%ld => {", i_iter->first);
      map<Numberz::type,Insanity>::const_iterator i2_iter;
      for (i2_iter = i_iter->second.begin();
           i2_iter != i_iter->second.end();
           ++i2_iter) {
        printf("%d => {", i2_iter->first);
        map<Numberz::type, UserId> userMap = i2_iter->second.userMap;
        map<Numberz::type, UserId>::const_iterator um;
        printf("{");
        for (um = userMap.begin(); um != userMap.end(); ++um) {
          printf("%d => %ld, ", um->first, um->second);
        }
        printf("}, ");

        vector<Xtruct> xtructs = i2_iter->second.xtructs;
        vector<Xtruct>::const_iterator x;
        printf("{");
        for (x = xtructs.begin(); x != xtructs.end(); ++x) {
          printf("{\"%s\", %d, %d, %ld}, ", x->string_thing.c_str(), (int)x->byte_thing, x->i32_thing, x->i64_thing);
        }
        printf("}");

        printf("}, ");
      }
      printf("}, ");
    }
    printf("}\n");


  }

  void testMulti(Xtruct &hello, const int8_t arg0, const int32_t arg1, const int64_t arg2, const std::map<int16_t, std::string>  &arg3, const Numberz::type arg4, const UserId arg5) {
    printf("[C -> C++] testMulti()\n");

    hello.string_thing = "Hello2";
    hello.byte_thing = arg0;
    hello.i32_thing = arg1;
    hello.i64_thing = (int64_t)arg2;
  }

  void testException(const std::string &arg)
    throw(Xception, apache::thrift::TException)
  {
    printf("[C -> C++] testException(%s)\n", arg.c_str());
    if (arg.compare("Xception") == 0) {
      Xception e;
      e.errorCode = 1001;
      e.message = arg;
      throw e;
    } else if (arg.compare("ApplicationException") == 0) {
      apache::thrift::TException e;
      throw e;
    } else {
      Xtruct result;
      result.string_thing = arg;
      return;
    }
  }

  void testMultiException(Xtruct &result, const std::string &arg0, const std::string &arg1) throw(Xception, Xception2) {

    printf("[C -> C++] testMultiException(%s, %s)\n", arg0.c_str(), arg1.c_str());

    if (arg0.compare("Xception") == 0) {
      Xception e;
      e.errorCode = 1001;
      e.message = "This is an Xception";
      throw e;
    } else if (arg0.compare("Xception2") == 0) {
      Xception2 e;
      e.errorCode = 2002;
      e.struct_thing.string_thing = "This is an Xception2";
      throw e;
    } else {
      result.string_thing = arg1;
      return;
    }
  }

  void testOneway(int sleepFor) {
    printf("testOneway(%d): Sleeping...\n", sleepFor);
    sleep(sleepFor);
    printf("testOneway(%d): done sleeping!\n", sleepFor);
  }
};

// C CLIENT
extern "C" {

#include "t_test_thrift_test.h"
#include "t_test_thrift_test_types.h"
#include <thrift/c_glib/transport/thrift_socket.h>
#include <thrift/c_glib/protocol/thrift_protocol.h>
#include <thrift/c_glib/protocol/thrift_binary_protocol.h>

static void
test_thrift_client (void)
{
  ThriftSocket *tsocket = NULL;
  ThriftBinaryProtocol *protocol = NULL;
  TTestThriftTestClient *client = NULL;
  TTestThriftTestIf *iface = NULL;
  GError *error = NULL;
  gchar *string = NULL;
  gint8 byte = 0;
  gint16 i16 = 0;
  gint32 i32 = 0, another_i32 = 56789; 
  gint64 i64 = 0;
  double dbl = 0.0;
  TTestXtruct *xtruct_in, *xtruct_out;
  TTestXtruct2 *xtruct2_in, *xtruct2_out;
  GHashTable *map_in = NULL, *map_out = NULL;
  GHashTable *set_in = NULL, *set_out = NULL;
  GArray *list_in = NULL, *list_out = NULL;
  TTestNumberz enum_in, enum_out;
  TTestUserId user_id_in, user_id_out; 
  GHashTable *insanity_in = NULL;
  TTestXtruct *xtruct1, *xtruct2;
  TTestInsanity *insanity_out = NULL;
  TTestXtruct *multi_in = NULL;
  GHashTable *multi_map_out = NULL;
  TTestXception *xception = NULL;
  TTestXception2 *xception2 = NULL;

  // initialize gobject
  g_type_init ();

  // create a C client
  tsocket = (ThriftSocket *) g_object_new (THRIFT_TYPE_SOCKET, 
                          "hostname", "localhost",
                          "port", TEST_PORT, NULL);
  protocol = (ThriftBinaryProtocol *) g_object_new (THRIFT_TYPE_BINARY_PROTOCOL,
                           "transport",
                           tsocket, NULL);
  client = (TTestThriftTestClient *) g_object_new (T_TEST_TYPE_THRIFT_TEST_CLIENT, "input_protocol", protocol, "output_protocol", protocol, NULL);
  iface = T_TEST_THRIFT_TEST_IF (client);

  // open and send
  thrift_transport_open (THRIFT_TRANSPORT(tsocket), NULL);

  assert (t_test_thrift_test_client_test_void (iface, &error) == TRUE);
  assert (error == NULL);

  assert (t_test_thrift_test_client_test_string (iface, &string, "test123", &error) == TRUE);
  assert (strcmp (string, "test123") == 0);
  g_free (string);
  assert (error == NULL);

  assert (t_test_thrift_test_client_test_byte (iface, &byte, (gint8) 5, &error) == TRUE);
  assert (byte == 5);
  assert (error == NULL);

  assert (t_test_thrift_test_client_test_i32 (iface, &i32, 123, &error) == TRUE);
  assert (i32 == 123);
  assert (error == NULL);

  assert (t_test_thrift_test_client_test_i64 (iface, &i64, 12345, &error) == TRUE);
  assert (i64 == 12345);
  assert (error == NULL);

  assert (t_test_thrift_test_client_test_double (iface, &dbl, 5.6, &error) == TRUE);
  assert (dbl == 5.6);
  assert (error == NULL);

  xtruct_out = (TTestXtruct *) g_object_new (T_TEST_TYPE_XTRUCT, NULL);
  xtruct_out->byte_thing = 1;
  xtruct_out->__isset_byte_thing = TRUE;
  xtruct_out->i32_thing = 15;
  xtruct_out->__isset_i32_thing = TRUE;
  xtruct_out->i64_thing = 151;
  xtruct_out->__isset_i64_thing = TRUE;
  xtruct_out->string_thing = g_strdup ("abc123");
  xtruct_out->__isset_string_thing = TRUE;
  xtruct_in = (TTestXtruct *) g_object_new(T_TEST_TYPE_XTRUCT, NULL);
  assert (t_test_thrift_test_client_test_struct (iface, &xtruct_in, xtruct_out, &error) == TRUE);
  assert (error == NULL);

  xtruct2_out = (TTestXtruct2 *) g_object_new (T_TEST_TYPE_XTRUCT2, NULL);
  xtruct2_out->byte_thing = 1;
  xtruct2_out->__isset_byte_thing = TRUE;
  if (xtruct2_out->struct_thing != NULL)
    g_object_unref(xtruct2_out->struct_thing);
  xtruct2_out->struct_thing = xtruct_out;
  xtruct2_out->__isset_struct_thing = TRUE;
  xtruct2_out->i32_thing = 123;
  xtruct2_out->__isset_i32_thing = TRUE;
  xtruct2_in = (TTestXtruct2 *) g_object_new (T_TEST_TYPE_XTRUCT2, NULL);
  assert (t_test_thrift_test_client_test_nest (iface, &xtruct2_in, xtruct2_out, &error) == TRUE);
  assert (error == NULL);

  g_object_unref (xtruct2_out);
  g_object_unref (xtruct2_in);
  g_object_unref (xtruct_in);

  map_out = g_hash_table_new (NULL, NULL);
  map_in = g_hash_table_new (NULL, NULL);  g_hash_table_insert (map_out, &i32, &i32);
  assert (t_test_thrift_test_client_test_map (iface, &map_in, map_out, &error) == TRUE);
  assert (error == NULL);
  g_hash_table_destroy (map_out);
  g_hash_table_destroy (map_in);

  map_out = g_hash_table_new (NULL, NULL);
  map_in = g_hash_table_new (NULL, NULL);
  g_hash_table_insert (map_out, g_strdup ("a"), g_strdup ("123"));
  g_hash_table_insert (map_out, g_strdup ("a b"), g_strdup ("with spaces "));
  g_hash_table_insert (map_out, g_strdup ("same"), g_strdup ("same"));
  g_hash_table_insert (map_out, g_strdup ("0"), g_strdup ("numeric key"));
  assert (t_test_thrift_test_client_test_string_map (iface, &map_in, map_out, &error) == TRUE);
  assert (error == NULL);
  g_hash_table_destroy (map_out);
  g_hash_table_destroy (map_in);

  set_out = g_hash_table_new (NULL, NULL);
  set_in = g_hash_table_new (NULL, NULL);
  g_hash_table_insert (set_out, &i32, &i32);
  assert (t_test_thrift_test_client_test_set (iface, &set_in, set_out, &error) == TRUE);
  assert (error == NULL);
  g_hash_table_destroy (set_out);
  g_hash_table_destroy (set_in);

  list_out = g_array_new(TRUE, TRUE, sizeof(gint32));
  list_in = g_array_new(TRUE, TRUE, sizeof(gint32));
  another_i32 = 456;
  g_array_append_val (list_out, i32);
  g_array_append_val (list_out, another_i32);
  assert (t_test_thrift_test_client_test_list (iface, &list_in, list_out, &error) == TRUE);
  assert (error == NULL);
  g_array_free (list_out, TRUE);
  g_array_free (list_in, TRUE);

  enum_out = T_TEST_NUMBERZ_ONE;
  assert (t_test_thrift_test_client_test_enum (iface, &enum_in, enum_out, &error) == TRUE);
  assert (enum_in == enum_out);
  assert (error == NULL);

  user_id_out = 12345;
  assert (t_test_thrift_test_client_test_typedef (iface, &user_id_in, user_id_out, &error) == TRUE);
  assert (user_id_in == user_id_out);
  assert (error == NULL);

  map_in = g_hash_table_new (NULL, NULL);
  assert (t_test_thrift_test_client_test_map_map (iface, &map_in, i32, &error) == TRUE);
  assert (error == NULL);
  g_hash_table_destroy (map_in);

  // insanity
  insanity_out = (TTestInsanity *) g_object_new (T_TEST_TYPE_INSANITY, NULL);
  insanity_out->userMap = g_hash_table_new (NULL, NULL);
  g_hash_table_insert (insanity_out->userMap, GINT_TO_POINTER (enum_out), &user_id_out);

  xtruct1 = (TTestXtruct *) g_object_new (T_TEST_TYPE_XTRUCT, NULL);
  xtruct1->byte_thing = 1;
  xtruct1->__isset_byte_thing = TRUE;
  xtruct1->i32_thing = 15;
  xtruct1->__isset_i32_thing = TRUE;
  xtruct1->i64_thing = 151;
  xtruct1->__isset_i64_thing = TRUE;
  xtruct1->string_thing = g_strdup ("abc123");
  xtruct1->__isset_string_thing = TRUE;
  xtruct2 = (TTestXtruct *) g_object_new (T_TEST_TYPE_XTRUCT, NULL);
  xtruct2->byte_thing = 1;
  xtruct2->__isset_byte_thing = TRUE;
  xtruct2->i32_thing = 15;
  xtruct2->__isset_i32_thing = TRUE;
  xtruct2->i64_thing = 151;
  xtruct2->__isset_i64_thing = TRUE;
  xtruct2->string_thing = g_strdup ("abc123");
  xtruct2->__isset_string_thing = TRUE;

  insanity_in = g_hash_table_new (NULL, NULL);
  g_ptr_array_add (insanity_out->xtructs, xtruct1);
  g_ptr_array_add (insanity_out->xtructs, xtruct2);
  assert (t_test_thrift_test_client_test_insanity (iface, &insanity_in, insanity_out, &error) == TRUE);

  g_hash_table_unref (insanity_in);
  g_ptr_array_free (insanity_out->xtructs, TRUE);

  multi_map_out = g_hash_table_new (NULL, NULL);
  string = g_strdup ("abc123");
  g_hash_table_insert (multi_map_out, &i16, string);
  multi_in = (TTestXtruct *) g_object_new (T_TEST_TYPE_XTRUCT, NULL);
  assert (t_test_thrift_test_client_test_multi (iface, &multi_in, byte, i32, i64, multi_map_out, enum_out, user_id_out, &error) == TRUE);
  assert (multi_in->i32_thing == i32);
  assert (multi_in->i64_thing == i64);
  g_object_unref (multi_in);
  g_hash_table_unref (multi_map_out);
  g_free (string);

  assert (t_test_thrift_test_client_test_exception (iface, "Xception", &xception, &error) == FALSE);
  assert (xception->errorCode == 1001);
  g_error_free (error);
  error = NULL;
  g_object_unref (xception);
  xception = NULL;

  assert (t_test_thrift_test_client_test_exception (iface, "ApplicationException", &xception, &error) == FALSE);
  g_error_free (error);
  error = NULL;
  g_object_unref (xception);
  xception = NULL;

  assert (t_test_thrift_test_client_test_exception (iface, "Test", &xception, &error) == TRUE);
  assert (error == NULL);

  multi_in = (TTestXtruct*) g_object_new (T_TEST_TYPE_XTRUCT, NULL);
  assert (t_test_thrift_test_client_test_multi_exception (iface, &multi_in, "Xception", NULL, &xception, &xception2, &error) == FALSE);
  assert (xception->errorCode == 1001);
  assert (xception2 == NULL);
  g_error_free (error);
  error = NULL;
  g_object_unref (xception);
  g_object_unref (multi_in);
  xception = NULL;
  multi_in = NULL;

  multi_in = (TTestXtruct*) g_object_new (T_TEST_TYPE_XTRUCT, NULL);
  assert (t_test_thrift_test_client_test_multi_exception (iface, &multi_in, "Xception2", NULL, &xception, &xception2, &error) == FALSE);
  assert (xception2->errorCode == 2002);
  assert (xception == NULL);
  g_error_free (error);
  error = NULL;
  g_object_unref (xception2);
  g_object_unref (multi_in);
  xception2 = NULL;
  multi_in = NULL;

  multi_in = (TTestXtruct*) g_object_new (T_TEST_TYPE_XTRUCT, NULL);
  assert (t_test_thrift_test_client_test_multi_exception (iface, &multi_in, NULL , NULL, &xception, &xception2, &error) == TRUE);
  assert (error == NULL);
  g_object_unref(multi_in);
  multi_in = NULL;

  assert (t_test_thrift_test_client_test_oneway (iface, 1, &error) == TRUE);
  assert (error == NULL);

  /* sleep to let the oneway call go through */
  sleep (5);

  thrift_transport_close (THRIFT_TRANSPORT(tsocket), NULL);
  g_object_unref (client);
  g_object_unref (protocol);
  g_object_unref (tsocket);
}


} /* extern "C" */


static void
bailout (int signum)
{
  exit (1);
}

int
main (int argc, char **argv)
{
  int status;
  int pid = fork ();
  assert (pid >= 0);

  if (pid == 0) /* child */
  {
    shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
    shared_ptr<TestHandler> testHandler(new TestHandler());
    shared_ptr<ThriftTestProcessor> testProcessor(new ThriftTestProcessor(testHandler));
    shared_ptr<TServerSocket> serverSocket(new TServerSocket(TEST_PORT));
    shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
    TSimpleServer simpleServer(testProcessor, serverSocket, transportFactory, protocolFactory);
    signal (SIGALRM, bailout);
    alarm (60);
    simpleServer.serve();
  } else {
    sleep (1);
    test_thrift_client ();
    kill (pid, SIGINT);
    wait (&status) == pid;
  }

  return 0;
}

