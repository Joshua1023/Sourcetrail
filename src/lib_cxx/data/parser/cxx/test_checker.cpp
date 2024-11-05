#include <clang/Basic/SourceManager.h>

#include "ParseLocation.h"
#include "test_checker.h"
#include "utilityClang.h"
#include "utilityString.h"

namespace misra {
  void reportError(const MatchFinder::MatchResult& result, const clang::Decl* decl, std::shared_ptr<ParserClient> client, std::shared_ptr<CanonicalFilePathCache> canonicalFilePathCache, const FilePath& sourceFilePath, const string error_message) {
      Id fileId = 0;
      FilePath filePath;
      size_t lineNumber = 0;
      size_t columnNumber = 0;
      
      const clang::SourceManager& sourceManager = result.Context->getSourceManager();
      clang::SourceLocation loc = sourceManager.getExpansionLoc(decl->getLocation());
      if (loc.isInvalid()) {
        loc = decl->getLocation();
      }

      clang::FileID clangFileId = sourceManager.getFileID(loc);
      if (sourceManager.getFileEntryForID(clangFileId) != nullptr) {
        ParseLocation location = utility::getParseLocation(
          loc, sourceManager, nullptr, canonicalFilePathCache);
        fileId = location.fileId;
        filePath = canonicalFilePathCache->getCanonicalFilePath(fileId);
        lineNumber = location.startLineNumber;
        columnNumber = location.startColumnNumber;
      }
      else {
        const clang::OptionalFileEntryRef fileEntry = sourceManager.getFileEntryRefForID(sourceManager.getMainFileID());
        if (fileEntry) {
          filePath = canonicalFilePathCache->getCanonicalFilePath(*fileEntry);
          fileId = client->recordFile(
            filePath, false /*keeps the "indexed" state if the file already exists*/);
          lineNumber = 1;
          columnNumber = 1;
        }
      }
      if (fileId != 0) {
        client->recordError(
          utility::decodeFromUtf8(error_message),
          false,
          canonicalFilePathCache->getFileRegister()->hasFilePath(filePath),
          sourceFilePath,
          ParseLocation(fileId, lineNumber, columnNumber));
      }
  }

namespace rule_6_3 {

class Callback : public MatchFinder::MatchCallback {
 public:
  Callback(const FilePath& sourceFilePath): sourceFilePath_(sourceFilePath){}
  void Init(std::shared_ptr<ParserClient> client, std::shared_ptr<CanonicalFilePathCache> canonicalFilePathCache, MatchFinder* finder) {
    client_ = client;
    canonicalFilePathCache_ = canonicalFilePathCache;
    finder->addMatcher(
        recordDecl(isUnion(), unless(isExpansionInSystemHeader())).bind("rd"),
        this);
  }

  void run(const MatchFinder::MatchResult& result) override {
    const clang::RecordDecl* rd = result.Nodes.getNodeAs<clang::RecordDecl>("rd");
    for (const clang::FieldDecl* fd : rd->fields()) {
      if (fd->isBitField()) {
        const string error_message =
        "[C0703][misra-c2012-6.3]: A bit field shall not be declared as a member of a union";
        reportError(result, fd, client_, canonicalFilePathCache_, sourceFilePath_, error_message);
      }
        
    }
  }

 private:
  std::shared_ptr<ParserClient> client_;
  std::shared_ptr<CanonicalFilePathCache> canonicalFilePathCache_;
  const FilePath& sourceFilePath_;
};

void ASTChecker::Init(std::shared_ptr<ParserClient> client, std::shared_ptr<CanonicalFilePathCache> canonicalFilePathCache) {
  client_ = client;
  canonicalFilePathCache_ = canonicalFilePathCache;
  callback_ = new Callback(sourceFilePath_);
  callback_->Init(client_, canonicalFilePathCache_, &finder_);
}

}  // namespace rule_6_3
}  // namespace misra
