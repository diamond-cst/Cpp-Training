#include <QApplication>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QString>
#include <QStringList>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>
#include <algorithm>
#include <functional>
#include <random>

static bool isRedCard(const QString& card) {
    return card.contains("♥") || card.contains("♦");
}

static void clearLayout(QLayout* layout) {
    while (QLayoutItem* item = layout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            widget->deleteLater();
        }
        if (QLayout* childLayout = item->layout()) {
            clearLayout(childLayout);
        }
        delete item;
    }
}

static void clearGridLayout(QGridLayout* layout) {
    while (QLayoutItem* item = layout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }

    for (int i = 0; i < 12; ++i) {
        layout->setColumnStretch(i, 0);
    }
}

static QLabel* createCardWidget(const QString& card, bool compact = false) {
    QLabel* label = new QLabel(card);
    label->setObjectName("playingCard");
    label->setAlignment(Qt::AlignCenter);
    label->setFixedSize(compact ? 44 : 48, compact ? 58 : 64);
    label->setStyleSheet(QString(
        "QLabel#playingCard {"
        " background: #fffefb;"
        " color: %1;"
        " border: 1px solid #d8c7aa;"
        " border-radius: 9px;"
        " font-size: %2px;"
        " font-weight: 900;"
        " padding: 2px;"
        "}"
    ).arg(isRedCard(card) ? "#bd3434" : "#1f2925")
     .arg(compact ? 17 : 18));
    return label;
}

class PilePanel : public QFrame {
private:
    QLabel* titleLabel;
    QLabel* hintLabel;
    QGridLayout* cardsLayout;
    std::function<void()> clickHandler;

public:
    explicit PilePanel(int index, QWidget* parent = nullptr)
        : QFrame(parent), titleLabel(nullptr), hintLabel(nullptr), cardsLayout(nullptr) {
        setObjectName("pilePanel");
        setCursor(Qt::PointingHandCursor);

        QVBoxLayout* root = new QVBoxLayout(this);
        root->setContentsMargins(18, 14, 18, 14);
        root->setSpacing(10);

        titleLabel = new QLabel(QString("牌堆 %1").arg(index + 1));
        titleLabel->setObjectName("pileTitle");
        root->addWidget(titleLabel);

        cardsLayout = new QGridLayout;
        cardsLayout->setHorizontalSpacing(10);
        cardsLayout->setVerticalSpacing(10);
        cardsLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        root->addLayout(cardsLayout, 1);

        hintLabel = new QLabel("点击选择这一堆");
        hintLabel->setObjectName("pileHint");
        root->addWidget(hintLabel);
    }

    void setCards(const QStringList& cards) {
        clearLayout(cardsLayout);
        for (int i = 0; i < cards.size(); ++i) {
            cardsLayout->addWidget(createCardWidget(cards[i], true), i / 4, i % 4);
        }
    }

    void setClickHandler(std::function<void()> handler) {
        clickHandler = handler;
    }

protected:
    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton && isEnabled() && clickHandler) {
            clickHandler();
        }
        QFrame::mousePressEvent(event);
    }
};

class MagicTrickWindow : public QWidget {
private:
    QStringList deck;
    QVector<QStringList> piles;
    int currentRound;

