#include "config.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <regex>

#include <yaml-cpp/yaml.h>
#include <argparse/argparse.hpp>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "rime.h"
#include "output_collector.h"

using namespace std;
namespace fs = std::filesystem;

static void lua_setresult(lua_State *L, Result& result);
static bool lua_eval(lua_State *L, const char *expr);
static void apply_patch(const fs::path& working_dir, const std::string& schema_id, const YAML::Node& patch_yaml);

int
main(int argc, char *argv[])
{
    argparse::ArgumentParser program(PROJECT_NAME, PROJECT_VERSION);
    program.add_argument("FILE")
        .help("test spec file");
    program.add_argument("-C", "--cache-dir")
        .help("cache built artifacts");
    program.add_argument("-R", "--regex")
        .help("regex to match desired deployments")
        .default_value(".*");
    try {
        program.parse_args(argc, argv);
    } catch (const std::exception &err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    std::string spec_path = program.get("FILE");
    std::regex pat(program.get("-R"));
    std::optional<std::string> cache_dir;
    if (program.is_used("-C")) {
        cache_dir = program.get("-C");
    }

    auto doc = YAML::LoadFile(spec_path);
    auto schema_id = doc["schema"].as<std::string>();
    auto source_dir = fs::path(spec_path).remove_filename() / fs::path(doc["source_dir"].as<std::string>());
    std::cout << "SELECT " << schema_id << " from " << source_dir << "\n";

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    if (doc["script"]) {
        if (luaL_dostring(L, doc["script"].as<string>().c_str()) != LUA_OK) {
            std::cerr << "Error evaluating Lua: " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
            return -2;
        }
    }

    std::vector<std::string> passlist, faillist;
    for (auto pair : doc["deploy"]) {
        auto name = pair.first.as<std::string>();
        auto dspec = pair.second;
        if (!std::regex_match(name, pat)) {
            continue;
        }
        std::cout << "DEPLOY " << name << "\n";

        // Patch and Deploy
        Rime rime(source_dir, schema_id, cache_dir, /* deploy */ false);
        if (dspec["patch"]) {
            apply_patch(rime.get_working_dir(), schema_id, dspec["patch"]);
        }
        rime.deploy();

        // Run each test
        for (const auto& test : dspec["tests"]) {
            auto keys = test["send"].as<std::string>();
            auto expr = "return ("s + test["assert"].as<std::string>() + ")";
            std::string test_id = schema_id + "::" + name + "::" + keys;
            std::cout << "- " << test_id << "... " << std::flush;
            std::string stdout, stderr;
            bool pass = false;
            {
                OutputCollector g(stdout, stderr);
                auto session = rime.create_session();
                session->select_schema(schema_id);
                if (dspec["options"]) {
                    for (auto kv : dspec["options"]) {
                        const auto& k = kv.first.as<std::string>();
                        const auto& v = kv.second.as<bool>();
                        session->set_option(k, v);
                    }
                }
                auto result = session->send_keys(keys);
                if (!result)
                    goto done;

                lua_setresult(L, *result);
                pass = lua_eval(L, expr.c_str());
            }

        done:
            std::cout << (pass ? "PASS" : "FAIL") << std::endl;
            if (!pass) {
                if (stdout.length() > 0) {
                    std::cout << "\n========= STDOUT =========\n" << stdout << "\n" << std::endl;
                }
                if (stderr.length() > 0) {
                    std::cout << "\n========= STDERR =========\n" << stderr << "\n" << std::endl;
                }
            }
            (pass ? passlist : faillist).push_back(test_id);
            continue;
        }
        std::cout << "\n";
    }
    lua_close(L);

    size_t n_fail = faillist.size(), n_pass = passlist.size();
    if (n_fail) {
        std::cout << "\n\n" << n_fail << "/" << (n_fail + n_pass)
                  << " tests failed:\n";
        for (const auto& id : faillist) {
            std::cout << id << "\n";
        }
        return n_fail < 126 ? n_fail : 126;
    } else {
        std::cout << "Eveything OK!\n";
        return 0;
    }
}

static void
lua_setresult(lua_State *L, Result& result)
{
    // commit
    if (result.commit) {
        lua_pushstring(L, result.commit->c_str());
    } else {
        lua_pushnil(L);
    }
    lua_setglobal(L, "commit");

    // cand
    const auto& cands = result.candidates;
    lua_newtable(L);
    for (size_t i = 0; i < cands.size(); ++i) {
        lua_newtable(L);
        lua_pushstring(L, "text");
        lua_pushstring(L, cands[i].text.c_str());
        lua_settable(L, -3);
        lua_pushstring(L, "comment");
        if (cands[i].comment) {
            lua_pushstring(L, cands[i].comment->c_str());
        } else {
            lua_pushstring(L, "");
        }
        lua_settable(L, -3);
        lua_rawseti(L, -2, i + 1);
    }
    lua_setglobal(L, "cand");

    // preedit
    if (result.preedit) {
        lua_pushstring(L, result.preedit->c_str());
    } else {
        lua_pushnil(L);
    }
    lua_setglobal(L, "preedit");
}

static bool
lua_eval(lua_State *L, const char *expr)
{
    if (luaL_dostring(L, expr) != LUA_OK) {
        std::cerr << "Error evaluating Lua: " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);
        return false;
    } else {
        bool pass = lua_toboolean(L, -1);
        lua_pop(L, 1);
        return pass;
    }
}

// Create a <schema>.custom.yaml
static void
apply_patch(const fs::path& working_dir, const std::string& schema_id,
            const YAML::Node& patch)
{
    std::ofstream fout(working_dir / "data" / (schema_id + ".custom.yaml"));
    YAML::Node doc;
    doc["patch"] = patch;
    fout << doc;
}
