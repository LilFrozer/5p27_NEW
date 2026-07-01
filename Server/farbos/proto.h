
#pragma once

#include <cstdint>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>

using u32 = unsigned;
using u16 = uint16_t;
using u8 = uint8_t;

namespace proto_farbos {

const u16 CNT_SUMMATOR_LINES = 20;

struct farbos_header
{
    u32 dk : 20;
    u32 tk : 4;
    u32 prmbl : 8;

    u32 snp;

    u32 : 16;
    u32 marker: 8;
    u32 : 8;

    u32 crc : 16;
    u32 py : 16;
};

struct frame_upr_css
{
    u32 : 22;
    u32 nkch : 6;
    u32 : 4;

    u32 pr_zi : 1;
    u32 : 1;
    u32 pr_ps : 1;
    u32 : 1;
    u32 pr_nlchm : 1;
    u32 : 27;

    u32 res1{0};

    u32 pr_rk2 : 1;
    u32 : 22;
    u32 ki : 5;
    u32 zero : 3;
    u32 pr_pk_dsgp : 1;

    u32 res2{0};

    u32 : 1;
    u32 kns : 7;
    u32 : 24;

    u32 res3{0};
    u32 res4{0};
    u32 res5{0};
    u32 res6{0};
    u32 res7{0};
    u32 res8{0};
};

struct Koef
{
    u32 kod_per_lych : 5;
    u32 : 3;
    u32 koef_1kor_priem : 12;
    u32 koef_2kor_priem : 12;
};

struct SinCos
{
    // 1,2,3,4,dop1,dop2
    int sincos[6]{};
};

struct frame_upr_sum
{
    u32 ump : 5;
    u32 : 2;
    u32 repack : 3;
    u32 pr_stt : 1;
    u32 pr_dst : 1;
    u32 C1Imit : 1;
    u32 : 9;
    u32 nkch : 6;
    u32 pp : 4;

    u32 pr_zi : 1;
    u32 pr_fdk : 1;
    u32 pr_ps : 1;
    u32 pr_att : 1;
    u32 pr_ns : 1;
    u32 pr_rsvr : 1;
    u32 pr_akp : 1;
    u32 pr_shp : 1;
    u32 dzi : 10;
    u32 vvi : 14;

    u32 stUz : 1;
    u32 ampVH : 3;
    u32 rdd : 14;
    u32 kd : 14;

    u32 pr_rk2 : 1;
    u32 pr_onCi : 1;
    u32 pr_afr : 1;
    u32 numSum : 4;
    u32 pr_4 : 1;
    u32 pr_5 : 1;
    u32 pr_3 : 1;
    u32 kd_ashp : 8;
    u32 ki_blk : 5;
    u32 ki : 8;
    u32 fsgp : 1;

    u32 tho : 26;
    u32 kod_error : 6;

    u32 pr_vob : 1;
    u32 kns : 7;
    u32 mask_kdo_off : 24;

    u32 lVum : 8;
    u32 rVum : 8;
    u32 pum : 8;
    u32 mode_pum : 1;
    u32 ni_arch : 5;
    u32 : 1;
    u32 pr_avch : 1;

    u32 rpp : 4;
    u32 kod_fv : 5;
    u32 : 8;
    u32 : 9;
    u32 front_vum_off : 1;
    u32 : 5;

    u32 : 16;
    u32 dps : 12;
    u32 pr_on_dps : 1;
    u32 : 3;

    u32 snp;

    u32 res1{0};
    u32 res2{0};

    u32 maskC1_sum : 24;
    u32 valueC1 : 4;
    u32 res3 : 4;

    u32 maskC1_kdo : 12;
    u32 res4 : 20;

    u32 verKdo : 1;
    u32 dataC1 : 1;
    u32 pr_nfdk : 1;
    u32 fdk_noise : 1;
    u32 res5 : 28;

    u32 res6{0};

    Koef koefCor[CNT_SUMMATOR_LINES]{};
    SinCos sinCos[CNT_SUMMATOR_LINES]{};
};

static std::string extract14BytesToHex(unsigned long* pdata, int byteOffset) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');

    // Работаем по 4 байта (unsigned long), переставляя байты
    u8* raw = reinterpret_cast<u8*>(pdata) + byteOffset;

    // Обрабатываем 3 полных слова (12 байт) + 2 байта от 4-го слова
    for(int word{};word<4;++word) {
        int bytesInWord = (word < 3) ? 4 : 2; // последнее слово — только 2 байта
        for (int b{bytesInWord - 1};b>=0;--b) {
            // реверс байт внутри слова
            ss << std::setw(2) << static_cast<int>(raw[word * 4 + b]);
        }
    }

    return ss.str();
}

static std::vector<u8> hexToBytes(const std::string& hex) {
    std::vector<u8> bytes;
    for(size_t i{};i<hex.length();i+=2) {
        std::string byteString = hex.substr(i, 2);
        u8 byte = static_cast<u8>(std::stoi(byteString, nullptr, 16));
        bytes.push_back(byte);
    }
    return bytes;
}

static u16 Calc_CRC(std::vector<u8> &bytes) {
    u16 crc = 0, poly = 0x1021;
    for(size_t i{};i<bytes.size();++i) {
        crc ^= bytes[i] << 8;
        for(size_t j{};j<8;++j) {
            if(crc & 0x8000)
                crc = (crc << 1) ^ poly;
            else
                crc <<= 1;
            crc &= 0xFFFF; // Ограничение до 16 бит
        }
    }
    return crc;
}

}
