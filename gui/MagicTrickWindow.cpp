#include "MagicTrickWindow.h"
#include "CardWidgets.h"
#include "GuiUtils.h"
#include "Leaderboard.h"
#include "ReplayManager.h"
#include "ThreePileCardTrick.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>

MagicTrickWindow::MagicTrickWindow(QWidget* parent)
    : QWidget(parent), stack(nullptr), menuStatusLabel(nullptr), menuReplayLabel(nullptr),
      playerNameEdit(nullptr), trickTypeCombo(nullptr), deckSizeCombo(nullptr), modeGroup(nullptr),
      numericCardsCheck(nullptr), hideFacesCheck(nullptr), loadPlayerEdit(nullptr),
      loadSaveList(nullptr), loadSelectedSaveButton(nullptr),
      gameTitleLabel(nullptr), gameStatusLabel(nullptr), gameRoundBadge(nullptr),
      deckTitleLabel(nullptr), deckCardsLayout(nullptr), beginDealButton(nullptr),
      revealConfirmArea(nullptr), revealCorrectButton(nullptr), revealWrongButton(nullptr),
      localPileArea(nullptr), localDeckSection(nullptr), pauseGameButton(nullptr),
      saveGameButton(nullptr), backToMenuButton(nullptr),
      leaderboardTable(nullptr), replayList(nullptr),
      replayContent(nullptr), networkStatusLabel(nullptr), networkRoundBadge(nullptr),
      networkHostEdit(nullptr), networkPortSpin(nullptr), listenButton(nullptr),
      connectButton(nullptr), disconnectButton(nullptr), networkControlsPanel(nullptr),
      networkDeckSection(nullptr),
      networkPileArea(nullptr), networkRevealConfirmArea(nullptr),
      networkRevealCorrectButton(nullptr), networkRevealWrongButton(nullptr),
      networkReadyButton(nullptr), networkDeckTitleLabel(nullptr), networkDeckLayout(nullptr),
      networkServer(nullptr), networkSocket(nullptr), networkIsMagician(false),
      networkRound(0), networkPendingChoice(0), localGamePaused(false), localStatusVisibleBeforePause(true) {
    setObjectName("window");
    setWindowTitle("21张牌魔术 - Qt 主界面");
    resize(1180, 780);
    setMinimumSize(980, 700);
    applyStyle();

    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(18, 18, 18, 18);
    root->setSpacing(0);

    stack = new QStackedWidget;
    root->addWidget(stack);

    stack->addWidget(buildMenuPage());
    stack->addWidget(buildSetupPage());
    stack->addWidget(buildLoadPage());
    stack->addWidget(buildGamePage());
    stack->addWidget(buildLeaderboardPage());
    stack->addWidget(buildReplayPage());
    stack->addWidget(buildHelpPage());
    stack->addWidget(buildNetworkPage());

    showMenu();
}

MagicTrickWindow::~MagicTrickWindow() {
    resetNetwork();
}

