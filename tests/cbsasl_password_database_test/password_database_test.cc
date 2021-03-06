/*
 *     Copyright 2016 Couchbase, Inc.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

#include <gtest/gtest.h>

#include <platform/platform.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <cbsasl/cbsasl.h>
#include <cbsasl/password_database.h>

#include <cbsasl/user.h>
#include <platform/base64.h>

class PasswordMetaTest : public ::testing::Test {
public:
    void SetUp() {
        root.reset(cJSON_CreateObject());
        cJSON_AddStringToObject(root.get(), "h",
                                "NP0b1Ji5jWG/ZV6hPzOIk3lmTmw=");
        cJSON_AddStringToObject(root.get(), "s",
                                "iiU7hLv7l3yOoEgXusJvT2i1J2A=");
        cJSON_AddNumberToObject(root.get(), "i", 10);
    }

    unique_cJSON_ptr root;
};

TEST_F(PasswordMetaTest, TestNormalInit) {
    Couchbase::User::PasswordMetaData md;
    EXPECT_NO_THROW(md = Couchbase::User::PasswordMetaData(root.get()));
    EXPECT_EQ("iiU7hLv7l3yOoEgXusJvT2i1J2A=", md.getSalt());
    EXPECT_EQ("NP0b1Ji5jWG/ZV6hPzOIk3lmTmw=",
              Couchbase::Base64::encode(md.getPassword()));
    EXPECT_EQ(10, md.getIterationCount());
}

TEST_F(PasswordMetaTest, UnknownLabel) {
    cJSON_AddStringToObject(root.get(), "extra", "foo");
    EXPECT_THROW(Couchbase::User::PasswordMetaData md(root.get()),
                 std::runtime_error);
}

TEST_F(PasswordMetaTest, TestMissingHash) {
    cJSON_DeleteItemFromObject(root.get(), "h");
    EXPECT_THROW(Couchbase::User::PasswordMetaData md(root.get()),
                 std::runtime_error);
}

TEST_F(PasswordMetaTest, TestInvalidDatatypeForHash) {
    cJSON_DeleteItemFromObject(root.get(), "h");
    cJSON_AddNumberToObject(root.get(), "h", 5);
    EXPECT_THROW(Couchbase::User::PasswordMetaData md(root.get()),
                 std::runtime_error);
}

TEST_F(PasswordMetaTest, TestMissingSalt) {
    cJSON_DeleteItemFromObject(root.get(), "s");
    EXPECT_THROW(Couchbase::User::PasswordMetaData md(root.get()),
                 std::runtime_error);
}

TEST_F(PasswordMetaTest, TestInvalidDatatypeForSalt) {
    cJSON_DeleteItemFromObject(root.get(), "s");
    cJSON_AddNumberToObject(root.get(), "s", 5);
    EXPECT_THROW(Couchbase::User::PasswordMetaData md(root.get()),
                 std::runtime_error);
}

TEST_F(PasswordMetaTest, TestMissingIterationCount) {
    cJSON_DeleteItemFromObject(root.get(), "i");
    EXPECT_THROW(Couchbase::User::PasswordMetaData md(root.get()),
                 std::runtime_error);
}

TEST_F(PasswordMetaTest, TestInvalidDatatypeForIterationCount) {
    cJSON_DeleteItemFromObject(root.get(), "i");
    cJSON_AddStringToObject(root.get(), "i", "foo");
    EXPECT_THROW(Couchbase::User::PasswordMetaData md(root.get()),
                 std::runtime_error);
}

TEST_F(PasswordMetaTest, TestInvalidBase64EncodingForHash) {
    cJSON_DeleteItemFromObject(root.get(), "h");
    cJSON_AddStringToObject(root.get(), "h", "!@#$%^&*");
    EXPECT_THROW(Couchbase::User::PasswordMetaData md(root.get()),
                 std::invalid_argument);
}

TEST_F(PasswordMetaTest, TestInvalidBase64EncodingForSalt) {
    cJSON_DeleteItemFromObject(root.get(), "s");
    cJSON_AddStringToObject(root.get(), "s", "!@#$%^&*");
    EXPECT_THROW(Couchbase::User::PasswordMetaData md(root.get()),
                 std::invalid_argument);
}

class UserTest : public ::testing::Test {
public:
    void SetUp() {
        root.reset(cJSON_CreateObject());
        cJSON_AddStringToObject(root.get(), "n", "username");
        cJSON_AddStringToObject(root.get(), "plain",
                                Couchbase::Base64::encode("secret").c_str());

        auto* obj = cJSON_CreateObject();
        cJSON_AddStringToObject(obj, "h",
                                "NP0b1Ji5jWG/ZV6hPzOIk3lmTmw=");
        cJSON_AddStringToObject(obj, "s",
                                "iiU7hLv7l3yOoEgXusJvT2i1J2A=");
        cJSON_AddNumberToObject(obj, "i", 10);

        cJSON_AddItemToObject(root.get(), "sha1", obj);

        obj = cJSON_CreateObject();
        cJSON_AddStringToObject(obj, "h",
                                "BGq4Rd/YH5nfqeV2CtL0lTBLZezuBQVpdTHDGFAwW8w=");
        cJSON_AddStringToObject(obj, "s",
                                "i5Jn//LLM0245cscYnldCjM/HMC7Hj2U1HT6iXqCC0E=");
        cJSON_AddNumberToObject(obj, "i", 10);
        cJSON_AddItemToObject(root.get(), "sha256", obj);

        obj = cJSON_CreateObject();
        cJSON_AddStringToObject(obj, "h",
                                "KZuRjeXbF6NR5rrrQMyHAOvkFq7dUSQ6H08uV"
                                    "ae6TPUTKs4DZNSCenq+puXq5t9zrW9oZb"
                                    "Ic/6wUODFh3ZKAOQ==");
        cJSON_AddStringToObject(obj, "s",
                                "nUNk2ZbAZTabxboF+OBQws3zNJpxePtnuF8Kw"
                                    "cylC3h/NnQQ9FqU0YYohjJhvGRNbxjPTT"
                                    "SuYOgxBG4FMV1W3A==");
        cJSON_AddNumberToObject(obj, "i", 10);
        cJSON_AddItemToObject(root.get(), "sha512", obj);
    }

    unique_cJSON_ptr root;
};

TEST_F(UserTest, TestNormalInit) {
    Couchbase::User u;
    EXPECT_NO_THROW(u = Couchbase::User(root.get()));
    EXPECT_EQ("username", u.getUsername());
    EXPECT_NO_THROW(u.getPassword(Mechanism::SCRAM_SHA512));
    EXPECT_NO_THROW(u.getPassword(Mechanism::SCRAM_SHA256));
    EXPECT_NO_THROW(u.getPassword(Mechanism::SCRAM_SHA1));
    EXPECT_NO_THROW(u.getPassword(Mechanism::PLAIN));

    {
        auto& md = u.getPassword(Mechanism::SCRAM_SHA512);
        EXPECT_EQ(10, md.getIterationCount());
        EXPECT_EQ("nUNk2ZbAZTabxboF+OBQws3zNJpxePtnuF8Kw"
                      "cylC3h/NnQQ9FqU0YYohjJhvGRNbxjPTT"
                      "SuYOgxBG4FMV1W3A==", md.getSalt());
        EXPECT_EQ("KZuRjeXbF6NR5rrrQMyHAOvkFq7dUSQ6H08uV"
                      "ae6TPUTKs4DZNSCenq+puXq5t9zrW9oZb"
                      "Ic/6wUODFh3ZKAOQ==",
                  Couchbase::Base64::encode(md.getPassword()));
    }

    {
        auto& md = u.getPassword(Mechanism::SCRAM_SHA256);
        EXPECT_EQ(10, md.getIterationCount());
        EXPECT_EQ("i5Jn//LLM0245cscYnldCjM/HMC7Hj2U1HT6iXqCC0E=", md.getSalt());
        EXPECT_EQ("BGq4Rd/YH5nfqeV2CtL0lTBLZezuBQVpdTHDGFAwW8w=",
                  Couchbase::Base64::encode(md.getPassword()));
    }

    {
        auto& md = u.getPassword(Mechanism::SCRAM_SHA1);
        EXPECT_EQ(10, md.getIterationCount());
        EXPECT_EQ("iiU7hLv7l3yOoEgXusJvT2i1J2A=", md.getSalt());
        EXPECT_EQ("NP0b1Ji5jWG/ZV6hPzOIk3lmTmw=",
                  Couchbase::Base64::encode(md.getPassword()));
    }

    {
        auto& md = u.getPassword(Mechanism::PLAIN);
        EXPECT_EQ(0, md.getIterationCount());
        EXPECT_EQ("", md.getSalt());
        EXPECT_EQ("secret", md.getPassword());
    }
}

TEST_F(UserTest, TestNoPlaintext) {
    cJSON_DeleteItemFromObject(root.get(), "plain");
    Couchbase::User u;
    EXPECT_NO_THROW(u = Couchbase::User(root.get()));
    EXPECT_NO_THROW(u.getPassword(Mechanism::SCRAM_SHA512));
    EXPECT_NO_THROW(u.getPassword(Mechanism::SCRAM_SHA256));
    EXPECT_NO_THROW(u.getPassword(Mechanism::SCRAM_SHA1));
    EXPECT_THROW(u.getPassword(Mechanism::PLAIN),
                 std::invalid_argument);
}

TEST_F(UserTest, TestNoSha512) {
    cJSON_DeleteItemFromObject(root.get(), "sha512");
    Couchbase::User u;
    EXPECT_NO_THROW(u = Couchbase::User(root.get()));
    EXPECT_THROW(u.getPassword(Mechanism::SCRAM_SHA512),
                 std::invalid_argument);
    EXPECT_NO_THROW(u.getPassword(Mechanism::SCRAM_SHA256));
    EXPECT_NO_THROW(u.getPassword(Mechanism::SCRAM_SHA1));
    EXPECT_NO_THROW(u.getPassword(Mechanism::PLAIN));
}

TEST_F(UserTest, TestNoSha256) {
    cJSON_DeleteItemFromObject(root.get(), "sha256");
    Couchbase::User u;
    EXPECT_NO_THROW(u = Couchbase::User(root.get()));
    EXPECT_THROW(u.getPassword(Mechanism::SCRAM_SHA256),
                 std::invalid_argument);
    EXPECT_NO_THROW(u.getPassword(Mechanism::SCRAM_SHA512));
    EXPECT_NO_THROW(u.getPassword(Mechanism::SCRAM_SHA1));
    EXPECT_NO_THROW(u.getPassword(Mechanism::PLAIN));
}

TEST_F(UserTest, TestNoSha1) {
    cJSON_DeleteItemFromObject(root.get(), "sha1");
    Couchbase::User u;
    EXPECT_NO_THROW(u = Couchbase::User(root.get()));
    EXPECT_THROW(u.getPassword(Mechanism::SCRAM_SHA1),
                 std::invalid_argument);
    EXPECT_NO_THROW(u.getPassword(Mechanism::SCRAM_SHA512));
    EXPECT_NO_THROW(u.getPassword(Mechanism::SCRAM_SHA256));
    EXPECT_NO_THROW(u.getPassword(Mechanism::PLAIN));
}

TEST_F(UserTest, InvalidLabel) {
    cJSON_AddStringToObject(root.get(), "gssapi", "foo");
    EXPECT_THROW(Couchbase::User u(root.get()), std::runtime_error);
}

class PasswordDatabaseTest : public ::testing::Test {
public:
    void SetUp() {
        unique_cJSON_ptr root(cJSON_CreateObject());
        auto* array = cJSON_CreateArray();

        cJSON_AddItemToArray(array,
                             Couchbase::User("trond",
                                             "secret1").to_json().release());
        cJSON_AddItemToArray(array,
                             Couchbase::User("mike",
                                             "secret2").to_json().release());
        cJSON_AddItemToArray(array,
                             Couchbase::User("anne",
                                             "secret3").to_json().release());
        cJSON_AddItemToArray(array,
                             Couchbase::User("will",
                                             "secret4").to_json().release());
        cJSON_AddItemToArray(array,
                             Couchbase::User("dave",
                                             "secret5").to_json().release());

        cJSON_AddItemToObject(root.get(), "users", array);

        char* ptr = cJSON_Print(root.get());
        json.assign(ptr);
        cJSON_Free(ptr);
    }

    std::string json;
};

TEST_F(PasswordDatabaseTest, TestNormalInit) {
    Couchbase::PasswordDatabase db;
    EXPECT_NO_THROW(db = Couchbase::PasswordDatabase(json, false));

    EXPECT_FALSE(db.find("trond").isDummy());
    EXPECT_FALSE(db.find("mike").isDummy());
    EXPECT_FALSE(db.find("anne").isDummy());
    EXPECT_FALSE(db.find("will").isDummy());
    EXPECT_FALSE(db.find("dave").isDummy());
    EXPECT_TRUE(db.find("unknown").isDummy());
}

TEST_F(PasswordDatabaseTest, EmptyConstructor) {
    EXPECT_NO_THROW(Couchbase::PasswordDatabase db);
}

TEST_F(PasswordDatabaseTest, DetectIllegalLabel) {
    EXPECT_THROW(Couchbase::PasswordDatabase db("{ \"foo\": [] }", false),
                 std::runtime_error);
}

TEST_F(PasswordDatabaseTest, DetectIllegalUsersType) {
    EXPECT_THROW(Couchbase::PasswordDatabase db("{ \"users\": 24 }", false),
                 std::runtime_error);
}

TEST_F(PasswordDatabaseTest, CreateFromJsonDatabaseNoUsers) {
    Couchbase::PasswordDatabase db;
    EXPECT_NO_THROW(
        db = Couchbase::PasswordDatabase("{ \"users\": [] }", false));

    EXPECT_TRUE(db.find("trond").isDummy());
    EXPECT_TRUE(db.find("unknown").isDummy());
}

TEST_F(PasswordDatabaseTest, CreateFromJsonDatabaseExtraLabel) {
    EXPECT_THROW(
        Couchbase::PasswordDatabase db("{ \"users\": [], \"foo\", 2 }", false),
        std::runtime_error);
}
