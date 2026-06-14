#include <QApplication>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QString>
#include <QStringList>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>
#include <algorithm>
#include <random>

class MagicTrickWindow : public QWidget {
private:
    QStringList deck;
    QVector<QStringList> piles;
    int currentRound;

    QLabel* titleLabel;
    QLabel* statusLabel;
    QLabel* deckLabel;
    QGridLayout* pileLayout;
    QPushButton* pileButtons[3];
    QPushButton* restartButton;

public:
    MagicTrickWindow(QWidget* parent = nullptr)
        : QWidget(parent), currentRound(0), titleLabel(nullptr), statusLabel(nullptr),
          deckLabel(nullptr), pileLayout(nullptr), restartButton(nullptr) {
        setWindowTitle("21 Card Magic Trick - Qt GUI");
        resize(860, 560);
        setStyleSheet(
            "QWidget { background: #f6f8fb; color: #1f2933; font-family: Helvetica; }"
            "QLabel#title { font-size: 26px; font-weight: 700; }"
            "QLabel#status { font-size: 16px; color: #52616f; }"
            "QLabel#deck { background: white; border: 1px solid #d9e2ec; border-radius: 8px; padding: 14px; font-size: 17px; }"
            "QPushButton { background: white; border: 1px solid #bcccdc; border-radius: 8px; padding: 14px; font-size: 16px; text-align: left; }"
            "QPushButton:hover { border-color: #2f80ed; background: #eef6ff; }"
            "QPushButton#restart { background: #2f80ed; color: white; font-weight: 700; text-align: center; }"
        );

        QVBoxLayout* root = new QVBoxLayout(this);
        root->setContentsMargins(28, 24, 28, 24);
        root->setSpacing(16);

        titleLabel = new QLabel("21张牌魔术图形界面");
        titleLabel->setObjectName("title");
        root->addWidget(titleLabel);

        statusLabel = new QLabel;
        statusLabel->setObjectName("status");
        root->addWidget(statusLabel);

        deckLabel = new QLabel;
        deckLabel->setObjectName("deck");
        deckLabel->setWordWrap(true);
        root->addWidget(deckLabel);

        pileLayout = new QGridLayout;
        pileLayout->setSpacing(14);
        for (int i = 0; i < 3; ++i) {
            pileButtons[i] = new QPushButton;
            pileButtons[i]->setMinimumHeight(150);
            pileButtons[i]->setEnabled(false);
            pileLayout->addWidget(pileButtons[i], 0, i);

            connect(pileButtons[i], &QPushButton::clicked, this, [this, i]() {
                choosePile(i);
            });
        }
        root->addLayout(pileLayout);

        restartButton = new QPushButton("重新开始");
        restartButton->setObjectName("restart");
        restartButton->setFixedHeight(46);
        connect(restartButton, &QPushButton::clicked, this, [this]() {
            startGame();
        });
        root->addWidget(restartButton);

        startGame();
    }

private:
    void startGame() {
        currentRound = 0;
        deck = createDeck();
        shuffleDeck();
        deckLabel->setText("请先从下面21张牌中记住一张：\n" + deck.join(" "));
        statusLabel->setText("点击任意牌堆开始第1轮。每轮请选择包含你记住那张牌的牌堆。");
        dealAndShowPiles();
    }

    QStringList createDeck() const {
        QStringList cards;
        const QStringList suits = {"♥", "♦", "♣"};
        const QStringList ranks = {"A", "2", "3", "4", "5", "6", "7"};

        for (const QString& suit : suits) {
            for (const QString& rank : ranks) {
                cards << QString("[%1%2]").arg(rank, suit);
            }
        }
        return cards;
    }

    void shuffleDeck() {
        std::random_device rd;
        std::mt19937 rng(rd());
        std::shuffle(deck.begin(), deck.end(), rng);
    }

    void dealAndShowPiles() {
        piles = QVector<QStringList>(3);
        for (int i = 0; i < deck.size(); ++i) {
            piles[i % 3] << deck[i];
        }

        ++currentRound;
        for (int i = 0; i < 3; ++i) {
            pileButtons[i]->setText(QString("牌堆 %1\n\n%2")
                                    .arg(i + 1)
                                    .arg(piles[i].join("  ")));
            pileButtons[i]->setEnabled(true);
        }
        statusLabel->setText(QString("第 %1 轮：请点击包含你记住牌的牌堆。").arg(currentRound));
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
            pileButtons[i]->setEnabled(false);
        }

        QString card = deck[10];
        deckLabel->setText("三轮选择已经完成。");
        statusLabel->setText("揭晓：你记住的牌应该是第11张。");
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
