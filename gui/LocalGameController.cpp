#include "MagicTrickWindow.h"
#include "CardWidgets.h"
#include "GuiUtils.h"
#include "ConfigurableCardTrick.h"
#include "Leaderboard.h"
#include "ReplayManager.h"
#include "ThreePileCardTrick.h"
#include "TwentyOneCardTrick.h"
#include "TwentySevenCardTrick.h"
#include "Utils.h"

#include <QAbstractButton>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFileInfo>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <fstream>

QWidget* MagicTrickWindow::buildSetupPage() {
    QVBoxLayout* root = new QVBoxLayout;
    root->setContentsMargins(24, 22, 24, 22);
    root->setSpacing(14);

    root->addLayout(titleRow("开始新游戏", "返回主界面"));
    root->addWidget(makeDivider());

    QFrame* form = makeTablePanel("setupCard");
    form->setMaximumWidth(980);
    QVBoxLayout* formRoot = new QVBoxLayout(form);
    formRoot->setContentsMargins(28, 24, 28, 24);
    formRoot->setSpacing(18);

    QHBoxLayout* setupHeader = new QHBoxLayout;
    setupHeader->setSpacing(8);
    setupHeader->addWidget(makeCardWidget("A♠", true));
    setupHeader->addWidget(makeCardWidget("K♥", true));
    setupHeader->addWidget(makeCardWidget("Q♣", true));
    setupHeader->addSpacing(12);
    setupHeader->addWidget(makeTextLabel("牌桌配置", "setupTitle"));
    setupHeader->addStretch(1);
    formRoot->addLayout(setupHeader);

    QGridLayout* grid = new QGridLayout;
    grid->setHorizontalSpacing(16);
    grid->setVerticalSpacing(20);

    playerNameEdit = new QLineEdit("玩家");
    trickTypeCombo = new QComboBox(form);
    trickTypeCombo->addItems({"21张牌魔术 - 经典版", "27张牌魔术 - 进阶版", "可配置牌数魔术"});
    trickTypeCombo->hide();
    deckSizeCombo = new QComboBox(form);
    deckSizeCombo->addItems({"15", "21", "27"});
    deckSizeCombo->setCurrentText("21");
    deckSizeCombo->hide();

    playerNameEdit->setMaximumWidth(560);

    auto choiceChip = [](const QString& text) {
        QPushButton* button = makeButton(text, "choiceChip");
        button->setCheckable(true);
        button->setFixedHeight(60);
        button->setMinimumWidth(120);
        return button;
    };

    QWidget* trickChoices = new QWidget;
    trickChoices->setMinimumHeight(64);
    QHBoxLayout* trickChoiceLayout = new QHBoxLayout(trickChoices);
    trickChoiceLayout->setContentsMargins(0, 0, 0, 0);
    trickChoiceLayout->setSpacing(10);
    QPushButton* classicButton = choiceChip("21 张经典");
    QPushButton* advancedButton = choiceChip("27 张进阶");
    QPushButton* customButton = choiceChip("可配置牌数");
    customButton->setMinimumWidth(138);
    QButtonGroup* trickChoiceGroup = new QButtonGroup(this);
    trickChoiceGroup->setExclusive(true);
    trickChoiceGroup->addButton(classicButton, 0);
    trickChoiceGroup->addButton(advancedButton, 1);
    trickChoiceGroup->addButton(customButton, 2);
    classicButton->setChecked(true);
    trickChoiceLayout->addStretch(1);
    trickChoiceLayout->addWidget(classicButton);
    trickChoiceLayout->addWidget(advancedButton);
    trickChoiceLayout->addWidget(customButton);
    trickChoiceLayout->addStretch(1);

    QWidget* deckChoices = new QWidget;
    deckChoices->setMinimumHeight(64);
    QHBoxLayout* deckChoiceLayout = new QHBoxLayout(deckChoices);
    deckChoiceLayout->setContentsMargins(0, 0, 0, 0);
    deckChoiceLayout->setSpacing(10);
    QPushButton* deck15Button = choiceChip("15 张");
    QPushButton* deck21Button = choiceChip("21 张");
    QPushButton* deck27Button = choiceChip("27 张");
    deck15Button->setMinimumWidth(96);
    deck21Button->setMinimumWidth(96);
    deck27Button->setMinimumWidth(96);
    QButtonGroup* deckChoiceGroup = new QButtonGroup(this);
    deckChoiceGroup->setExclusive(true);
    deckChoiceGroup->addButton(deck15Button, 15);
    deckChoiceGroup->addButton(deck21Button, 21);
    deckChoiceGroup->addButton(deck27Button, 27);
    deck21Button->setChecked(true);
    deckChoiceLayout->addStretch(1);
    deckChoiceLayout->addWidget(deck15Button);
    deckChoiceLayout->addWidget(deck21Button);
    deckChoiceLayout->addWidget(deck27Button);
    deckChoiceLayout->addStretch(1);

    grid->addWidget(makeTextLabel("玩家名称", "formLabel"), 0, 0);
    grid->addWidget(playerNameEdit, 0, 1);
    grid->setColumnMinimumWidth(0, 92);
    grid->setColumnStretch(1, 1);
    formRoot->addLayout(grid);

    QFrame* trickTypeBox = makeTablePanel("setupOptionGroup");
    QHBoxLayout* trickTypeLayout = new QHBoxLayout(trickTypeBox);
    trickTypeLayout->setContentsMargins(18, 12, 18, 12);
    trickTypeLayout->setSpacing(16);
    trickTypeLayout->addWidget(makeTextLabel("魔术类型", "setupOptionTitle"));
    trickTypeLayout->addWidget(trickChoices, 1);
    formRoot->addWidget(trickTypeBox);

    QFrame* deckSizeBox = makeTablePanel("setupOptionGroup");
    QHBoxLayout* deckSizeLayout = new QHBoxLayout(deckSizeBox);
    deckSizeLayout->setContentsMargins(18, 12, 18, 12);
    deckSizeLayout->setSpacing(16);
    deckSizeLayout->addWidget(makeTextLabel("可配置牌数", "setupOptionTitle"));
    deckSizeLayout->addWidget(deckChoices, 1);
    formRoot->addWidget(deckSizeBox);

    QFrame* modeBox = makeTablePanel("setupOptionGroup");
    QHBoxLayout* modeLayout = new QHBoxLayout(modeBox);
    modeLayout->setContentsMargins(18, 14, 18, 14);
    modeLayout->setSpacing(16);
    modeLayout->addWidget(makeTextLabel("游戏模式", "setupOptionTitle"));
    QPushButton* audienceMode = choiceChip("观众互动");
    QPushButton* magicianMode = choiceChip("魔术师练习");
    audienceMode->setChecked(true);
    modeGroup = new QButtonGroup(this);
    modeGroup->addButton(audienceMode, 0);
    modeGroup->addButton(magicianMode, 1);
    modeLayout->addStretch(1);
    modeLayout->addWidget(audienceMode);
    modeLayout->addWidget(magicianMode);
    modeLayout->addStretch(1);
    formRoot->addWidget(modeBox);

    QFrame* displayBox = makeTablePanel("setupOptionGroup");
    QHBoxLayout* displayLayout = new QHBoxLayout(displayBox);
    displayLayout->setContentsMargins(18, 14, 18, 14);
    displayLayout->setSpacing(16);
    displayLayout->addWidget(makeTextLabel("显示选项", "setupOptionTitle"));
    QPushButton* standardCardsRadio = choiceChip("标准扑克牌");
    numericCardsCheck = choiceChip("数字牌");
    standardCardsRadio->setChecked(true);
    QButtonGroup* displayChoiceGroup = new QButtonGroup(this);
    displayChoiceGroup->setExclusive(true);
    displayChoiceGroup->addButton(standardCardsRadio, 0);
    displayChoiceGroup->addButton(numericCardsCheck, 1);
    hideFacesCheck = new QCheckBox(form);
    hideFacesCheck->hide();
    QPushButton* hideFacesButton = choiceChip("隐藏牌面");
    hideFacesButton->setCheckable(true);
    connect(hideFacesButton, &QPushButton::toggled,
            hideFacesCheck, &QCheckBox::setChecked);
    displayLayout->addStretch(1);
    displayLayout->addWidget(standardCardsRadio);
    displayLayout->addWidget(numericCardsCheck);
    displayLayout->addWidget(hideFacesButton);
    displayLayout->addStretch(1);
    formRoot->addWidget(displayBox);

    QPushButton* startButton = makeButton("进入游戏", "primaryButton");
    startButton->setMinimumWidth(180);
    QLabel* hint = makeTextLabel("经典版固定 21 张，进阶版固定 27 张；选择“可配置牌数魔术”时才使用牌数选项。", "status", true);
    QHBoxLayout* actionRow = new QHBoxLayout;
    actionRow->setSpacing(16);
    actionRow->addWidget(hint, 1);
    actionRow->addWidget(startButton, 0, Qt::AlignRight | Qt::AlignVCenter);
    formRoot->addLayout(actionRow);

    root->addWidget(form, 0, Qt::AlignHCenter);
    root->addStretch(1);

    auto setDeckChoiceEnabled = [deck15Button, deck21Button, deck27Button](bool enabled) {
        deck15Button->setEnabled(enabled);
        deck21Button->setEnabled(enabled);
        deck27Button->setEnabled(enabled);
    };
    connect(trickChoiceGroup, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::idClicked),
            this, [this, setDeckChoiceEnabled](int index) {
                trickTypeCombo->setCurrentIndex(index);
                deckSizeCombo->setEnabled(index == 2);
                setDeckChoiceEnabled(index == 2);
            });
    connect(deckChoiceGroup, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::idClicked),
            this, [this](int deckSize) { deckSizeCombo->setCurrentText(QString::number(deckSize)); });
    deckSizeCombo->setEnabled(false);
    setDeckChoiceEnabled(false);
    connect(startButton, &QPushButton::clicked, this, [this]() { startLocalGame(false); });

    return makePageShell(root);
}

