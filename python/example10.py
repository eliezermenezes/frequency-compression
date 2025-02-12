import numpy as np
import sounddevice as sd
from scipy.signal import firwin, lfilter
import tkinter as tk
from tkinter import ttk
import time

# Configurações de áudio
fs = 44100  # Frequência de amostragem
block_size = 2048  # Tamanho do bloco
f_cutoff = 1000  # Frequência de corte do filtro passa-baixas (em Hz)

# Variáveis globais para controle de amplitude (A) e frequência de modulação (f_mod)
A = 1.0  # Amplitude inicial
f_mod = 500  # Frequência de modulação inicial (em Hz)

# Função de deslocamento de frequência com modulação
def frequency_shift(input_signal, fs, A, f_mod, t):
    # Calcula a FFT do sinal de entrada
    n = len(input_signal)
    fft_signal = np.fft.fft(input_signal)

    # Frequência de modulação baseada no tempo
    shift_bins = int(A * f_mod * np.sin(2 * np.pi * f_mod * t) * n / fs)

    # Desloca a frequência movendo a FFT
    fft_signal_shifted = np.roll(fft_signal, shift_bins)

    # Retorna o sinal deslocado de volta ao domínio do tempo
    return np.fft.ifft(fft_signal_shifted).real

# Filtro passa-baixas FIR de ordem 3
def lowpass_filter(data, cutoff, fs, order=3):
    nyquist = 0.5 * fs
    normal_cutoff = cutoff / nyquist
    b = firwin(order + 1, normal_cutoff)
    return lfilter(b, 1.0, data)

# Função de callback para processamento em tempo real
def audio_callback(indata, outdata, frames, time, status):
    if status:
        print(status)

    global A, f_mod, last_time
    current_time = time.inputBufferAdcTime

    # Deslocamento de frequência com modulação (no domínio da frequência)
    audio_shifted = frequency_shift(indata[:, 0], fs, A, f_mod, current_time - last_time)

    # Filtro passa-baixas para evitar aliasing
    audio_filtered = lowpass_filter(audio_shifted, f_cutoff, fs)

    # Passa o áudio processado para a saída
    outdata[:] = audio_filtered.reshape(-1, 1)

    last_time = current_time  # Atualiza o tempo para o próximo callback

# Função para atualizar os parâmetros A e f_mod via interface
def update_parameters(val_A, val_f_mod):
    global A, f_mod
    A = float(val_A)
    f_mod = float(val_f_mod)
    print(f"Atualizando: A = {A}, f_mod = {f_mod}")

# Interface gráfica com Tkinter
root = tk.Tk()
root.title("Controle de Deslocamento de Frequência")

# Criação de controle deslizante para Frequência de Modulação (f_mod)
label_f_mod = ttk.Label(root, text="Frequência de Modulação (f_mod)")
label_f_mod.grid(row=1, column=0)

slider_f_mod = ttk.Scale(root, from_=50.0, to_=2000.0, orient="horizontal")
slider_f_mod.set(f_mod)  # Valor inicial
slider_f_mod.grid(row=1, column=1)

# Criação de controle deslizante para Amplitude (A)
label_A = ttk.Label(root, text="Amplitude (A)")
label_A.grid(row=0, column=0)

slider_A = ttk.Scale(root, from_=0.0, to_=10.0, orient="horizontal")
slider_A.set(A)  # Valor inicial
slider_A.grid(row=0, column=1)

# Atualiza os parâmetros A e f_mod quando os sliders são alterados
slider_A.bind("<Motion>", lambda event: update_parameters(slider_A.get(), slider_f_mod.get()))
slider_f_mod.bind("<Motion>", lambda event: update_parameters(slider_A.get(), slider_f_mod.get()))

# Inicia o stream de áudio
last_time = 0  # Variável para armazenar o tempo da última execução
with sd.Stream(callback=audio_callback, samplerate=fs, blocksize=block_size, dtype=np.float32, channels=1, latency='low'):
    print("🎤 Iniciando captura e reprodução com controle em tempo real...")
    # Rodando a interface gráfica
    root.mainloop()
