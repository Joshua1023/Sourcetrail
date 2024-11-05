#ifndef ANALYZER_MISRA_rule_6_3_CHECKER_H_
#define ANALYZER_MISRA_rule_6_3_CHECKER_H_

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Tooling/Tooling.h>

#include "CanonicalFilePathCache.h"
#include "FilePath.h"
#include "ParserClient.h"

using namespace std;
using namespace clang::ast_matchers;

namespace misra {

  void reportError(const MatchFinder::MatchResult& result, const clang::Decl* decl, std::shared_ptr<ParserClient> client, std::shared_ptr<CanonicalFilePathCache> canonicalFilePathCache, const FilePath& sourceFilePath, const string error_message);

namespace rule_6_3 {

class Callback;

class ASTChecker {
 public:
  ASTChecker(const FilePath& sourceFilePath): sourceFilePath_(sourceFilePath){}
  void Init(std::shared_ptr<ParserClient> client, std::shared_ptr<CanonicalFilePathCache> canonicalFilePathCache);

  MatchFinder* GetMatchFinder() { return &finder_; }

 private:
  MatchFinder finder_;
  Callback* callback_;
  std::shared_ptr<ParserClient> client_;
  std::shared_ptr<CanonicalFilePathCache> canonicalFilePathCache_;
  const FilePath& sourceFilePath_;
};
}
}

#endif  // ANALYZER_MISRA_rule_6_3_CHECKER_H_
