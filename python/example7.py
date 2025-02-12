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
A = 1000    # Amplitude do deslocamento (Hz)
f_mod = 2   # Frequência da modulação (Hz)

# Criar a frequência variável ao longo do tempo
f_shift_t = A * np.sin(2 * np.pi * f_mod * t)

# Aplicar deslocamento dinâmico ao áudio
audio_shifted = audio * np.exp(1j * 2 * np.pi * np.cumsum(f_shift_t) / fs)

# Converter de volta para real
audio_shifted_real = np.real(audio_shifted)

# Salvar o áudio deslocado dinamicamente
sf.write("../media/audio_downsampled_dynamic.wav", audio_shifted_real, fs)

# Plotar a variação da frequência ao longo do tempo
plt.figure(figsize=(10, 4))
plt.plot(t[:fs], f_shift_t[:fs])  # Mostrar apenas os primeiros segundos
plt.title("Variação da Frequência de Deslocamento ao Longo do Tempo")
plt.xlabel("Tempo (s)")
plt.ylabel("Frequência de Deslocamento (Hz)")
plt.grid()
plt.show()

print("Áudio deslocado salvo como 'audio_downsampled_dynamic.wav'.")
