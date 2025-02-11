#include <gst/gst.h>
#include <iostream>
#include <fstream>

// Arquivos para armazenar os dados
std::ofstream arquivo_original("media/audio_original.raw", std::ios::binary);
std::ofstream arquivo_processado("media/audio_processado.raw", std::ios::binary);

// Callback para capturar os buffers
static GstPadProbeReturn buffer_probe_callback(GstPad *pad, GstPadProbeInfo *info, gpointer user_data) {
    GstBuffer *buffer = GST_PAD_PROBE_INFO_BUFFER(info);
    if (!buffer) return GST_PAD_PROBE_OK;

    GstMapInfo map;
    if (gst_buffer_map(buffer, &map, GST_MAP_READ)) {
        std::string label = static_cast<char*>(user_data);
        std::cout << label << " - Tamanho do buffer: " << map.size << " bytes" << std::endl;

        // Exibir os primeiros 10 bytes para depuração
        std::cout << label << " - Dados: ";
        for (gsize i = 0; i < map.size && i < 10; i++) {
            std::cout << (int) map.data[i] << " ";
        }
        std::cout << std::endl;

        // Salvar os dados no arquivo correspondente
        if (label == "ORIGINAL") {
            arquivo_original.write(reinterpret_cast<const char*>(map.data), map.size);
        } else if (label == "PROCESSADO") {
            arquivo_processado.write(reinterpret_cast<const char*>(map.data), map.size);
        }

        gst_buffer_unmap(buffer, &map);
    }
    return GST_PAD_PROBE_OK;
}

int main(int argc, char *argv[]) {
    gst_init(&argc, &argv);

    // Criando o pipeline manualmente para acessar os pads
    GstElement *pipeline = gst_pipeline_new("audio_pipeline");
    GstElement *source = gst_element_factory_make("autoaudiosrc", "source");
    GstElement *convert = gst_element_factory_make("audioconvert", "convert");
    GstElement *resample = gst_element_factory_make("audioresample", "resample");
    GstElement *capsfilter = gst_element_factory_make("capsfilter", "capsfilter");
    GstElement *sink = gst_element_factory_make("autoaudiosink", "sink");

    if (!pipeline || !source || !convert || !resample || !capsfilter || !sink) {
        std::cerr << "Erro ao criar elementos do GStreamer!" << std::endl;
        return -1;
    }

    // Definir a nova taxa de amostragem para 160 Hz
    GstCaps *caps = gst_caps_new_simple("audio/x-raw", "rate", G_TYPE_INT, 160, nullptr);
    g_object_set(capsfilter, "caps", caps, nullptr);
    gst_caps_unref(caps);

    // Construir o pipeline
    gst_bin_add_many(GST_BIN(pipeline), source, convert, resample, capsfilter, sink, nullptr);
    if (!gst_element_link_many(source, convert, resample, capsfilter, sink, nullptr)) {
        std::cerr << "Erro ao conectar elementos do pipeline!" << std::endl;
        return -1;
    }

    // Adicionar probe antes da reamostragem (Áudio Original)
    GstPad *pad_original = gst_element_get_static_pad(convert, "src");
    if (pad_original) {
        gst_pad_add_probe(pad_original, GST_PAD_PROBE_TYPE_BUFFER, buffer_probe_callback, (gpointer) "ORIGINAL", nullptr);
        gst_object_unref(pad_original);
    }

    // Adicionar probe depois da reamostragem (Áudio Processado)
    GstPad *pad_processado = gst_element_get_static_pad(resample, "src");
    if (pad_processado) {
        gst_pad_add_probe(pad_processado, GST_PAD_PROBE_TYPE_BUFFER, buffer_probe_callback, (gpointer) "PROCESSADO", nullptr);
        gst_object_unref(pad_processado);
    }

    // Iniciar o pipeline
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    std::cout << "Capturando e processando áudio... Pressione Ctrl+C para parar." << std::endl;

    // Loop para manter o programa rodando
    GstBus *bus = gst_element_get_bus(pipeline);
    GstMessage *msg;
    while ((msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
                                             static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS)))) {
        gst_message_unref(msg);
        break;
    }

    // Finalizar
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(bus);
    gst_object_unref(pipeline);

    // Fechar os arquivos
    arquivo_original.close();
    arquivo_processado.close();

    std::cout << "Processamento finalizado. Arquivos gerados: audio_original.raw e audio_processado.raw" << std::endl;
    return 0;
}
