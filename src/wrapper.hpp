#ifndef HTTPS_EVERYWHERE_LIB_CPP_SRC_WRAPPER_HPP_
#define HTTPS_EVERYWHERE_LIB_CPP_SRC_WRAPPER_HPP_

#include <string>

extern "C" {
#include "lib.h"
}

namespace httpse {

typedef enum {
  NO_OP = 0,
  REWRITE_URL = 1,
} RewriteAction;

typedef struct {
  RewriteAction action;
  std::string new_url;
} RewriteResult;

class HttpsEverywhereClient {
 public:
  HttpsEverywhereClient();
  ~HttpsEverywhereClient();
  void LoadRules(const std::string rules);
  RewriteResult RewriteUrl(std::string url) const;
 private:
  C_HttpseClient* client_ptr_;
};


} // namespace httpse

#endif
