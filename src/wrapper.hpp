#ifndef HTTPS_EVERYWHERE_LIB_CPP_SRC_WRAPPER_HPP_
#define HTTPS_EVERYWHERE_LIB_CPP_SRC_WRAPPER_HPP_

#include <string>

extern "C" {
#include "lib.h"
}

#if defined(HTTPSE_SHARED_LIBRARY)
#if defined(WIN32)
#if defined(HTTPSE_IMPLEMENTATION)
#define HTTPSE_EXPORT __declspec(dllexport)
#else
#define HTTPSE_EXPORT __declspec(dllimport)
#endif  // defined(HTTPSE_IMPLEMENTATION)
#else  // defined(WIN32)
#if defined(HTTPSE_IMPLEMENTATION)
#define HTTPSE_EXPORT __attribute__((visibility("default")))
#else
#define HTTPSE_EXPORT
#endif  // defined(HTTPSE_IMPLEMENTATION)
#endif
#else  // defined(HTTPSE_SHARED_LIBRARY)
#define HTTPSE_EXPORT
#endif

namespace httpse {

typedef enum HTTPSE_EXPORT {
  NO_OP = 0,
  REWRITE_URL = 1,
} RewriteAction;

typedef struct HTTPSE_EXPORT {
  RewriteAction action;
  std::string new_url;
} RewriteResult;

class HTTPSE_EXPORT HttpsEverywhereClient {
 public:
  HttpsEverywhereClient();
  ~HttpsEverywhereClient();
  bool LoadRules(const std::string rules);
  RewriteResult RewriteUrl(std::string url) const;
 private:
  C_HttpseClient* client_ptr_;
};


} // namespace httpse

#endif
