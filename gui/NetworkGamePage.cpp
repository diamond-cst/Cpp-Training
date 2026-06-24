#include "MagicTrickWindow.h"
#include "CardWidgets.h"
#include "GuiUtils.h"
#include "Utils.h"

#include <QAbstractSocket>
#include <QFrame>
#include <QGridLayout>
#include <QHostAddress>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTcpServer>
#include <QTcpSocket>
#include <QVBoxLayout>
#include <algorithm>
#include <random>

QWidget* MagicTrickWindow::buildNetworkPage() {
    QVBoxLayout* root = new QVBoxLayout;
    root->setContentsMargins(22, 20, 22, 20);
    root->setSpacing(0);

    QFrame* table = makeTablePanel("woodPanel");
    QVBoxLayout* tableRoot = new QVBoxLayout(table);
    tableRoot->setContentsMargins(34, 28, 34, 28);
    tableRoot->setSpacing(14);
    tableRoot->addLayout(titleRow("网络双人对战", "返回主界面"));
    tableRoot->addWidget(makeDivider());

    QFrame* controls = makeTablePanel("tablePanel");
    networkControlsPanel = controls;
    QGridLayout* grid = new QGridLayout(controls);
    grid->setContentsMargins(14, 12, 14, 12);
    grid->setHorizontalSpacing(10);
    grid->setVerticalSpacing(10);

    networkHostEdit = new QLineEdit("127.0.0.1");
    networkPortSpin = new QSpinBox;
    networkPortSpin->setRange(1024, 65535);
    networkPortSpin->setValue(5000);
    listenButton = makeButton("我是魔术师：创建房间", "primaryButton");
    connectButton = makeButton("我是观众：进入房间", "primaryButton");
    disconnectButton = makeButton("断开连接");
    disconnectButton->setEnabled(false);
    networkRoundBadge = makeTextLabel("未连接", "badge");
    networkRoundBadge->setAlignment(Qt::AlignCenter);

    grid->addWidget(makeTextLabel("魔术师地址", "subtitle"), 0, 0);
    grid->addWidget(networkHostEdit, 0, 1);
    grid->addWidget(makeTextLabel("端口", "subtitle"), 0, 2);
    grid->addWidget(networkPortSpin, 0, 3);
    grid->addWidget(listenButton, 1, 0, 1, 2);
    grid->addWidget(connectButton, 1, 2, 1, 2);
    grid->addWidget(disconnectButton, 1, 4);
    grid->addWidget(networkRoundBadge, 0, 4);
    grid->setColumnStretch(1, 1);
    tableRoot->addWidget(controls);

    networkStatusLabel = makeTextLabel("选择角色后开始连接。", "status", true);
    tableRoot->addWidget(networkStatusLabel);

    QFrame* deckSection = makeTablePanel("tablePanel");
    networkDeckSection = deckSection;
    QVBoxLayout* deckRoot = new QVBoxLayout(deckSection);
    deckRoot->setContentsMargins(14, 12, 14, 12);
    deckRoot->setSpacing(8);
    networkDeckTitleLabel = makeTextLabel("网络牌组", "subtitle");
    networkDeckTitleLabel->setAlignment(Qt::AlignCenter);
    deckRoot->addWidget(networkDeckTitleLabel);
    networkDeckLayout = new QGridLayout;
    networkDeckLayout->setHorizontalSpacing(8);
    networkDeckLayout->setVerticalSpacing(8);
    networkDeckLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    deckRoot->addLayout(networkDeckLayout);

    networkReadyButton = makeButton("我记住了，开始发牌", "primaryButton");
    networkReadyButton->setMinimumWidth(240);
    networkReadyButton->setVisible(false);
    deckRoot->addWidget(networkReadyButton, 0, Qt::AlignHCenter);

    networkRevealConfirmArea = new QWidget;
    QHBoxLayout* revealConfirmLayout = new QHBoxLayout(networkRevealConfirmArea);
    revealConfirmLayout->setContentsMargins(0, 4, 0, 0);
    revealConfirmLayout->setSpacing(12);
    revealConfirmLayout->addStretch(1);
    networkRevealWrongButton = makeButton("没猜中", "dangerButton");
    networkRevealCorrectButton = makeButton("猜中了", "primaryButton");
    networkRevealWrongButton->setMinimumWidth(128);
    networkRevealCorrectButton->setMinimumWidth(128);
    revealConfirmLayout->addWidget(networkRevealWrongButton);
    revealConfirmLayout->addWidget(networkRevealCorrectButton);
    revealConfirmLayout->addStretch(1);
    deckRoot->addWidget(networkRevealConfirmArea);
    networkRevealConfirmArea->setVisible(false);
    tableRoot->addWidget(deckSection);

    networkPileArea = new QWidget;
    QHBoxLayout* piles = new QHBoxLayout(networkPileArea);
    piles->setContentsMargins(0, 0, 0, 0);
    piles->setSpacing(12);
    for (int i = 0; i < 3; ++i) {
        networkPilePanels[i] = new PilePanel(i);
        networkPilePanels[i]->setEnabled(false);
        networkPilePanels[i]->setClickHandler([this, i]() { chooseNetworkPile(i + 1); });
        piles->addWidget(networkPilePanels[i], 1);
    }
    tableRoot->addWidget(networkPileArea, 1);
    networkPileArea->setVisible(false);
    root->addWidget(table, 1);

    connect(listenButton, &QPushButton::clicked, this, [this]() { startNetworkHost(); });
    connect(connectButton, &QPushButton::clicked, this, [this]() { startNetworkClient(); });
    connect(disconnectButton, &QPushButton::clicked, this, [this]() { resetNetwork(); });
    connect(networkReadyButton, &QPushButton::clicked, this, [this]() {
        if (!networkSocket || networkIsMagician) {
            return;
        }
        sendNetworkLine("READY");
        networkReadyButton->setEnabled(false);
        networkReadyButton->setVisible(false);
        networkStatusLabel->setText("已准备好，等待魔术师开始第一轮发牌。");
    });
    connect(networkRevealWrongButton, &QPushButton::clicked, this, [this]() {
        sendNetworkLine("CONFIRM|N");
        if (networkRevealConfirmArea) networkRevealConfirmArea->setVisible(false);
        Utils::playResultSound(false);
        if (networkStatusLabel) networkStatusLabel->setVisible(true);
        networkStatusLabel->setText("确认已发送，本局网络魔术完成。");
    });
    connect(networkRevealCorrectButton, &QPushButton::clicked, this, [this]() {
        sendNetworkLine("CONFIRM|Y");
        if (networkRevealConfirmArea) networkRevealConfirmArea->setVisible(false);
        Utils::playResultSound(true);
        if (networkStatusLabel) networkStatusLabel->setVisible(true);
        networkStatusLabel->setText("确认已发送，本局网络魔术完成。");
    });

    return makePageShell(root);
}

