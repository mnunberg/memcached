/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
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
#pragma once

#include <gtest/gtest.h>
#include <string>
#include <cJSON_utils.h>

/**
 * The test environment added to the Google Test Framework.
 *
 * The environment is set up once before the first test is run, and
 * shut down after the last test is run.
 */
class McdEnvironment : public ::testing::Environment {
public:
    /**
     * Create the test environment. This method is called automatically
     * from the Google Test Framework. You should not try to access any
     * of the members before this method is called. As long as you only
     * try to access this class from a test case you're on the safe side
     */
    virtual void SetUp();

    /**
     * Tear down the test environment. This call invalidates the object
     * and calling the members may return rubbish.
     */
    virtual void TearDown();

    /**
     * Get the name of the RBAC file used.
     *
     * @return the absolute path of the file containing the RBAC data
     */
    const std::string& getRbacFilename() const {
        return rbac_file_name;
    }

    /**
     * Get the name of the configuration file used by the audit daemon.
     *
     * @return the absolute path of the file containing the audit config
     */
    const std::string& getAuditFilename() const {
        return audit_file_name;
    }

    /**
     * Get the name of the directory containing the audit logs
     *
     * @return the absolute path of the directory containing the audit config
     */
    const std::string& getAuditLogDir() const {
        return audit_log_dir;
    }

    /**
     * Get a handle to the current audit configuration so that you may
     * modify the configuration (you may write it to the audit configuration
     * file by calling <code>rewriteAuditConfig()</code>
     *
     * NOTE: You should _NOT_ release the returned cJSON object, you may
     * only add/replace sub objects by using the appropriate cJSON methods.
     *
     * @return the root object of the audit configuration.
     */
    cJSON* getAuditConfig() {
        return audit_config.get();
    }

    /**
     * Dump the internal representation of the audit configuration
     * (returned by <code>getAuditConfig()</code>) to the configuration file
     * (returned by <getAuditFilename()</config>)
     */
    void rewriteAuditConfig();

    /**
     * Get the name of the current working directory. This value is cached,
     * and the logic will break if someone suddenly start adding tests
     * changing the current working directory.
     *
     * @return the cached value of the current working directory.
     */
    const std::string& getCurrentWorkingDirectory() const {
        return cwd;
    }

protected:
    /**
     * Generate a temporary file based on a certain pattern by utilizing
     * mkfile.
     *
     * @param pattern the pattern to use for the name of the temp file
     * @return the new unique file name
     * @throws std::exception upon failures
     */
    std::string generateTempFile(const char* pattern);

private:
    void SetupAuditFile();

    void SetupRbacFile();

    void SetupIsaslPw();

    std::string isasl_file_name;
    std::string rbac_file_name;
    std::string audit_file_name;
    std::string audit_log_dir;
    std::string cwd;
    unique_cJSON_ptr audit_config;
    static char isasl_env_var[256];
};

extern McdEnvironment* mcd_env;
