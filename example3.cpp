#include <gst/gst.h>
#include <iostream>

int main(int argc, char *argv[]) {
    gst_init(&argc, &argv); // Inicializa o GStreamer

    // Criação do pipeline para rebaixar a taxa de amostragem
    GstElement *pipeline = gst_parse_launch(
        "filesrc location=audio.wav ! decodebin ! "
        "audioconvert ! audioresample ! "
        "capsfilter caps=audio/x-raw,rate=16000 ! "
        "wavenc ! filesink location=audio_output_gst.wav", 
        NULL);

    if (!pipeline) {
        std::cerr << "Erro ao criar o pipeline GStreamer!" << std::endl;
        return -1;
    }

    // Inicia o pipeline
    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        std::cerr << "Erro ao executar o pipeline!" << std::endl;
        gst_object_unref(pipeline);
        return -1;
    }

    // Espera até o pipeline terminar o processamento
    GstBus *bus = gst_element_get_bus(pipeline);
    GstMessage *msg;
    while ((msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
                                             static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS)))) {
        gst_message_unref(msg);
        break;
    }

    // Libera os recursos do pipeline
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    gst_object_unref(bus);

    std::cout << "Processamento concluído! Arquivo salvo como output.wav" << std::endl;
    return 0;
}

// Example:
// gst-launch-1.0 filesrc location=audio.wav ! decodebin ! audioconvert ! audioresample ! "audio/x-raw, rate=16000" ! autoaudiosink

// Run
// g++ -o example3 example3.cpp `pkg-config --cflags --libs gstreamer-1.0` -std=c++11
// ./example3