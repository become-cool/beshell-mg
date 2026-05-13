#include <beshell/BeShell.class.hpp>
#include "./HTTPClient.hpp"
#include "./HTTPRequest.hpp"
#include "../Mg.hpp"
// #include "qjs_utils.h"

using namespace std ;


namespace be::mg {
    DEFINE_NCLASS_META(HTTPClient, EventEmitter)
    std::vector<JSCFunctionListEntry> HTTPClient::methods = {
        JS_CFUNC_DEF("send", 0, HTTPClient::send),
        JS_CFUNC_DEF("close", 0, HTTPClient::close),
        JS_CFUNC_DEF("disconnect", 0, HTTPClient::close),
        JS_CFUNC_DEF("isConnected", 0, HTTPClient::isConnected),
        JS_CFUNC_DEF("enableChunkEvent", 0, HTTPClient::enableChunkEvent),
        JS_CFUNC_DEF("setClientKey", 0, HTTPClient::setClientKey),
        JS_CFUNC_DEF("enableClientAuth", 0, HTTPClient::enableClientAuth),
        JS_CFUNC_DEF("disableClientAuth", 0, HTTPClient::disableClientAuth),
    } ;

    HTTPClient::HTTPClient(JSContext * ctx, struct mg_connection * conn, JSValue callback)
        : EventEmitter(ctx,build(ctx))
        , callback(JS_DupValue(ctx,callback))
        , conn(conn)
    {
        if(conn) {
            conn->fn_data = this ;
        }
        _isConnected = conn ? conn->is_listening : false ;
    }
    
    HTTPClientHandler HTTPClient::handler = nullptr ;

    HTTPClient::~HTTPClient(){
        if(conn) {
            if( conn->fn_data == this ){
                conn->fn_data = nullptr ;
            }
        }
        JS_FreeValue(ctx, callback) ;
        callback = JS_NULL ;
    }
    
    void HTTPClient::setConn(struct mg_connection * conn){
        this->conn = conn ;
    }

    /**
     * 发送数据
     * 
     * @module mg
     * @component beshell-mg
     * @class HTTPClient
     * @method send
     * @param data:string 数据
     * @return bool
     */
    JSValue HTTPClient::send(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        ASSERT_ARGC(1)
        THIS_NCLASS(HTTPClient,client)

        bool res = false ;

        // ws/wss
        if(client->_isWS) {
            if( JS_IsArrayBuffer(argv[0]) ){
                size_t size ;
                char * buff = (char *)JS_GetArrayBuffer(ctx, &size, argv[0]) ;
                res = mg_ws_send(client->conn, buff, size, WEBSOCKET_OP_BINARY);
            }
            else {
                ARGV_TO_CSTRING_LEN_E(0, data, len, "arg data must be a string")
                res = mg_ws_send(client->conn, data, len, WEBSOCKET_OP_TEXT);
                JS_FreeCString(client->ctx, data) ;
            }
        }

        // http/https
        else {
            if( JS_IsArrayBuffer(argv[0]) ){
                size_t datalen = 0;
                uint8_t * data = (uint8_t *)JS_GetArrayBuffer(ctx, &datalen, argv[0]) ;
                res = mg_send(client->conn, data, datalen) ;
            }
            else {
                ARGV_TO_CSTRING_LEN_E(0, data, len, "arg data must be a string")
                res = mg_send(client->conn, data, len) ;
                JS_FreeCString(ctx, data) ;
            }
        }

        return res? JS_TRUE : JS_FALSE;
    }

    /**
     * 关闭连接
     * 
     * @module mg
     * @component beshell-mg
     * @class HTTPClient
     * @method close
     * @return undefined
     */
    JSValue HTTPClient::close(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        THIS_NCLASS(HTTPClient,client)
        if(!client->conn) {
            JSTHROW("not connected")
        }
        client->conn->is_closing = true ;
        return JS_UNDEFINED ;
    }


