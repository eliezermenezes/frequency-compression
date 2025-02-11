import numpy as np
import scipy.signal as signal
import scipy.io.wavfile as wav

# Carregar arquivo de áudio
fs, data = wav.read("../media/temp_output.wav")  # fs = taxa de amostragem original

# Definir fator de rebaixamento
M = 2
fs_new = fs // M

# Aplicar filtro passa-baixa
nyquist = fs / 2
cutoff = nyquist / M  # Frequência de corte
b, a = signal.butter(4, cutoff / nyquist, btype='low')  # Filtro Butterworth
data_filtered = signal.filtfilt(b, a, data)

# Reamostrar descartando amostras
data_downsampled = data_filtered[::M]

# Salvar novo arquivo de áudio
wav.write("../media/audio_processado_output.wav", fs_new, data_downsampled.astype(np.int16))
