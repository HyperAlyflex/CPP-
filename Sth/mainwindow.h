#pragma once

#include "gomokugame.h"

#include <QMainWindow>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onCellClicked(int x, int y);
    void onHoverChanged(int x, int y, bool valid);
    void onNewGameClicked();
    void onUndoClicked();
    void onExitClicked();
    void refreshUi();

private:
    QString playerText(GomokuGame::Player p) const;
    QString formatTime(qint64 ms) const;
    void updateStatusText();
    void updateButtons();
    void showGameOverIfNeeded(GomokuGame::GameStatus status);

    Ui::MainWindow *ui = nullptr;
    GomokuGame game_;
    QTimer uiTimer_;
    bool gameOverShown_ = false;
};
