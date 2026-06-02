#pragma once

#include <beshell/NativeClass.hpp>
#include "../../../../dep/mongoose.h"

namespace be::mg {
    class HTTPServer: public ::be::NativeClass {
        DECLARE_NCLASS_META
    private:
        static std::vector<JSCFunctionListEntry> methods ;

        struct mg_connection * conn ;
        JSValue callback ;
        bool telweb: 1 ;
        bool ssl: 1;
        void * ssl_cert_path;
        void * ssl_key_path;
        
        std::string cert_path ;
        std::string certkey_path ;

        static void eventHandler(struct mg_connection * conn, int ev, void *ev_data) ;

    public:
        HTTPServer(JSContext * ctx, struct mg_connection * conn, JSValue callback=JS_NULL) ;
        ~HTTPServer() ;
        
        static JSValue close(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) ;
        static JSValue startTelweb(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) ;
        static JSValue stopTelweb(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) ;
        static JSValue setHandler(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) ;
        
        static JSValue listenHttp(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) ;
    } ;
    
    // 为了保持向后兼容，保留 Server 作为 HTTPServer 的别名
    typedef HTTPServer Server ;
} 