void MagicTrickWindow::resetNetwork() {
    if (networkSocket) {
        networkSocket->disconnectFromHost();
        networkSocket->deleteLater();
        networkSocket = nullptr;
    }
    if (networkServer) {
        networkServer->close();
        networkServer->deleteLater();
        networkServer = nullptr;
    }
    networkBuffer.clear();
    networkDeck.clear();
    networkPiles.clear();
    networkRound = 0;
    networkPendingChoice = 0;
    networkIsMagician = false;
    if (listenButton) listenButton->setEnabled(true);
    if (connectButton) connectButton->setEnabled(true);
    if (disconnectButton) disconnectButton->setEnabled(false);
    if (networkControlsPanel) networkControlsPanel->setVisible(true);
    if (networkReadyButton) {
        networkReadyButton->setVisible(false);
        networkReadyButton->setEnabled(false);
    }
    if (networkRoundBadge) networkRoundBadge->setText("未连接");
    if (networkStatusLabel) networkStatusLabel->setText("连接已断开，可重新选择角色。");
    if (networkStatusLabel) networkStatusLabel->setVisible(true);
    if (networkDeckTitleLabel) networkDeckTitleLabel->setText("网络牌组");
    if (networkDeckSection) networkDeckSection->setVisible(true);
    if (networkPileArea) networkPileArea->setVisible(false);
    if (networkRevealConfirmArea) networkRevealConfirmArea->setVisible(false);
    if (networkDeckLayout) resetGrid(networkDeckLayout);
    if (networkPilePanels[0]) {
        for (int i = 0; i < 3; ++i) {
            networkPilePanels[i]->setCards(QStringList());
            networkPilePanels[i]->setEnabled(false);
        }
    }
}

