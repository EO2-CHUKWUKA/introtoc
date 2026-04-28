#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>    /* offsetof */
#include "waveform.h"
#include "io.h"
int main(int argc, char *argv[]) {
    if (argc != 2 ) {
        fprintf(stderr,
                "Usage: %s <csv_file>\n"
                "Example: %s power_quality_log(2).csv\n",
                argv[0], argv[0]);
        return EXIT_FAILURE;
    }

    const char *input_file  = argv[1];
    const char *output_file = "results.txt";
    /* ---- 2. Load CSV into heap-allocated array ---- */
    int n = 0;
    WaveformSample *samples = load_csv(input_file, &n);

    if (!samples) {
        /* load_csv already printed the error */
    return EXIT_FAILURE;
}
    printf("Loaded %d samples from '%s'\n\n", n, input_file);

    /* ---- 3. Analyse each phase ----
     *
     * offsetof(WaveformSample, phase_X_voltage) gives the byte
     * offset of each phase field inside the struct.
     * This lets us reuse every analysis function for all three
     * phases without duplicating code.
     * --------------------------------------------------------- */
    PhaseResult results[3];

    results[0] = analyse_phase(samples, n, 'A',
                               (int)offsetof(WaveformSample, phase_A_voltage));

    results[1] = analyse_phase(samples, n, 'B',
                               (int)offsetof(WaveformSample, phase_B_voltage));

    results[2] = analyse_phase(samples, n, 'C',
                               (int)offsetof(WaveformSample, phase_C_voltage));

    /* ---- 4. System-wide metrics ---- */
    double freq    = mean_frequency(samples, n);
    double pf      = mean_power_factor(samples, n);
    double thd_avg = mean_thd(samples, n);

    int total_clipped = results[0].clipped_count
                        + results[1].clipped_count
                        + results[2].clipped_count;

    /* ---- 5. Print to terminal ---- */
    print_report(results, freq, pf, thd_avg, total_clipped);
/* ---- 6. Write results.txt ---- */
    if (write_report(output_file, results, freq, pf, thd_avg, total_clipped) == 0) {
        printf("\nReport written to '%s'\n", output_file);
    }

    /* ---- 7. Free all heap memory ---- */
    free(samples);
    samples = NULL;   /* defensive: nullify after free */

    return EXIT_SUCCESS;
}
