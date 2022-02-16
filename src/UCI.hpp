#pragma once

#include "MoveGeneration.hpp"
#include "ChessEngine.hpp"
#include "GameState.hpp"
#include "Search.hpp"
#include "Utils.hpp"
#include "Move.hpp"

#include <random>
#include <chrono>
#include <future>
#include <atomic>
#include <chrono>

using namespace std::chrono_literals;

namespace thread {
    static std::future<void> search;
}

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

            if (!searching) {
                if      (cmd == "position"  ) { UCI::SetPosition(Board, tokens);      }
                else if (cmd == "show"      ) { std::cout << Board << std::endl;      }
                else if (cmd == "go"        ) { UCI::Go(Board, tokens);               }
                else if (cmd == "isready"   ) { std::cout << "readyok" << std::endl;  }
                else if (cmd == "uci"       ) { UCI::Init();                          }
                else if (cmd == "ucinewgame") { Board = GameState(STARTING_POSITION); }
                else if (cmd == "quit"      ) { break;                                }
            } else {
                if      (cmd == "stop"      ) { Search::stop = true;                  }
                else if (cmd == "quit"      ) { Search::stop = true; break;           }
            }
            // else if (cmd == "debug") {}
            // else if (cmd == "setoption") {}
            // else if (cmd == "register") {}
            // else if (cmd == "later") {}
            // else if (cmd == "name") {}
            // else if (cmd == "code") {}
            // else if (cmd == "ponderhit") {}
            // else if (cmd == "id") {}
            // else if (cmd == "uciok") {}
            // else if (cmd == "readyok") {}
            // else if (cmd == "bestmove") {}
            // else if (cmd == "copyprotection") {}
            // else if (cmd == "registration") {}
            // else if (cmd == "info") {}
            // else if (cmd == "option") {}
            // else if (cmd == "type") {}
            // else if (cmd == "default") {}
            // else if (cmd == "min") {}
            // else if (cmd == "max") {}
            // else if (cmd == "var") {}
            // else if (cmd == "min") {}
            // else if (cmd == "min") {}
        }
    }

private:
    static inline std::atomic<bool> searching = false;

    static void Go(GameState& Board, std::istringstream& tokens) {

        thread::search = std::async(std::launch::async, [&]() { searching.store(true);
            std::string token; tokens >> token;

            auto depth = 8;
            if (token == "depth")
                depth = (tokens >> token, std::atoi(token.c_str()));

            Search::Init();
            auto started  = std::chrono::steady_clock::now();
            for (int current_depth = 1; current_depth <= depth; ++current_depth) {
                auto score = Search::AlphaBetaNegamax(Board, current_depth);
                auto finished = std::chrono::steady_clock::now();


                auto ms = std::chrono::duration_cast
                         <std::chrono::milliseconds>
                         (finished-started).count();
                auto sec = ms / 1000.00000f;
                std::uint64_t nps = Search::nodes / sec;

                std::stringstream score_str;
                if      (score >  10000) score_str << "mate "  << (CHECKMATE-score)/2;
                else if (score < -10000) score_str << "mate -" << (CHECKMATE+score)/2;
                else                     score_str << "cp "    << score;

                std::cout <<  "info"
                          << " depth " << current_depth
                          << " score " << score_str.str()
                          << " nodes " << Search::nodes
                          << " time "  << ms
                          << " nps "   << nps
                          << " pv "    << PrincipalVariation::ToString()
                          << std::endl;

                if (Search::stop && not (Search::stop = false)) break;
            }

            std::cout << "besthash " << HashTable.GetBestMove(Board)      << std::endl;
            std::cout << "bestmove " << PrincipalVariation::GetBestMove() << std::endl;

            searching.store(false);
        });
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

        auto uci_origin = EnumSquare((uci_move[0] - 'a') + ((uci_move[1] - '0') - 1) * 8);
        auto uci_target = EnumSquare((uci_move[2] - 'a') + ((uci_move[3] - '0') - 1) * 8);
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
                } else if (!(move.flags & 0b1000)) MAKE_MOVE;
                #undef MAKE_MOVE
            }
        } return false;
    }
};