void MagicTrickWindow::startNetworkHost() {
    resetNetwork();
    networkIsMagician = true;
    networkServer = new QTcpServer(this);
    int port = networkPortSpin->value();
    if (!networkServer->listen(QHostAddress::Any, static_cast<quint16>(port))) {
        networkStatusLabel->setText("创建房间失败：" + networkServer->errorString());
        return;
    }
    listenButton->setEnabled(false);
    connectButton->setEnabled(false);
    disconnectButton->setEnabled(true);
    networkRoundBadge->setText("等待观众");
    networkStatusLabel->setText(QString("房间已创建，端口 %1。等待观众进入。").arg(port));
    connect(networkServer, &QTcpServer::newConnection, this, [this]() {
        networkSocket = networkServer->nextPendingConnection();
        setupNetworkSocket();
        if (networkControlsPanel) networkControlsPanel->setVisible(false);
        networkStatusLabel->setText("观众已加入，正在发牌。");
        sendNetworkLine("WELCOME|21张牌网络对战");
        initializeNetworkDeck();
        sendNetworkLine("DECK|" + joinCards(networkDeck));
        if (networkDeckTitleLabel) networkDeckTitleLabel->setText("等待观众记牌");
        renderNetworkDeck(networkDeck);
        networkRoundBadge->setText("记牌");
        networkStatusLabel->setText("牌组已发送，等待观众记牌并确认准备。");
    });
}

void MagicTrickWindow::startNetworkClient() {
    resetNetwork();
    networkIsMagician = false;
    networkSocket = new QTcpSocket(this);
    setupNetworkSocket();
    listenButton->setEnabled(false);
    connectButton->setEnabled(false);
    disconnectButton->setEnabled(true);
    networkRoundBadge->setText("连接中");
    networkStatusLabel->setText("正在进入魔术师房间。");
    networkSocket->connectToHost(networkHostEdit->text(), static_cast<quint16>(networkPortSpin->value()));
}

void MagicTrickWindow::setupNetworkSocket() {
    connect(networkSocket, &QTcpSocket::readyRead, this, [this]() { readNetworkLines(); });
    connect(networkSocket, &QTcpSocket::connected, this, [this]() {
        if (networkControlsPanel) {
            networkControlsPanel->setVisible(false);
        }
        networkStatusLabel->setText("已连接，等待魔术师发牌。");
        networkRoundBadge->setText("已连接");
    });
    connect(networkSocket, &QTcpSocket::disconnected, this, [this]() {
        if (networkControlsPanel) {
            networkControlsPanel->setVisible(true);
        }
        if (networkStatusLabel) {
            networkStatusLabel->setText("远端连接已断开。");
        }
        if (disconnectButton) {
            disconnectButton->setEnabled(false);
        }
    });
    connect(networkSocket, &QAbstractSocket::errorOccurred,
            this, [this]() {
                if (networkControlsPanel) {
                    networkControlsPanel->setVisible(true);
                }
                if (networkStatusLabel && networkSocket) {
                    networkStatusLabel->setText("网络错误：" + networkSocket->errorString());
                }
            });
}

