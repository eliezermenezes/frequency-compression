import numpy as np
import matplotlib.pyplot as plt

# Carregar os arquivos RAW
original = np.fromfile("../media/audio_original.raw", dtype=np.int16)
processado = np.fromfile("../media/audio_processado.raw", dtype=np.int16)

# Criar os gráficos
plt.figure(figsize=(12, 5))
plt.subplot(2, 1, 1)
plt.plot(original[:500], label="Original (44100 Hz)")
plt.legend()
plt.subplot(2, 1, 2)
plt.plot(processado[:500], label="Processado (16000 Hz)", color='r')
plt.legend()
plt.show()
