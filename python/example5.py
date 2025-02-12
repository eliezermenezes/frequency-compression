import numpy as np
import matplotlib.pyplot as plt

# Parâmetros do sinal
fs = 1000  # Taxa de amostragem (Hz)
T = 1      # Duração (s)
t = np.linspace(0, T, fs, endpoint=False)  # Vetor de tempo

# Criando um sinal senoidal com frequência de 50 Hz
f_original = 50
x = np.sin(2 * np.pi * f_original * t)

# Aplicando a Transformada de Fourier para visualizar o espectro
X_f = np.fft.fftshift(np.fft.fft(x))
freqs = np.fft.fftshift(np.fft.fftfreq(len(t), 1/fs))

# Deslocamento de frequência: multiplicação por e^(j2πf0t)
f_shift = 1000  # Frequência de deslocamento (Hz)
x_shifted = x * np.exp(1j * 2 * np.pi * f_shift * t)

# Aplicando a FFT ao sinal deslocado
X_f_shifted = np.fft.fftshift(np.fft.fft(x_shifted))

# Plotando os espectros
plt.figure(figsize=(12, 6))

plt.subplot(2, 1, 1)
plt.plot(freqs, np.abs(X_f))
plt.title("Espectro do Sinal Original")
plt.xlabel("Frequência (Hz)")
plt.ylabel("Magnitude")
plt.grid()

plt.subplot(2, 1, 2)
plt.plot(freqs, np.abs(X_f_shifted))
plt.title("Espectro do Sinal Deslocado")
plt.xlabel("Frequência (Hz)")
plt.ylabel("Magnitude")
plt.grid()

plt.tight_layout()
plt.show()
