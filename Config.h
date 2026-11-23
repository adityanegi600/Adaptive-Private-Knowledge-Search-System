#pragma once
#include <string>
#include <cstddef>
namespace rar {
static const std::string APP_NAME = "Reason-Act-Reflect";
static const std::string APP_VERSION = "0.2.0";
static const std::string DOCUMENT_DB = "data/documents.db";
static const std::string FEEDBACK_DB = "data/feedback.db";
static const std::string HISTORY_DB = "data/search_history.db";
static const std::size_t MAX_RESULTS = 20;
static const std::size_t PREVIEW_LENGTH = 220;
static const double DEFAULT_TERM_WEIGHT = 1.0;
static const double MIN_TERM_WEIGHT = 0.20;
static const double MAX_TERM_WEIGHT = 3.00;
}
