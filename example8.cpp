#include <iostream>
#include <vector>
#include <cmath>
#include <sndfile.h>
#include <fftw3.h>
#include <fstream>

#define PI 3.14159265358979323846

// Aplicação da FFT para análise de frequência
std::vector<double> computeFFT(const std::vector<double>& signal) {
    int N = signal.size();
    fftw_complex *in, *out;
    fftw_plan p;

    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);

    for (int i = 0; i < N; i++) {
        in[i][0] = signal[i];  // Parte real
        in[i][1] = 0.0;        // Parte imaginária
    }

    p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(p);

    std::vector<double> spectrum(N);
    for (int i = 0; i < N; i++) {
        spectrum[i] = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]); // Módulo da FFT
    }

    fftw_destroy_plan(p);
    fftw_free(in);
    fftw_free(out);

    return spectrum;
}

// Função de downsampling
std::vector<double> downsample(const std::vector<double>& signal, int factor) {
    std::vector<double> downsampled;
    for (size_t i = 0; i < signal.size(); i += factor) {
        downsampled.push_back(signal[i]);
    }
    return downsampled;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Uso: " << argv[0] << " <arquivo_entrada.wav> <frequencia_destino_Hz> <arquivo_saida.wav>\n";
        return 1;
    }

    const char* input_file = argv[1];
    int target_frequency = std::stoi(argv[2]);
    const char* output_file = argv[3];

    // Abrir arquivo WAV
    SF_INFO sfinfo;
    SNDFILE* infile = sf_open(input_file, SFM_READ, &sfinfo);
    if (!infile) {
        std::cerr << "Erro ao abrir o arquivo WAV!\n";
        return 1;
    }

    int sample_rate = sfinfo.samplerate;
    int num_samples = sfinfo.frames;
    int num_channels = sfinfo.channels;
    int downsample_factor = sample_rate / target_frequency;

    std::vector<double> samples(num_samples);
    sf_read_double(infile, samples.data(), num_samples);
    sf_close(infile);

    // Aplicar FFT antes do downsampling
    std::vector<double> original_fft = computeFFT(samples);

    // Aplicar downsampling
    std::vector<double> downsampled_samples = downsample(samples, downsample_factor);

    // Salvar novo arquivo WAV
    SF_INFO out_sfinfo = sfinfo;
    out_sfinfo.samplerate = target_frequency;
    out_sfinfo.frames = downsampled_samples.size();
    SNDFILE* outfile = sf_open(output_file, SFM_WRITE, &out_sfinfo);
    if (!outfile) {
        std::cerr << "Erro ao criar o arquivo WAV de saída!\n";
        return 1;
    }

    sf_write_double(outfile, downsampled_samples.data(), downsampled_samples.size());
    sf_close(outfile);

    // Aplicar FFT depois do downsampling
    std::vector<double> processed_fft = computeFFT(downsampled_samples);

    // Salvar FFTs para análise no Python
    std::ofstream fft_original("media/fft_original.dat");
    for (const auto& val : original_fft) fft_original << val << "\n";
    fft_original.close();

    std::ofstream fft_processed("media/fft_processed.dat");
    for (const auto& val : processed_fft) fft_processed << "\n";
    fft_processed.close();

    std::cout << "Processamento concluído! Arquivo de saída: " << output_file << "\n";
    return 0;
}
