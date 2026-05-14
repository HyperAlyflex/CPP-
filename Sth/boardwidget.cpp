#include "boardwidget.h"

#include <QMouseEvent>
#include <QPainter>

BoardWidget::BoardWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(520, 520);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);
}

void BoardWidget::setGame(GomokuGame *game)
{
    game_ = game;
    update();
}

void BoardWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    QRectF rect = boardRect();
    p.fillRect(rect, QColor(240, 200, 130));

    double cell = cellSize();
    QPointF o = origin();

    p.setPen(QPen(Qt::black, 1));
    for (int i = 0; i < GomokuGame::BoardSize; ++i) {
        double x = o.x() + i * cell;
        double y = o.y() + i * cell;
        p.drawLine(QPointF(x, o.y()), QPointF(x, o.y() + (GomokuGame::BoardSize - 1) * cell));
        p.drawLine(QPointF(o.x(), y), QPointF(o.x() + (GomokuGame::BoardSize - 1) * cell, y));
    }

    if (!game_) {
        return;
    }

    QPoint last = game_->lastMove();
    for (int y = 0; y < GomokuGame::BoardSize; ++y) {
        for (int x = 0; x < GomokuGame::BoardSize; ++x) {
            GomokuGame::Player player = game_->at(x, y);
            if (player == GomokuGame::Player::None) {
                continue;
            }

            QPointF c(o.x() + x * cell, o.y() + y * cell);
            double r = cell * 0.42;
            QRectF stoneRect(c.x() - r, c.y() - r, r * 2, r * 2);

            if (player == GomokuGame::Player::Black) {
                p.setBrush(Qt::black);
                p.setPen(Qt::NoPen);
            } else {
                p.setBrush(Qt::white);
                p.setPen(QPen(Qt::black, 1));
            }
            p.drawEllipse(stoneRect);

            if (last == QPoint(x, y)) {
                p.setBrush(Qt::NoBrush);
                p.setPen(QPen(Qt::red, 2));
                p.drawEllipse(QPointF(c.x(), c.y()), r * 0.22, r * 0.22);
            }
        }
    }

    if (game_->status() == GomokuGame::GameStatus::Playing && hasHover_) {
        int hx = hoverCell_.x();
        int hy = hoverCell_.y();
        if (hx >= 0 && hy >= 0 && hx < GomokuGame::BoardSize && hy < GomokuGame::BoardSize) {
            if (game_->at(hx, hy) == GomokuGame::Player::None) {
                QPointF c(o.x() + hx * cell, o.y() + hy * cell);
                double r = cell * 0.42;
                QRectF stoneRect(c.x() - r, c.y() - r, r * 2, r * 2);

                p.save();
                p.setOpacity(0.35);
                if (game_->currentPlayer() == GomokuGame::Player::Black) {
                    p.setBrush(Qt::black);
                    p.setPen(Qt::NoPen);
                } else {
                    p.setBrush(Qt::white);
                    p.setPen(QPen(Qt::black, 1));
                }
                p.drawEllipse(stoneRect);
                p.restore();
            }
        }
    }
}

void BoardWidget::mousePressEvent(QMouseEvent *event)
{
    if (!game_) {
        return;
    }

    int x = -1;
    int y = -1;
    if (!positionToCell(event->pos(), x, y)) {
        return;
    }
    emit cellClicked(x, y);
}

void BoardWidget::mouseMoveEvent(QMouseEvent *event)
{
    int x = -1;
    int y = -1;
    bool valid = positionToCell(event->pos(), x, y);

    QPoint newCell(x, y);
    bool changed = false;

    if (valid) {
        if (!hasHover_ || hoverCell_ != newCell) {
            hasHover_ = true;
            hoverCell_ = newCell;
            changed = true;
        }
    } else {
        if (hasHover_) {
            hasHover_ = false;
            hoverCell_ = QPoint(-1, -1);
            changed = true;
        }
    }

    if (changed) {
        emit hoverChanged(x, y, valid);
        update();
    }
}

void BoardWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)
    if (hasHover_) {
        hasHover_ = false;
        hoverCell_ = QPoint(-1, -1);
        emit hoverChanged(-1, -1, false);
        update();
    }
}

QPointF BoardWidget::origin() const
{
    QRectF rect = boardRect();
    double cell = cellSize();
    return QPointF(rect.left() + cell, rect.top() + cell);
}

double BoardWidget::cellSize() const
{
    QRectF rect = boardRect();
    int n = GomokuGame::BoardSize - 1;
    double w = rect.width();
    double h = rect.height();
    double cell = qMin(w, h) / (n + 2);
    return cell;
}

QRectF BoardWidget::boardRect() const
{
    double margin = 12;
    QRectF r = rect().adjusted(margin, margin, -margin, -margin);
    double side = qMin(r.width(), r.height());
    QPointF topLeft(r.center().x() - side / 2, r.center().y() - side / 2);
    return QRectF(topLeft, QSizeF(side, side));
}

bool BoardWidget::positionToCell(const QPoint &pos, int &x, int &y) const
{
    QRectF rect = boardRect();
    if (!rect.contains(pos)) {
        return false;
    }

    double cell = cellSize();
    QPointF o = origin();

    double fx = (pos.x() - o.x()) / cell;
    double fy = (pos.y() - o.y()) / cell;

    int ix = qRound(fx);
    int iy = qRound(fy);

    if (ix < 0 || ix >= GomokuGame::BoardSize || iy < 0 || iy >= GomokuGame::BoardSize) {
        return false;
    }

    double cx = o.x() + ix * cell;
    double cy = o.y() + iy * cell;
    double dx = pos.x() - cx;
    double dy = pos.y() - cy;
    double dist2 = dx * dx + dy * dy;
    if (dist2 > (cell * 0.5) * (cell * 0.5)) {
        return false;
    }

    x = ix;
    y = iy;
    return true;
}
