import numpy as np
import matplotlib.pyplot as plt

# Carregar os dados da FFT
fft_original = np.loadtxt("../media/fft_original.dat")
fft_processed = np.loadtxt("../media/fft_processed.dat")

# Criar os gráficos
plt.figure(figsize=(12, 6))

plt.subplot(2, 1, 1)
plt.plot(fft_original[:len(fft_original)//2])
plt.title("Espectro de Frequência Antes da Redução")
plt.xlabel("Frequência (Hz)")
plt.ylabel("Magnitude")

plt.subplot(2, 1, 2)
plt.plot(fft_processed[:len(fft_processed)//2], color='r')
plt.title("Espectro de Frequência Após Redução")
plt.xlabel("Frequência (Hz)")
plt.ylabel("Magnitude")

plt.tight_layout()
plt.show()