void MagicTrickWindow::initializeNetworkDeck() {
    networkRound = 0;
    networkDeck.clear();
    const QStringList suits = {"♥", "♦", "♣"};
    const QStringList ranks = {"A", "2", "3", "4", "5", "6", "7"};
    for (const QString& suit : suits) {
        for (const QString& rank : ranks) {
            networkDeck << rank + suit;
        }
    }
    std::mt19937 rng(std::random_device{}());
    std::shuffle(networkDeck.begin(), networkDeck.end(), rng);
}

void MagicTrickWindow::dealNetworkRound() {
    if (networkReadyButton) {
        networkReadyButton->setVisible(false);
        networkReadyButton->setEnabled(false);
    }
    networkPiles = QVector<QStringList>(3);
    for (int i = 0; i < networkDeck.size(); ++i) {
        networkPiles[i % 3] << networkDeck[i];
    }
    ++networkRound;
    networkRoundBadge->setText(QString("第 %1 / 3 轮").arg(networkRound));
    if (networkStatusLabel) {
        networkStatusLabel->setVisible(true);
    }
    if (networkDeckSection) {
        networkDeckSection->setVisible(false);
    }
    if (networkPileArea) {
        networkPileArea->setVisible(true);
    }
    renderNetworkPiles(true);
    sendNetworkLine(QString("ROUND|%1").arg(networkRound));
    for (int i = 0; i < 3; ++i) {
        sendNetworkLine(QString("PILE|%1|%2").arg(i + 1).arg(joinCards(networkPiles[i])));
    }
    sendNetworkLine("ENDROUND");
    networkStatusLabel->setText(QString("第 %1 轮牌堆已发送，等待观众选择。").arg(networkRound));
}

void MagicTrickWindow::chooseNetworkPile(int pileNumber) {
    if (!networkSocket) {
        return;
    }
    if (networkIsMagician) {
        if (networkPendingChoice <= 0) {
            return;
        }
        if (pileNumber != networkPendingChoice) {
            if (networkStatusLabel) {
                networkStatusLabel->setVisible(true);
                networkStatusLabel->setText(QString("观众选择的是第 %1 堆，请点击对应牌堆。")
                                                .arg(networkPendingChoice));
            }
            return;
        }
        applyNetworkChoice(pileNumber);
        return;
    }
    sendNetworkLine(QString("CHOICE|%1").arg(pileNumber));
    networkStatusLabel->setText(QString("已选择第 %1 堆，等待魔术师确认。").arg(pileNumber));
    for (int i = 0; i < 3; ++i) {
        networkPilePanels[i]->setEnabled(false);
    }
}

void MagicTrickWindow::applyNetworkChoice(int chosenPile) {
    networkPendingChoice = 0;
    for (int i = 0; i < 3; ++i) {
        networkPilePanels[i]->setEnabled(false);
    }

    QStringList nextDeck;
    int order[3] = {0, 1, 2};
    if (chosenPile == 1) {
        order[0] = 1;
        order[1] = 0;
        order[2] = 2;
    } else if (chosenPile == 3) {
        order[0] = 0;
        order[1] = 2;
        order[2] = 1;
    }
    for (int idx : order) {
        nextDeck << networkPiles[idx];
    }
    networkDeck = nextDeck;
    if (networkRound >= 3) {
        sendNetworkLine("ACK|FINAL");
        showNetworkRevealSelection();
    } else {
        sendNetworkLine(QString("ACK|ROUND|%1").arg(networkRound));
        dealNetworkRound();
        networkStatusLabel->setText(QString("观众选择了第 %1 堆，已发出第 %2 轮牌堆，等待观众继续选择。")
                                        .arg(chosenPile)
                                        .arg(networkRound));
    }
}