QWidget* MagicTrickWindow::buildLoadPage() {
    QVBoxLayout* root = new QVBoxLayout;
    root->setContentsMargins(24, 22, 24, 22);
    root->setSpacing(16);
    root->addLayout(titleRow("加载游戏", "返回主界面"));
    root->addWidget(makeDivider());

    QFrame* form = makeTablePanel("tablePanel");
    QGridLayout* grid = new QGridLayout(form);
    grid->setContentsMargins(18, 16, 18, 16);
    grid->setHorizontalSpacing(14);
    grid->setVerticalSpacing(14);
    loadPlayerEdit = new QLineEdit("玩家");
    grid->addWidget(makeTextLabel("玩家名称", "subtitle"), 0, 0);
    grid->addWidget(loadPlayerEdit, 0, 1);
    QPushButton* loadButton = makeButton("读取存档", "primaryButton");
    grid->addWidget(loadButton, 1, 1, Qt::AlignRight);
    grid->setColumnStretch(1, 1);

    root->addWidget(form);

    QFrame* listPanel = makeTablePanel("tablePanel");
    QVBoxLayout* listLayout = new QVBoxLayout(listPanel);
    listLayout->setContentsMargins(16, 14, 16, 14);
    listLayout->setSpacing(10);
    listLayout->addWidget(makeTextLabel("匹配到的存档", "subtitle"));
    loadSaveList = new QListWidget;
    loadSaveList->setMinimumHeight(190);
    listLayout->addWidget(loadSaveList);
    loadSelectedSaveButton = makeButton("加载选中存档", "primaryButton");
    loadSelectedSaveButton->setEnabled(false);
    listLayout->addWidget(loadSelectedSaveButton, 0, Qt::AlignRight);
    root->addWidget(listPanel, 1);

    root->addWidget(makeTextLabel("点击“读取存档”会先列出该玩家的存档；选中后再加载，加载后从当前轮继续。", "status", true));

    connect(loadButton, &QPushButton::clicked, this, [this]() { loadLocalGame(); });
    connect(loadSaveList, &QListWidget::currentRowChanged, this, [this](int row) {
        loadSelectedSaveButton->setEnabled(row >= 0);
    });
    connect(loadSelectedSaveButton, &QPushButton::clicked, this, [this]() { loadSelectedLocalGame(); });
    return makePageShell(root);
}

