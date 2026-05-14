#pragma once

#include "gomokugame.h"

#include <QWidget>

class BoardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BoardWidget(QWidget *parent = nullptr);

    void setGame(GomokuGame *game);

signals:
    void cellClicked(int x, int y);
    void hoverChanged(int x, int y, bool valid);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QPointF origin() const;
    double cellSize() const;
    QRectF boardRect() const;

    bool positionToCell(const QPoint &pos, int &x, int &y) const;

    GomokuGame *game_ = nullptr;
    bool hasHover_ = false;
    QPoint hoverCell_{-1, -1};
};
