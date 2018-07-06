/*
  Stockfish, a UCI chess playing engine derived from Glaurung 2.1
  Copyright (C) 2004-2008 Tord Romstad (Glaurung author)
  Copyright (C) 2008-2014 Marco Costalba, Joona Kiiski, Tord Romstad

  Stockfish is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Stockfish is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <cassert>
#include <iomanip>
#include <sstream>
#include <stack>

#include "movegen.h"
#include "notation.h"
#include "position.h"
#include "thread.h"

using namespace std;

static const char* PieceToChar[COLOR_NB] = { " PNBRQK", " pnbrqk" };

/// move_to_san() takes a position and a legal Move as input and returns its
/// short algebraic notation representation.

const string move_to_san(const Position& pos, Move m) {

    if (m == MOVE_NONE)
        return "(none)";

    if (m == MOVE_NULL)
        return "(null)";


//   assert(MoveList<LEGAL>(pos).contains(m));

    Bitboard others, b;
    string san;
    Color us = pos.side_to_move();
    Square from = from_sq(m);
    Square to = to_sq(m);
    Piece pc = pos.piece_on(from);
    PieceType pt = type_of(pc);

    if (type_of(m) == CASTLING)
        san = to > from ? "O-O" : "O-O-O";
    else
    {
        if (pt != PAWN)
        {
            san = PieceToChar[WHITE][pt]; // Upper case

            // A disambiguation occurs if we have more then one piece of type 'pt'
            // that can reach 'to' with a legal move.
            others = b = (pos.attacks_from(pt, to) & pos.pieces(us, pt)) ^ from;

            while (b)
            {
                Square s = pop_lsb(&b);
                if (!pos.legal(make_move(s, to)))
                    others ^= s;
            }

            if (!others)
            {   /* Disambiguation is not needed */
            }

            else if (!(others & file_bb(from)))
                san += to_char(file_of(from));

            else if (!(others & rank_bb(from)))
                san += to_char(rank_of(from));

            else
                san += to_string(from);
        }
        else if (pos.capture(m))
            san = to_char(file_of(from));

        if (pos.capture(m))
            san += 'x';

        san += to_string(to);

        if (type_of(m) == PROMOTION)
            san += string("=") + PieceToChar[WHITE][promotion_type(m)];
    }

    bool bGc = pos.gives_check(m);
    if (bGc)
    {
        san += "+";
        //StateInfo st;
        //pos.do_move(m, st, bGc);
        //san += MoveList<LEGAL>(pos).size() ? "+" : "#";
        //pos.undo_move(m);
    }

    return san;
}


/// pretty_pv() formats human-readable search information, typically to be
/// appended to the search log file. It uses the two helpers below to pretty
/// format the time and score respectively.

static string format(int64_t msecs) {

    const int MSecMinute = 1000 * 60;
    const int MSecHour   = 1000 * 60 * 60;

    int64_t hours   =   msecs / MSecHour;
    int64_t minutes =  (msecs % MSecHour) / MSecMinute;
    int64_t seconds = ((msecs % MSecHour) % MSecMinute) / 1000;

    stringstream ss;

    if (hours)
        ss << hours << ':';

    ss << setfill('0') << setw(2) << minutes << ':' << setw(2) << seconds;

    return ss.str();
}

static string format(Value v) {

    stringstream ss;

    if (v >= VALUE_MATE_IN_MAX_PLY)
        ss << "#" << (VALUE_MATE - v + 1) / 2;

    else if (v <= VALUE_MATED_IN_MAX_PLY)
        ss << "-#" << (VALUE_MATE + v) / 2;

    else
        ss << setprecision(2) << fixed << showpos << double(v) / PawnValueEg;

    return ss.str();
}

string pretty_pv( int depth, Value value, int64_t msecs) {

    const uint64_t K = 1000;
    const uint64_t M = 1000000;

    std::stack<StateInfo> st;
    string san, str, padding;
    stringstream ss;

    ss << setw(2) << depth << setw(8) << format(value) << setw(8) << format(msecs);

    if (Threads.nodes_searched() < M)
        ss << setw(8) << Threads.nodes_searched() / 1 << "  ";

    else if (Threads.nodes_searched() < K * M)
        ss << setw(7) << Threads.nodes_searched() / K << "K  ";

    else
        ss << setw(7) << Threads.nodes_searched() / M << "M  ";

    str = ss.str();

    return str;
}

