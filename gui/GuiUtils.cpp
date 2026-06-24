#include "GuiUtils.h"

#include <QRegularExpression>
#include <cctype>
#include <iomanip>
#include <sstream>

QString cleanCardText(const QString& value) {
    QString text = value;
    text.remove(QRegularExpression("\\x1B\\[[0-9;]*m"));
    text = text.trimmed();
    if (text.startsWith("[") && text.endsWith("]") && text.size() > 2) {
        text = text.mid(1, text.size() - 2);
    }
    return text;
}

QString toQString(const std::string& value) {
    return QString::fromStdString(value);
}

std::string toStdString(const QString& value) {
    return value.trimmed().toStdString();
}

QString cardColor(const QString& card) {
    return (card.contains("♥") || card.contains("♦")) ? "#b83b3b" : "#1f2933";
}

QStringList toQStringList(const std::vector<std::string>& values) {
    QStringList result;
    for (const auto& value : values) {
        result << cleanCardText(toQString(value));
    }
    return result;
}

QVector<QStringList> toPileLists(const std::vector<std::vector<std::string>>& piles) {
    QVector<QStringList> result;
    for (const auto& pile : piles) {
        result.push_back(toQStringList(pile));
    }
    return result;
}

namespace {

std::string sanitizePlayerName(const std::string& name) {
    std::ostringstream result;
    for (char ch : name) {
        unsigned char uch = static_cast<unsigned char>(ch);
        if ((uch >= '0' && uch <= '9') ||
            (uch >= 'A' && uch <= 'Z') ||
            (uch >= 'a' && uch <= 'z')) {
            result << ch;
        } else if (uch == ' ' || uch == '-' || uch == '_' || uch == '.') {
            result << '_';
        } else {
            result << "_x" << std::uppercase << std::hex << std::setw(2)
                   << std::setfill('0') << static_cast<int>(uch) << std::dec;
        }
    }
    std::string value = result.str();
    return value.empty() ? "player" : value;
}

} // namespace

std::string playerSaveFilename(const std::string& playerName) {
    return "saves/player_" + sanitizePlayerName(playerName) + ".dat";
}

QString joinCards(const QStringList& cards) {
    return cards.join(" ");
}

QStringList splitCards(const QString& cards) {
    return cards.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
}
