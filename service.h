#pragma once

#include <string>

// Command argument parser

enum class EArgParams
{
    error,
    log_none,
    log_base,
    log_all,
    run_app
};
extern EArgParams cmdArgsParser(uint argc, char* argv[], std::string& app);

// Pressing the Q key
extern bool kbHitQ();

// Getiing current time (HH:MM:SS)
extern std::string getCurrTime();

// Run app
extern void runApp(std::string& name);
extern bool appFinished();

// Print log

enum class EPrintMode
{
    log_none,
    log_base,
    log_all,
};
extern void setPrintMode(EPrintMode m);
enum class ELogType
{
    base,
    plain
};
extern void printLog(ELogType t, std::string s);
