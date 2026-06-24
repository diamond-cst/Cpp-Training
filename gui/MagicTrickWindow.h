#ifndef MAGIC_TRICK_WINDOW_H
#define MAGIC_TRICK_WINDOW_H

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QWidget>
#include <memory>
#include <string>

class PilePanel;
class QAbstractButton;
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QFrame;
class QGridLayout;
class QHBoxLayout;
class QLabel;
class QLayout;
class QLineEdit;
class QListWidget;
class QPlainTextEdit;
class QPushButton;
class QSpinBox;
class QStackedWidget;
class QTableWidget;
class QTcpServer;
class QTcpSocket;
class ReplayManager;
class ThreePileCardTrick;

class MagicTrickWindow : public QWidget {
private:
    enum Page {
        MenuPage,
        SetupPage,
        LoadPage,
        GamePage,
        LeaderboardPage,
        ReplayPage,
        HelpPage,
        NetworkPage
    };

    QStackedWidget* stack;
    QLabel* menuStatusLabel;
    QLabel* menuReplayLabel;

    QLineEdit* playerNameEdit;
    QComboBox* trickTypeCombo;
    QComboBox* deckSizeCombo;
    QButtonGroup* modeGroup;
    QAbstractButton* numericCardsCheck;
    QCheckBox* hideFacesCheck;
    QLineEdit* loadPlayerEdit;
    QListWidget* loadSaveList;
    QPushButton* loadSelectedSaveButton;

    QLabel* gameTitleLabel;
    QLabel* gameStatusLabel;
    QLabel* gameRoundBadge;
    QLabel* deckTitleLabel;
    QGridLayout* deckCardsLayout;
    QPushButton* beginDealButton;
    QWidget* revealConfirmArea;
    QPushButton* revealCorrectButton;
    QPushButton* revealWrongButton;
    QWidget* localPileArea;
    PilePanel* pilePanels[3];
    QWidget* localDeckSection;
    QPushButton* pauseGameButton;
    QPushButton* saveGameButton;
    QPushButton* backToMenuButton;

    QTableWidget* leaderboardTable;
    QListWidget* replayList;
    QPlainTextEdit* replayContent;

    QLabel* networkStatusLabel;
    QLabel* networkRoundBadge;
    QLineEdit* networkHostEdit;
    QSpinBox* networkPortSpin;
    QPushButton* listenButton;
    QPushButton* connectButton;
    QPushButton* disconnectButton;
    QWidget* networkControlsPanel;
    QWidget* networkDeckSection;
    QWidget* networkPileArea;
    QWidget* networkRevealConfirmArea;
    QPushButton* networkRevealCorrectButton;
    QPushButton* networkRevealWrongButton;
    QPushButton* networkReadyButton;
    QLabel* networkDeckTitleLabel;
    QGridLayout* networkDeckLayout;
    PilePanel* networkPilePanels[3];

    std::unique_ptr<ThreePileCardTrick> currentTrick;
    std::unique_ptr<ReplayManager> currentReplay;
    std::string currentPlayerName;

    QTcpServer* networkServer;
    QTcpSocket* networkSocket;
    QByteArray networkBuffer;
    bool networkIsMagician;
    int networkRound;
    int networkPendingChoice;
    QStringList networkDeck;
    QVector<QStringList> networkPiles;
    bool localGamePaused;
    bool localStatusVisibleBeforePause;
    QString localStatusTextBeforePause;

public:
    explicit MagicTrickWindow(QWidget* parent = nullptr);
    ~MagicTrickWindow() override;

private:
    void applyStyle();
    QWidget* makePageShell(QLayout* contentLayout);
    QPushButton* navButton(const QString& text);
    QFrame* makeStatSection(const QString& title, const QString& body);
    QFrame* makeSection(const QString& title, QWidget* body);
    QHBoxLayout* titleRow(const QString& title, const QString& backText);

    QWidget* buildMenuPage();
    QWidget* buildSetupPage();
    QWidget* buildLoadPage();
    QWidget* buildGamePage();
    QWidget* buildLeaderboardPage();
    QWidget* buildReplayPage();
    QWidget* buildHelpPage();
    QWidget* buildNetworkPage();

    void showMenu();
    void refreshMenuStatus();

    void startLocalGame(bool fromLoad);
    void renderLocalGameStart();
    void renderLoadedLocalGame();
    void dealAndRenderLocalPiles();
    void toggleLocalPause();
    void setLocalPaused(bool paused);
    void chooseLocalPile(int pileNumber);
    void showPracticeRevealSelection();
    void revealAudienceResult();
    void finishLocalGame(int selectedIndex, bool audienceConfirmedCorrect);
    void saveCurrentGame();
    void loadLocalGame();
    void loadSelectedLocalGame();
    void updateLeaderboardRecord(const std::string& playerName,
                                 int finalScore,
                                 bool correct,
                                 bool hasResult);

    void showLeaderboard();
    void refreshLeaderboardTable();
    void showReplays();
    void loadReplayContent(const QString& filename);

    void resetNetwork();
    void startNetworkHost();
    void startNetworkClient();
    void setupNetworkSocket();
    void initializeNetworkDeck();
    void dealNetworkRound();
    void chooseNetworkPile(int pileNumber);
    void applyNetworkChoice(int chosenPile);
    void showNetworkRevealSelection();
    void revealNetworkCard(int selectedIndex);
    void readNetworkLines();
    void handleNetworkLine(const QString& line);
    void sendNetworkLine(const QString& line);
    void renderNetworkDeck(const QStringList& cards);
    void renderNetworkPiles(bool enabled, bool playSounds = true);
};

#endif // MAGIC_TRICK_WINDOW_H
