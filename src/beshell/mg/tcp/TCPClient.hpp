#pragma once

#include <beshell/EventEmitter.hpp>
#include "../../../dep/mongoose.h"

namespace be::mg {
    class TCPClient: public be::EventEmitter {
        DECLARE_NCLASS_META
    private:
        static std::vector<JSCFunctionListEntry> methods ;

        JSValue callback = JS_NULL ;
        struct mg_connection * conn = NULL ;
        bool _isConnected = false ;

        static void eventHandler(struct mg_connection * conn, int ev, void *ev_data) ;

    public:
        TCPClient(JSContext * ctx, struct mg_connection * conn, JSValue callback) ;
        ~TCPClient() ;

        static JSValue connect(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) ;
        static JSValue disconnect(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) ;
        static JSValue send(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) ;
    
    } ;
}