QWidget* MagicTrickWindow::buildGamePage() {
    QVBoxLayout* root = new QVBoxLayout;
    root->setContentsMargins(22, 20, 22, 20);
    root->setSpacing(0);

    QFrame* table = makeTablePanel("woodPanel");
    QVBoxLayout* tableRoot = new QVBoxLayout(table);
    tableRoot->setContentsMargins(34, 28, 34, 28);
    tableRoot->setSpacing(14);

    QHBoxLayout* header = new QHBoxLayout;
    gameTitleLabel = makeTextLabel("本地游戏", "pageTitle");
    gameRoundBadge = makeTextLabel("第 1 / 3 轮", "badge");
    gameRoundBadge->setAlignment(Qt::AlignCenter);
    gameRoundBadge->setMinimumWidth(110);
    pauseGameButton = makeButton("暂停");
    pauseGameButton->setFixedWidth(92);
    saveGameButton = makeButton("保存");
    saveGameButton->setFixedWidth(92);
    backToMenuButton = makeButton("主菜单");
    backToMenuButton->setFixedWidth(92);
    header->addWidget(gameTitleLabel);
    header->addStretch(1);
    header->addWidget(gameRoundBadge);
    header->addWidget(pauseGameButton);
    header->addWidget(saveGameButton);
    header->addWidget(backToMenuButton);
    tableRoot->addLayout(header);
    tableRoot->addWidget(makeDivider());

    gameStatusLabel = makeTextLabel("", "status", true);
    tableRoot->addWidget(gameStatusLabel);

    QFrame* deckSection = makeTablePanel("tablePanel");
    localDeckSection = deckSection;
    QVBoxLayout* deckRoot = new QVBoxLayout(deckSection);
    deckRoot->setContentsMargins(16, 14, 16, 14);
    deckRoot->setSpacing(8);
    deckTitleLabel = makeTextLabel("请先记住一张牌", "subtitle");
    deckTitleLabel->setAlignment(Qt::AlignCenter);
    deckCardsLayout = new QGridLayout;
    deckCardsLayout->setHorizontalSpacing(8);
    deckCardsLayout->setVerticalSpacing(8);
    deckCardsLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    deckRoot->addWidget(deckTitleLabel);
    deckRoot->addLayout(deckCardsLayout);
    beginDealButton = makeButton("我记住了，开始发牌", "primaryButton");
    beginDealButton->setMinimumWidth(240);
    deckRoot->addWidget(beginDealButton, 0, Qt::AlignHCenter);

    revealConfirmArea = new QWidget;
    QHBoxLayout* revealConfirmLayout = new QHBoxLayout(revealConfirmArea);
    revealConfirmLayout->setContentsMargins(0, 4, 0, 0);
    revealConfirmLayout->setSpacing(12);
    revealConfirmLayout->addStretch(1);
    revealWrongButton = makeButton("没猜中", "dangerButton");
    revealCorrectButton = makeButton("猜中了", "primaryButton");
    revealWrongButton->setMinimumWidth(128);
    revealCorrectButton->setMinimumWidth(128);
    revealConfirmLayout->addWidget(revealWrongButton);
    revealConfirmLayout->addWidget(revealCorrectButton);
    revealConfirmLayout->addStretch(1);
    deckRoot->addWidget(revealConfirmArea);
    revealConfirmArea->setVisible(false);
    tableRoot->addWidget(deckSection);

    localPileArea = new QWidget;
    QHBoxLayout* pileRow = new QHBoxLayout;
    pileRow->setContentsMargins(0, 0, 0, 0);
    pileRow->setSpacing(12);
    for (int i = 0; i < 3; ++i) {
        pilePanels[i] = new PilePanel(i);
        pilePanels[i]->setClickHandler([this, i]() { chooseLocalPile(i + 1); });
        pileRow->addWidget(pilePanels[i], 1);
    }
    localPileArea->setLayout(pileRow);
    tableRoot->addWidget(localPileArea, 1);
    root->addWidget(table, 1);

    connect(beginDealButton, &QPushButton::clicked, this, [this]() { dealAndRenderLocalPiles(); });
    connect(revealWrongButton, &QPushButton::clicked, this, [this]() {
        finishLocalGame(currentTrick->getRevealIndexForGui(), false);
    });
    connect(revealCorrectButton, &QPushButton::clicked, this, [this]() {
        finishLocalGame(currentTrick->getRevealIndexForGui(), true);
    });
    connect(pauseGameButton, &QPushButton::clicked, this, [this]() { toggleLocalPause(); });
    connect(saveGameButton, &QPushButton::clicked, this, [this]() { saveCurrentGame(); });
    connect(backToMenuButton, &QPushButton::clicked, this, [this]() { showMenu(); });
    return makePageShell(root);
}

