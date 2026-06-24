#include "CardWidgets.h"
#include "GuiUtils.h"

#include <QApplication>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QVBoxLayout>
#include <cstring>

#ifdef __APPLE__
#include <AudioToolbox/AudioToolbox.h>
#include <CoreFoundation/CoreFoundation.h>
#endif

namespace {

const int kDealCardIntervalMs = 70;
const int kCardFadeDurationMs = 190;

void playSystemCue(const char* soundPath) {
#ifdef __APPLE__
    static SystemSoundID dealSound = 0;
    static SystemSoundID revealSound = 0;
    static bool dealInitialized = false;
    static bool revealInitialized = false;
    SystemSoundID* soundId = nullptr;
    bool* initialized = nullptr;
    if (std::strcmp(soundPath, "/System/Library/Sounds/Tink.aiff") == 0) {
        soundId = &dealSound;
        initialized = &dealInitialized;
    } else {
        soundId = &revealSound;
        initialized = &revealInitialized;
    }
    if (!*initialized) {
        *initialized = true;
        CFStringRef path = CFStringCreateWithCString(kCFAllocatorDefault,
                                                     soundPath,
                                                     kCFStringEncodingUTF8);
        CFURLRef url = path
            ? CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
                                            path,
                                            kCFURLPOSIXPathStyle,
                                            false)
            : nullptr;
        if (path) {
            CFRelease(path);
        }
        if (url) {
            AudioServicesCreateSystemSoundID(url, soundId);
            CFRelease(url);
        }
    }
    if (*soundId != 0) {
        AudioServicesPlaySystemSound(*soundId);
        return;
    }
#endif
    QApplication::beep();
}

void playDealCue() {
    playSystemCue("/System/Library/Sounds/Tink.aiff");
}

void playRevealCue() {
    playSystemCue("/System/Library/Sounds/Glass.aiff");
}

void animateCardEntry(QWidget* widget, int order, bool playSound = true) {
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(widget);
    effect->setOpacity(0.0);
    widget->setGraphicsEffect(effect);

    QSequentialAnimationGroup* group = new QSequentialAnimationGroup(widget);
    group->addPause(kDealCardIntervalMs * order);

    QPropertyAnimation* soundCue = new QPropertyAnimation(effect, "opacity", group);
    soundCue->setDuration(1);
    soundCue->setStartValue(0.0);
    soundCue->setEndValue(0.0);
    if (playSound) {
        QObject::connect(soundCue, &QPropertyAnimation::finished, []() {
            playDealCue();
        });
    }
    group->addAnimation(soundCue);

    QPropertyAnimation* fade = new QPropertyAnimation(effect, "opacity", group);
    fade->setDuration(kCardFadeDurationMs);
    fade->setStartValue(0.0);
    fade->setEndValue(1.0);
    group->addAnimation(fade);

    QObject::connect(group, &QSequentialAnimationGroup::finished, effect, [effect]() {
        effect->setOpacity(1.0);
    });
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

} // namespace

void playRevealCueSound() {
    playRevealCue();
}

QLabel* makeTextLabel(const QString& text, const QString& objectName, bool wrap) {
    QLabel* label = new QLabel(text);
    label->setObjectName(objectName);
    label->setWordWrap(wrap);
    return label;
}

QPushButton* makeButton(const QString& text, const QString& objectName) {
    QPushButton* button = new QPushButton(text);
    button->setObjectName(objectName);
    button->setMinimumHeight(42);
    button->setCursor(Qt::PointingHandCursor);
    return button;
}

QFrame* makeTablePanel(const QString& objectName) {
    QFrame* panel = new QFrame;
    panel->setObjectName(objectName);
    return panel;
}

QFrame* makeDivider(const QString& objectName) {
    QFrame* divider = new QFrame;
    divider->setObjectName(objectName);
    divider->setFixedHeight(2);
    return divider;
}