double logistic_mobility_score(size_t mobility)
{
    switch (mobility)
    {
#ifdef old
    case 20:
        return 0.750001;
    case 21:
        return 0.750002;
    case 22:
        return 0.750005;
    case 23:
        return 0.750013;
    case 24:
        return 0.750034;
    case 25:
        return 0.750093;
    case 26:
        return 0.750252;
    case 27:
        return 0.750683;
    case 28:
        return 0.751854;
    case 29:
        return 0.75502;
    case 30:
        return 0.76349;
    case 31:
        return 0.785569;
    case 32:
        return 0.839402;
    case 33:
        return 0.951706;
    case 34:
        return 1.125;
    case 35:
        return 1.29829;
    case 36:
        return 1.4106;
    case 37:
        return 1.46443;
    case 38:
        return 1.48651;
    case 39:
        return 1.49498;
    case 40:
        return 1.49815;
    case 41:
        return 1.49932;
    case 42:
        return 1.49975;
    case 43:
        return 1.49991;
    case 44:
        return 1.49997;
    case 45:
        return 1.49999;
#else
    case 0:
        return 0.350198;
        break;
    case 1:
        return 0.350918;
        break;
    case 2:
        return 0.353303;
        break;
    case 3:
        return 0.359504;
        break;
    case 4:
        return 0.37254;
        break;
    case 5:
        return 0.395363;
        break;
    case 6:
        return 0.429588;
        break;
    case 7:
        return 0.474697;
        break;
    case 8:
        return 0.528165;
        break;
    case 9:
        return 0.586295;
        break;
    case 10:
        return 0.64524;
        break;
    case 11:
        return 0.701766;
        break;
    case 12:
        return 0.753616;
        break;
    case 13:
        return 0.799533;
        break;
    case 14:
        return 0.839079;
        break;
    case 15:
        return 0.8724;
        break;
    case 16:
        return 0.899999;
        break;
    case 17:
        return 0.922552;
        break;
    case 18:
        return 0.940788;
        break;
    case 19:
        return 0.955412;
        break;
    case 20:
        return 0.967065;
        break;
    case 21:
        return 0.976302;
        break;
    case 22:
        return 0.983597;
        break;
    case 23:
        return 0.989341;
        break;
    case 24:
        return 0.993851;
        break;
    case 25:
        return 0.997387;
        break;
    case 26:
        return 1.00016;
        break;
    case 27:
        return 1.00232;
        break;
    case 28:
        return 1.00401;
        break;
    case 29:
        return 1.00533;
        break;
    case 30:
        return 1.00636;
        break;
    case 31:
        return 1.00716;
        break;
    case 32:
        return 1.00779;
        break;
    case 33:
        return 1.00828;
        break;
    case 34:
        return 1.00866;
        break;
    case 35:
        return 1.00896;
        break;
    case 36:
        return 1.00919;
        break;
    case 37:
        return 1.00937;
        break;
    case 38:
        return 1.00951;
        break;
    case 39:
        return 1.00962;
        break;
    case 40:
        return 1.0097;
        break;
    case 41:
        return 1.00977;
        break;
    case 42:
        return 1.00982;
        break;
    case 43:
        return 1.00986;
        break;
    case 44:
        return 1.00989;
        break;
    case 45:
        return 1.00991;
        break;
    case 46:
        return 1.00993;
        break;
    case 47:
        return 1.00995;
        break;
    case 48:
        return 1.00996;
        break;
    case 49:
        return 1.00997;
        break;
    case 50:
        return 1.00998;
        break;
    case 51:
        return 1.00998;
        break;
    case 52:
        return 1.00999;
        break;
    case 53:
        return 1.00999;
        break;
    case 54:
        return 1.00999;
        break;
    case 55:
        return 1.00999;
        break;
    case 56:
        return 1.00999;
        break;

    default:
        return 1.01;
        break;

#endif
    }
    if (mobility < 20) return 0.75;
    if (mobility > 45) return 1.5;
    assert(false);
    return 0;
}
