#include "../../src/wrapper.hpp"
#include <iostream>
#include <string>
#include <vector>

httpse::HttpsEverywhereClient new_from_rules(std::string rules) {
  auto httpse_client = httpse::HttpsEverywhereClient();
  httpse_client.LoadRules(rules);
  return httpse_client;
}

template<class T> bool test_eq(const T &actual, const T &expected) {
  if(expected != actual) {
    std::cout << "Expected: [" << expected << "], Actual: [" << actual << "]" << std::endl;
    return false;
  }
  return true;
}

bool TestSuccessfulRedirect() {
  bool passed = true;

  auto httpse_client = new_from_rules(R"([{"name":"01.org","target":["01.org","www.01.org","download.01.org","lists.01.org","ml01.01.org"],"securecookie":[{"host":".+","name":".+"}],"rule":[{"from":"^http:","to":"https:"}]},{"name":"0bin.net","target":["0bin.net","www.0bin.net"],"securecookie":[{"host":"^0bin\\.net","name":".+"}],"rule":[{"from":"^http://www\\.0bin\\.net/","to":"https://0bin.net/"},{"from":"^http:","to":"https:"}]}])");

  auto result = httpse_client.RewriteUrl("http://01.org/test/3.html");
  passed &= test_eq(result.action, httpse::RewriteAction::REWRITE_URL);
  passed &= test_eq<std::string>(result.new_url, "https://01.org/test/3.html");

  result = httpse_client.RewriteUrl("http://download.01.org/index.php");
  passed &= test_eq(result.action, httpse::RewriteAction::REWRITE_URL);
  passed &= test_eq<std::string>(result.new_url, "https://download.01.org/index.php");

  result = httpse_client.RewriteUrl("http://www.0bin.net/api/v1/bin");
  passed &= test_eq(result.action, httpse::RewriteAction::REWRITE_URL);
  passed &= test_eq<std::string>(result.new_url, "https://0bin.net/api/v1/bin");

  return passed;
}

bool TestUnsuccessfulRedirect() {
  bool passed = true;

  auto httpse_client = new_from_rules(R"([{"name":"01.org","target":["01.org","www.01.org","download.01.org","lists.01.org","ml01.01.org"],"securecookie":[{"host":".+","name":".+"}],"rule":[{"from":"^http:","to":"https:"}]},{"name":"0bin.net","target":["0bin.net","www.0bin.net"],"securecookie":[{"host":"^0bin\\.net","name":".+"}],"rule":[{"from":"^http://www\\.0bin\\.net/","to":"https://0bin.net/"},{"from":"^http:","to":"https:"}]}])");

  auto result = httpse_client.RewriteUrl("http://01.com/test/3.html");
  passed &= test_eq(result.action, httpse::RewriteAction::NO_OP);
  passed &= test_eq<std::string>(result.new_url, "");

  result = httpse_client.RewriteUrl("http://localhost:8080");
  passed &= test_eq(result.action, httpse::RewriteAction::NO_OP);
  passed &= test_eq<std::string>(result.new_url, "");

  result = httpse_client.RewriteUrl("http://ml.ml01.01.org/e");
  passed &= test_eq(result.action, httpse::RewriteAction::NO_OP);
  passed &= test_eq<std::string>(result.new_url, "");

  return passed;
}

int main(int argc, char** argv) {
  auto tests = {
      TestSuccessfulRedirect,
      TestUnsuccessfulRedirect,
  };

  int passed = 0;
  for (auto i = tests.begin(); i < tests.end(); i++) {
    passed += (*i)();
  }

  std::cout << passed << " of " << tests.size() << " tests passed" << std::endl;
}
