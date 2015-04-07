#include "fft.h"
#include <complex.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <fftw3.h> // Used as a reference.

// TODO: Portable
#include <time.h>
static double mufft_get_time(void)
{
    struct timespec tv;
    clock_gettime(CLOCK_MONOTONIC, &tv);
    return tv.tv_sec + tv.tv_nsec / 1000000000.0;
}

static double bench_fftw_1d(unsigned N, unsigned iterations, unsigned flags)
{
    complex float *input = fftwf_malloc(N * sizeof(complex float));
    complex float *output = fftwf_malloc(N * sizeof(complex float));

    fftwf_plan plan = fftwf_plan_dft_1d(N, input, output,
            FFTW_FORWARD, flags);

    srand(0);
    for (unsigned i = 0; i < N; i++)
    {
        float real = (float)rand() / RAND_MAX - 0.5f;
        float imag = (float)rand() / RAND_MAX - 0.5f;
        input[i] = real + _Complex_I * imag;
    }

    double start_time = mufft_get_time();
    for (unsigned i = 0; i < iterations; i++)
    {
        fftwf_execute(plan);
    }
    double end_time = mufft_get_time();

    fftwf_free(input);
    fftwf_free(output);
    fftwf_destroy_plan(plan);

    return end_time - start_time;
}

static double bench_fftw_1d_real(unsigned N, unsigned iterations, unsigned flags)
{
    unsigned fftN = N / 2 + 1;
    float *input = fftwf_malloc(N * sizeof(float));
    float *dummy = fftwf_malloc(N * sizeof(float));
    complex float *output = fftwf_malloc(fftN * sizeof(complex float));

    fftwf_plan plan_r2c = fftwf_plan_dft_r2c_1d(N, input, output,
            flags);
    fftwf_plan plan_c2r = fftwf_plan_dft_c2r_1d(N, output, dummy,
            flags);

    srand(0);
    for (unsigned i = 0; i < N; i++)
    {
        float real = (float)rand() / RAND_MAX - 0.5f;
        input[i] = real;
    }

    double start_time = mufft_get_time();
    for (unsigned i = 0; i < iterations; i++)
    {
        fftwf_execute(plan_r2c);
        fftwf_execute(plan_c2r);
    }
    double end_time = mufft_get_time();

    fftwf_free(input);
    fftwf_free(dummy);
    fftwf_free(output);
    fftwf_destroy_plan(plan_r2c);
    fftwf_destroy_plan(plan_c2r);

    return end_time - start_time;
}

static double bench_fftw_2d(unsigned Nx, unsigned Ny, unsigned iterations, unsigned flags)
{
    complex float *input = fftwf_malloc(Nx * Ny * sizeof(complex float));
    complex float *output = fftwf_malloc(Nx * Ny * sizeof(complex float));

    fftwf_plan plan = fftwf_plan_dft_2d(Ny, Nx, input, output,
            FFTW_FORWARD, flags);

    srand(0);
    for (unsigned i = 0; i < Nx * Ny; i++)
    {
        float real = (float)rand() / RAND_MAX - 0.5f;
        float imag = (float)rand() / RAND_MAX - 0.5f;
        input[i] = real + _Complex_I * imag;
    }

    double start_time = mufft_get_time();
    for (unsigned i = 0; i < iterations; i++)
    {
        fftwf_execute(plan);
    }
    double end_time = mufft_get_time();

    fftwf_free(input);
    fftwf_free(output);
    fftwf_destroy_plan(plan);

    return end_time - start_time;
}

static double bench_fft_1d(unsigned N, unsigned iterations, unsigned flags)
{
    complex float *input = mufft_alloc(N * sizeof(complex float));
    complex float *output = mufft_alloc(N * sizeof(complex float));

    srand(0);
    for (unsigned i = 0; i < N; i++)
    {
        float real = (float)rand() / RAND_MAX - 0.5f;
        float imag = (float)rand() / RAND_MAX - 0.5f;
        input[i] = real + _Complex_I * imag;
    }

    mufft_plan_1d *muplan = mufft_create_plan_1d_c2c(N, MUFFT_FORWARD, flags);

    double start_time = mufft_get_time();
    for (unsigned i = 0; i < iterations; i++)
    {
        mufft_execute_plan_1d(muplan, output, input);
    }
    double end_time = mufft_get_time();

    mufft_free(input);
    mufft_free(output);
    mufft_free_plan_1d(muplan);

    return end_time - start_time;
}

