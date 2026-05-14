#pragma once

#include <QElapsedTimer>
#include <QPoint>
#include <QVector>

#include <array>

class GomokuGame
{
public:
    static constexpr int BoardSize = 15;
    static constexpr qint64 DefaultTurnMs = 5 * 60 * 1000;

    enum class Player {
        None = 0,
        Black,
        White
    };

    enum class GameStatus {
        Playing = 0,
        BlackWin,
        WhiteWin,
        Draw
    };

    enum class EndReason {
        None = 0,
        FiveInRow,
        Draw,
        Timeout,
        ForbiddenLongLine
    };

    struct Move {
        QPoint pos;
        Player player = Player::None;
        qint64 consumedMs = 0;
    };

    GomokuGame();

    void newGame();

    bool placeStone(int x, int y);
    bool undo();
    void tick();

    Player at(int x, int y) const;
    Player currentPlayer() const;
    GameStatus status() const;
    EndReason endReason() const;
    Player loser() const;

    QPoint lastMove() const;
    int moveCount() const;

    qint64 remainingMs(Player player) const;
    qint64 initialMs() const;

private:
    using Row = std::array<Player, BoardSize>;
    using Board = std::array<Row, BoardSize>;

    bool isInside(int x, int y) const;
    int countLine(int x, int y, int dx, int dy, Player player) const;
    void updateStatusAfterMove(int x, int y, Player player);

    Board board_{};
    QVector<Move> moves_;
    QPoint lastMove_{-1, -1};

    Player currentPlayer_ = Player::Black;
    GameStatus status_ = GameStatus::Playing;
    EndReason endReason_ = EndReason::None;
    Player loser_ = Player::None;

    qint64 initialMs_ = DefaultTurnMs;
    qint64 blackRemainingMs_ = DefaultTurnMs;
    qint64 whiteRemainingMs_ = DefaultTurnMs;
    QElapsedTimer turnTimer_;
};
