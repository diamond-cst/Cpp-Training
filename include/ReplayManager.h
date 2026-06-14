#ifndef REPLAYMANAGER_H
#define REPLAYMANAGER_H

#include <string>
#include <vector>

// 回放记录管理器 (Replay record manager)
class ReplayManager {
private:
    std::string directory;
    std::string textFilename;
    std::string htmlFilename;
    std::vector<std::string> lines;
    bool active;

public:
    explicit ReplayManager(const std::string& dir = "replays");

    void start(const std::string& playerName,
               const std::string& trickName,
               const std::string& modeName);
    void recordRound(int round,
                     const std::string& pile1,
                     const std::string& pile2,
                     const std::string& pile3,
                     int chosenPile);
    void recordReveal(const std::string& card, bool correct, int finalScore);
    void save() const;
    void exportHtml() const;
    bool isActive() const;
    std::string getTextFilename() const;
    std::string getHtmlFilename() const;

    static std::vector<std::string> listReplayFiles(const std::string& dir = "replays");
    static void displayReplayFile(const std::string& filename);

private:
    void ensureDirectory() const;
    static std::string sanitizeFilePart(const std::string& value);
    static std::string escapeHtml(const std::string& value);
};

#endif // REPLAYMANAGER_H
