#include <cstdio>
#include <string>
#include <stdexcept>
#include <errno.h>
#include <unistd.h>

#include "output_collector.h"

static std::string read_file(FILE* f);

OutputCollector::OutputCollector(std::string& out, std::string& err)
    : result_stdout(out), result_stderr(err)
{
    if ((saved_stdout = dup(STDOUT_FILENO)) == -1 || 
        (saved_stderr = dup(STDERR_FILENO)) == -1) {
        if (saved_stdout >= 0) close(saved_stdout);
        throw std::runtime_error("Failed to save descriptors");
    }
    if (!(tmp_out = tmpfile()) || !(tmp_err = tmpfile())) {
        close(saved_stdout);
        close(saved_stderr);
        if (tmp_out) fclose(tmp_out);
        throw std::runtime_error("Failed to create temp files");
    }
    int res1, res2;
    if ((res1 = dup2(fileno(tmp_out), STDOUT_FILENO)) == -1 ||
        (res2 = dup2(fileno(tmp_err), STDERR_FILENO)) == -1) {
        if (res1 >= 0) {
            dup2(saved_stdout, STDOUT_FILENO);
        }
        close(saved_stdout);
        close(saved_stderr);
        fclose(tmp_out);
        fclose(tmp_err);
        throw std::runtime_error("Failed to redirect stdout");
    }
}

OutputCollector::~OutputCollector() {
    fflush(tmp_out);
    fflush(tmp_err);
    result_stdout = read_file(tmp_out);
    result_stderr = read_file(tmp_err);
    fclose(tmp_err);
    fclose(tmp_out);
    dup2(saved_stdout, STDOUT_FILENO); // We shouldn't throw.
    dup2(saved_stderr, STDERR_FILENO);
    close(saved_stdout);
    close(saved_stderr);
}

static std::string
read_file(FILE* f)
{
    std::string result;
    rewind(f);
    char buf[1024];
    while (true) {
        size_t n = fread(buf, 1, sizeof(buf), f);
        if (n > 0) {
            result.append(buf, n);
        }
        if (n < sizeof(buf)) {
            if (ferror(f)) {
                if (errno == EINTR) {
                    clearerr(f);
                    continue;
                }
                break; // Don't throw. Just truncate the output.
            }
            break;
        }
    }
    return result;
}
