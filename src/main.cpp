#include "config.h"

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

#include <yaml-cpp/yaml.h>
#include <argparse/argparse.hpp>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "rime.h"

using namespace std;
namespace fs = std::filesystem;

static void lua_setresult(lua_State *L, Result& result);
static bool lua_eval(lua_State *L, const char *expr);

int
main(int argc, char *argv[])
{
    argparse::ArgumentParser program(PROJECT_NAME, PROJECT_VERSION);
    program.add_argument("FILE")
        .help("test spec file");
    program.add_argument("-C", "--cache-dir")
        .help("cache built artifacts");
    try {
        program.parse_args(argc, argv);
    } catch (const std::exception &err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    std::string spec_path = program.get("FILE");
    std::optional<std::string> cache_dir;
    if (program.is_used("-C")) {
        cache_dir = program.get("-C");
    }

    auto doc = YAML::LoadFile(spec_path);
    auto schema_id = doc["schema"].as<std::string>();
    auto source_dir = fs::path{ doc["source_dir"].as<std::string>() };
    std::cout << "SELECT " << schema_id << " from " << source_dir << "\n";

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    std::vector<std::string> passlist, faillist;
    for (auto pair : doc["deploy"]) {
        auto name = pair.first.as<std::string>();
        auto dspec = pair.second;
        std::cout << "DEPLOY " << name << "\n";

        // Patch and Deploy
        Rime rime(source_dir, schema_id, cache_dir);
        // TODO: patch
        rime.deploy();

        // Run each test
        for (const auto& test : dspec["tests"]) {
            auto keys = test["send"].as<std::string>();
            auto expr = "return ("s + test["assert"].as<std::string>() + ")";
            std::string test_id = schema_id + "::" + name + "::" + keys;

            auto session = rime.create_session();

            if (dspec["options"]) {
                for (auto kv : doc["options"]) {
                    const auto& k = kv.first.as<std::string>();
                    const auto& v = kv.second.as<bool>();
                    session->set_option(k, v);
                }
            }

            auto result = session->send_keys(keys);
            std::cout << "[....] " << test_id;
            bool pass = false;
            if (!result)
                goto done;

            lua_setresult(L, *result);
            pass = lua_eval(L, expr.c_str());

        done:
            std::cout << "\r[" << (pass ? "PASS" : "FAIL") << "]\n";
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
        return faillist.size();
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
        if (cands[i].comment) {
            lua_pushstring(L, "comment");
            lua_pushstring(L, cands[i].comment->c_str());
            lua_settable(L, -3);
        }
        lua_rawseti(L, -2, i + 1);
    }
    lua_setglobal(L, "cand");
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