void MagicTrickWindow::applyStyle() {
    setStyleSheet(
        "* { font-family: 'PingFang SC', 'Microsoft YaHei', 'Helvetica Neue', Arial; letter-spacing: 0px; }"
        "QWidget#window { background: #100907; color: #f8f1df; }"
        "QFrame#pageShell { background-image: url(:/assets/felt_table.png); border: 5px solid #7b4a22; border-radius: 22px; }"
        "QFrame#tablePanel { background: rgba(7, 32, 24, 218); border: 2px solid #d1a756; border-radius: 18px; }"
        "QFrame#woodPanel { background: #2c160d; border-image: url(:/assets/wood_frame.png) 24 24 24 24 stretch stretch; border-width: 18px; }"
        "QFrame#sideNav { background: rgba(12, 31, 24, 224); border: 2px solid #c69b45; border-radius: 16px; }"
        "QFrame#heroPanel { background: rgba(8, 34, 26, 228); border: 3px solid #d1a756; border-radius: 22px; }"
        "QFrame#menuTopBar { background: rgba(10, 34, 26, 198); border: 2px solid #a5813e; border-radius: 16px; }"
        "QFrame#menuStage { background: rgba(4, 42, 31, 226); border: 3px solid #d1a756; border-radius: 22px; }"
        "QFrame#menuBottomBar { background: rgba(12, 24, 18, 214); border: 2px solid #a5813e; border-radius: 14px; }"
        "QFrame#setupCard { background: rgba(6, 34, 25, 226); border: 3px solid #d1a756; border-radius: 22px; }"
        "QFrame#setupOptionGroup { background: rgba(9, 31, 23, 176); border: 1px solid #a5813e; border-radius: 12px; }"
        "QFrame#section, QFrame#featureCard { background: rgba(14, 43, 33, 226); border: 1px solid #a5813e; border-radius: 12px; }"
        "QFrame#featureCard:hover { background: rgba(22, 62, 47, 236); border: 2px solid #f0cd70; }"
        "QFrame#goldDivider { background: #d1a756; border: none; }"
        "QFrame#pilePanel { background: rgba(11, 42, 31, 232); border: 2px solid #a5813e; border-radius: 14px; }"
        "QFrame#pilePanel:hover { background: rgba(20, 71, 51, 238); border: 3px solid #f0cd70; }"
        "QFrame#pilePanel:disabled { background: rgba(28, 37, 31, 210); border-color: #6e6048; }"
        "QLabel#appTitle { color: #f8d76f; font-size: 34px; font-weight: 900; }"
        "QLabel#appKicker { color: #d8c087; font-size: 14px; font-weight: 750; }"
        "QLabel#logoMark { background: #f7f1df; color: #8f3234; border: 3px solid #d1a756; border-radius: 11px; font-size: 27px; font-weight: 900; padding: 8px; }"
        "QLabel#avatarMark { background: #f7f1df; color: #8f3234; border: 3px solid #d1a756; border-radius: 34px; font-size: 24px; font-weight: 900; padding: 8px; }"
        "QLabel#titlePlaque { color: #f8d76f; background: transparent; border: none; font-size: 46px; font-weight: 900; padding: 6px 18px; }"
        "QLabel#cardBackArt { image: url(:/assets/card_back.png); background: transparent; }"
        "QLabel#pageTitle { color: #f8d76f; font-size: 28px; font-weight: 900; }"
        "QLabel#heroTitle { color: #fff7df; font-size: 34px; font-weight: 900; }"
        "QLabel#heroSubtitle { color: #d7dac5; font-size: 15px; font-weight: 650; }"
        "QLabel#setupTitle { color: #f8d76f; font-size: 20px; font-weight: 900; }"
        "QLabel#setupOptionTitle { color: #f8d76f; font-size: 14px; font-weight: 900; }"
        "QLabel#formLabel { color: #d7dac5; font-size: 14px; font-weight: 750; }"
        "QLabel#subtitle, QLabel#bodyText { color: #d7dac5; font-size: 14px; }"
        "QLabel#featureTitle { color: #f8d76f; font-size: 16px; font-weight: 900; }"
        "QLabel#featureBody { color: #ece0bf; font-size: 14px; }"
        "QLabel#status { background: rgba(35, 25, 14, 218); color: #fff3cc; border: 2px solid #d1a756; border-radius: 12px; padding: 10px 14px; font-size: 14px; font-weight: 750; }"
        "QLabel#menuRank { background: rgba(35, 25, 14, 170); color: #fff3cc; border: 1px solid #a5813e; border-radius: 10px; padding: 8px 12px; font-size: 13px; font-weight: 750; }"
        "QLabel#badge { background: #7f2422; color: #fff7df; border: 2px solid #d1a756; border-radius: 12px; padding: 7px 12px; font-size: 14px; font-weight: 900; }"
        "QLabel#pileTitle { color: #f8d76f; font-size: 17px; font-weight: 900; }"
        "QLabel#pileHint { color: #e0d1a6; font-size: 13px; font-weight: 700; }"
        "QPushButton { background: #f8f1df; color: #172b23; border: 2px solid #caa655; border-radius: 10px; padding: 9px 13px; font-size: 14px; font-weight: 850; text-align: left; }"
        "QPushButton:hover { background: #fff8e6; border: 3px solid #f0cd70; }"
        "QPushButton:pressed { background: #ead7a5; }"
        "QPushButton:disabled { color: #7d867f; background: #d6d0c0; border-color: #9d9584; }"
        "QPushButton#primaryButton, QPushButton#navPrimary { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ffe48d, stop:0.42 #d7a745, stop:1 #9b6824); color: #201207; border: 3px solid #f4d27a; border-radius: 16px; text-align: center; font-size: 16px; font-weight: 900; }"
        "QPushButton#primaryButton:hover, QPushButton#navPrimary:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #fff1a5, stop:0.45 #efc763, stop:1 #b9792b); }"
        "QPushButton#dangerButton { background: #7f2422; color: #fff7df; border: 2px solid #d1a756; text-align: center; }"
        "QPushButton#dangerButton:hover { background: #a83d40; }"
        "QPushButton#topMenuButton { min-height: 40px; min-width: 90px; text-align: center; background: rgba(22, 25, 16, 230); color: #f7e7bd; border: 2px solid #a5813e; }"
        "QPushButton#topMenuButton:hover { background: rgba(45, 43, 24, 238); color: #fff4cd; border: 3px solid #f0cd70; }"
        "QPushButton#navButton { min-height: 48px; text-align: center; padding-left: 14px; padding-right: 14px; background: rgba(22, 25, 16, 230); color: #f7e7bd; border: 2px solid #a5813e; }"
        "QPushButton#navButton:hover { background: rgba(45, 43, 24, 238); color: #fff4cd; border: 3px solid #f0cd70; }"
        "QPushButton#choiceChip { min-height: 48px; text-align: center; background: rgba(21, 31, 20, 218); color: #f7e7bd; border: 2px solid #a5813e; border-radius: 10px; padding: 0px 16px; font-size: 15px; font-weight: 900; }"
        "QPushButton#choiceChip:hover { background: rgba(43, 45, 24, 230); border: 3px solid #f0cd70; }"
        "QPushButton#choiceChip:checked { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ffe48d, stop:1 #c88a1a); color: #201207; border: 3px solid #f4d27a; }"
        "QPushButton#choiceChip:disabled { background: rgba(28, 37, 31, 160); color: #7d867f; border-color: #6e6048; }"
        "QPushButton#navPrimary { min-height: 62px; font-size: 22px; }"
        "QLineEdit, QComboBox, QSpinBox { background: #f8f1df; border: 2px solid #caa655; border-radius: 9px; padding: 8px 10px; font-size: 14px; min-height: 24px; color: #172b23; }"
        "QComboBox::drop-down { width: 34px; border-left: 1px solid #caa655; background: #ead7a5; border-top-right-radius: 7px; border-bottom-right-radius: 7px; }"
        "QComboBox QAbstractItemView { background: #f8f1df; color: #172b23; border: 2px solid #caa655; border-radius: 10px; padding: 4px; outline: 0px; selection-background-color: #d7a745; selection-color: #201207; font-size: 14px; }"
        "QComboBox QAbstractItemView::item { min-height: 34px; padding: 7px 12px; border-radius: 6px; color: #172b23; background: #f8f1df; }"
        "QComboBox QAbstractItemView::item:selected, QComboBox QAbstractItemView::item:hover { background: #d7a745; color: #201207; }"
        "QGroupBox { color: #f8d76f; font-weight: 850; border: 1px solid #a5813e; border-radius: 10px; margin-top: 12px; padding: 12px; background: rgba(10, 33, 25, 155); }"
        "QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 4px; }"
        "QRadioButton, QCheckBox { color: #f8f1df; font-size: 14px; spacing: 8px; }"
        "QTableWidget, QListWidget, QPlainTextEdit { background: #fbf2dd; border: 2px solid #caa655; border-radius: 10px; color: #172b23; font-size: 13px; selection-background-color: #d7a745; selection-color: #172b23; }"
        "QHeaderView::section { background: #d7a745; color: #172b23; border: none; padding: 8px; font-weight: 900; }"
    );
}

