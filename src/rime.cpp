#include <iostream>
#include "rime.h"

namespace fs = std::filesystem;

Rime::Rime(fs::path source_dir, 
           const std::string& schema_id, 
           std::optional<fs::path> cache_dir)
    : source_dir(source_dir)
    , working_dir(fs::temp_directory_path() / "mira")
    , api(rime_get_api())
    , schema_id(schema_id)
    , cache_dir(cache_dir)
{
    auto user_data_dir = working_dir / "data";
    auto log_dir = working_dir / "log";

    // Prepare working dir
    fs::remove_all(working_dir);
    if (!fs::exists(working_dir)) {
        fs::create_directories(working_dir);
        fs::create_directories(user_data_dir);
        fs::create_directories(log_dir);
    }
    fs::copy(source_dir, working_dir / "data", fs::copy_options::recursive);

    // Initialize Rime
    RIME_STRUCT(RimeTraits, traits);
    traits.app_name = "rime.mira";
    traits.user_data_dir = user_data_dir.c_str();
    traits.log_dir = log_dir.c_str();
    if (cache_dir)
        traits.staging_dir = cache_dir->c_str();
    api->setup(&traits);
    api->initialize(NULL);
}

Rime::~Rime()
{
    api->finalize();
    // fs::remove_all(working_dir);
}

void
Rime::deploy()
{
    if (api->start_maintenance(/* full_check */ true))
        api->join_maintenance_thread();
}

std::optional<Session>
Rime::create_session()
{
    RimeSessionId session = api->create_session();
    if (!session)
        return std::nullopt;
    else
        return Session(session);
}

Session::Session(RimeSessionId session)
    : session(session)
    , api(rime_get_api())
{}

Session::~Session() {
    api->destroy_session(session);
}

bool
Session::select_schema(const std::string& schema_id)
{
    return api->select_schema(session, schema_id.c_str());
}

void
Session::set_option(const std::string& k, bool v)
{
    api->set_option(session, k.c_str(), v);
}

std::optional<Result>
Session::send_keys(const std::string &keys)
{
    if (!api->simulate_key_sequence(session, keys.c_str())) {
        std::cerr << "cannot simulate key sequence '" << keys << "'\n";
        return std::nullopt;
    }
    
    Result result;

    // Get committed
    RIME_STRUCT(RimeCommit, commit);
    if (api->get_commit(session, &commit)) {
        if (commit.text) {
            result.commit = commit.text;
        }
        api->free_commit(&commit);
    }

    // Get candidates
    RimeCandidateListIterator it = {0};
    if (api->candidate_list_begin(session, &it)) {
        for (int cnt = 0; cnt < 100 && api->candidate_list_next(&it); ++cnt) {
            Candidate cand;
            cand.text = it.candidate.text;
            if (it.candidate.comment)
                cand.comment = it.candidate.comment;
            result.candidates.push_back(cand);
        }
        api->candidate_list_end(&it);
    }
    api->destroy_session(session);
    return result;
}