QWidget* makeCardWidget(const QString& rawCard, bool compact, bool selectable) {
    QString card = cleanCardText(rawCard);
    QWidget* widget = selectable ? static_cast<QWidget*>(new QPushButton(card))
                                 : static_cast<QWidget*>(new QLabel(card));
    widget->setObjectName(selectable ? "cardButton" : "playingCard");
    widget->setFixedSize(compact ? 48 : 58, compact ? 66 : 78);
    if (card == "??") {
        widget->setStyleSheet(
            "#playingCard, #cardButton {"
            " image: url(:/assets/card_back.png);"
            " background: transparent;"
            " border: none;"
            " color: transparent;"
            " padding: 0px;"
            "}"
            "#cardButton:hover { border: 2px solid #f0cd70; border-radius: 8px; }");
        if (QLabel* label = qobject_cast<QLabel*>(widget)) {
            label->setText("");
        }
        if (QPushButton* button = qobject_cast<QPushButton*>(widget)) {
            button->setText("");
        }
    } else {
        widget->setStyleSheet(QString(
            "#playingCard, #cardButton {"
            " background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #fff8e8, stop:0.58 #f7e7c8, stop:1 #e0c99b);"
            " color: %1;"
            " border: 2px solid #4d3424;"
            " border-radius: 9px;"
            " font-size: %2px;"
            " font-weight: 900;"
            " padding: 2px;"
            "}"
            "#cardButton:hover { border: 3px solid #f0cd70; background: #fffef7; }"
        ).arg(cardColor(card)).arg(compact ? 18 : 20));
    }

    if (QLabel* label = qobject_cast<QLabel*>(widget)) {
        label->setAlignment(Qt::AlignCenter);
    }
    if (QPushButton* button = qobject_cast<QPushButton*>(widget)) {
        button->setFlat(false);
        button->setCursor(Qt::PointingHandCursor);
    }
    return widget;
}

QWidget* makeLargeCardWidget(const QString& rawCard, bool playEntrySound) {
    QString card = cleanCardText(rawCard);
    QLabel* label = new QLabel(card);
    label->setObjectName("largePlayingCard");
    label->setAlignment(Qt::AlignCenter);
    label->setFixedSize(116, 156);
    label->setStyleSheet(QString(
        "#largePlayingCard {"
        " background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #fff8e8, stop:0.58 #f7e7c8, stop:1 #e0c99b);"
        " color: %1;"
        " border: 3px solid #4d3424;"
        " border-radius: 14px;"
        " font-size: 36px;"
        " font-weight: 900;"
        " padding: 4px;"
        "}"
    ).arg(cardColor(card)));
    animateCardEntry(label, 0, playEntrySound);
    return label;
}

void clearLayout(QLayout* layout) {
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

void resetGrid(QGridLayout* layout) {
    clearLayout(layout);
    for (int column = 0; column < 16; ++column) {
        layout->setColumnStretch(column, 0);
    }
}

void renderDeckCards(QGridLayout* layout,
                     const QStringList& cards,
                     bool selectable,
                     QWidget* receiver,
                     std::function<void(int)> clickHandler,
                     bool playSounds) {
    resetGrid(layout);
    const int columns = cards.size() <= 1 ? 1 : (cards.size() <= 15 ? 5 : 7);
    for (int i = 0; i < cards.size(); ++i) {
        QWidget* widget = makeCardWidget(cards[i], false, selectable);
        if (selectable) {
            QPushButton* button = qobject_cast<QPushButton*>(widget);
            QObject::connect(button, &QPushButton::clicked, receiver ? receiver : button,
                             [clickHandler, i]() {
                                 if (clickHandler) {
                                     clickHandler(i);
                                 }
                             });
        }
        layout->addWidget(widget, i / columns, i % columns, Qt::AlignCenter);
        animateCardEntry(widget, i, playSounds);
    }
    for (int column = 0; column < columns; ++column) {
        layout->setColumnStretch(column, 1);
    }
}

PilePanel::PilePanel(int index, QWidget* parent)
    : QFrame(parent), titleLabel(nullptr), hintLabel(nullptr), cardsLayout(nullptr) {
    setObjectName("pilePanel");
    setCursor(Qt::PointingHandCursor);
    setMinimumSize(230, 238);

    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(16, 14, 16, 14);
    root->setSpacing(9);

    titleLabel = makeTextLabel(QString("牌堆 %1").arg(index + 1), "pileTitle");
    root->addWidget(titleLabel);

    cardsLayout = new QGridLayout;
    cardsLayout->setHorizontalSpacing(8);
    cardsLayout->setVerticalSpacing(8);
    cardsLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    root->addLayout(cardsLayout, 1);

    hintLabel = makeTextLabel("点击选择这一堆", "pileHint", true);
    root->addWidget(hintLabel);
}

void PilePanel::setCards(const QStringList& cards, int animationOffset, bool playSounds) {
    resetGrid(cardsLayout);
    const int columns = cards.size() > 7 ? 3 : 4;
    for (int i = 0; i < cards.size(); ++i) {
        QWidget* cardWidget = makeCardWidget(cards[i], true);
        cardsLayout->addWidget(cardWidget, i / columns, i % columns);
        animateCardEntry(cardWidget, animationOffset + i, playSounds);
    }
}

void PilePanel::setHint(const QString& text) {
    hintLabel->setText(text);
}

void PilePanel::setClickHandler(std::function<void()> handler) {
    clickHandler = handler;
}

void PilePanel::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && isEnabled() && clickHandler) {
        clickHandler();
    }
    QFrame::mousePressEvent(event);
}
