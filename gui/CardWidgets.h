#ifndef CARD_WIDGETS_H
#define CARD_WIDGETS_H

#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QString>
#include <QStringList>
#include <QWidget>
#include <functional>

QLabel* makeTextLabel(const QString& text, const QString& objectName, bool wrap = false);
QPushButton* makeButton(const QString& text, const QString& objectName = "secondaryButton");
QFrame* makeTablePanel(const QString& objectName = "tablePanel");
QFrame* makeDivider(const QString& objectName = "goldDivider");
QWidget* makeCardWidget(const QString& rawCard, bool compact = false, bool selectable = false);
QWidget* makeLargeCardWidget(const QString& rawCard, bool playEntrySound = false);
void playRevealCueSound();
void clearLayout(QLayout* layout);
void resetGrid(QGridLayout* layout);
void renderDeckCards(QGridLayout* layout,
                     const QStringList& cards,
                     bool selectable,
                     QWidget* receiver,
                     std::function<void(int)> clickHandler,
                     bool playSounds = true);

class PilePanel : public QFrame {
private:
    QLabel* titleLabel;
    QLabel* hintLabel;
    QGridLayout* cardsLayout;
    std::function<void()> clickHandler;

public:
    explicit PilePanel(int index, QWidget* parent = nullptr);

    void setCards(const QStringList& cards, int animationOffset = 0, bool playSounds = true);
    void setHint(const QString& text);
    void setClickHandler(std::function<void()> handler);

protected:
    void mousePressEvent(QMouseEvent* event) override;
};

#endif // CARD_WIDGETS_H
