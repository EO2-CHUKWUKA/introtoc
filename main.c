#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>    /* offsetof */
#include "waveform.h"
#include "io.h"

int main(void) {
    const char *input_file = "C:\\Users\\EO2-CHUKWUKA\\OneDrive - UWE Bristol\\c program\\power_quality_log(2).csv";
    const char *output_file = "C:\\Users\\EO2-CHUKWUKA\\OneDrive - UWE Bristol\\results.txt";

    /* ---- 1. Load CSV into heap-allocated array ---- */
    int n = 0;
    WaveformSample *samples = load_csv(input_file, &n);

    if (!samples) {
        fprintf(stderr, "ERROR: Could not load the CSV file.\n");
        return EXIT_FAILURE;
    }

    printf("Loaded %d samples from '%s'\n\n", n, input_file);

    /* ---- 2. Analyse each phase ----
     * offsetof() gives the byte position of each voltage field
     * inside the struct, so one function handles all three phases.
     * ----------------------------------------------------------- */
    PhaseResult results[3];

    results[0] = analyse_phase(samples, n, 'A',
                               (int)offsetof(WaveformSample, phase_A_voltage));

    results[1] = analyse_phase(samples, n, 'B',
                               (int)offsetof(WaveformSample, phase_B_voltage));

    results[2] = analyse_phase(samples, n, 'C',
                               (int)offsetof(WaveformSample, phase_C_voltage));

    /* ---- 3. System-wide metrics ---- */
    double freq    = mean_frequency(samples, n);
    double pf      = mean_power_factor(samples, n);
    double thd_avg = mean_thd(samples, n);

    int total_clipped = results[0].clipped_count
                        + results[1].clipped_count
                        + results[2].clipped_count;

    /* ---- 4. Print to terminal ---- */
    print_report(results, freq, pf, thd_avg, total_clipped);

    /* ---- 5. Write results.txt ---- */
    if (write_report(output_file, results, freq, pf, thd_avg, total_clipped) == 0) {
        printf("\nReport written to '%s'\n", output_file);
    }

    /* ---- 6. Free all heap memory ---- */
    free(samples);
    samples = NULL;

    return EXIT_SUCCESS;
}