static double bench_fft_1d_real(unsigned N, unsigned iterations, unsigned flags)
{
    unsigned fftN = N / 2;
    float *input = mufft_alloc(N * sizeof(float));
    float *dummy = mufft_alloc(N * sizeof(float));
    complex float *output = mufft_alloc(fftN * sizeof(complex float));

    srand(0);
    for (unsigned i = 0; i < N; i++)
    {
        float real = (float)rand() / RAND_MAX - 0.5f;
        input[i] = real;
    }

    mufft_plan_1d *muplan_r2c = mufft_create_plan_1d_r2c(N, flags);
    mufft_plan_1d *muplan_c2r = mufft_create_plan_1d_c2r(N, flags);

    double start_time = mufft_get_time();
    for (unsigned i = 0; i < iterations; i++)
    {
        mufft_execute_plan_1d(muplan_r2c, output, input);
        mufft_execute_plan_1d(muplan_c2r, dummy, output); // To avoid input degrading over time.
    }
    double end_time = mufft_get_time();

    mufft_free(input);
    mufft_free(dummy);
    mufft_free(output);
    mufft_free_plan_1d(muplan_r2c);
    mufft_free_plan_1d(muplan_c2r);

    return end_time - start_time;
}


static double bench_fft_2d(unsigned Nx, unsigned Ny, unsigned iterations, unsigned flags)
{
    complex float *input = mufft_alloc(Nx * Ny * sizeof(complex float));
    complex float *output = mufft_alloc(Nx * Ny * sizeof(complex float));

    srand(0);
    for (unsigned i = 0; i < Nx * Ny; i++)
    {
        float real = (float)rand() / RAND_MAX - 0.5f;
        float imag = (float)rand() / RAND_MAX - 0.5f;
        input[i] = real + _Complex_I * imag;
    }

    mufft_plan_2d *muplan = mufft_create_plan_2d_c2c(Nx, Ny, MUFFT_FORWARD, flags);

    double start_time = mufft_get_time();
    for (unsigned i = 0; i < iterations; i++)
    {
        mufft_execute_plan_2d(muplan, output, input);
    }
    double end_time = mufft_get_time();

    mufft_free(input);
    mufft_free(output);
    mufft_free_plan_2d(muplan);

    return end_time - start_time;
}

static void run_benchmark_1d(unsigned N, unsigned iterations)
{
    double flops = 5.0 * N * log2(N); // Estimation
    double fftw_time = bench_fftw_1d(N, iterations, FFTW_ESTIMATE);
    double fftw_measured_time = bench_fftw_1d(N, iterations, FFTW_MEASURE);
    double mufft_time = bench_fft_1d(N, iterations, 0);
    flops *= iterations;

    double fftw_mflops = flops / (1000000.0 * fftw_time);
    double fftw_measured_mflops = flops / (1000000.0 * fftw_measured_time);
    double mufft_mflops = flops / (1000000.0 * mufft_time);

    printf("FFTW C2C estimate:  %06u %12.3f Mflops %12.3f us iteration\n",
            N, fftw_mflops, 1000000.0 * fftw_time / iterations);
    printf("FFTW C2C measure:   %06u %12.3f Mflops %12.3f us iteration\n",
            N, fftw_measured_mflops, 1000000.0 * fftw_measured_time / iterations);
    printf("muFFT C2C:          %06u %12.3f Mflops %12.3f us iteration\n",
            N, mufft_mflops, 1000000.0 * mufft_time / iterations);
}

