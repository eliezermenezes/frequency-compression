#include <iostream>
#include <vector>
#include <cmath>
#include <mpg123.h>
#include <sndfile.h>
#include <fftw3.h>
#include <fstream>

#define PI 3.14159265358979323846

// Aplicação da FFT para análise de frequência
std::vector<double> computeFFT(const std::vector<double>& signal) {
    int N = signal.size();
    if (N < 512) {
        std::cerr << "Aviso: Sinal muito curto para FFT!" << std::endl;
        return {};
    }

    fftw_complex *in, *out;
    fftw_plan p;

    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);

    for (int i = 0; i < N; i++) {
        in[i][0] = signal[i];
        in[i][1] = 0.0;
    }

    p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(p);

    std::vector<double> spectrum(N);
    for (int i = 0; i < N; i++) {
        spectrum[i] = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
    }

    fftw_destroy_plan(p);
    fftw_free(in);
    fftw_free(out);

    return spectrum;
}

// Função para converter MP3 para WAV usando mpg123
bool convertMP3ToWAV(const char* mp3_file, const char* wav_file) {
    mpg123_handle *mh;
    int err;
    if ((mh = mpg123_new(nullptr, &err)) == nullptr) {
        std::cerr << "Erro ao inicializar mpg123!" << std::endl;
        return false;
    }

    if (mpg123_open(mh, mp3_file) != MPG123_OK) {
        std::cerr << "Erro ao abrir arquivo MP3!" << std::endl;
        return false;
    }

    long rate;
    int channels, encoding;
    mpg123_getformat(mh, &rate, &channels, &encoding);

    std::vector<short> buffer(4096);
    SNDFILE *outfile;
    SF_INFO sfinfo = {0};
    sfinfo.samplerate = rate;
    sfinfo.channels = channels;
    sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

    outfile = sf_open(wav_file, SFM_WRITE, &sfinfo);
    if (!outfile) {
        std::cerr << "Erro ao criar arquivo WAV!" << std::endl;
        return false;
    }

    size_t bytes_read;
    while (mpg123_read(mh, reinterpret_cast<unsigned char*>(buffer.data()), buffer.size() * sizeof(short), &bytes_read) == MPG123_OK) {
        sf_write_short(outfile, buffer.data(), bytes_read / sizeof(short));
    }

    sf_close(outfile);
    mpg123_close(mh);
    mpg123_delete(mh);

    return true;
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
        std::cerr << "Uso: " << argv[0] << " <arquivo_entrada.mp3> <frequencia_destino_Hz> <arquivo_saida.mp3>\n";
        return 1;
    }

    const char* input_mp3 = argv[1];
    int target_frequency = std::stoi(argv[2]);
    const char* output_mp3 = argv[3];

    const char* temp_wav = "media/temp_input.wav";
    const char* output_wav = "media/temp_output.wav";

    // Converter MP3 para WAV
    if (!convertMP3ToWAV(input_mp3, temp_wav)) {
        std::cerr << "Erro ao converter MP3 para WAV!" << std::endl;
        return 1;
    }

    // Abrir arquivo WAV convertido
    SF_INFO sfinfo;
    SNDFILE* infile = sf_open(temp_wav, SFM_READ, &sfinfo);
    if (!infile) {
        std::cerr << "Erro ao abrir o arquivo WAV convertido!" << std::endl;
        return 1;
    }

    int sample_rate = sfinfo.samplerate;
    int num_samples = sfinfo.frames;
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
    SNDFILE* outfile = sf_open(output_wav, SFM_WRITE, &out_sfinfo);
    if (!outfile) {
        std::cerr << "Erro ao criar o arquivo WAV de saída!" << std::endl;
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

    // Converter WAV processado de volta para MP3
    // std::string command = "ffmpeg -y -i temp_output.wav -codec:a libmp3lame -qscale:a 2 " + std::string(output_mp3);
    // system(command.c_str());

    std::cout << "Processamento concluído! Arquivo MP3 final: " << output_mp3 << "\n";
    return 0;
}
