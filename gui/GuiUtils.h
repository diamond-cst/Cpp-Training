#ifndef GUI_UTILS_H
#define GUI_UTILS_H

#include <QString>
#include <QStringList>
#include <QVector>
#include <string>
#include <vector>

QString cleanCardText(const QString& value);
QString toQString(const std::string& value);
std::string toStdString(const QString& value);
QString cardColor(const QString& card);
QStringList toQStringList(const std::vector<std::string>& values);
QVector<QStringList> toPileLists(const std::vector<std::vector<std::string>>& piles);
std::string playerSaveFilename(const std::string& playerName);
QString joinCards(const QStringList& cards);
QStringList splitCards(const QString& cards);

#endif // GUI_UTILS_H
