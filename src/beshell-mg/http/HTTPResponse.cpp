/**
 * HTTPResponse 类用于 mg.HTTPServer 回调函数的参数 rspn , 不需要创建该类: 
 * 
 * * [mg.listenHttp()](../mg/#%E5%87%BD%E6%95%B0-listenhttp)
 * 
 * * [mg.HTTPServer.setHandler](HTTPServer.html#%E6%96%B9%E6%B3%95-sethandler)
 * 
 * 
 * 
 * @module mg
 * @component beshell-mg
 * @class HTTPResponse
 */

#include "./HTTPResponse.hpp"
#include "./HTTPRequest.hpp"
#include <beshell/mallocf.h>
#include <beshell/fs/FS.hpp>

using namespace std ;



#define NOT_WS_FUNC(methodName)                                                 \
    if(rspn->conn->is_websocket) {                                              \
        JSTHROW(methodName"() do not invoke with a websocket connection.")      \
    }

#define MUST_BE_WS_FUNC(methodName)                                             \
    if(!rspn->conn->is_websocket) {                                             \
        JSTHROW(methodName"() must be invoked with a websocket connection.")    \
    }


namespace be::mg {
    DEFINE_NCLASS_META(HTTPResponse, NativeClass)
    std::vector<JSCFunctionListEntry> HTTPResponse::methods = {
        JS_CFUNC_DEF("wsSend", 0, HTTPResponse::wsSend),
        JS_CFUNC_DEF("httpUpgrade", 0, HTTPResponse::httpUpgrade),
        JS_CFUNC_DEF("serveDir", 0, HTTPResponse::serveDir),
        JS_CFUNC_DEF("close", 0, HTTPResponse::close),
        JS_CFUNC_DEF("redirect", 0, HTTPResponse::redirect),
        JS_CFUNC_DEF("reply", 0, HTTPResponse::reply),
    } ;

    HTTPResponse::HTTPResponse(JSContext * ctx, struct mg_connection * conn)
        : NativeClass(ctx,build(ctx))
        , conn(conn)
    {}

    /**
     * 回复内容
     * 
     * @module mg
     * @component beshell-mg
     * @class HTTPResponse
     * @method reply
     * @param content:string 回复内容
     * @param code:number=200 回复状态码, 默认为 200
     * @param header:string=null 回复头, 格式为 "key: value\r\n"
     * @return undefined
     */
    JSValue HTTPResponse::reply(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        ASSERT_ARGC(1)
        THIS_NCLASS(HTTPResponse, rspn)
        NOT_WS_FUNC("mg.HTTPResponse.reply")
        
        int32_t code = 200 ;
        if(argc>1) {
            if( JS_ToInt32(ctx, &code, argv[1])!=0 ){
                JSTHROW("arg code must be a number")
            }
        }

        if( JS_IsString(argv[0]) ) {
            const char * body = JS_ToCString(ctx, argv[0]) ;

            char * header = NULL ;
            if(argc>2){
                header = (char *)JS_ToCString(ctx, argv[2]) ;
            }

            mg_http_reply(rspn->conn, code, header, body) ;

            JS_FreeCString(ctx, body) ;
            if(header) {
                JS_FreeCString(ctx, header) ;
            }
        }

        else {
            JSTHROW("invalid type of arg body")
        }

        return JS_UNDEFINED ;
    }


    /**
     * 重定向请求
     * 
     * @module mg
     * @component beshell-mg
     * @class HTTPResponse
     * @method redirect
     * @param url:string 重定向链接地址
     * @return undefined
     */
    JSValue HTTPResponse::redirect(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        ASSERT_ARGC(1)
        THIS_NCLASS(HTTPResponse, rspn)
        const char * url = JS_ToCString(ctx, argv[0]) ;
        
        char * header = mallocf("Location: %s\r\n", (char*)url) ;
        if(!header) {
            JSTHROW("out of memory?") ;
        }

        mg_http_reply(rspn->conn, 302, header, "Redirection") ;

        JS_FreeCString(ctx, url) ;
        free(header) ;
        return JS_UNDEFINED ;
    }