void MagicTrickWindow::startLocalGame(bool fromLoad) {
    try {
        if (!fromLoad) {
            currentPlayerName = toStdString(playerNameEdit->text());
            if (currentPlayerName.empty()) {
                currentPlayerName = "玩家";
            }

            const int trickIndex = trickTypeCombo->currentIndex();
            if (trickIndex == 0) {
                currentTrick.reset(new TwentyOneCardTrick(false));
            } else if (trickIndex == 1) {
                currentTrick.reset(new TwentySevenCardTrick(false));
            } else {
                currentTrick.reset(new ConfigurableCardTrick(deckSizeCombo->currentText().toInt(),
                                                             false,
                                                             false,
                                                             modeGroup->checkedId() == 1));
            }
            currentTrick->setPlayerName(currentPlayerName);
            currentTrick->setMagicianMode(modeGroup->checkedId() == 1);
            currentTrick->setNumericCards(numericCardsCheck->isChecked());
            currentTrick->setHideFaces(hideFacesCheck->isChecked());
            currentTrick->setSoundEnabled(true);
            currentTrick->initializeForGui();
        }

        currentReplay.reset(new ReplayManager("replays"));
        currentReplay->start(currentTrick->getPlayerName(),
                             currentTrick->getName(),
                             currentTrick->isMagicianMode() ? "魔术师练习" : "观众互动");
        currentTrick->setReplayManager(currentReplay.get());
        currentTrick->setSoundEnabled(true);
        currentPlayerName = currentTrick->getPlayerName();

        gameTitleLabel->setText(QString("%1 - %2")
                                    .arg(toQString(currentTrick->getName()),
                                         toQString(currentTrick->getPlayerName())));
        stack->setCurrentIndex(GamePage);
        setLocalPaused(false);
        if (pauseGameButton) {
            pauseGameButton->setEnabled(true);
        }
        if (fromLoad) {
            renderLoadedLocalGame();
        } else {
            renderLocalGameStart();
        }
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "无法开始游戏", e.what());
    }
}