QWidget* MagicTrickWindow::makePageShell(QLayout* contentLayout) {
    QFrame* shell = new QFrame;
    shell->setObjectName("pageShell");
    shell->setLayout(contentLayout);
    return shell;
}

QPushButton* MagicTrickWindow::navButton(const QString& text) {
    return makeButton(text, "navButton");
}

QFrame* MagicTrickWindow::makeStatSection(const QString& title, const QString& body) {
    QFrame* section = new QFrame;
    section->setObjectName("featureCard");
    QVBoxLayout* layout = new QVBoxLayout(section);
    layout->setContentsMargins(18, 16, 18, 16);
    layout->setSpacing(8);
    layout->addWidget(makeTextLabel(title, "featureTitle"));
    layout->addWidget(makeTextLabel(body, "featureBody", true));
    return section;
}

QFrame* MagicTrickWindow::makeSection(const QString& title, QWidget* body) {
    QFrame* section = new QFrame;
    section->setObjectName("section");
    QVBoxLayout* layout = new QVBoxLayout(section);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->setSpacing(10);
    if (!title.isEmpty()) {
        layout->addWidget(makeTextLabel(title, "subtitle"));
    }
    layout->addWidget(body);
    return section;
}

QHBoxLayout* MagicTrickWindow::titleRow(const QString& title, const QString& backText) {
    QHBoxLayout* row = new QHBoxLayout;
    row->setSpacing(12);
    row->addWidget(makeTextLabel(title, "pageTitle"));
    row->addStretch(1);
    QPushButton* back = makeButton(backText);
    back->setFixedWidth(128);
    row->addWidget(back);
    connect(back, &QPushButton::clicked, this, [this]() { showMenu(); });
    return row;
}

