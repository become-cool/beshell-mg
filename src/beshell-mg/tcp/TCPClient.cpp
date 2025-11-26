#include "TCPClient.hpp"
#include "../Mg.hpp"

using namespace std ;

namespace be::mg {
    DEFINE_NCLASS_META(TCPClient, EventEmitter)
    std::vector<JSCFunctionListEntry> TCPClient::methods = {
        JS_CFUNC_DEF("send", 0, TCPClient::send),
        JS_CFUNC_DEF("connect", 0, TCPClient::connect),
        JS_CFUNC_DEF("disconnect", 0, TCPClient::disconnect),
    } ;

    TCPClient::TCPClient(JSContext * ctx, struct mg_connection * conn, JSValue callback)
        : EventEmitter(ctx,build(ctx,JS_NULL))
        , callback(JS_DupValue(ctx,callback))
        , conn(conn)
    {
        if(conn) {
            conn->fn_data = this ;
        }
    }
    
    TCPClient::~TCPClient(){
        if(conn) {
            if( conn->fn_data == this ){
                conn->fn_data = nullptr ;
            }
        }
        JS_FreeValue(ctx, callback) ;
        callback = JS_NULL ;
    }
    
    void TCPClient::eventHandler(struct mg_connection * conn, int ev, void * ev_data) {

        
        if(!conn->fn_data) {
            return ;
        }
        TCPClient * client = (TCPClient *)conn->fn_data ;

        // if(MG_EV_POLL!=ev){
        //     ds(Mg::eventName(ev))
        // }

        switch (ev)
        {
            case MG_EV_OPEN :{
                client->_isConnected = true ;
                JSValue evname = JS_NewString(client->ctx, "open") ;
                JS_CALL_ARG1(client->ctx, client->callback, evname)
                JS_FreeValue(client->ctx, evname) ;
                break ;
            }
            case MG_EV_CONNECT :{
                client->_isConnected = true ;
                JSValue evname = JS_NewString(client->ctx, "connect") ;
                JS_CALL_ARG1(client->ctx, client->callback, evname)
                JS_FreeValue(client->ctx, evname) ;
                break ;
            }
            case MG_EV_READ:{
                if(conn->recv.len) {
                    JSValue evname = JS_NewString(client->ctx, "msg") ;
                    JSValue ab = JS_NewArrayBuffer(client->ctx, conn->recv.buf, conn->recv.len, nofreeArrayBuffer, nullptr, false) ;
                    JS_CALL_ARG2(client->ctx, client->callback, evname, ab)
                    JS_FreeValue(client->ctx, evname) ;
                    JS_FreeValue(client->ctx, ab) ;
                }
                break ;
            }
            case MG_EV_CLOSE:{
                client->_isConnected = false ;

                JSValue evname = JS_NewString(client->ctx, "close") ;
                JS_CALL_ARG1(client->ctx, client->callback, evname)
                JS_FreeValue(client->ctx, evname) ;

                // release hold for mg connection
                JS_FreeValue(client->ctx, client->jsobj) ;

                break ;
            }
            case MG_EV_ERROR:{
                client->_isConnected = false ;

                JSValue evname = JS_NewString(client->ctx, "error") ;
                JSValue msg = JS_NewString(client->ctx, (const char *)ev_data) ;
                JS_CALL_ARG2(client->ctx, client->callback, evname, msg)
                JS_FreeValue(client->ctx, evname) ;
                JS_FreeValue(client->ctx, msg) ;
                break ;
            }
        }
    }

    JSValue TCPClient::connect(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        
        ASSERT_ARGC(2)
        if( !JS_IsFunction(ctx, argv[1]) ) {
            JSTHROW("arg callback must be a function")
        }
        ARGV_TO_CSTRING_LEN_E(0, url, urlLen, "arg url must be a string")

        TCPClient * client = new TCPClient(ctx, nullptr, argv[1]) ;
        struct mg_connection * conn = mg_connect(&Mg::mgr, url, TCPClient::eventHandler, client) ;
        client->conn = conn ;

        if(conn==NULL) {
            JS_ThrowReferenceError(ctx, "could not connect to url: %s", url) ;
            JS_FreeCString(ctx, url) ;
            return JS_EXCEPTION ;
        }

        JS_FreeCString(ctx, url) ;

        // hold for mg connection
        JS_DupValue(ctx, client->jsobj) ;

        return JS_DupValue(ctx, client->jsobj) ;
    }
    
    JSValue TCPClient::disconnect(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        THIS_NCLASS(TCPClient,client)
        if(!client->conn) {
            JSTHROW("not connected")
        }
        client->conn->is_closing = true ;
        return JS_UNDEFINED ;
    }

    JSValue TCPClient::send(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        ASSERT_ARGC(1)
        THIS_NCLASS(TCPClient, client)

        if(!client->conn) {
            JSTHROW("not connected")
        }

        bool res = false ;

        if( JS_IsArrayBuffer(argv[0]) ) {
            size_t datalen = 0;
            uint8_t * data = (uint8_t *)JS_GetArrayBuffer(ctx, &datalen, argv[0]) ;
            if(!data) {
                JSTHROW("Failed to get ArrayBuffer data")
            }
            res = mg_send(client->conn, data, datalen) ;
        }
        else {
            ARGV_TO_CSTRING_LEN_E(0, data, len, "arg data must be a string")
            res = mg_send(client->conn, data, len) ;
            JS_FreeCString(ctx, data) ;
        }

        return res? JS_TRUE : JS_FALSE ;
    }
}