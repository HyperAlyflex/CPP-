#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->boardWidget->setGame(&game_);

    connect(ui->boardWidget, &BoardWidget::cellClicked, this, &MainWindow::onCellClicked);
    connect(ui->boardWidget, &BoardWidget::hoverChanged, this, &MainWindow::onHoverChanged);
    connect(ui->newGameButton, &QPushButton::clicked, this, &MainWindow::onNewGameClicked);
    connect(ui->undoButton, &QPushButton::clicked, this, &MainWindow::onUndoClicked);
    connect(ui->exitButton, &QPushButton::clicked, this, &MainWindow::onExitClicked);

    uiTimer_.setInterval(200);
    connect(&uiTimer_, &QTimer::timeout, this, &MainWindow::refreshUi);
    uiTimer_.start();

    refreshUi();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onCellClicked(int x, int y)
{
    if (!game_.placeStone(x, y)) {
        ui->boardWidget->update();
        refreshUi();
        return;
    }
    ui->boardWidget->update();
    refreshUi();
}

void MainWindow::onHoverChanged(int x, int y, bool valid)
{
    if (!valid) {
        ui->hoverLabel->setText(QStringLiteral("预落子：-"));
        return;
    }
    if (game_.status() != GomokuGame::GameStatus::Playing) {
        ui->hoverLabel->setText(QStringLiteral("预落子：-"));
        return;
    }
    if (game_.at(x, y) != GomokuGame::Player::None) {
        ui->hoverLabel->setText(QStringLiteral("预落子：-"));
        return;
    }

    ui->hoverLabel->setText(QStringLiteral("预落子：(%1, %2)").arg(x + 1).arg(y + 1));
}

void MainWindow::onNewGameClicked()
{
    game_.newGame();
    gameOverShown_ = false;
    ui->boardWidget->update();
    refreshUi();
}

void MainWindow::onUndoClicked()
{
    if (!game_.undo()) {
        return;
    }
    gameOverShown_ = false;
    ui->boardWidget->update();
    refreshUi();
}

void MainWindow::onExitClicked()
{
    close();
}

void MainWindow::refreshUi()
{
    game_.tick();

    ui->currentPlayerLabel->setText(playerText(game_.currentPlayer()));

    ui->blackTimeLabel->setText(QStringLiteral("黑方剩余：%1").arg(formatTime(game_.remainingMs(GomokuGame::Player::Black))));
    ui->whiteTimeLabel->setText(QStringLiteral("白方剩余：%1").arg(formatTime(game_.remainingMs(GomokuGame::Player::White))));

    updateStatusText();
    updateButtons();
    showGameOverIfNeeded(game_.status());
}

QString MainWindow::playerText(GomokuGame::Player p) const
{
    if (p == GomokuGame::Player::Black) {
        return QStringLiteral("黑");
    }
    if (p == GomokuGame::Player::White) {
        return QStringLiteral("白");
    }
    return QStringLiteral("-");
}

QString MainWindow::formatTime(qint64 ms) const
{
    qint64 totalSeconds = ms / 1000;
    qint64 minutes = totalSeconds / 60;
    qint64 seconds = totalSeconds % 60;
    return QStringLiteral("%1:%2")
        .arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(seconds, 2, 10, QLatin1Char('0'));
}

void MainWindow::updateStatusText()
{
    QString text = QStringLiteral("状态：");
    switch (game_.status()) {
    case GomokuGame::GameStatus::Playing:
        text += QStringLiteral("进行中");
        break;
    case GomokuGame::GameStatus::BlackWin:
        text += QStringLiteral("黑胜");
        break;
    case GomokuGame::GameStatus::WhiteWin:
        text += QStringLiteral("白胜");
        break;
    case GomokuGame::GameStatus::Draw:
        text += QStringLiteral("平局");
        break;
    }

    if (game_.endReason() == GomokuGame::EndReason::Timeout) {
        if (game_.loser() == GomokuGame::Player::Black) {
            text += QStringLiteral("（黑方超时）");
        } else if (game_.loser() == GomokuGame::Player::White) {
            text += QStringLiteral("（白方超时）");
        }
    } else if (game_.endReason() == GomokuGame::EndReason::ForbiddenLongLine) {
        text += QStringLiteral("（黑棋长连禁手）");
    }

    ui->statusLabel->setText(text);
}

void MainWindow::updateButtons()
{
    ui->undoButton->setEnabled(game_.moveCount() > 0);
}

void MainWindow::showGameOverIfNeeded(GomokuGame::GameStatus status)
{
    if (status == GomokuGame::GameStatus::Playing) {
        return;
    }
    if (gameOverShown_) {
        return;
    }

    QString title = QStringLiteral("对局结束");
    QString msg;

    if (status == GomokuGame::GameStatus::Draw) {
        msg = QStringLiteral("平局！");
    } else if (game_.endReason() == GomokuGame::EndReason::Timeout) {
        if (game_.loser() == GomokuGame::Player::Black) {
            msg = QStringLiteral("黑方超时，白方胜利！");
        } else {
            msg = QStringLiteral("白方超时，黑方胜利！");
        }
    } else if (game_.endReason() == GomokuGame::EndReason::ForbiddenLongLine) {
        msg = QStringLiteral("黑棋长连（>=6）为禁手，白方胜利！");
    } else if (status == GomokuGame::GameStatus::BlackWin) {
        msg = QStringLiteral("黑方胜利！");
    } else {
        msg = QStringLiteral("白方胜利！");
    }

    gameOverShown_ = true;
    QMessageBox::information(this, title, msg);
}