void MagicTrickWindow::showNetworkRevealSelection() {
    if (!networkIsMagician) {
        return;
    }
    networkRoundBadge->setText("揭晓");
    if (networkStatusLabel) {
        networkStatusLabel->setVisible(false);
    }
    if (networkDeckSection) {
        networkDeckSection->setVisible(true);
    }
    if (networkPileArea) {
        networkPileArea->setVisible(false);
    }
    if (networkRevealConfirmArea) {
        networkRevealConfirmArea->setVisible(false);
    }
    if (networkDeckTitleLabel) {
        networkDeckTitleLabel->setText("三轮完成，请点击你要揭晓的牌");
    }
    renderNetworkPiles(false, false);
    renderDeckCards(networkDeckLayout,
                    networkDeck,
                    true,
                    this,
                    [this](int index) { revealNetworkCard(index); },
                    false);
}

void MagicTrickWindow::revealNetworkCard(int selectedIndex) {
    if (!networkIsMagician || selectedIndex < 0 || selectedIndex >= networkDeck.size()) {
        return;
    }
    const QString card = cleanCardText(networkDeck.value(selectedIndex));
    playRevealCueSound();
    networkRoundBadge->setText("揭晓");
    if (networkStatusLabel) {
        networkStatusLabel->setVisible(false);
    }
    if (networkDeckSection) {
        networkDeckSection->setVisible(true);
    }
    if (networkPileArea) {
        networkPileArea->setVisible(false);
    }
    if (networkRevealConfirmArea) {
        networkRevealConfirmArea->setVisible(false);
    }
    if (networkDeckTitleLabel) {
        networkDeckTitleLabel->setText("你猜的是：");
    }
    resetGrid(networkDeckLayout);
    networkDeckLayout->addWidget(makeLargeCardWidget(card), 0, 0, Qt::AlignCenter);
    sendNetworkLine("REVEAL|" + card);
}

void MagicTrickWindow::readNetworkLines() {
    networkBuffer += networkSocket->readAll();
    while (true) {
        int newline = networkBuffer.indexOf('\n');
        if (newline < 0) {
            break;
        }
        QByteArray rawLine = networkBuffer.left(newline);
        networkBuffer.remove(0, newline + 1);
        handleNetworkLine(QString::fromUtf8(rawLine).trimmed());
    }
}

