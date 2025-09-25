#ifndef VERIFIC_STREAM_H
#define VERIFIC_STREAM_H

namespace Verific {

    class verific_stream {
    };

    class verific_ifstream : public verific_stream {
        public:
        verific_ifstream(const char *){
        }
    };

    class VerificStreamCallBackHandler {
        public:
        linefile_type GetFromLocation();
    };

}


#endif
