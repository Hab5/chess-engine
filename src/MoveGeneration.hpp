#pragma once

#include "ChessEngine.hpp"
#include "GameState.hpp"
#include "GetAttack.hpp"
#include "Utils.hpp"
#include "Move.hpp"

#include <iomanip>

using MoveList = std::array<Move, 218>;

class MoveGeneration final {
public:
    template <EnumColor Color>
    static inline auto Run(GameState& Board) noexcept -> std::tuple<MoveList, int> {

        MoveList moves;

        auto iterator = moves.begin();
        PseudoLegal<Color, Pawns  >(Board, iterator);
        PseudoLegal<Color, Knights>(Board, iterator);
        PseudoLegal<Color, Bishops>(Board, iterator);
        PseudoLegal<Color, Rooks  >(Board, iterator);
        PseudoLegal<Color, Queens >(Board, iterator);
        PseudoLegal<Color, King   >(Board, iterator);

        const auto nmoves = std::distance(moves.begin(), iterator);
        return { moves, nmoves };
    }

private:

    template <EnumColor Color, EnumPiece Piece> static inline
    auto PseudoLegal(GameState& Board, MoveList::iterator& Moves) noexcept {
        constexpr auto Allies = Color, Enemies = ~Allies;
        auto set       = (Board[Allies] & Board[Piece  ]);
        auto occupancy = (Board[Allies] | Board[Enemies]);

        while (set) {
        EnumSquare origin = Utils::PopLS1B(set);

        /////////////////////////////////////// PAWNS ////////////////////////////////////////

        if constexpr(Piece == Pawns) {
            constexpr auto StartingRank  = (Allies == White ? Rank_2 : Rank_7);
            constexpr auto PromotionRank = (Allies == White ? Rank_8 : Rank_1);
            constexpr auto Up            = (Allies == White ? North  : South );

            auto attacks = GetAttack<Allies, Piece>::On(origin) & Board[Enemies];
            while (attacks) {
                auto attack = Utils::PopLS1B(attacks);
                *Moves++ = Move::Encode<Piece>(origin, attack, Capture);
                if (attack & PromotionRank) { --Moves;
                    *Moves++ = Move::Encode<Piece>(origin, attack, PromotionKnight | Capture);
                    *Moves++ = Move::Encode<Piece>(origin, attack, PromotionBishop | Capture);
                    *Moves++ = Move::Encode<Piece>(origin, attack, PromotionRook   | Capture);
                    *Moves++ = Move::Encode<Piece>(origin, attack, PromotionQueen  | Capture);
                }
            }

            auto empty = ~occupancy;
            EnumSquare target = origin + Up;
            if (target & empty) {
                *Moves++ = Move::Encode<Piece>(origin, target, Quiet);
                if ((origin & StartingRank) && ((target+Up) & empty))
                    *Moves++ = Move::Encode<Piece>(origin, (target+Up), DoublePush);
                if (target & PromotionRank) { --Moves;
                    *Moves++ = Move::Encode<Piece>(origin, target, PromotionKnight);
                    *Moves++ = Move::Encode<Piece>(origin, target, PromotionBishop);
                    *Moves++ = Move::Encode<Piece>(origin, target, PromotionRook  );
                    *Moves++ = Move::Encode<Piece>(origin, target, PromotionQueen );
                }
            }

            if (Board.en_passant)
                if (GetAttack<Allies, Piece>::On(origin) & Board.en_passant)
                    *Moves++ = Move::Encode<Piece>(origin, Board.en_passant, EnPassant);
        }

        /////////////////////////////////// KNIGHTS / KING ///////////////////////////////////

        if constexpr(Piece == Knights || Piece == King) {
            auto attacks = GetAttack<Piece>::On(origin) & ~Board[Allies];
            while (attacks) {
                auto attack = Utils::PopLS1B(attacks);
                if (attack & ~Board[Enemies])
                     *Moves++ = Move::Encode<Piece>(origin, attack, Quiet);
                else *Moves++ = Move::Encode<Piece>(origin, attack, Capture);
            }

            if constexpr(Piece == King) {
                constexpr auto king = (Allies == White ? e1:e8);

                constexpr auto Kk = (Allies == White ? 0 : 2);
                if (Board.castling_rights[Kk]) {
                    if (!(((king+1) | (king+2)) & occupancy))
                        if (!GameState::InCheck<Allies>(Board, king)
                        &&  !GameState::InCheck<Allies>(Board, king+1)
                        &&  !GameState::InCheck<Allies>(Board, king+2))
                            *Moves++ = Move::Encode<Piece>(origin, king+2, CastleKing);
                }

                constexpr auto Qq = (Allies == White ? 1 : 3);
                if (Board.castling_rights[Qq]) {
                    if (!(((king-1) | (king-2) | (king-3)) & occupancy))
                        if (!GameState::InCheck<Allies>(Board, king)
                        &&  !GameState::InCheck<Allies>(Board, king-1)
                        &&  !GameState::InCheck<Allies>(Board, king-2))
                            *Moves++ = Move::Encode<Piece>(origin, king-2, CastleQueen);
                }
            }
        }

        ////////////////////////////// BISHOPS / ROOKS / QUEEN ///////////////////////////////

        if constexpr(Piece == Bishops || Piece == Rooks || Piece == Queens) {
            auto attacks = Bitboard(0);
            attacks = GetAttack<Piece>::On(origin, occupancy) & ~Board[Allies];
            while (attacks) {
                auto attack = Utils::PopLS1B(attacks);
                if (attack & ~Board[Enemies])
                     *Moves++ = Move::Encode<Piece>(origin, attack, Quiet);
                else *Moves++ = Move::Encode<Piece>(origin, attack, Capture);
            }
        }

        /////////////////////////////////////////////////////////////////////////////////////

        }
    }

     MoveGeneration()=delete;
    ~MoveGeneration()=delete;
};