static void run_benchmark_1d_real(unsigned N, unsigned iterations)
{
    double flops = 5.0 * N * log2(N); // Estimation
    double fftw_time = bench_fftw_1d_real(N, iterations, FFTW_ESTIMATE);
    double fftw_measured_time = bench_fftw_1d_real(N, iterations, FFTW_MEASURE);
    double mufft_time = bench_fft_1d_real(N, iterations, 0);
    flops *= iterations;

    double fftw_mflops = flops / (1000000.0 * fftw_time);
    double fftw_measured_mflops = flops / (1000000.0 * fftw_measured_time);
    double mufft_mflops = flops / (1000000.0 * mufft_time);

    printf("FFTW R2C-C2R estimate:  %06u %12.3f Mflops %12.3f us iteration\n",
            N, fftw_mflops, 1000000.0 * fftw_time / iterations);
    printf("FFTW R2C-C2R measure:   %06u %12.3f Mflops %12.3f us iteration\n",
            N, fftw_measured_mflops, 1000000.0 * fftw_measured_time / iterations);
    printf("muFFT R2C-C2R:          %06u %12.3f Mflops %12.3f us iteration\n",
            N, mufft_mflops, 1000000.0 * mufft_time / iterations);
}

static void run_benchmark_2d(unsigned Nx, unsigned Ny, unsigned iterations)
{
    double flops = 5.0 * Ny * Nx * log2(Nx) + 5.0 * Nx * Ny * log2(Ny); // Estimation
    double fftw_time = bench_fftw_2d(Nx, Ny, iterations, FFTW_ESTIMATE);
    double fftw_measured_time = bench_fftw_2d(Nx, Ny, iterations, FFTW_MEASURE);
    double mufft_time = bench_fft_2d(Nx, Ny, iterations, 0);
    flops *= iterations;

    double fftw_mflops = flops / (1000000.0 * fftw_time);
    double fftw_measured_mflops = flops / (1000000.0 * fftw_measured_time);
    double mufft_mflops = flops / (1000000.0 * mufft_time);

    printf("FFTW estimate:  %04u by %04u, %12.3f Mflops %12.3f us iteration\n",
            Nx, Ny, fftw_mflops, 1000000.0 * fftw_time / iterations);
    printf("FFTW measure:   %04u by %04u, %12.3f Mflops %12.3f us iteration\n",
            Nx, Ny, fftw_measured_mflops, 1000000.0 * fftw_measured_time / iterations);
    printf("muFFT:          %04u by %04u, %12.3f Mflops %12.3f us iteration\n",
            Nx, Ny, mufft_mflops, 1000000.0 * mufft_time / iterations);
}

int main(int argc, char *argv[])
{
    if (argc == 2 || argc > 4)
    {
        fprintf(stderr, "Usage: %s [iterations] [Nx] [Ny]\n",
                argv[0]);
        return 1;
    }

    if (argc == 1)
    {
        printf("\n1D benchmarks ...\n");
        for (unsigned N = 4; N <= 128 * 1024; N <<= 1)
        {
            run_benchmark_1d(N, 400000000ull / N);
            run_benchmark_1d_real(N, 400000000ull / N);
        }

        printf("\n2D benchmarks ...\n");
        for (unsigned Ny = 4; Ny <= 1024; Ny <<= 1)
        {
            for (unsigned Nx = 4; Nx <= 1024; Nx <<= 1)
            {
                run_benchmark_2d(Nx, Ny, 400000000ull / (Nx * Ny));
            }
        }
    }
    else if (argc == 3)
    {
        unsigned iterations = strtoul(argv[1], NULL, 0);
        unsigned Nx = strtoul(argv[2], NULL, 0);
        run_benchmark_1d(Nx, iterations);
        run_benchmark_1d_real(Nx, iterations);
    }
    else if (argc == 4)
    {
        unsigned iterations = strtoul(argv[1], NULL, 0);
        unsigned Nx = strtoul(argv[2], NULL, 0);
        unsigned Ny = strtoul(argv[3], NULL, 0);
        run_benchmark_2d(Nx, Ny, iterations);
    }
}
