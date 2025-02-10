// Capturar áudio do microfone e reproduzir em tempo real

#include <gst/gst.h>
#include <iostream>

int main(int argc, char *argv[]) {
    gst_init(&argc, &argv); // Inicializa o GStreamer

    // Criação do pipeline para capturar áudio e reamostrar
    GstElement *pipeline = gst_parse_launch(
        "autoaudiosrc ! audioconvert ! audioresample ! "
        "capsfilter caps=audio/x-raw,rate=16000 ! "
        "autoaudiosink", 
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

    std::cout << "Capturando e processando áudio em tempo real... Pressione Ctrl+C para sair." << std::endl;

    // Loop para manter a execução até o usuário interromper (Ctrl+C)
    GstBus *bus = gst_element_get_bus(pipeline);
    GstMessage *msg;
    while ((msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
                                             static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS)))) {
        gst_message_unref(msg);
        break;
    }

    // Libera recursos
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    gst_object_unref(bus);

    std::cout << "Processo finalizado." << std::endl;
    return 0;
}

// Example:
// gst-launch-1.0 autoaudiosrc ! audioconvert ! audioresample ! "audio/x-raw,rate=16000" ! autoaudiosink

// Run
// g++ -o example4 example4.cpp `pkg-config --cflags --libs gstreamer-1.0` -std=c++11
// ./example4