    /**
     * 服务器主动关闭访问连接
     * 
     * @module mg
     * @component beshell-mg
     * @class HTTPResponse
     * @method close
     * @return undefined
     */
    JSValue HTTPResponse::close(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        THIS_NCLASS(HTTPResponse, rspn)
        rspn->conn->is_closing = 1 ;
        return JS_UNDEFINED ;
    }


    /**
     * 执行文件服务
     * 
     * 该方法用于实现静态文件服务器，用法请参考：[简单 HTTP Web 后端的例子](../../guide/http-server.html#_2-%E7%AE%80%E5%8D%95-http-web-%E5%90%8E%E7%AB%AF%E7%9A%84%E4%BE%8B%E5%AD%90)
     * 
     * @module mg
     * @component beshell-mg
     * @class HTTPResponse
     * @method serveDir
     * @param req:[HTTPRequest](HTTPRequest.md) 请求对象，将事件回调函数传入的 req 直接传递给 serveDir 即可
     * @param dir:string 文件根目录
     * @return undefined
     */
    JSValue HTTPResponse::serveDir(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        ASSERT_ARGC(2)
        THIS_NCLASS(HTTPResponse, rspn)
        NOT_WS_FUNC("mg.HTTPResponse.replay")

        JSVALUE_TO_NCLASS(HTTPRequest,argv[0],req)

        ARGV_TO_CSTRING_E(1, _path, "arg path must be a string")
        string path = FS::toVFSPath(_path) ;

        struct mg_http_serve_opts opts = {.root_dir = path.c_str()};
        mg_http_serve_dir(rspn->conn, req->opaque(), &opts);

        JS_FreeCString(ctx, _path) ;

        return JS_UNDEFINED ;
    }

    /**
     * http 协议升级
     * 
     * 将 http 请求升级为 WebSocket 通讯，用法请参考：[WebSocket 的例子](../../guide/http-server.html#_3-websocket-%E7%9A%84%E4%BE%8B%E5%AD%90)
     * 
     * @module mg
     * @component beshell-mg
     * @class HTTPResponse
     * @method httpUpgrade
     * @param req:[HTTPRequest](HTTPRequest.md) 请求对象，将事件回调函数传入的 req 直接传递给 serveDir 即可
     * @return undefined
     */
    JSValue HTTPResponse::httpUpgrade(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        THIS_NCLASS(HTTPResponse, rspn)
        ASSERT_ARGC(1)
        NOT_WS_FUNC("mg.HTTPResponse.upgrade")
        JSVALUE_TO_NCLASS(HTTPRequest,argv[0],req)
        struct mg_http_message * msg = req->opaque() ;
        if(!msg) {
            JS_ThrowReferenceError(ctx, "mg.HttpMessage object has free.");
            return JS_EXCEPTION ;
        }
        mg_ws_upgrade(rspn->conn, msg, NULL);
        return JS_UNDEFINED ;
    }

    /**
     * 发送 WebSocket 数据帧
     * 
     * 用法请参考：[WebSocket 的例子](../../guide/http-server.html#_3-websocket-%E7%9A%84%E4%BE%8B%E5%AD%90)
     * 
     * @module mg
     * @component beshell-mg
     * @class HTTPResponse
     * @method wsSend
     * @param data:string 数据帧内容
     * @return undefined
     */
    JSValue HTTPResponse::wsSend(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        ASSERT_ARGC(1)
        THIS_NCLASS(HTTPResponse, rspn)
        MUST_BE_WS_FUNC("mg.HTTPResponse.wsSend")

        if( JS_IsString(argv[0]) ){
            ARGV_TO_CSTRING_LEN(0, str, len)
            mg_ws_send(rspn->conn, str, len, WEBSOCKET_OP_TEXT) ;
            JS_FreeCString(ctx, str) ;
        }
        
        // if( JS_IsArray(argv[0]) ){

        // }
        // else {

        // }

        return JS_UNDEFINED ;
    }
}