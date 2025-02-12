import numpy as np
import matplotlib.pyplot as plt
import soundfile as sf
import scipy.signal as signal

# Carregar arquivo de áudio
filename = "../media/audio.wav"
audio, fs = sf.read(filename)

# Converter para mono, se necessário
if audio.ndim > 1:
    audio = np.mean(audio, axis=1)

# Criar vetor de tempo
t = np.arange(len(audio)) / fs

# Parâmetros do deslocamento dinâmico
A = 2000    # Amplitude do deslocamento (Hz)
f_mod = 2   # Frequência da modulação (Hz)
f_cutoff = fs // 4  # Freq. de corte para evitar aliasing

# Criar a frequência variável ao longo do tempo
f_shift_t = A * np.sin(2 * np.pi * f_mod * t)

# Aplicar um filtro passa-baixas antes do deslocamento
def butter_lowpass_filter(data, cutoff, fs, order=6):
    nyquist = 0.5 * fs
    normal_cutoff = cutoff / nyquist
    b, a = signal.butter(order, normal_cutoff, btype='low', analog=False)
    return signal.filtfilt(b, a, data)

# Filtrar o sinal antes do deslocamento
audio_filtered = butter_lowpass_filter(audio, f_cutoff, fs)

# Aplicar deslocamento dinâmico
audio_shifted = audio_filtered * np.exp(1j * 2 * np.pi * np.cumsum(f_shift_t) / fs)

# Converter de volta para real
audio_shifted_real = np.real(audio_shifted)

# Aplicar filtro novamente após o deslocamento
audio_final = butter_lowpass_filter(audio_shifted_real, f_cutoff, fs)

# Salvar o áudio filtrado e deslocado
sf.write("../media/audio_downsampled_filtered.wav", audio_final, fs)

# Plotar espectros antes e depois
freqs, X_f_original = signal.welch(audio, fs, nperseg=1024)
freqs_filtered, X_f_filtered = signal.welch(audio_final, fs, nperseg=1024)

plt.figure(figsize=(12, 6))

plt.subplot(2, 1, 1)
plt.semilogy(freqs, X_f_original)
plt.title("Espectro do Áudio Original")
plt.xlabel("Frequência (Hz)")
plt.ylabel("Magnitude")
plt.grid()

plt.subplot(2, 1, 2)
plt.semilogy(freqs_filtered, X_f_filtered)
plt.title("Espectro do Áudio Após Deslocamento + Filtro")
plt.xlabel("Frequência (Hz)")
plt.ylabel("Magnitude")
plt.grid()

plt.tight_layout()
plt.show()

print("Áudio deslocado e filtrado salvo como 'audio_downsampled_filtered.wav'.")
