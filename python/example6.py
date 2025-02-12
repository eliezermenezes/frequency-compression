import numpy as np
import matplotlib.pyplot as plt
import soundfile as sf
import scipy.signal as signal

# Carregar um arquivo de áudio (Substitua pelo seu arquivo .wav)
filename = "../media/audio.wav"
audio, fs = sf.read(filename)

# Verificar se o áudio é estéreo e converter para mono
if audio.ndim > 1:
    audio = np.mean(audio, axis=1)

# Definir a frequência de deslocamento
f_shift = 3000  # Hz

# Criar um vetor de tempo para multiplicação
t = np.arange(len(audio)) / fs
audio_shifted = audio * np.exp(1j * 2 * np.pi * f_shift * t)

# Converter de volta para o domínio do tempo (somente parte real)
audio_shifted_real = np.real(audio_shifted)

# Salvar o áudio deslocado
sf.write("../media/audio_downsampled.wav", audio_shifted_real, fs)

# Plotar espectro antes e depois
freqs, X_f = signal.welch(audio, fs, nperseg=1024)
freqs_shifted, X_f_shifted = signal.welch(audio_shifted_real, fs, nperseg=1024)

plt.figure(figsize=(12, 6))

plt.subplot(2, 1, 1)
plt.semilogy(freqs, X_f)
plt.title("Espectro do Áudio Original")
plt.xlabel("Frequência (Hz)")
plt.ylabel("Magnitude")
plt.grid()

plt.subplot(2, 1, 2)
plt.semilogy(freqs_shifted, X_f_shifted)
plt.title("Espectro do Áudio Deslocado")
plt.xlabel("Frequência (Hz)")
plt.ylabel("Magnitude")
plt.grid()

plt.tight_layout()
plt.show()

print("Áudio deslocado salvo como 'audio_downsampled.wav'.")