    // enum {
    // MG_EV_ERROR,      // Error                        char *error_message
    // MG_EV_OPEN,       // Connection created           NULL
    // MG_EV_POLL,       // mg_mgr_poll iteration        uint64_t *uptime_millis
    // MG_EV_RESOLVE,    // Host name is resolved        NULL
    // MG_EV_CONNECT,    // Connection established       NULL
    // MG_EV_ACCEPT,     // Connection accepted          NULL
    // MG_EV_TLS_HS,     // TLS handshake succeeded      NULL
    // MG_EV_READ(7),       // Data received from socket    long *bytes_read
    // MG_EV_WRITE,      // Data written to socket       long *bytes_written
    // MG_EV_CLOSE,      // Connection closed            NULL
    // MG_EV_HTTP_HDRS(10),  // HTTP headers                 struct mg_http_message *
    // MG_EV_HTTP_MSG,   // Full HTTP request/response   struct mg_http_message *
    // MG_EV_WS_OPEN,    // Websocket handshake done     struct mg_http_message *
    // MG_EV_WS_MSG,     // Websocket msg, text or bin   struct mg_ws_message *
    // MG_EV_WS_CTL,     // Websocket control msg        struct mg_ws_message *
    // MG_EV_MQTT_CMD,   // MQTT low-level command       struct mg_mqtt_message *
    // MG_EV_MQTT_MSG,   // MQTT PUBLISH received        struct mg_mqtt_message *
    // MG_EV_MQTT_OPEN,  // MQTT CONNACK received        int *connack_status_code
    // MG_EV_SNTP_TIME,  // SNTP time received           uint64_t *epoch_millis
    // MG_EV_WAKEUP,     // mg_wakeup() data received    struct mg_str *data
    // MG_EV_USER        // Starting ID for user events
    // };
    void HTTPClient::eventHandler(struct mg_connection * conn, int ev, void * ev_data) {
        
        // if(ev!=MG_EV_POLL) {
        //     printf("Client::eventHandler() event: %d\n", ev) ;
        // }

        if(!conn->fn_data) {
            return ;
        }
        HTTPClient * client = (HTTPClient *)conn->fn_data ;
        
        if(handler && client && handler(client, conn, ev, ev_data, conn->fn_data)) {
            return ;
        }

        switch(ev) {

            case MG_EV_CONNECT: {

                if(client && client->_isTLS && Mg::ca.length()>0) {
                    struct mg_tls_opts opts = {
                        .ca = mg_str(Mg::ca.c_str()),
                        .name = mg_str(client->_host.c_str())
                    };

                    // 双向认证证书
                    if(client->useClientCert){
                        opts.cert = mg_str(client->clientCert.c_str());
                        opts.key = mg_str(client->clientKey.c_str());
                    }

                    mg_tls_init(conn, &opts);
                }

                client->_isConnected = true ;

                JSValue evname = JS_NewString(client->ctx, Mg::eventName(ev)) ;
                JS_CALL_ARG1(client->ctx, client->callback, evname)
                JS_FreeValue(client->ctx, evname) ;
                break ;
            }

            case MG_EV_READ: {
                if(!client->_enableChunkEvent || !client->headerLength){
                    break ;
                }
                long read_len = *(long *)ev_data ;
                
                if(read_len>conn->recv.size) {
                    printf("recv buf overflow %ld > %d\n", read_len, conn->recv.size) ;
                    break ;
                }

                // printf("[headerLength=%d,recv.len=%d, n=%d]"
                //     // "----------\n%.*s---------------------"
                //     "\n", client->headerLength, conn->recv.len, read_len
                //     // , conn->recv.len, (char *)conn->recv.buf
                // ) ;

                if(conn->recv.len) {
                    JSValue evname = JS_NewString(client->ctx, "http.chunk") ;
                    JSValue ab = JS_NewArrayBuffer(client->ctx, conn->recv.buf, conn->recv.len, nofreeArrayBuffer, nullptr, false) ;
                    JS_CALL_ARG2(client->ctx, client->callback, evname, ab)
                    JS_FreeValue(client->ctx, evname) ;
                    JS_FreeValue(client->ctx, ab) ;
                }
                else {
                    // printf("chunk len 0?\n") ;
                    // abort() ;
                }
                conn->recv.len = 0;
                
                break;
            }

            case MG_EV_HTTP_HDRS: {
                if( client->_enableChunkEvent ){

                    // MG_EV_HTTP_HDRS 会重复触发
                    if(client->headerLength) {
                        break ;
                    }
                    
                    const char *header_end = strnstr((char *)conn->recv.buf, "\r\n\r\n", conn->recv.len) ;
                    if(header_end) {

                        client->headerLength = header_end - (char *)conn->recv.buf + 4 ; // 4 = "\r\n\r\n" 的长度
                        // dn2(client->headerLength, conn->recv.len)
                        // printf("%.*s", conn->recv.len, (char *)conn->recv.buf) ;

                        // http.head 事件
                        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
                        HTTPRequest * req = new HTTPRequest(client->ctx, hm) ;
                        req->shared() ; // 智能回收模式 (jsobj gc 回收时自动 delete req)

                        JSValue evname = JS_NewString(client->ctx, "http.head") ;
                        JS_CALL_ARG2(client->ctx, client->callback, evname, req->jsobj)
                        JS_FreeValue(client->ctx, evname) ;
                        JS_FreeValue(client->ctx, req->jsobj) ;

                        // http.chunk 事件
                        if( conn->recv.len > client->headerLength ){

                            uint8_t * chunk = conn->recv.buf + client->headerLength ;
                            JSValue evname = JS_NewString(client->ctx, "http.chunk") ;

                            // 使用 nofreeArrayBuffer 避免数据拷贝，回调函数里不应该保留这个 ArrayBuffer 的引用
                            JSValue ab = JS_NewArrayBuffer(client->ctx, chunk, conn->recv.len-client->headerLength, nofreeArrayBuffer, nullptr, false) ;
                            JS_CALL_ARG2(client->ctx, client->callback, evname, ab)
                            JS_FreeValue(client->ctx, evname) ;
                            JS_FreeValue(client->ctx, ab) ;
                        }
                        
                        conn->recv.len = 0 ;
                    }
                    else{
                        printf("header not end ???\n") ;
                    }
                }
            }
            
            case MG_EV_HTTP_MSG: 
            {
                // 如果是 chunked 的数据，已经在 MG_EV_READ 事件中处理了
                if(client->_enableChunkEvent) {
                    break ;
                }

                // moogose https 协议，会在 close 事件以后触发 msg 事件
                if(!client || !client->ctx) {
                    break ;
                }

                struct mg_http_message *hm = (struct mg_http_message *) ev_data;
                HTTPRequest * req = new HTTPRequest(client->ctx, hm) ;
                req->shared() ; // 智能回收模式 (jsobj gc 回收时自动 delete req)

                JSValue evname = JS_NewString(client->ctx, Mg::eventName(ev)) ;

                JS_CALL_ARG2(client->ctx, client->callback, evname, req->jsobj)

                JS_FreeValue(client->ctx, req->jsobj) ;
                JS_FreeValue(client->ctx, evname) ;

                // mg_http_delete_chunk(conn, hm) ;

                break ;
            }

            case MG_EV_CLOSE : {
                
                client->_isConnected = false ;

                JSValue evname = JS_NewString(client->ctx, "close") ;
                JS_CALL_ARG1(client->ctx, client->callback, evname)
                JS_FreeValue(client->ctx, evname) ;
                
                // moogose https 协议，会在 close 事件以后触发 msg 事件
                if(conn->fn_data==client) {
                    conn->fn_data = NULL ;

                    delete client ;
                    client = NULL ;
                    conn->fn_data = NULL ;
                }
                break ;
            }

            case MG_EV_ERROR:
            
                client->_isConnected = false ;

                if(ev_data) {
                    JSValue evname = JS_NewString(client->ctx, Mg::eventName(ev)) ;
                    JSValue msg = JS_NewString(client->ctx, (const char *)ev_data) ;
                    JS_CALL_ARG2(client->ctx, client->callback, evname, msg)
                    JS_FreeValue(client->ctx, evname) ;
                    JS_FreeValue(client->ctx, msg) ;
                    break ;
                }

            default: {
                JSValue evname = JS_NewString(client->ctx, Mg::eventName(ev)) ;
                JS_CALL_ARG1(client->ctx, client->callback, evname)
                JS_FreeValue(client->ctx, evname) ;
                break ;
            }
        }
    }

