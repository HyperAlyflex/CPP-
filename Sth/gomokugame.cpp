#include "gomokugame.h"

#include <QtMath>

GomokuGame::GomokuGame()
{
    newGame();
}

void GomokuGame::newGame()
{
    for (auto &row : board_) {
        row.fill(Player::None);
    }

    moves_.clear();
    lastMove_ = QPoint(-1, -1);
    currentPlayer_ = Player::Black;
    status_ = GameStatus::Playing;
    endReason_ = EndReason::None;
    loser_ = Player::None;
    blackRemainingMs_ = initialMs_;
    whiteRemainingMs_ = initialMs_;
    turnTimer_.restart();
}

bool GomokuGame::placeStone(int x, int y)
{
    if (!isInside(x, y)) {
        return false;
    }
    tick();
    if (board_[y][x] != Player::None) {
        return false;
    }
    if (status_ != GameStatus::Playing) {
        return false;
    }

    qint64 available = (currentPlayer_ == Player::Black) ? blackRemainingMs_ : whiteRemainingMs_;
    qint64 consumed = turnTimer_.elapsed();
    if (consumed > available) {
        if (currentPlayer_ == Player::Black) {
            blackRemainingMs_ = 0;
            status_ = GameStatus::WhiteWin;
        } else {
            whiteRemainingMs_ = 0;
            status_ = GameStatus::BlackWin;
        }
        endReason_ = EndReason::Timeout;
        loser_ = currentPlayer_;
        return false;
    }

    if (currentPlayer_ == Player::Black) {
        blackRemainingMs_ = qMax<qint64>(0, blackRemainingMs_ - consumed);
    } else if (currentPlayer_ == Player::White) {
        whiteRemainingMs_ = qMax<qint64>(0, whiteRemainingMs_ - consumed);
    }

    board_[y][x] = currentPlayer_;
    lastMove_ = QPoint(x, y);
    moves_.push_back(Move{QPoint(x, y), currentPlayer_, consumed});

    updateStatusAfterMove(x, y, currentPlayer_);
    if (status_ == GameStatus::Playing) {
        currentPlayer_ = (currentPlayer_ == Player::Black) ? Player::White : Player::Black;
        turnTimer_.restart();
    }

    return true;
}

bool GomokuGame::undo()
{
    if (moves_.isEmpty()) {
        return false;
    }

    Move last = moves_.back();
    moves_.pop_back();

    int x = last.pos.x();
    int y = last.pos.y();
    if (isInside(x, y)) {
        board_[y][x] = Player::None;
    }

    if (last.player == Player::Black) {
        blackRemainingMs_ += last.consumedMs;
    } else if (last.player == Player::White) {
        whiteRemainingMs_ += last.consumedMs;
    }

    currentPlayer_ = last.player;
    status_ = GameStatus::Playing;
    endReason_ = EndReason::None;
    loser_ = Player::None;
    lastMove_ = moves_.isEmpty() ? QPoint(-1, -1) : moves_.back().pos;
    turnTimer_.restart();

    return true;
}

void GomokuGame::tick()
{
    if (status_ != GameStatus::Playing) {
        return;
    }

    qint64 remain = remainingMs(currentPlayer_);
    if (remain > 0) {
        return;
    }

    if (currentPlayer_ == Player::Black) {
        blackRemainingMs_ = 0;
        status_ = GameStatus::WhiteWin;
    } else {
        whiteRemainingMs_ = 0;
        status_ = GameStatus::BlackWin;
    }
    endReason_ = EndReason::Timeout;
    loser_ = currentPlayer_;
}

GomokuGame::Player GomokuGame::at(int x, int y) const
{
    if (!isInside(x, y)) {
        return Player::None;
    }
    return board_[y][x];
}

GomokuGame::Player GomokuGame::currentPlayer() const
{
    return currentPlayer_;
}

GomokuGame::GameStatus GomokuGame::status() const
{
    return status_;
}

GomokuGame::EndReason GomokuGame::endReason() const
{
    return endReason_;
}

GomokuGame::Player GomokuGame::loser() const
{
    return loser_;
}

QPoint GomokuGame::lastMove() const
{
    return lastMove_;
}

int GomokuGame::moveCount() const
{
    return moves_.size();
}

qint64 GomokuGame::remainingMs(Player player) const
{
    qint64 base = 0;
    if (player == Player::Black) {
        base = blackRemainingMs_;
    } else if (player == Player::White) {
        base = whiteRemainingMs_;
    }

    if (status_ == GameStatus::Playing && currentPlayer_ == player) {
        base = qMax<qint64>(0, base - turnTimer_.elapsed());
    }
    return base;
}

qint64 GomokuGame::initialMs() const
{
    return initialMs_;
}

bool GomokuGame::isInside(int x, int y) const
{
    return x >= 0 && x < BoardSize && y >= 0 && y < BoardSize;
}

int GomokuGame::countLine(int x, int y, int dx, int dy, Player player) const
{
    int cx = x;
    int cy = y;
    int count = 0;
    while (isInside(cx, cy) && at(cx, cy) == player) {
        ++count;
        cx += dx;
        cy += dy;
    }
    return count;
}

void GomokuGame::updateStatusAfterMove(int x, int y, Player player)
{
    const int dirs[4][2] = {
        {1, 0},
        {0, 1},
        {1, 1},
        {1, -1},
    };

    int maxCount = 1;
    for (const auto &d : dirs) {
        int dx = d[0];
        int dy = d[1];
        int count = 1;
        count += countLine(x + dx, y + dy, dx, dy, player);
        count += countLine(x - dx, y - dy, -dx, -dy, player);
        maxCount = qMax(maxCount, count);
    }

    if (player == Player::Black && maxCount >= 6) {
        status_ = GameStatus::WhiteWin;
        endReason_ = EndReason::ForbiddenLongLine;
        loser_ = Player::Black;
        return;
    }
    if (maxCount >= 5) {
        status_ = (player == Player::Black) ? GameStatus::BlackWin : GameStatus::WhiteWin;
        endReason_ = EndReason::FiveInRow;
        loser_ = Player::None;
        return;
    }

    if (moves_.size() >= BoardSize * BoardSize) {
        status_ = GameStatus::Draw;
        endReason_ = EndReason::Draw;
        loser_ = Player::None;
    }
}
