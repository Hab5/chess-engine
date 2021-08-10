#pragma once

#include "ChessEngine.hpp"

template <EnumPiece Piece>
constexpr std::array<Bitboard, 64> Magics {};

template <>
constexpr inline std::array<Bitboard, 64> Magics<Rooks> {
    0xd800010804000a0ULL,  0x44004403000a000ULL,  0x3000904c0102000ULL,  0x2080044800100082ULL,
    0x2000e0008046010ULL,  0x100050004008208ULL,  0x1100209100220004ULL, 0xa08003a080144100ULL,
    0x4244800440002084ULL, 0x401004008830020ULL,  0x2002001042042080ULL, 0x2180801000845800ULL,
    0x50800800140280ULL,   0x25000401002268ULL,   0x201010402000100ULL,  0x445000100009062ULL,
    0x104060800080c000ULL, 0x90404c4000a01002ULL, 0x1010008010846000ULL, 0x1030018010280280ULL,
    0x280800c000801ULL,    0x2008004008006ULL,    0x4000040002100108ULL, 0x61b220000942441ULL,
    0x2000401080002280ULL, 0x20004004d008ULL,     0x9102002200304080ULL, 0x2421080080801000ULL,
    0x211009100080124ULL,  0x4102000200043810ULL, 0x18c100010002000cULL, 0x20104200288304ULL,
    0x400020800080ULL,     0x810092000c00040ULL,  0x2002048012004020ULL, 0x300809000800800ULL,
    0x80448c0080800800ULL, 0x111820080800400ULL,  0x8a080104000210ULL,   0x8a42000514ULL,
    0x4008204000898001ULL, 0x6020201001404002ULL, 0x804204120020ULL,     0x700101a0110038ULL,
    0x1200340008008080ULL, 0x812001004020088ULL,  0xb02a08040001ULL,     0x20040880d20023ULL,
    0x380402080110300ULL,  0x300088c000290100ULL, 0x2120100280200080ULL, 0x828821000c100100ULL,
    0x10862c0108008080ULL, 0x8010040002008080ULL, 0x5008003001a0080ULL,  0x800100004180ULL,
    0x2008010204102ULL,    0x40002104108041ULL,   0x4010800a0022ULL,     0x1000e089000411ULL,
    0x4100040800307bULL,   0x2803000208840041ULL, 0x5000094800900a04ULL, 0x130000220c4081ULL
};

template <>
constexpr inline std::array<Bitboard, 64> Magics<Bishops> {
    0x2a120802119200ULL,   0x1844044092020080ULL, 0x20a2081300a40400ULL, 0x88184900000000ULL,
    0x401110404082000bULL, 0x1126900420218000ULL, 0x10883412211246ULL,   0x1008090080a04ULL,
    0x9400ec2048051100ULL, 0x2000044410a20200ULL, 0x402210301020020ULL,  0x1001510502000000ULL,
    0x844c1420300090ULL,   0x40880c400010ULL,     0xcc24404044018ULL,    0x6010508221a02ULL,
    0x808a084088082080ULL, 0x1200003a4010200ULL,  0x188002c0868a201ULL,  0x40140c4008000ULL,
    0x862400a00088ULL,     0xc81011080c54000ULL,  0x2000900500901020ULL, 0xa00600820882ULL,
    0x20162020040400ULL,   0x80105030021c3501ULL, 0x88081001cc10ULL,     0x441040001c40080ULL,
    0x10030010a00804ULL,   0xc10490002008200ULL,  0x20092060c011c88ULL,  0x8091002001041900ULL,
    0x92202024b22e08ULL,   0x80020a8206200804ULL, 0x120104800100b80ULL,  0x8200404800408200ULL,
    0x48c0024100081100ULL, 0x101000c200804500ULL, 0x292046400010084ULL,  0x800a009020120204ULL,
    0x6001012021411083ULL, 0x205440414006000ULL,  0x854840401005a00ULL,  0xa01100a800ULL,
    0x210122000400ULL,     0x41101010004408a0ULL, 0x108a1184002100ULL,   0x100c088881084a08ULL,
    0x9155082230040082ULL, 0xe40805128200000ULL,  0x51042184100000ULL,   0x230001138c040004ULL,
    0x4600000821010086ULL, 0x16141c28220010ULL,   0x24820840c504008ULL,  0x248010800810120ULL,
    0x3801620300884000ULL, 0x622004046501042ULL,  0x200010100411000ULL,  0x1260200108411090ULL,
    0x20a0000808302400ULL, 0x1000404084210ULL,    0x448203c10060062ULL,  0xc822100a012105ULL
};