    /**
     * 启用分块传输事件
     * 
     * 启用后，客户端会触发 `http.head` 和 `http.chunk` 事件，用于处理大文件下载。
     * 
     * @module mg
     * @component beshell-mg
     * @class HTTPClient
     * @method enableChunkEvent
     * @return undefined
     */
    JSValue HTTPClient::enableChunkEvent(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        THIS_NCLASS(HTTPClient,client)
        client->_enableChunkEvent = true ;
        return JS_UNDEFINED ;
    }

    /**
     * 检查客户端是否已连接
     * 
     * @module mg
     * @component beshell-mg
     * @class HTTPClient
     * @method isConnected
     * @return bool 已连接返回 true，否则返回 false
     */
    JSValue HTTPClient::isConnected(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        THIS_NCLASS(HTTPClient,client)
        return client->_isConnected? JS_TRUE: JS_FALSE ;
    }

    /**
     * 创建一个 HTTP/WebSocket 客户端连接
     * 
     * 支持 `http://`, `https://`, `ws://`, `wss://` 协议。
     * 
     * 回调函数原型：
     *     callback(event:string, request:[HTTPRequest](HTTPRequest.html)): void
     * 
     * 事件类型：
     * - `connect`: 连接成功
     * - `http.msg`: 收到 HTTP 响应
     * - `http.head`: 收到 HTTP 头（启用分块传输时）
     * - `http.chunk`: 收到数据块（启用分块传输时）
     * - `close`: 连接关闭
     * - `error`: 发生错误
     * 
     * @module mg
     * @component beshell-mg
     * @class HTTPClient
     * @method connect
     * @param url:string 连接地址，例如 `"http://www.example.com/path"` 或 `"wss://ws.example.com"`
     * @param callback:function 事件回调函数
     * @return [HTTPClient](HTTPClient.html) 返回 HTTPClient 实例
     */
    JSValue HTTPClient::connect(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {

        ASSERT_ARGC(2)
        if( !JS_IsFunction(ctx, argv[1]) ) {
            JSTHROW("arg callback must be a function")
        }
        ARGV_TO_CSTRING_LEN_E(0, url, urlLen, "arg url must be a string")

        HTTPClient * client = new HTTPClient(ctx, nullptr, argv[1]) ;
        struct mg_connection * conn = NULL ;
        if ( strncmp(url,"http://", 7)==0 || strncmp(url,"https://", 8)==0 ) {
            conn = mg_http_connect(&Mg::mgr, url, HTTPClient::eventHandler, client) ;
            client->_isWS = false ;

            if(url[4]=='s') {
                client->_isTLS = true ;
            }
        }

        else if( strncmp(url,"ws://", 5)==0 || strncmp(url,"wss://", 6)==0 ) {
            conn = mg_ws_connect(&Mg::mgr, url, HTTPClient::wsEventHandler, client, NULL) ;
            client->_isWS = true ;
            
            if(url[2]=='s') {
                client->_isTLS = true ;
            }
        }

        else {
            delete client ;
            JS_FreeCString(ctx, url) ;
            JSTHROW("not support url protocol")
        }

        if(client->_isTLS) {
            struct mg_str host = mg_url_host(url) ;
            client->_host = string(host.buf, host.len) ;
        }

        client->conn = conn ;
        if(conn==NULL) {
            JS_ThrowReferenceError(ctx, "could not connect to url: %s", url) ;
            JS_FreeCString(ctx, url) ;
            return JS_EXCEPTION ;
        }

        JS_FreeCString(ctx, url) ;
        
        return JS_DupValue(ctx, client->jsobj) ;
    }
    /**
     * 设置客户端证书和私钥（用于双向 TLS 认证）
     * 
     * @module mg
     * @component beshell-mg
     * @class HTTPClient
     * @method setClientKey
     * @param key:string 客户端私钥（PEM 格式）
     * @param cert:string 客户端证书（PEM 格式）
     * @return undefined
     */
    JSValue HTTPClient::setClientKey(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        THIS_NCLASS(HTTPClient, that)
        CHECK_ARGC(2)
        ARGV_TO_STRING(0, that->clientKey)
        ARGV_TO_STRING(1, that->clientCert)
        return JS_UNDEFINED ;
    }
    /**
     * 启用双向 TLS 认证
     * 
     * 启用后，客户端会在 TLS 握手时发送证书。
     * 
     * @module mg
     * @component beshell-mg
     * @class HTTPClient
     * @method enableClientAuth
     * @return undefined
     */
    JSValue HTTPClient::enableClientAuth(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        THIS_NCLASS(HTTPClient, that)
        that->useClientCert = true ;
        return JS_UNDEFINED ;
    }
    /**
     * 禁用双向 TLS 认证
     * 
     * @module mg
     * @component beshell-mg
     * @class HTTPClient
     * @method disableClientAuth
     * @return undefined
     */
    JSValue HTTPClient::disableClientAuth(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        THIS_NCLASS(HTTPClient, that)
        that->useClientCert = false ;
        return JS_UNDEFINED ;
    }

    void HTTPClient::setHandler(HTTPClientHandler _handler) {
        handler = _handler ;
    }

    
    bool HTTPClient::isTLS() const {
        return _isTLS ;
    }
    bool HTTPClient::isWS() const {
        return _isWS ;
    }
    std::string HTTPClient::host() const {
        return _host ;
    }
}
