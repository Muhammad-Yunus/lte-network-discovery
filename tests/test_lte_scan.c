#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "lte_scan.h"

static int tests_run = 0;
static int tests_passed = 0;

#define ASSERT_STR_EQ(a, b, msg) do { \
    if (strcmp((a), (b)) != 0) { \
        fprintf(stderr, "FAIL: %s: expected \"%s\", got \"%s\"\n", msg, (b), (a)); \
        return 0; \
    } \
} while(0)

#define ASSERT_INT_EQ(a, b, msg) do { \
    if ((a) != (b)) { \
        fprintf(stderr, "FAIL: %s: expected %d, got %d\n", msg, (b), (a)); \
        return 0; \
    } \
} while(0)

#define RUN_TEST(fn) do { \
    tests_run++; \
    printf("  %-40s ", #fn); \
    if (fn()) { tests_passed++; printf("PASS\n"); } \
    else { printf("FAIL\n"); } \
} while(0)

static int test_lookup_telkomsel_b3(void) {
    const lte_operator_entry_t* e = lte_scan_lookup_operator(1300);
    assert(e != NULL);
    ASSERT_INT_EQ(e->mcc, 510, "MCC");
    ASSERT_INT_EQ(e->mnc, 10, "MNC");
    ASSERT_STR_EQ(e->operator_name, "Telkomsel", "name");
    return 1;
}

static int test_lookup_xl_b3(void) {
    const lte_operator_entry_t* e = lte_scan_lookup_operator(1450);
    assert(e != NULL);
    ASSERT_INT_EQ(e->mcc, 510, "MCC");
    ASSERT_INT_EQ(e->mnc, 11, "MNC");
    ASSERT_STR_EQ(e->operator_name, "XL Axiata", "name");
    return 1;
}

static int test_lookup_indosat_b8(void) {
    const lte_operator_entry_t* e = lte_scan_lookup_operator(3650);
    assert(e != NULL);
    ASSERT_INT_EQ(e->mcc, 510, "MCC");
    ASSERT_INT_EQ(e->mnc, 21, "MNC");
    ASSERT_STR_EQ(e->operator_name, "Indosat Ooredoo", "name");
    return 1;
}

static int test_lookup_smartfren_b5(void) {
    const lte_operator_entry_t* e = lte_scan_lookup_operator(2500);
    assert(e != NULL);
    ASSERT_INT_EQ(e->mcc, 510, "MCC");
    ASSERT_INT_EQ(e->mnc, 9, "MNC");
    ASSERT_STR_EQ(e->operator_name, "Smartfren", "name");
    return 1;
}

static int test_lookup_unknown(void) {
    const lte_operator_entry_t* e = lte_scan_lookup_operator(99999);
    if (e != NULL) {
        fprintf(stderr, "FAIL: expected NULL for unknown EARFCN\n");
        return 0;
    }
    return 1;
}

static int test_lookup_b40(void) {
    const lte_operator_entry_t* e = lte_scan_lookup_operator(38700);
    assert(e != NULL);
    ASSERT_STR_EQ(e->operator_name, "Telkomsel", "name");
    ASSERT_STR_EQ(e->band_name, "Band 40", "band");
    return 1;
}

static int test_lookup_hutchison_b8(void) {
    const lte_operator_entry_t* e = lte_scan_lookup_operator(3700);
    assert(e != NULL);
    ASSERT_STR_EQ(e->operator_name, "Hutchison 3", "name");
    ASSERT_INT_EQ(e->mnc, 89, "MNC");
    return 1;
}

static int test_lookup_b28(void) {
    const lte_operator_entry_t* e = lte_scan_lookup_operator(9200);
    assert(e != NULL);
    ASSERT_STR_EQ(e->operator_name, "Indosat Ooredoo", "name");
    return 1;
}

static int test_result_str(void) {
    lte_scan_result_t r;
    memset(&r, 0, sizeof(r));
    r.earfcn = 1300;
    r.freq_mhz = 1835.0f;
    r.pci = 123;
    r.nof_prb = 50;
    r.nof_ports = 2;
    r.rsrp_dbm = -85.3f;
    r.mcc = 510;
    r.mnc = 10;
    r.from_sib1 = false;
    strncpy(r.operator_name, "Telkomsel", sizeof(r.operator_name));

    char buf[512];
    lte_scan_result_str(&r, buf, sizeof(buf));

    assert(strstr(buf, "Telkomsel") != NULL);
    assert(strstr(buf, "EARFCN 1300") != NULL);
    assert(strstr(buf, "PCI 123") != NULL);
    assert(strstr(buf, "510/10") != NULL);
    return 1;
}

static int test_table_sanity(void) {
    assert(lte_operator_table_id_size > 0);
    for (int i = 1; i < lte_operator_table_id_size; i++) {
        if (lte_operator_table_id[i].earfcn_min <= lte_operator_table_id[i-1].earfcn_max) {
            fprintf(stderr, "Overlap at index %d: [%d-%d] vs [%d-%d]\n",
                i,
                lte_operator_table_id[i-1].earfcn_min, lte_operator_table_id[i-1].earfcn_max,
                lte_operator_table_id[i].earfcn_min, lte_operator_table_id[i].earfcn_max);
            return 0;
        }
    }
    return 1;
}

int main(void) {
    printf("Running lte_scan unit tests...\n");

    RUN_TEST(test_lookup_telkomsel_b3);
    RUN_TEST(test_lookup_xl_b3);
    RUN_TEST(test_lookup_indosat_b8);
    RUN_TEST(test_lookup_smartfren_b5);
    RUN_TEST(test_lookup_unknown);
    RUN_TEST(test_lookup_b40);
    RUN_TEST(test_lookup_hutchison_b8);
    RUN_TEST(test_lookup_b28);
    RUN_TEST(test_result_str);
    RUN_TEST(test_table_sanity);

    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
