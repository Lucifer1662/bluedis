#include "LoggerAOF.h"


std::unordered_set<std::string> LoggerAOF::writeOperations = { "hset","set","incr" };