void MagicTrickWindow::handleNetworkLine(const QString& line) {
    if (line.isEmpty()) {
        return;
    }
    if (networkIsMagician) {
        if (line.startsWith("CHOICE|")) {
            int chosenPile = line.mid(7).toInt();
            if (chosenPile >= 1 && chosenPile <= 3) {
                networkPendingChoice = chosenPile;
                networkStatusLabel->setVisible(true);
                networkStatusLabel->setText(QString("观众选择了第 %1 堆，请点击对应牌堆完成收牌。")
                                                .arg(chosenPile));
                for (int i = 0; i < 3; ++i) {
                    const bool selected = (i + 1 == chosenPile);
                    networkPilePanels[i]->setEnabled(selected);
                    networkPilePanels[i]->setHint(selected ? "点击此堆继续" : "观众未选择这一堆");
                }
            }
        } else if (line == "READY") {
            if (networkRound == 0 && !networkDeck.isEmpty()) {
                networkStatusLabel->setText("观众已准备好，开始第一轮发牌。");
                dealNetworkRound();
            }
        } else if (line.startsWith("CONFIRM|")) {
            bool correct = line.mid(8).toUpper() == "Y";
            if (networkStatusLabel) networkStatusLabel->setVisible(true);
            networkStatusLabel->setText(correct ? "观众确认猜对，本局网络魔术完成。"
                                                 : "观众确认未猜对，本局网络魔术完成。");
        }
        return;
    }

    if (line.startsWith("WELCOME|")) {
        networkStatusLabel->setText("已进入房间，等待魔术师发牌。");
    } else if (line.startsWith("DECK|")) {
        networkDeck = splitCards(line.mid(5));
        if (networkDeckSection) {
            networkDeckSection->setVisible(true);
        }
        if (networkPileArea) {
            networkPileArea->setVisible(false);
        }
        if (networkDeckTitleLabel) networkDeckTitleLabel->setText("请先记住一张牌");
        renderNetworkDeck(networkDeck);
        if (networkReadyButton) {
            networkReadyButton->setVisible(true);
            networkReadyButton->setEnabled(true);
        }
        networkRoundBadge->setText("记牌");
        networkStatusLabel->setText("请先从完整牌列中记住一张牌。记好后点击“开始发牌”。");
    } else if (line.startsWith("ROUND|")) {
        networkRound = line.mid(6).toInt();
        networkPiles = QVector<QStringList>(3);
        networkRoundBadge->setText(QString("第 %1 / 3 轮").arg(networkRound));
    } else if (line.startsWith("PILE|")) {
        QStringList parts = line.split("|");
        if (parts.size() >= 3) {
            int pileNumber = parts[1].toInt();
            if (pileNumber >= 1 && pileNumber <= 3) {
                networkPiles[pileNumber - 1] = splitCards(parts.mid(2).join("|"));
            }
        }
    } else if (line == "ENDROUND") {
        if (networkStatusLabel) {
            networkStatusLabel->setVisible(true);
        }
        if (networkDeckSection) {
            networkDeckSection->setVisible(false);
        }
        if (networkPileArea) {
            networkPileArea->setVisible(true);
        }
        renderNetworkPiles(true);
        networkStatusLabel->setText(QString("第 %1 轮：点击包含你记住那张牌的牌堆。").arg(networkRound));
    } else if (line == "ACK|FINAL") {
        networkStatusLabel->setVisible(true);
        networkStatusLabel->setText("魔术师正在从最终牌列中选择揭晓牌。");
    } else if (line.startsWith("ACK|ROUND|")) {
        networkStatusLabel->setText("魔术师已收到选择，等待下一轮。");
    } else if (line.startsWith("REVEAL|")) {
        QString card = cleanCardText(line.mid(7));
        playRevealCueSound();
        networkRoundBadge->setText("揭晓");
        if (networkStatusLabel) {
            networkStatusLabel->setVisible(false);
        }
        if (networkDeckSection) {
            networkDeckSection->setVisible(true);
        }
        if (networkPileArea) {
            networkPileArea->setVisible(false);
        }
        if (networkRevealConfirmArea) {
            networkRevealConfirmArea->setVisible(true);
        }
        if (networkDeckTitleLabel) networkDeckTitleLabel->setText("魔术师猜的是：");
        resetGrid(networkDeckLayout);
        networkDeckLayout->addWidget(makeLargeCardWidget(card), 0, 0, Qt::AlignCenter);
        renderNetworkPiles(false, false);
    }
}

void MagicTrickWindow::sendNetworkLine(const QString& line) {
    if (!networkSocket) {
        return;
    }
    QByteArray payload = line.toUtf8();
    payload.append('\n');
    networkSocket->write(payload);
}

void MagicTrickWindow::renderNetworkDeck(const QStringList& cards) {
    renderDeckCards(networkDeckLayout, cards, false, this, nullptr);
}

void MagicTrickWindow::renderNetworkPiles(bool enabled, bool playSounds) {
    int animationOffset = 0;
    for (int i = 0; i < 3; ++i) {
        QStringList cards = i < networkPiles.size() ? networkPiles[i] : QStringList();
        networkPilePanels[i]->setCards(cards, animationOffset, playSounds);
        animationOffset += cards.size();
        networkPilePanels[i]->setEnabled(enabled && !networkIsMagician);
        networkPilePanels[i]->setHint(networkIsMagician ? "等待观众选择" : "点击选择这一堆");
    }
}
