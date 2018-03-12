//
// Created by Oleksandr Deundiak on 3/5/18.
//

#include <pythia.h>
#include <pythia_init.h>
#include "unity.h"

static const char deblinded_hex[769] = "0687AC42F1B4B1C1B8427267041F6F723B57845FF537F1F7290A1897FAA9767C175CD7F612BA53DEF7B2A2DEA93B55580782D8E1A4FD00231D642ABF792A9AEB870C258CA645E8C719EBFBF96F8713EB9D118A944665C7CE475B8C0EA7AA5B3E0C2C7BC16439DB5ADB730AAD872404EBBB947278E27CD1C0358CF410E97CE460738D778D6C7C6A9CA055296B91C4CBDB0C2FC0F4C2933B82FB53F742409D8B9F819A8436993164FA721AA69E626CF52AB71FE5213EF7B0CB1D1B742AE6000E740716929E7A00A5855D1556208215F8793D288D089370CB8A67C18DACFF0C63706DD61A0D8F09CBBB0C12E64F133640CB1239F36AE48DDC72CFCDACA6F5383D8D4BDCDCAA8C13EE809D4FA850C76A81965916AFDE6CDB8E4BD41EADAC9D91E084161D917B6A9268C6A991A217ED0F4E75738F53607FA23E20D8B184A4DDAC3F36ABB4248B900ED9DCD320FDDCD943151E0C6F2C509364C02F401CB67545E3F730FF7DD31AF3E729ADAB669BDF09F65EBC5114FA35ECE725AA9658960F361234AD";
static const uint8_t password[9] = "password";
static const uint8_t w[11] = "virgil.com";
static const uint8_t t[6] = "alice";
static const uint8_t msk[14] = "master secret";
static const uint8_t ssk[14] = "server secret";

void test0_Config() {
    conf_print();
}

void simple_proto(gt_t deblinded) {
    ep_t blinded;
    ep_new(blinded);
    bn_t rInv;
    bn_new(rInv);
    pythia_blind(blinded, rInv, password, 8);

    gt_t y;
    gt_new(y);

    bn_t kw;
    bn_new(kw);

    ep2_t tTilde;
    ep2_new(tTilde);

    pythia_eval(y, kw, tTilde, w, 10, t, 5, blinded, msk, 13, ssk, 13);

    pythia_deblind(deblinded, y, rInv);

    uint8_t *bin = calloc(512, sizeof(uint8_t));
    bn_write_bin(bin, 512, rInv);

    ep_free(blinded);
    bn_free(rInv);
    gt_free(y);
    bn_free(kw);
    ep2_free(tTilde);
}

void test1_SimpleProto() {
    int status_code = pythia_init();
    TEST_ASSERT_EQUAL_INT(status_code, 0);

    gt_t deblinded1;
    gt_new(deblinded1);


    uint8_t deblinded_bin[384];
    const char *pos = deblinded_hex;
    for (size_t count = 0; count < 384; count++) {
        sscanf(pos, "%2hhx", &deblinded_bin[count]);
        pos += 2;
    }

    gt_read_bin(deblinded1, deblinded_bin, 384);

    for (int i = 0; i < 10; i++) {
        gt_t deblinded2;
        gt_new(deblinded2);
        simple_proto(deblinded2);

        TEST_ASSERT_EQUAL_INT(gt_cmp(deblinded1, deblinded2), CMP_EQ);

        gt_free(deblinded2);
    }

    gt_free(deblinded1);
}


void test2_FullProto() {
    for (int i = 0; i < 100; i++) {
        const uint8_t password[9] = "password";
        const uint8_t w[11] = "virgil.com";
        const uint8_t t[6] = "alice";
        const uint8_t msk[14] = "master secret";
        const uint8_t ssk[14] = "server secret";

        ep_t blinded;
        ep_new(blinded);
        bn_t rInv;
        bn_new(rInv);
        pythia_blind(blinded, rInv, password, 8);

        gt_t y;
        gt_new(y);

        bn_t kw;
        bn_new(kw);

        ep2_t tTilde;
        ep2_new(tTilde);

        pythia_eval(y, kw, tTilde, w, 10, t, 5, blinded, msk, 13, ssk, 13);

        g1_t p;
        g1_new(p);

        bn_t c;
        bn_new(c);

        bn_t u;
        bn_new(u);

        pythia_prove(p, c, u, blinded, tTilde, kw, y);

        TEST_ASSERT_NOT_EQUAL(pythia_verify(blinded, t, 5, y, p, c, u), 0);

        bn_free(u);
        bn_free(c);
        g1_free(p);
        ep2_free(tTilde);
        bn_free(kw);
        gt_free(y);
        bn_free(rInv);
        ep_free(blinded);
    }
}

void test3_UpdateDelta() {
    const uint8_t password[9] = "password";
    const uint8_t w[11] = "virgil.com";
    const uint8_t t[6] = "alice";
    const uint8_t msk0[14] = "master secret";
    const uint8_t ssk[14] = "server secret";

    ep_t blinded;
    ep_new(blinded);
    bn_t rInv;
    bn_new(rInv);
    pythia_blind(blinded, rInv, password, 8);

    gt_t y;
    gt_new(y);

    bn_t kw;
    bn_new(kw);

    ep2_t tTilde;
    ep2_new(tTilde);

    pythia_eval(y, kw, tTilde, w, 10, t, 5, blinded, msk0, 13, ssk, 13);

    gt_t deblinded0;
    gt_new(deblinded0);
    pythia_deblind(deblinded0, y, rInv);

    const uint8_t msk1[14] = "secret master";

    bn_t del;
    bn_new(del);
    gt_t pPrime;
    gt_new(pPrime);

    pythia_get_delta(del, pPrime, w, 10, msk0, 13, ssk, 13, w, 10, msk1, 13, ssk, 13);

    gt_t deblinded1;
    gt_new(deblinded1);
    pythia_update(deblinded1, deblinded0, del);

    ep_t blinded1;
    ep_new(blinded1);
    bn_t rInv1;
    bn_new(rInv1);
    pythia_blind(blinded1, rInv1, password, 8);

    gt_t y1;
    gt_new(y1);

    bn_t kw1;
    bn_new(kw1);

    ep2_t tTilde1;
    ep2_new(tTilde1);

    pythia_eval(y1, kw1, tTilde1, w, 10, t, 5, blinded1, msk1, 13, ssk, 13);

    gt_t deblinded2;
    gt_new(deblinded2);
    pythia_deblind(deblinded2, y1, rInv1);

    TEST_ASSERT_EQUAL_INT(gt_cmp(deblinded1, deblinded2), CMP_EQ);
}

int main() {
    UNITY_BEGIN();

    RUN_TEST(test0_Config);
    RUN_TEST(test1_SimpleProto);
    RUN_TEST(test2_FullProto);
    RUN_TEST(test3_UpdateDelta);

    return UNITY_END();
}