void MagicTrickWindow::renderLocalGameStart() {
    setLocalPaused(false);
    gameStatusLabel->setVisible(true);
    renderDeckCards(deckCardsLayout,
                    toQStringList(currentTrick->getWorkingDeckCardsForGui(true)),
                    false,
                    this,
                    nullptr);
    deckTitleLabel->setText(currentTrick->isMagicianMode() ? "请先记住全部牌列" : "请先记住一张牌");
    gameRoundBadge->setText("准备");
    gameStatusLabel->setText(currentTrick->isMagicianMode()
        ? "魔术师练习模式：先记住完整牌列。记好后点击“开始发牌”，再按模拟观众提示选择牌堆。"
        : "先从完整牌列中记住一张牌。记好后点击“开始发牌”，再选择目标牌所在的牌堆。");
    if (beginDealButton) {
        beginDealButton->setVisible(true);
        beginDealButton->setEnabled(true);
    }
    if (revealConfirmArea) {
        revealConfirmArea->setVisible(false);
    }
    if (localPileArea) {
        localPileArea->setVisible(false);
    }
    for (int i = 0; i < 3; ++i) {
        pilePanels[i]->setCards(QStringList());
        pilePanels[i]->setEnabled(false);
    }
    if (currentTrick->isComplete()) {
        if (beginDealButton) {
            beginDealButton->setVisible(false);
        }
        if (pauseGameButton) {
            pauseGameButton->setEnabled(false);
        }
        gameRoundBadge->setText("已完成");
        gameStatusLabel->setText("这个存档已经完成三轮，可以返回主菜单开始新游戏。");
        return;
    }
}

