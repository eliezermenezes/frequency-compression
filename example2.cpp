#include <iostream>
#include <vector>
#include <cmath>
#include <sndfile.h> // Biblioteca para manipulação de arquivos WAV

#define PI 3.14159265358979323846

// Função para gerar coeficientes FIR com uma janela de Hamming
std::vector<double> generate_fir_coefficients(int filter_order, double cutoff_frequency, double sampling_rate) {
    std::vector<double> coefficients(filter_order + 1);
    double norm_cutoff = cutoff_frequency / (sampling_rate / 2); // Normalizando a frequência de corte

    for (int i = 0; i <= filter_order; i++) {
        int middle = filter_order / 2;
        if (i == middle) {
            coefficients[i] = norm_cutoff;
        } else {
            double sinc_value = sin(PI * norm_cutoff * (i - middle)) / (PI * (i - middle));
            coefficients[i] = sinc_value * (0.54 - 0.46 * cos(2 * PI * i / filter_order)); // Janela de Hamming
        }
    }
    return coefficients;
}

// Aplica o filtro FIR ao áudio
std::vector<double> apply_fir_filter(const std::vector<double>& input_signal, const std::vector<double>& coefficients) {
    int filter_size = coefficients.size();
    int signal_size = input_signal.size();
    std::vector<double> output_signal(signal_size, 0.0);

    for (int n = 0; n < signal_size; n++) {
        for (int k = 0; k < filter_size; k++) {
            if (n >= k) {
                output_signal[n] += coefficients[k] * input_signal[n - k];
            }
        }
    }
    return output_signal;
}

// Função para reduzir a taxa de amostragem (decimação)
std::vector<double> downsample(const std::vector<double>& signal, int factor) {
    std::vector<double> downsampled_signal;
    for (size_t i = 0; i < signal.size(); i += factor) {
        downsampled_signal.push_back(signal[i]);
    }
    return downsampled_signal;
}

// Função para processar um arquivo WAV
void process_audio(const char* input_file, const char* output_file, int filter_order, double cutoff_freq, int downsample_factor) {
    // Abrir arquivo WAV
    SF_INFO sfinfo;
    SNDFILE* infile = sf_open(input_file, SFM_READ, &sfinfo);
    if (!infile) {
        std::cerr << "Erro ao abrir arquivo WAV!" << std::endl;
        return;
    }

    // Ler áudio do arquivo
    std::vector<double> input_audio(sfinfo.frames);
    sf_read_double(infile, input_audio.data(), sfinfo.frames);
    sf_close(infile);

    // Criar filtro FIR
    std::vector<double> fir_coeffs = generate_fir_coefficients(filter_order, cutoff_freq, sfinfo.samplerate);

    // Aplicar filtro FIR
    std::vector<double> filtered_audio = apply_fir_filter(input_audio, fir_coeffs);

    // Reduzir a taxa de amostragem (decimação)
    std::vector<double> downsampled_audio = downsample(filtered_audio, downsample_factor);

    // Configurar metadados para o novo arquivo WAV
    SF_INFO out_sfinfo = sfinfo;
    out_sfinfo.frames = downsampled_audio.size();
    out_sfinfo.samplerate = sfinfo.samplerate / downsample_factor;

    // Salvar o novo arquivo WAV
    SNDFILE* outfile = sf_open(output_file, SFM_WRITE, &out_sfinfo);
    if (!outfile) {
        std::cerr << "Erro ao salvar arquivo WAV!" << std::endl;
        return;
    }
    sf_write_double(outfile, downsampled_audio.data(), downsampled_audio.size());
    sf_close(outfile);

    std::cout << "Processamento concluído! Arquivo salvo como " << output_file << std::endl;
}

int main() {
    // Configuração do filtro e do downsampling
    const char* input_wav = "audio.wav";
    const char* output_wav = "audio_output.wav";
    int filter_order = 31;        // Ordem do filtro FIR
    double cutoff_frequency = 4000.0; // Frequência de corte (Hz)
    int downsample_factor = 2;    // Fator de redução da taxa de amostragem

    // Processa o arquivo WAV
    process_audio(input_wav, output_wav, filter_order, cutoff_frequency, downsample_factor);

    return 0;
}

// Run
// g++ -o example2 example2.cpp -lsndfile -std=c++11
// ./example2