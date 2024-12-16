#pragma once

#include <filesystem>
#include <rime_api.h>
#include <vector>
#include <optional>

#ifdef TESTING
#  define private public
#endif

struct Candidate {
    std::string text;
    std::optional<std::string> comment;
};

struct Result {
    std::optional<std::string> commit;
    std::vector<Candidate> candidates;
};

class Session;

class Rime {
public:
    Rime(std::filesystem::path source_dir, 
         const std::string& schema_id,
         std::optional<std::filesystem::path> cache_dir = std::nullopt,
         bool deploy_now = true);
    ~Rime();

    void deploy();
    std::optional<Session> create_session();

    const std::filesystem::path& get_working_dir() const;

private:
    RimeApi *api;
    RimeSessionId session;

    std::filesystem::path working_dir;
    std::string schema_id;
};

class Session {
public:
    explicit Session(RimeSessionId session);
    Session(const Session &) = delete;
    ~Session();

    bool select_schema(const std::string& schema_id);
    void set_option(const std::string& key, bool value);
    std::optional<Result> send_keys(const std::string &key_sequence);

private:
    RimeSessionId session;
    RimeApi *api;
};
