#pragma once

#include <beshell/NativeClass.hpp>
#include "../../../../dep/mongoose.h"

namespace be::mg {
    class HTTPResponse: public ::be::NativeClass {
        DECLARE_NCLASS_META
    private:
        static std::vector<JSCFunctionListEntry> methods ;
        struct mg_connection * conn = nullptr ;

    public:
        HTTPResponse(JSContext * ctx, struct mg_connection * conn) ;
    
        static JSValue wsSend(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) ;
        static JSValue httpUpgrade(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) ;
        static JSValue serveDir(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) ;
        static JSValue close(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) ;
        static JSValue redirect(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) ;
        static JSValue reply(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) ;

    } ;
    
    // 为了保持向后兼容，保留 Response 作为 HTTPResponse 的别名
    typedef HTTPResponse Response ;
}