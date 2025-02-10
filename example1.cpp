#include <iostream>
#include <vector>
#include <cmath>

#define PI 3.14159265358979323846

// Função para gerar coeficientes FIR usando uma janela de Hamming
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

// Aplica o filtro FIR ao sinal de entrada
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

int main() {
    // Parâmetros do filtro
    int filter_order = 31;        // Ordem do filtro (deve ser ímpar)
    double cutoff_frequency = 500.0; // Frequência de corte em Hz
    double sampling_rate = 8000.0;   // Taxa de amostragem em Hz

    // Gera coeficientes do filtro FIR
    std::vector<double> fir_coefficients = generate_fir_coefficients(filter_order, cutoff_frequency, sampling_rate);

    // Criando um sinal de teste (exemplo: um pulso)
    std::vector<double> input_signal(100, 0.0);
    input_signal[50] = 1.0; // Pulso no meio do sinal

    // Aplicando o filtro FIR ao sinal
    std::vector<double> filtered_signal = apply_fir_filter(input_signal, fir_coefficients);

    // Exibir saída filtrada
    std::cout << "Sinal Filtrado:\n";
    for (size_t i = 0; i < filtered_signal.size(); i++) {
        std::cout << filtered_signal[i] << "\n";
    }

    return 0;
}

// Run
// g++ -o example1 example1.cpp -std=c++11
// ./example1