void MagicTrickWindow::renderLoadedLocalGame() {
    setLocalPaused(false);
    resetGrid(deckCardsLayout);
    gameStatusLabel->setVisible(true);
    if (beginDealButton) {
        beginDealButton->setVisible(false);
        beginDealButton->setEnabled(false);
    }
    if (revealConfirmArea) {
        revealConfirmArea->setVisible(false);
    }
    if (localPileArea) {
        localPileArea->setVisible(false);
    }
    for (int i = 0; i < 3; ++i) {
        pilePanels[i]->setCards(QStringList());
        pilePanels[i]->setEnabled(false);
    }

    if (currentTrick->isComplete()) {
        if (pauseGameButton) {
            pauseGameButton->setEnabled(false);
        }
        gameRoundBadge->setText("已完成");
        deckTitleLabel->setText("这个存档已经完成");
        gameStatusLabel->setText("这个存档已经完成三轮，可以返回主菜单开始新游戏。");
        return;
    }

    deckTitleLabel->setText("正在恢复当前轮次");
    gameStatusLabel->setText("已加载存档，将从当前轮次继续选择牌堆。");
    dealAndRenderLocalPiles();
}

void MagicTrickWindow::dealAndRenderLocalPiles() {
    if (localGamePaused) {
        return;
    }
    gameStatusLabel->setVisible(true);
    if (beginDealButton) {
        beginDealButton->setVisible(false);
    }
    if (revealConfirmArea) {
        revealConfirmArea->setVisible(false);
    }
    if (localPileArea) {
        localPileArea->setVisible(true);
    }
    resetGrid(deckCardsLayout);
    deckTitleLabel->setText("选择目标牌所在的牌堆");
    currentTrick->dealCurrentRoundForGui();
    QVector<QStringList> piles = toPileLists(currentTrick->getCurrentPilesForGui(false));
    int animationOffset = 0;
    for (int i = 0; i < 3; ++i) {
        pilePanels[i]->setCards(piles[i], animationOffset);
        animationOffset += piles[i].size();
        pilePanels[i]->setEnabled(true);
        pilePanels[i]->setHint("点击选择这一堆");
    }

    int displayRound = currentTrick->getCurrentRound() + 1;
    gameRoundBadge->setText(QString("第 %1 / 3 轮").arg(displayRound));
    if (currentTrick->isMagicianMode()) {
        int targetPile = currentTrick->getPracticeTargetPileForGui();
        gameStatusLabel->setText(QString("第 %1 轮：模拟观众说目标牌在第 %2 堆，请点击对应牌堆。")
                                     .arg(displayRound)
                                     .arg(targetPile));
    } else {
        gameStatusLabel->setText(QString("第 %1 轮：选择包含你记住那张牌的牌堆。").arg(displayRound));
    }
}

void MagicTrickWindow::toggleLocalPause() {
    if (!currentTrick || currentTrick->isComplete()) {
        return;
    }
    setLocalPaused(!localGamePaused);
}