    QLabel* titleLabel;
    QLabel* subtitleLabel;
    QLabel* statusLabel;
    QLabel* roundBadge;
    QLabel* deckTitleLabel;
    QFrame* deckPanel;
    QGridLayout* deckCardsLayout;
    QGridLayout* pileLayout;
    PilePanel* pilePanels[3];
    QPushButton* restartButton;

public:
    MagicTrickWindow(QWidget* parent = nullptr)
        : QWidget(parent), currentRound(0), titleLabel(nullptr), subtitleLabel(nullptr),
          statusLabel(nullptr), roundBadge(nullptr), deckTitleLabel(nullptr),
          deckPanel(nullptr), deckCardsLayout(nullptr), pileLayout(nullptr),
          restartButton(nullptr) {
        setObjectName("window");
        setWindowTitle("21 Card Magic Trick");
        resize(1120, 760);
        setMinimumSize(980, 720);
        setStyleSheet(
            "* { font-family: 'PingFang SC', 'Helvetica Neue', Helvetica; }"
            "QWidget#window { background: #e9eee8; color: #1f2925; }"
            "QFrame, QLabel { background: transparent; }"
            "QFrame#stage { background: #fbf8ef; border: 1px solid #ddd2bd; border-radius: 18px; }"
            "QFrame#hero { background: #edf4ec; border: 1px solid #d8e3d5; border-radius: 16px; }"
            "QLabel#title { color: #1f3f35; font-size: 30px; font-weight: 800; letter-spacing: 0px; }"
            "QLabel#subtitle { color: #66766f; font-size: 15px; }"
            "QLabel#status { background: #f0f5ef; color: #41544c; border: 1px solid #d7e2d4; border-radius: 12px; padding: 12px 16px; font-size: 16px; font-weight: 650; }"
            "QLabel#roundBadge { background: #2f6d5f; color: #fffaf0; border-radius: 15px; padding: 7px 14px; font-size: 15px; font-weight: 800; }"
            "QFrame#deckPanel { background: #fffdf8; border: 1px solid #dfd2bb; border-radius: 14px; }"
            "QLabel#deckTitle { color: #657269; font-size: 15px; font-weight: 800; }"
            "QFrame#pilePanel { background: #fffdf8; border: 1px solid #dfd2bb; border-radius: 14px; }"
            "QFrame#pilePanel:hover { background: #f7fbf5; border: 2px solid #6f9b7d; }"
            "QFrame#pilePanel:disabled { background: #f1eadc; border-color: #d5c7aa; }"
            "QLabel#pileTitle { color: #20312d; font-size: 19px; font-weight: 900; }"
            "QLabel#pileHint { color: #5d7067; font-size: 14px; font-weight: 800; }"
            "QPushButton { background: #fffdf8; border: 1px solid #dfd2bb; border-radius: 14px; padding: 18px; color: #22312d; font-size: 18px; font-weight: 700; text-align: left; }"
            "QPushButton:hover { background: #f7fbf5; border: 2px solid #6f9b7d; }"
            "QPushButton:pressed { background: #edf4ec; }"
            "QPushButton:disabled { color: #8a918d; background: #eee6d5; border-color: #d3c8ae; }"
            "QPushButton#restart { background: #2f6d5f; color: #fffaf0; border: none; border-radius: 12px; padding: 12px 22px; font-size: 16px; font-weight: 800; text-align: center; }"
            "QPushButton#restart:hover { background: #25594e; }"
        );

        QVBoxLayout* outer = new QVBoxLayout(this);
        outer->setContentsMargins(24, 22, 24, 22);

        QFrame* stage = new QFrame;
        stage->setObjectName("stage");
        QVBoxLayout* root = new QVBoxLayout(stage);
        root->setContentsMargins(26, 22, 26, 22);
        root->setSpacing(14);
        outer->addWidget(stage);

        QFrame* hero = new QFrame;
        hero->setObjectName("hero");
        QHBoxLayout* heroLayout = new QHBoxLayout(hero);
        heroLayout->setContentsMargins(22, 16, 22, 16);
        heroLayout->setSpacing(18);

        QVBoxLayout* titleStack = new QVBoxLayout;
        titleStack->setSpacing(8);

        titleLabel = new QLabel("21张牌魔术");
        titleLabel->setObjectName("title");
        subtitleLabel = new QLabel("从牌面中记住一张，连续三轮选择它所在的牌堆");
        subtitleLabel->setObjectName("subtitle");
        subtitleLabel->setWordWrap(true);

        titleStack->addWidget(titleLabel);
        titleStack->addWidget(subtitleLabel);
        heroLayout->addLayout(titleStack, 1);

        roundBadge = new QLabel;
        roundBadge->setObjectName("roundBadge");
        roundBadge->setAlignment(Qt::AlignCenter);
        roundBadge->setMinimumWidth(100);
        heroLayout->addWidget(roundBadge, 0, Qt::AlignTop);

        root->addWidget(hero);

        QHBoxLayout* statusRow = new QHBoxLayout;
        statusRow->setContentsMargins(2, 0, 2, 0);
        statusLabel = new QLabel;
        statusLabel->setObjectName("status");
        statusLabel->setWordWrap(true);
        statusRow->addWidget(statusLabel, 1);

        restartButton = new QPushButton("重新洗牌");
        restartButton->setObjectName("restart");
        restartButton->setFixedSize(120, 44);
        connect(restartButton, &QPushButton::clicked, this, [this]() {
            startGame();
        });
        statusRow->addWidget(restartButton, 0, Qt::AlignRight);
        root->addLayout(statusRow);

        deckPanel = new QFrame;
        deckPanel->setObjectName("deckPanel");
        deckPanel->setMinimumHeight(236);
        QVBoxLayout* deckRoot = new QVBoxLayout(deckPanel);
        deckRoot->setContentsMargins(18, 14, 18, 14);
        deckRoot->setSpacing(10);

        deckTitleLabel = new QLabel("请先记住一张牌");
        deckTitleLabel->setObjectName("deckTitle");
        deckRoot->addWidget(deckTitleLabel);

        deckCardsLayout = new QGridLayout;
        deckCardsLayout->setHorizontalSpacing(10);
        deckCardsLayout->setVerticalSpacing(10);
        deckCardsLayout->setAlignment(Qt::AlignTop);
        deckRoot->addLayout(deckCardsLayout);
        root->addWidget(deckPanel);

        pileLayout = new QGridLayout;
        pileLayout->setSpacing(16);
        for (int i = 0; i < 3; ++i) {
            pilePanels[i] = new PilePanel(i);
            pilePanels[i]->setMinimumHeight(230);
            pilePanels[i]->setEnabled(false);
            pilePanels[i]->setClickHandler([this, i]() {
                choosePile(i);
            });
            pileLayout->addWidget(pilePanels[i], 0, i);
        }
        root->addLayout(pileLayout, 1);

        startGame();
    }

private:
    void startGame() {
        currentRound = 0;
        deck = createDeck();
        shuffleDeck();
        updateRoundBadge();
        deckTitleLabel->setText("请先记住一张牌");
        renderDeckCards(deck);
        statusLabel->setText("先在心里记住下方任意一张牌，然后点击包含它的牌堆。");
        dealAndShowPiles();
    }

