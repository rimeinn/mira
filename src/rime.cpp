#include <iostream>
#include <atomic>
#include "rime.h"

namespace fs = std::filesystem;

static std::atomic<bool> rime_initialized = false;

Rime::Rime(fs::path source_dir, 
           const std::string& schema_id, 
           std::optional<fs::path> cache_dir,
           bool deploy_now)
    : api(rime_get_api())
    , working_dir(fs::absolute(fs::temp_directory_path() / "mira"))
    , schema_id(schema_id)
{
    auto user_data_dir = working_dir / "data";
    auto log_dir = working_dir / "log";
    auto staging_dir = cache_dir
        ? fs::absolute(*cache_dir)
        : working_dir / "data" / "build";

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
    traits.staging_dir = staging_dir.c_str();
    if (!rime_initialized.load()) {
        api->setup(&traits);
        rime_initialized.store(true);
    }
    api->initialize(NULL);
    if (deploy_now && api->start_maintenance(/* full_check */ true))
        api->join_maintenance_thread();
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
    return std::optional<Session>(std::in_place, session);
}

const fs::path&
Rime::get_working_dir() const
{
    return working_dir;
}

Session::Session(RimeSessionId session)
    : session(session)
    , api(rime_get_api())
{
}

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

    // Get preedit
    RIME_STRUCT(RimeContext, context);
    if (api->get_context(session, &context)) {
        if (context.composition.length > 0 || context.menu.num_candidates > 0) {
            if(context.composition.preedit)
                result.preedit = context.composition.preedit;
        }
        api->free_context(&context);
    }
    return result;
}