void MagicTrickWindow::setLocalPaused(bool paused) {
    bool wasPaused = localGamePaused;
    localGamePaused = paused;
    if (pauseGameButton) {
        pauseGameButton->setText(paused ? "继续" : "暂停");
    }
    if (localDeckSection) {
        localDeckSection->setEnabled(!paused);
    }
    if (localPileArea) {
        localPileArea->setEnabled(!paused);
    }
    if (paused && !wasPaused) {
        localStatusVisibleBeforePause = gameStatusLabel->isVisible();
        localStatusTextBeforePause = gameStatusLabel->text();
        gameStatusLabel->setVisible(true);
        gameStatusLabel->setText("游戏已暂停。保存和返回主菜单仍然可用，点击“继续”恢复当前局。");
    } else if (!paused && wasPaused) {
        gameStatusLabel->setText(localStatusTextBeforePause);
        gameStatusLabel->setVisible(localStatusVisibleBeforePause);
    }
}

void MagicTrickWindow::chooseLocalPile(int pileNumber) {
    if (!currentTrick || currentTrick->isComplete() || localGamePaused) {
        return;
    }

    try {
        bool practiceCorrect = currentTrick->applyPileChoiceForGui(pileNumber);
        if (currentTrick->isMagicianMode() && !practiceCorrect) {
            gameStatusLabel->setText("这轮选择和模拟观众答案不一致，失误会计入最终练习分。");
        }

        if (currentTrick->isComplete()) {
            for (int i = 0; i < 3; ++i) {
                pilePanels[i]->setEnabled(false);
            }
            if (currentTrick->isMagicianMode()) {
                showPracticeRevealSelection();
            } else {
                revealAudienceResult();
            }
        } else {
            dealAndRenderLocalPiles();
        }
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "游戏状态错误", e.what());
    }
}

void MagicTrickWindow::showPracticeRevealSelection() {
    if (localPileArea) {
        localPileArea->setVisible(false);
    }
    playRevealCueSound();
    gameRoundBadge->setText("揭晓");
    gameStatusLabel->setVisible(false);
    deckTitleLabel->setText("三轮完成，请点击你要揭晓的牌");
    renderDeckCards(deckCardsLayout,
                    toQStringList(currentTrick->getWorkingDeckCardsForGui(true)),
                    true,
                    this,
                    [this](int index) { finishLocalGame(index, false); });
}

void MagicTrickWindow::revealAudienceResult() {
    if (localPileArea) {
        localPileArea->setVisible(false);
    }
    int revealIndex = currentTrick->getRevealIndexForGui();
    QString card = cleanCardText(toQString(currentTrick->getCardAtForGui(revealIndex).toString(false)));
    playRevealCueSound();
    gameRoundBadge->setText("揭晓");
    gameStatusLabel->setVisible(false);
    deckTitleLabel->setText("魔术师猜的是：");
    resetGrid(deckCardsLayout);
    deckCardsLayout->addWidget(makeLargeCardWidget(card), 0, 0, Qt::AlignCenter);
    if (revealConfirmArea) {
        revealConfirmArea->setVisible(true);
    }
}

void MagicTrickWindow::finishLocalGame(int selectedIndex, bool audienceConfirmedCorrect) {
    try {
        bool correct = currentTrick->finalizeRevealForGui(selectedIndex, audienceConfirmedCorrect);
        Utils::playResultSound(correct);
        updateLeaderboardRecord(currentTrick->getPlayerName(),
                                currentTrick->getScore(),
                                currentTrick->wasLastGuessCorrect(),
                                currentTrick->hasResult());
        if (currentReplay) {
            currentReplay->save();
            currentReplay->exportHtml();
        }
        if (pauseGameButton) {
            pauseGameButton->setEnabled(false);
        }

        QString result = correct ? "成功" : "失败";
        QString revealedCard = cleanCardText(toQString(currentTrick->getCardAtForGui(selectedIndex).toString(false)));
        if (revealConfirmArea) {
            revealConfirmArea->setVisible(false);
        }
        gameStatusLabel->setVisible(false);
        deckTitleLabel->setText(QString("揭晓完成：%1 · 最终分数 %2 · 用时 %3 秒")
                                    .arg(result)
                                    .arg(currentTrick->getScore())
                                    .arg(currentTrick->getElapsedSeconds()));
        resetGrid(deckCardsLayout);
        deckCardsLayout->addWidget(makeLargeCardWidget(revealedCard), 0, 0, Qt::AlignCenter);
        for (int i = 0; i < 3; ++i) {
            pilePanels[i]->setEnabled(false);
        }
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "揭晓失败", e.what());
    }
}

