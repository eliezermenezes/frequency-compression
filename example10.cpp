#include <iostream>
#include <vector>
#include <cmath>
#include <sndfile.h>
#include <fftw3.h>
#include <complex>
#include <fstream>
#include <mpg123.h>

#define PI 3.14159265358979323846

// Converter MP3 para WAV usando mpg123
bool convertMP3ToWAV(const std::string& mp3_file, const std::string& wav_file) {
    mpg123_handle *mh;
    int err;
    if ((mh = mpg123_new(nullptr, &err)) == nullptr) {
        std::cerr << "Erro ao inicializar mpg123!" << std::endl;
        return false;
    }

    if (mpg123_open(mh, mp3_file.c_str()) != MPG123_OK) {
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

    outfile = sf_open(wav_file.c_str(), SFM_WRITE, &sfinfo);
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

// Aplicação da FFT para análise de frequência
std::vector<std::complex<double>> computeFFT(const std::vector<double>& signal) {
    int N = signal.size();
    if (N < 512) {
        std::cerr << "Erro: Sinal muito curto para FFT!" << std::endl;
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

    std::vector<std::complex<double>> spectrum(N);
    for (int i = 0; i < N; i++) {
        spectrum[i] = std::complex<double>(out[i][0], out[i][1]);
    }

    fftw_destroy_plan(p);
    fftw_free(in);
    fftw_free(out);

    return spectrum;
}

// Função para salvar FFT no arquivo
void saveFFTtoFile(const std::vector<std::complex<double>>& spectrum, const std::string& filename) {
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Erro ao abrir " << filename << " para escrita!" << std::endl;
        return;
    }

    for (const auto& val : spectrum) {
        file << std::abs(val) << "\n";  // Escrevendo magnitude da FFT
    }
    
    file.close();
    std::cout << "FFT salva em: " << filename << std::endl;
}

// Redução de frequência pelo corte de espectro
std::vector<std::complex<double>> reduceFrequency(const std::vector<std::complex<double>>& spectrum, int original_rate, int target_rate) {
    int N = spectrum.size();
    int cutoff = (target_rate * N) / (2 * original_rate);
    std::vector<std::complex<double>> new_spectrum(N, std::complex<double>(0, 0));

    for (int i = 0; i < cutoff; i++) {
        new_spectrum[i] = spectrum[i];
        new_spectrum[N - i - 1] = spectrum[N - i - 1];
    }

    return new_spectrum;
}

// Aplicação da IFFT para reconstruir o sinal
std::vector<double> computeIFFT(const std::vector<std::complex<double>>& spectrum) {
    int N = spectrum.size();
    fftw_complex *in, *out;
    fftw_plan p;

    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);

    for (int i = 0; i < N; i++) {
        in[i][0] = spectrum[i].real();
        in[i][1] = spectrum[i].imag();
    }

    p = fftw_plan_dft_1d(N, in, out, FFTW_BACKWARD, FFTW_ESTIMATE);
    fftw_execute(p);

    std::vector<double> signal(N);
    for (int i = 0; i < N; i++) {
        signal[i] = out[i][0] / N;
    }

    fftw_destroy_plan(p);
    fftw_free(in);
    fftw_free(out);

    return signal;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Uso: " << argv[0] << " <arquivo_entrada.mp3 ou .wav> <frequencia_destino_Hz> <arquivo_saida.wav>\n";
        return 1;
    }

    std::string input_file = argv[1];
    int target_frequency = std::stoi(argv[2]);
    std::string output_file = argv[3];

    std::string temp_wav = "media/temp_input.wav";

    // Se for MP3, converter para WAV
    if (input_file.substr(input_file.find_last_of(".") + 1) == "mp3") {
        if (!convertMP3ToWAV(input_file, temp_wav)) {
            std::cerr << "Erro ao converter MP3 para WAV!" << std::endl;
            return 1;
        }
        input_file = temp_wav;
    }

    // Processamento do áudio
    SF_INFO sfinfo;
    SNDFILE* infile = sf_open(input_file.c_str(), SFM_READ, &sfinfo);
    if (!infile) {
        std::cerr << "Erro ao abrir o arquivo WAV!" << std::endl;
        return 1;
    }

    int sample_rate = sfinfo.samplerate;
    int num_samples = sfinfo.frames;
    std::vector<double> samples(num_samples);
    sf_read_double(infile, samples.data(), num_samples);
    sf_close(infile);

    std::vector<std::complex<double>> original_fft = computeFFT(samples);
    saveFFTtoFile(original_fft, "media/fft_original.dat");

    std::vector<std::complex<double>> filtered_fft = reduceFrequency(original_fft, sample_rate, target_frequency);
    saveFFTtoFile(filtered_fft, "media/fft_processed.dat");

    std::vector<double> processed_signal = computeIFFT(filtered_fft);

    SF_INFO out_sfinfo = sfinfo;
    out_sfinfo.samplerate = target_frequency;
    out_sfinfo.frames = processed_signal.size();
    SNDFILE* outfile = sf_open(output_file.c_str(), SFM_WRITE, &out_sfinfo);
    sf_write_double(outfile, processed_signal.data(), processed_signal.size());
    sf_close(outfile);

    std::cout << "Processamento concluído! Arquivo salvo: " << output_file << std::endl;
    return 0;
}

// Run:
// g++ -o example10 example10.cpp -lsndfile -lfftw3 -lmpg123 -lm -O2