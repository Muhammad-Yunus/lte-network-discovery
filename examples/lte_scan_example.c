/**
 * lte_scan_example.c - LTE Cell Scanner with 2-step workflow
 *
 * Step 1: Coarse scan — fast PSS-only scan (~30s per band)
 * Step 2: Fine scan — MIB decode + operator lookup per EARFCN
 *
 * Build (inside srsRAN build tree):
 *   cmake --build . --target lte_scan_example
 *
 * Or standalone:
 *   gcc -o lte_scan_example lte_scan_example.c lte_scan.c \
 *       -I../.. -I../../build/lib/include \
 *       -L../../build/lib -lsrsran_phy -lsrsran_rf -lsrsran_common \
 *       -lSoapySDR -lpthread -lm -lstdc++ -lsrsran_asn1
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "lte_scan.h"

static volatile int g_running = 1;
static lte_scan_t*  g_scan    = NULL;

static void sig_handler(int sig)
{
    (void)sig;
    g_running = 0;
    if (g_scan) {
        lte_scan_stop(g_scan);
    }
}

static void usage(const char* prog)
{
    printf("Usage: %s [options]\n", prog);
    printf("  -b band          Band number (default: 8)\n");
    printf("  -d rf_device     SoapySDR device (default: auto-detect)\n");
    printf("  -a rf_args       Device arguments\n");
    printf("  -g gain          RX gain in dB (default: 42)\n");
    printf("  -s earfcn_start  EARFCN start (default: full band)\n");
    printf("  -e earfcn_end    EARFCN end\n");
    printf("  -m max_prb       Max PRB for SIB1 (default: 15 for RTL-SDR)\n");
    printf("  -1               One-step mode (scan every EARFCN, slow)\n");
}

int main(int argc, char* argv[])
{
    int         band        = -1;
    const char* rf_device   = "";
    const char* rf_args     = "";
    float       gain        = 42.0f;
    int         earfcn_s    = -1;
    int         earfcn_e    = -1;
    uint32_t    max_prb     = 15;
    int         one_step    = 0;
    int         opt;

    while ((opt = getopt(argc, argv, "b:d:a:g:s:e:m:1h")) != -1) {
        switch (opt) {
            case 'b': band      = atoi(optarg); break;
            case 'd': rf_device = optarg;       break;
            case 'a': rf_args   = optarg;       break;
            case 'g': gain      = atof(optarg); break;
            case 's': earfcn_s  = atoi(optarg); break;
            case 'e': earfcn_e  = atoi(optarg); break;
            case 'm': max_prb   = atoi(optarg); break;
            case '1': one_step  = 1;            break;
            default:
                usage(argv[0]);
                return 1;
        }
    }

    if (band < 0) {
        usage(argv[0]);
        return 1;
    }

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    /* Initialize */
    lte_scan_t scan;
    g_scan = &scan;

    lte_scan_config_t cfg = {
        .rf_device     = rf_device,
        .rf_args       = rf_args,
        .rf_gain_dB    = gain,
        .max_prb_sib1  = max_prb,
        .psr_threshold = 2.0f,
        .try_sib1      = false,
    };

    if (lte_scan_init_ex(&scan, &cfg) != 0) {
        fprintf(stderr, "Failed to open SDR device\n");
        return 1;
    }

    printf("=== LTE Cell Scanner ===\n");
    printf("Band: %d | Gain: %.0f dB | Mode: %s\n\n",
           band, gain, one_step ? "one-step (slow)" : "two-step (fast)");

    int n;

    if (one_step) {
        /* One-step: scan every EARFCN (slow) */
        n = lte_scan_band(&scan, band, earfcn_s, earfcn_e);
    } else {
        /* Two-step: coarse + fine */
        printf("--- Step 1: Coarse scan (PSS only) ---\n");
        n = lte_scan_coarse(&scan, band, earfcn_s, earfcn_e);

        if (n > 0 && g_running) {
            printf("\n--- Step 2: Fine scan (MIB + operator) ---\n");
            for (int i = 0; i < n && g_running; i++) {
                lte_scan_fine(&scan, scan.coarse_earfcns[i]);
            }
        }
        n = scan.nof_results;
    }

    /* Print results */
    printf("\n========================================\n");
    printf("  RESULTS: %d cell(s) found on Band %d\n", n, band);
    printf("========================================\n\n");

    for (int i = 0; i < n; i++) {
        lte_scan_result_t* r = &scan.results[i];
        printf("Cell #%d\n", i + 1);
        printf("  EARFCN:  %d (%.1f MHz)\n", r->earfcn, r->freq_mhz);
        printf("  PCI:     %d\n", r->pci);
        printf("  PRB:     %d\n", r->nof_prb);
        printf("  Ports:   %d\n", r->nof_ports);
        printf("  Power:   %.1f dBm\n", r->rsrp_dbm);
        printf("  Operator: %s\n", r->operator_name);
        printf("  MCC/MNC: %03d/%0*u%s\n",
               r->mcc, r->mnc_3digit ? 3 : 2, r->mnc,
               r->from_sib1 ? " [from SIB1]" : " [from EARFCN table]");
        if (r->sib1_decoded) {
            printf("  TAC:     %u\n", r->tac);
            printf("  CellID:  %u\n", r->cell_id);
        }
        printf("\n");
    }

    lte_scan_free(&scan);
    g_scan = NULL;
    return 0;
}
