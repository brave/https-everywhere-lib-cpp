#include "wrapper.hpp"
#include <string>
#include <iostream>

extern "C" {
#include "lib.h"
}

namespace httpse {

HttpsEverywhereClient::HttpsEverywhereClient() : client_ptr_(new_client()) {}
HttpsEverywhereClient::~HttpsEverywhereClient() {}

bool HttpsEverywhereClient::LoadRules(const std::string rules) {
  return initialize_client(client_ptr_, rules.c_str());
}

RewriteResult HttpsEverywhereClient::RewriteUrl(const std::string url) const {
  const auto c_result = rewriter_rewrite_url(client_ptr_, url.c_str());
  return {
    .action = (RewriteAction) c_result.action,
    .new_url = std::string(c_result.new_url),
  };
}

} // namespace httpse
