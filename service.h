#pragma once

#include <string>

// Command argument parser

/// @brief List of acts after parsing of cmd arg list 
enum class EArgParams
{
    error,
    log_none,
    log_base,
    log_all,
    run_app
};

extern EArgParams cmdArgsParser(uint argc, char* argv[], std::string& app);

// Pressing the keyboard

extern bool kbHitQ();

// Timestamp

extern std::string getCurrTime();

// Run another app

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