void MagicTrickWindow::saveCurrentGame() {
    if (!currentTrick) {
        return;
    }
    try {
        std::string filename = playerSaveFilename(currentTrick->getPlayerName());
        currentTrick->saveState(filename);
        gameStatusLabel->setText(QString("游戏进度已保存到 %1。").arg(toQString(filename)));
        QMessageBox::information(this, "保存成功", QString("游戏已保存到：%1").arg(toQString(filename)));
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "保存失败", e.what());
    }
}

void MagicTrickWindow::loadLocalGame() {
    std::string playerName = toStdString(loadPlayerEdit->text());
    if (playerName.empty()) {
        playerName = "玩家";
    }
    QString exactFilename = toQString(playerSaveFilename(playerName));
    QFileInfo exactInfo(exactFilename);
    QString savePrefix = exactInfo.completeBaseName();

    loadSaveList->clear();
    loadSelectedSaveButton->setEnabled(false);

    QDir saveDir("saves");
    if (!saveDir.exists()) {
        QMessageBox::information(this, "没有存档", "saves 目录不存在。");
        return;
    }

    QFileInfoList files = saveDir.entryInfoList(QStringList() << "*.dat",
                                                QDir::Files,
                                                QDir::Time);
    for (const QFileInfo& fileInfo : files) {
        QString baseName = fileInfo.completeBaseName();
        if (baseName == savePrefix || baseName.startsWith(savePrefix + "_")) {
            QString display = QString("%1    %2")
                                  .arg(fileInfo.fileName(),
                                       fileInfo.lastModified().toString("yyyy-MM-dd HH:mm:ss"));
            QListWidgetItem* item = new QListWidgetItem(display);
            item->setData(Qt::UserRole, fileInfo.filePath());
            loadSaveList->addItem(item);
        }
    }

    if (loadSaveList->count() == 0) {
        QMessageBox::information(this,
                                 "没有存档",
                                 QString("没有找到玩家“%1”的存档。").arg(loadPlayerEdit->text().trimmed()));
        return;
    }

    loadSaveList->setCurrentRow(0);
}

void MagicTrickWindow::loadSelectedLocalGame() {
    QListWidgetItem* item = loadSaveList ? loadSaveList->currentItem() : nullptr;
    if (!item) {
        QMessageBox::information(this, "请选择存档", "请先从列表中选择一个存档。");
        return;
    }

    std::string filename = item->data(Qt::UserRole).toString().toStdString();
    try {
        std::unique_ptr<ThreePileCardTrick> trick(new TwentyOneCardTrick(false));
        try {
            trick->loadState(filename);
        } catch (...) {
            trick.reset(new TwentySevenCardTrick(false));
            try {
                trick->loadState(filename);
            } catch (...) {
                trick.reset(new ConfigurableCardTrick());
                trick->loadState(filename);
            }
        }
        currentTrick = std::move(trick);
        startLocalGame(true);
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "加载失败", e.what());
    }
}

void MagicTrickWindow::updateLeaderboardRecord(const std::string& playerName,
                                               int finalScore,
                                               bool correct,
                                               bool hasResult) {
    if (!hasResult) {
        return;
    }

    Leaderboard leaderboard("leaderboard.dat");
    PlayerRecord* existing = leaderboard.getPlayerRecord(playerName);
    int nextStreak = correct ? ((existing ? existing->streak : 0) + 1) : 0;
    PlayerRecord record(playerName, finalScore, 1, correct ? 1 : 0, nextStreak,
                        Utils::getCurrentTimestamp());
    leaderboard.addOrUpdateRecord(record);
}