QWidget* MagicTrickWindow::buildMenuPage() {
    QVBoxLayout* root = new QVBoxLayout;
    root->setContentsMargins(30, 24, 30, 24);
    root->setSpacing(14);

    QFrame* topFrame = makeTablePanel("menuTopBar");
    QHBoxLayout* topBar = new QHBoxLayout(topFrame);
    topBar->setContentsMargins(18, 14, 18, 14);
    topBar->setSpacing(12);
    QLabel* logo = makeTextLabel("A♥", "avatarMark");
    logo->setAlignment(Qt::AlignCenter);
    logo->setFixedSize(72, 72);
    topBar->addWidget(logo);

    QVBoxLayout* brand = new QVBoxLayout;
    brand->setSpacing(4);
    brand->addWidget(makeTextLabel("21张牌魔术", "appTitle"));
    brand->addWidget(makeTextLabel("读心牌桌 · Qt Edition", "appKicker"));
    topBar->addLayout(brand);
    topBar->addStretch(1);
    menuStatusLabel = makeTextLabel("", "menuRank", true);
    menuStatusLabel->setMinimumWidth(260);
    menuStatusLabel->setMaximumWidth(340);
    topBar->addWidget(menuStatusLabel);
    QPushButton* leaderboardButton = makeButton("排行榜", "topMenuButton");
    topBar->addWidget(leaderboardButton);
    root->addWidget(topFrame);

    QFrame* table = makeTablePanel("menuStage");
    QVBoxLayout* tableLayout = new QVBoxLayout(table);
    tableLayout->setContentsMargins(54, 34, 54, 34);
    tableLayout->setSpacing(16);
    tableLayout->addStretch(1);

    QHBoxLayout* fan = new QHBoxLayout;
    fan->setSpacing(8);
    fan->addStretch(1);
    fan->addWidget(makeCardWidget("10♠", true));
    fan->addWidget(makeCardWidget("A♠"));
    fan->addWidget(makeCardWidget("K♥"));
    fan->addWidget(makeCardWidget("Q♣"));
    fan->addWidget(makeCardWidget("J♦"));
    fan->addWidget(makeCardWidget("9♣", true));
    fan->addStretch(1);
    tableLayout->addLayout(fan);

    QLabel* plaque = makeTextLabel("21张牌魔术", "titlePlaque");
    plaque->setAlignment(Qt::AlignCenter);
    tableLayout->addWidget(plaque, 0, Qt::AlignHCenter);

    QLabel* subtitle = makeTextLabel("记住一张牌，穿过三轮牌堆选择，最后让魔术揭晓答案。", "heroSubtitle", true);
    subtitle->setAlignment(Qt::AlignCenter);
    tableLayout->addWidget(subtitle);

    QPushButton* startButton = makeButton("开始新游戏", "navPrimary");
    startButton->setMinimumWidth(360);
    startButton->setMaximumWidth(460);
    tableLayout->addStretch(1);
    tableLayout->addWidget(startButton, 0, Qt::AlignHCenter);
    tableLayout->addStretch(2);
    root->addWidget(table, 1);

    QFrame* bottomFrame = makeTablePanel("menuBottomBar");
    QHBoxLayout* menuRow = new QHBoxLayout(bottomFrame);
    menuRow->setContentsMargins(14, 12, 14, 12);
    menuRow->setSpacing(12);
    QPushButton* loadButton = navButton("加载游戏");
    QPushButton* replayButton = navButton("查看回放记录");
    QPushButton* networkButton = navButton("网络双人对战");
    QPushButton* helpButton = navButton("游戏说明");
    QPushButton* quitButton = navButton("退出");
    quitButton->setObjectName("dangerButton");
    quitButton->setFixedWidth(92);
    quitButton->setMinimumHeight(40);
    menuRow->addWidget(loadButton);
    menuRow->addWidget(replayButton);
    menuRow->addWidget(networkButton);
    menuRow->addWidget(helpButton);
    menuRow->addStretch(1);
    menuRow->addWidget(quitButton);
    root->addWidget(bottomFrame);

    connect(startButton, &QPushButton::clicked, this, [this]() { stack->setCurrentIndex(SetupPage); });
    connect(loadButton, &QPushButton::clicked, this, [this]() { stack->setCurrentIndex(LoadPage); });
    connect(leaderboardButton, &QPushButton::clicked, this, [this]() { showLeaderboard(); });
    connect(helpButton, &QPushButton::clicked, this, [this]() { stack->setCurrentIndex(HelpPage); });
    connect(replayButton, &QPushButton::clicked, this, [this]() { showReplays(); });
    connect(networkButton, &QPushButton::clicked, this, [this]() { stack->setCurrentIndex(NetworkPage); });
    connect(quitButton, &QPushButton::clicked, this, &QWidget::close);

    return makePageShell(root);
}

void MagicTrickWindow::showMenu() {
    refreshMenuStatus();
    stack->setCurrentIndex(MenuPage);
}

void MagicTrickWindow::refreshMenuStatus() {
    Leaderboard leaderboard("leaderboard.dat");
    auto records = leaderboard.getTopRecords(1);
    QString top = records.empty()
        ? "当前暂无排行榜记录。"
        : QString("当前第一名：%1，累计 %2 分。")
              .arg(toQString(records[0].name))
              .arg(records[0].score);
    menuStatusLabel->setText(top);

    if (menuReplayLabel) {
        auto replays = ReplayManager::listReplayFiles("replays");
        menuReplayLabel->setText(QString("排行榜记录：%1 条\n回放记录：%2 个")
                                     .arg(leaderboard.getRecordCount())
                                     .arg(static_cast<int>(replays.size())));
    }
}