    QStringList createDeck() const {
        QStringList cards;
        const QStringList suits = {"♥", "♦", "♣"};
        const QStringList ranks = {"A", "2", "3", "4", "5", "6", "7"};

        for (const QString& suit : suits) {
            for (const QString& rank : ranks) {
                cards << QString("%1%2").arg(rank, suit);
            }
        }
        return cards;
    }

    void shuffleDeck() {
        std::random_device rd;
        std::mt19937 rng(rd());
        std::shuffle(deck.begin(), deck.end(), rng);
    }

    void renderDeckCards(const QStringList& cards) {
        clearGridLayout(deckCardsLayout);
        int columns = cards.size() <= 1 ? 1 : 7;
        for (int i = 0; i < cards.size(); ++i) {
            deckCardsLayout->addWidget(createCardWidget(cards[i]),
                                       i / columns,
                                       i % columns,
                                       Qt::AlignCenter);
        }
        for (int column = 0; column < columns; ++column) {
            deckCardsLayout->setColumnStretch(column, 1);
        }
    }

    void updateRoundBadge() {
        int displayRound = currentRound == 0 ? 1 : currentRound;
        roundBadge->setText(QString("第 %1 / 3 轮").arg(displayRound));
    }

    void dealAndShowPiles() {
        piles = QVector<QStringList>(3);
        for (int i = 0; i < deck.size(); ++i) {
            piles[i % 3] << deck[i];
        }

        ++currentRound;
        updateRoundBadge();
        for (int i = 0; i < 3; ++i) {
            pilePanels[i]->setCards(piles[i]);
            pilePanels[i]->setEnabled(true);
        }
        statusLabel->setText(QString("第 %1 轮：选择包含你记住那张牌的牌堆。").arg(currentRound));
    }

    void choosePile(int chosenIndex) {
        QStringList newDeck;
        int order[3] = {0, 1, 2};
        if (chosenIndex == 0) {
            order[0] = 1;
            order[1] = 0;
            order[2] = 2;
        } else if (chosenIndex == 2) {
            order[0] = 0;
            order[1] = 2;
            order[2] = 1;
        }

        for (int idx : order) {
            newDeck << piles[idx];
        }
        deck = newDeck;

        if (currentRound >= 3) {
            revealCard();
        } else {
            dealAndShowPiles();
        }
    }

    void revealCard() {
        for (int i = 0; i < 3; ++i) {
            pilePanels[i]->setEnabled(false);
        }

        QString card = deck[10];
        roundBadge->setText("揭晓");
        deckTitleLabel->setText("三轮选择已经完成，答案牌是");
        renderDeckCards(QStringList() << card);
        statusLabel->setText("揭晓完成：你记住的牌应该就是这张。点击“重新洗牌”可以再演示一轮。");
        QMessageBox::information(this,
                                 "揭晓时刻",
                                 QString("你记住的牌是：%1").arg(card));
    }
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    MagicTrickWindow window;
    window.show();
    return app.exec();
}
