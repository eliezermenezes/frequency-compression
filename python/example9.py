import numpy as np
import sounddevice as sd
from scipy.signal import firwin, lfilter
import time

# Configurações de áudio
fs = 44100  # Frequência de amostragem
block_size = 4096  # Tamanho do bloco (ajustado para evitar underflow)
f_cutoff = 1000  # Frequência de corte do filtro passa-baixas (em Hz)
f_shift = 500  # Deslocamento de frequência desejado (em Hz)

# Função de deslocamento de frequência utilizando FFT
def frequency_shift(input_signal, fs, f_shift):
    # Calcula a FFT do sinal de entrada
    n = len(input_signal)
    fft_signal = np.fft.fft(input_signal)

    # Frequência de deslocamento no domínio da frequência
    shift_bins = int(f_shift * n / fs)

    # Desloca a frequência movendo a FFT
    fft_signal_shifted = np.roll(fft_signal, shift_bins)

    # Retorna o sinal deslocado de volta ao domínio do tempo
    return np.fft.ifft(fft_signal_shifted).real

# Filtro passa-baixas FIR de ordem 3 (mais rápido)
def lowpass_filter(data, cutoff, fs, order=3):
    nyquist = 0.5 * fs
    normal_cutoff = cutoff / nyquist
    b = firwin(order + 1, normal_cutoff)
    return lfilter(b, 1.0, data)

# Função de callback para processamento em tempo real
def audio_callback(indata, outdata, frames, time, status):
    if status:
        print(status)

    # Deslocamento de frequência (no domínio da frequência)
    audio_shifted = frequency_shift(indata[:, 0], fs, f_shift)

    # Filtro passa-baixas para evitar aliasing
    audio_filtered = lowpass_filter(audio_shifted, f_cutoff, fs)

    # Passa o áudio processado para a saída
    outdata[:] = audio_filtered.reshape(-1, 1)

# Inicia o stream de áudio
with sd.Stream(callback=audio_callback, samplerate=fs, blocksize=block_size, dtype=np.float32, channels=1, latency='low'):
    print("🎤 Iniciando captura e reprodução com deslocamento de frequência...")
    time.sleep(10)  # Teste por 10 segundos
