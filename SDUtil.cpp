#include "SDUtil.h"
bool SDUtil::begin() { return false; }
bool SDUtil::salvarCrash(const String& relatorio, String& caminho, String& erro) { caminho = ""; erro = "Sem leitor SD integrado"; return false; }
bool SDUtil::test() { return false; }
String SDUtil::getCardType() { return "SD indisponivel: AtomS3R sem leitor integrado"; }
uint64_t SDUtil::getCardSizeBytes() { return 0; }
uint64_t SDUtil::getUsedBytes(bool* valid) { if (valid) *valid = false; return 0; }
uint64_t SDUtil::getUsedBytesCooperative(bool* valid) { if (valid) *valid = false; return 0; }
void SDUtil::processMetrics() {  }
SdMetricsSnapshot SDUtil::metricsSnapshot() { return {}; }
bool SDUtil::exists(String path) { return false; }
bool SDUtil::writeText(String path, String text) { return false; }
bool SDUtil::appendText(String path, String text) { return false; }
String SDUtil::readText(String path) { return "SD indisponivel: AtomS3R sem leitor integrado"; }
bool SDUtil::removeFile(String path) { return false; }
bool SDUtil::createDir(String path) { return false; }
String SDUtil::listFiles(String path) { return "SD indisponivel: AtomS3R sem leitor integrado"; }
bool SDUtil::writeJson(String path, String jsonContent) { return false; }
String SDUtil::readJson(String path) { return "SD indisponivel: AtomS3R sem leitor integrado"; }
uint64_t SDUtil::getFileSize(String path) { return 0; }
String SDUtil::readTail(String path, size_t maxBytes) { return "SD indisponivel: AtomS3R sem leitor integrado"; }
int SDUtil::readLogTail(const char* path, String& content, bool& truncated) { content = "AtomS3R sem leitor SD integrado"; truncated = false; return 503; }
bool SDUtil::removeDir(String path) { return false; }
bool SDUtil::renameFile(const String& from,const String& to) { return false; }
