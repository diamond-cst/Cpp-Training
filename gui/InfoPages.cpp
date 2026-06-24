#include "MagicTrickWindow.h"
#include "CardWidgets.h"
#include "GuiUtils.h"
#include "Leaderboard.h"
#include "ReplayManager.h"

#include <QAbstractItemView>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <fstream>
#include <sstream>

QWidget* MagicTrickWindow::buildLeaderboardPage() {
    QVBoxLayout* root = new QVBoxLayout;
    root->setContentsMargins(24, 22, 24, 22);
    root->setSpacing(16);
    root->addLayout(titleRow("排行榜", "返回主界面"));
    root->addWidget(makeDivider());

    leaderboardTable = new QTableWidget(0, 6);
    leaderboardTable->setHorizontalHeaderLabels({"排名", "玩家", "分数", "游戏", "正确", "连胜"});
    leaderboardTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    leaderboardTable->verticalHeader()->setVisible(false);
    leaderboardTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    leaderboardTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    QFrame* tablePanel = makeTablePanel("tablePanel");
    QVBoxLayout* tableLayout = new QVBoxLayout(tablePanel);
    tableLayout->setContentsMargins(16, 14, 16, 14);
    tableLayout->addWidget(leaderboardTable);
    root->addWidget(tablePanel, 1);

    QPushButton* refresh = makeButton("刷新", "primaryButton");
    refresh->setFixedWidth(100);
    root->addWidget(refresh, 0, Qt::AlignRight);
    connect(refresh, &QPushButton::clicked, this, [this]() { refreshLeaderboardTable(); });
    return makePageShell(root);
}

QWidget* MagicTrickWindow::buildReplayPage() {
    QVBoxLayout* root = new QVBoxLayout;
    root->setContentsMargins(24, 22, 24, 22);
    root->setSpacing(16);
    root->addLayout(titleRow("回放记录", "返回主界面"));
    root->addWidget(makeDivider());

    QFrame* replayPanel = makeTablePanel("tablePanel");
    QHBoxLayout* body = new QHBoxLayout;
    body->setSpacing(12);
    body->setContentsMargins(16, 14, 16, 14);
    replayList = new QListWidget;
    replayList->setMinimumWidth(320);
    replayContent = new QPlainTextEdit;
    replayContent->setReadOnly(true);
    body->addWidget(replayList, 0);
    body->addWidget(replayContent, 1);
    replayPanel->setLayout(body);
    root->addWidget(replayPanel, 1);

    connect(replayList, &QListWidget::currentTextChanged,
            this, [this](const QString& filename) { loadReplayContent(filename); });
    return makePageShell(root);
}

QWidget* MagicTrickWindow::buildHelpPage() {
    QVBoxLayout* root = new QVBoxLayout;
    root->setContentsMargins(24, 22, 24, 22);
    root->setSpacing(16);
    root->addLayout(titleRow("游戏说明", "返回主界面"));
    root->addWidget(makeDivider());

    QFrame* helpPanel = makeTablePanel("tablePanel");
    QVBoxLayout* helpLayout = new QVBoxLayout(helpPanel);
    helpLayout->setContentsMargins(16, 14, 16, 14);
    QPlainTextEdit* help = new QPlainTextEdit;
    help->setReadOnly(true);
    help->setPlainText(
        "核心规则\n"
        "准备 21 张不同的牌，让观众记住一张。每轮从上到下依次发成三堆，观众指出目标牌所在的牌堆，程序把这堆放在中间重新合并。重复三轮后，目标牌会稳定出现在中间位置。\n\n"
        "本地玩法\n"
        "经典版使用 21 张牌，进阶版使用 27 张牌，可配置版支持 15 / 21 / 27 张牌。观众互动模式由玩家确认最终结果；魔术师练习模式会模拟观众选择，最后由玩家点击要揭晓的牌。\n\n"
        "数据功能\n"
        "游戏可按玩家名保存和加载。完成一局后会写入排行榜，并生成文本回放与 HTML 回放报告。\n\n"
        "网络双人对战\n"
        "一端选择魔术师并监听端口，另一端输入地址和同一端口加入。双方通过 Qt TCP 页面完成发牌、选堆和揭晓确认。");
    helpLayout->addWidget(help);
    root->addWidget(helpPanel, 1);
    return makePageShell(root);
}

void MagicTrickWindow::showLeaderboard() {
    refreshLeaderboardTable();
    stack->setCurrentIndex(LeaderboardPage);
}

void MagicTrickWindow::refreshLeaderboardTable() {
    Leaderboard leaderboard("leaderboard.dat");
    auto records = leaderboard.getTopRecords(10);
    leaderboardTable->setRowCount(static_cast<int>(records.size()));
    for (int row = 0; row < static_cast<int>(records.size()); ++row) {
        const PlayerRecord& record = records[row];
        QStringList cells = {
            QString::number(row + 1),
            toQString(record.name),
            QString::number(record.score),
            QString::number(record.gamesPlayed),
            QString::number(record.correctGuesses),
            QString::number(record.streak)
        };
        for (int column = 0; column < cells.size(); ++column) {
            QTableWidgetItem* item = new QTableWidgetItem(cells[column]);
            item->setTextAlignment(Qt::AlignCenter);
            leaderboardTable->setItem(row, column, item);
        }
    }
}

void MagicTrickWindow::showReplays() {
    replayList->clear();
    replayContent->clear();
    auto files = ReplayManager::listReplayFiles("replays");
    for (const auto& file : files) {
        replayList->addItem(toQString(file));
    }
    if (files.empty()) {
        replayContent->setPlainText("暂无回放记录。");
    } else {
        replayList->setCurrentRow(0);
    }
    stack->setCurrentIndex(ReplayPage);
}

void MagicTrickWindow::loadReplayContent(const QString& filename) {
    if (filename.isEmpty()) {
        return;
    }
    std::ifstream file(filename.toStdString());
    if (!file) {
        replayContent->setPlainText("回放文件读取失败。");
        return;
    }
    std::ostringstream oss;
    oss << file.rdbuf();
    replayContent->setPlainText(toQString(oss.str()));
}
