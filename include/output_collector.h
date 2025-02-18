#pragma once

#include <string>

class OutputCollector {
    int saved_stdout, saved_stderr;
    FILE *tmp_out, *tmp_err;
    std::string& result_stdout;
    std::string& result_stderr;

public:
    OutputCollector(std::string& result_stdout, std::string& result_stderr);
    ~OutputCollector();
};
