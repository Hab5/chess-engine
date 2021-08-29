#pragma once

#include "MoveGeneration.hpp"
#include "ChessEngine.hpp"
#include "GameState.hpp"
#include "Search.hpp"
#include "Utils.hpp"
#include "Move.hpp"

#include <random>
#include <chrono>

class UCI final {
public:

    static void Init() {
        std::cout << "id name chess-engine" << std::endl;
        std::cout << "id name hab"          << std::endl;
        std::cout << "uciok"                << std::endl;
    }

    static auto Hook(GameState& Board) {
        std::string input;
        std::cout.setf(std::ios::unitbuf);
        while (std::getline(std::cin, input)) {
            std::istringstream tokens(input);

            std::string cmd; tokens >> cmd;
            // std::cout << "cmd:" << cmd << std::endl;

            if      (cmd == "position"  ) { UCI::SetPosition(Board, tokens);      }
            else if (cmd == "show"      ) { std::cout << Board << std::endl;      }
            else if (cmd == "go"        ) { UCI::Go(Board, tokens);               }
            else if (cmd == "isready"   ) { std::cout << "readyok" << std::endl;  }
            else if (cmd == "uci"       ) { UCI::Init();                          }
            else if (cmd == "ucinewgame") { Board = GameState(STARTING_POSITION); }
            else if (cmd == "quit"      ) { break;                                }

            // else ()
            // if (cmd == "debug") {}
            // if (cmd == "setoption") {}
            // if (cmd == "register") {}
            // if (cmd == "later") {}
            // if (cmd == "name") {}
            // if (cmd == "code") {}
            // if (cmd == "ponderhit") {}
            // if (cmd == "id") {}
            // if (cmd == "uciok") {}
            // if (cmd == "readyok") {}
            // if (cmd == "bestmove") {}
            // if (cmd == "copyprotection") {}
            // if (cmd == "registration") {}
            // if (cmd == "info") {}
            // if (cmd == "option") {}
            // if (cmd == "type") {}
            // if (cmd == "default") {}
            // if (cmd == "min") {}
            // if (cmd == "max") {}
            // if (cmd == "var") {}
            // if (cmd == "min") {}
            // if (cmd == "min") {}
        }
    }

private:

    static void Go(GameState& Board, std::istringstream& tokens) { // TODO: score mate
        std::string token; tokens >> token;

        auto depth = 8;
        if (token == "depth") depth = (tokens >> token, std::atoi(token.c_str()));


        std::cout << "Iterative Deepening:\n";
        Search::Init();
        for (int deepening = 1; deepening <= depth; ++deepening) {
            auto started  = std::chrono::steady_clock::now();
            auto score = Search::AlphaBetaNegamax(Board, deepening);
            auto finished = std::chrono::steady_clock::now();

            auto ms = std::chrono::duration_cast
                    <std::chrono::milliseconds>
                    (finished-started).count();

            auto sec = ms / 1000.00000f;
            std::uint64_t nps = Search::nodes / sec;

            if (score >  10000) std::cout << "MATE#" << (CHECKMATE-score)/2 << '\n';
            if (score < -10000) std::cout << "MATE#" << (CHECKMATE+score)/2 << '\n';

            std::cout << "info"
                    << " depth "    << deepening
                    << " score cp " << score
                    << " nodes "    << Search::nodes
                    << " time "     << ms
                    << " nps "      << nps
                    << " pv "       << PrincipalVariation::ToString()
                    << std::endl;
        }

        std::cout << "bestmove " << PrincipalVariation::GetBestMove() << std::endl;

        // std::cout << "\nFixed Depth:\n";
        // auto started  = std::chrono::steady_clock::now();
        // auto score = (Search::Init(), Search::AlphaBetaNegamax(Board, depth));
        // auto finished = std::chrono::steady_clock::now();

        // auto ms = std::chrono::duration_cast
        //          <std::chrono::milliseconds>
        //          (finished-started).count();

        // auto sec = ms / 1000.00000f;
        // std::uint64_t nps = Search::nodes / sec;

        // if (score >  10000) std::cout << "MATE#" << (CHECKMATE-score)/2 << '\n';
        // if (score < -10000) std::cout << "MATE#" << (CHECKMATE+score)/2 << '\n';

        // std::cout << "info"
        //           << " depth "    << depth
        //           << " score cp " << score
        //           << " nodes "    << Search::nodes
        //           << " time "     << ms
        //           << " nps "      << nps
        //           << " pv "       << PrincipalVariation::ToString()
        //           << std::endl;

        // std::cout << "bestmove " << PrincipalVariation::GetBestMove() << std::endl;
    }

    static void SetPosition(GameState& Board, std::istringstream& tokens) {
        std::string token; tokens >> token;
        if (token == "startpos")
            Board = (tokens >> token, GameState(STARTING_POSITION));
        else if (token == "fen") {
            std::string fen;
            while (tokens >> token && token != "moves") fen += (token+" ");
            Board = GameState(fen);
        }

        if (token == "moves") {
            while (tokens >> token) {
                if (not (Board.to_play == White ?
                   PlayMove<White>(Board, token) :
                   PlayMove<Black>(Board, token)
                )) break;
            }
        }
    }

    template <EnumColor Color>
    static bool PlayMove(GameState& Board, std::string& uci_move) noexcept {
        std::for_each(uci_move.begin(), uci_move.end(),
            [](char& c) { c = std::tolower(c); });

        if (uci_move.size() < 4)
            return false;

        auto uci_origin    = EnumSquare((uci_move[0] - 'a') + ((uci_move[1] - '0') - 1) * 8);
        auto uci_target    = EnumSquare((uci_move[2] - 'a') + ((uci_move[3] - '0') - 1) * 8);
        auto uci_promotion = uci_move.size() == 5 ? uci_move[4] : 0;

        auto [move_list, nmoves] = MoveGeneration::Run<Color>(Board);

        for (auto move_index = 0; move_index < nmoves; move_index++) {
            auto move = move_list[move_index];
            if (uci_origin == move.origin && uci_target == move.target) {
                #define MAKE_MOVE return Move::Make<Color>(Board, move);
                if (uci_promotion) {
                    auto promotion = move.flags & 0b1011;
                    if (uci_promotion == 'n' && promotion == PromotionKnight) MAKE_MOVE;
                    if (uci_promotion == 'b' && promotion == PromotionBishop) MAKE_MOVE;
                    if (uci_promotion == 'r' && promotion == PromotionRook  ) MAKE_MOVE;
                    if (uci_promotion == 'q' && promotion == PromotionQueen ) MAKE_MOVE;
                } else if (!(move.flags & 0b1000))                            MAKE_MOVE;
                #undef MAKE_MOVE
            }
        } return false;
    }
};
