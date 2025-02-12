import numpy as np
import sounddevice as sd
import scipy.signal as signal

# Parâmetros
fs = 44100  # Taxa de amostragem (Hz)
block_size = 4096  # Tamanho do buffer de áudio
A = 2000  # Amplitude do deslocamento (Hz)
f_mod = 1  # Frequência de modulação (Hz)
f_cutoff = fs // 4  # Freq. de corte para filtro passa-baixas

# Filtro passa-baixas para evitar aliasing
def butter_lowpass_filter(data, cutoff, fs, order=6):
    nyquist = 0.5 * fs
    normal_cutoff = cutoff / nyquist
    b, a = signal.butter(order, normal_cutoff, btype='low', analog=False)
    return signal.filtfilt(b, a, data, axis=0)

# Inicializar fase acumulada para deslocamento contínuo
phase_acc = 0.0

# Callback para processar áudio em tempo real
def audio_callback(indata, outdata, frames, time, status):
    global phase_acc
    if status:
        print(status)

    # Converter para mono, se necessário
    audio_mono = np.mean(indata, axis=1)

    # Criar vetor de tempo para o bloco
    t = np.arange(len(audio_mono)) / fs

    # Criar deslocamento de frequência variável
    f_shift_t = A * np.sin(2 * np.pi * f_mod * (time.inputBufferAdcTime + t))

    # Atualizar fase acumulada para continuidade entre blocos
    phase_acc += np.cumsum(f_shift_t) / fs

    # Aplicar deslocamento de frequência
    audio_shifted = audio_mono * np.exp(1j * 2 * np.pi * phase_acc)

    # Converter para real
    audio_shifted_real = np.real(audio_shifted)

    # Aplicar filtro passa-baixas
    audio_final = butter_lowpass_filter(audio_shifted_real, f_cutoff, fs)

    # Copiar para buffer de saída
    outdata[:] = np.expand_dims(audio_final, axis=1)

# Configurar stream de áudio
with sd.Stream(callback=audio_callback, samplerate=fs, blocksize=block_size, dtype=np.float32, channels=1, latency='low'):
    print("🎤 Processando áudio em tempo real... Pressione Ctrl+C para parar.")
    while True:
        pass  # Loop infinito para manter a execução
