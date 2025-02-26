/* FujiNet web server

Server is built on the esp-idf webserer class in <esp_http_server.h>
The design of the class in this version of the esp-idf requires registering
URI handlers for every URI you expect to handle and does not support
wildcards. Because of this, we handle serving static files this way:

URI: "/" - Forces loading/parsing (see below) of /<FNWS_FILE_ROOT>/index.html
URI: "/file?<filename>" - Sends static file /<FNWS_FILE_ROOT>/<filename>
URI: "/favico.ico" - Sends /<FNWS_FILE_ROOT>/favico.ico
URI: "/print" - Sends current printer output to user

MIME types are assigned based on file extention.  See/update
    static std::map<string, string> mime_map

Unless parsable, files are sent in FNWS_SEND_BUFF_SIZE blocks.

If a file has an extention pre-determined to support parsing (see/update
    fnHttpServiceParser::is_parsable() for a the list) then the
    following happens:

    * The entire file contents are loaded into an in-memory string.
    * Anything with the pattern <%PARSE_TAG%> is replaced with an
    * appropriate value as determined by the 
    *       string substitute_tag(const string &tag)
    * function.
*/

#ifndef HTTPSERVICE_H
#define HTTPSERVICE_H

// #include <esp_http_server.h>
#include "mongoose.h"

// #include <map>
#include "string"

#include "fnFS.h"

// FNWS_FILE_ROOT should end in a slash '/'
#define FNWS_FILE_ROOT "/www/"
#define FNWS_SEND_BUFF_SIZE 512 // Used when sending files in chunks
#define FNWS_RECV_BUFF_SIZE 512 // Used when receiving POST data from client

#define MSG_ERR_OPENING_FILE     "Error opening file"
#define MSG_ERR_OUT_OF_MEMORY    "Ran out of memory"
#define MSG_ERR_UNEXPECTED_HTTPD "Unexpected web server error"
#define MSG_ERR_RECEIVE_FAILURE  "Failed to receive posted data"

#define PRINTER_BUSY_TIME 2000 // milliseconds to wait until printer is done

class fnHttpService 
{
    struct serverstate {
        struct mg_mgr *hServer;
        // httpd_handle_t hServer;
        FileSystem *_FS = nullptr;
    } state;

    enum _fnwserr
    {
        fnwserr_noerrr = 0,
        fnwserr_fileopen,
        fnwserr_memory,
        fnwserr_post_fail
    };

    // struct queryparts {
    //     std::string full_uri;
    //     std::string path;
    //     std::string query;
    // };

    // static void custom_global_ctx_free(void * ctx);
    // static httpd_handle_t start_server(serverstate &state);
    static struct mg_mgr * start_server(serverstate &state);
    static void cb(struct mg_connection *c, int ev, void *ev_data, void *fn_data);
    // static void stop_server(httpd_handle_t hServer);
    // static void return_http_error(httpd_req_t *req, _fnwserr errnum);
    static void return_http_error(struct mg_connection *c, _fnwserr errnum);
    static const char * find_mimetype_str(const char *extension);
    static const char * get_extension(const char *filename);
    static const char * get_basename(const char *filepath);
    // static void set_file_content_type(httpd_req_t *req, const char *filepath);
    static void set_file_content_type(struct mg_connection *c, const char *filepath);
    // static void send_file_parsed(httpd_req_t *req, const char *filename);
    static void send_file_parsed(struct mg_connection *c, const char *filename);
    // static void send_file(httpd_req_t *req, const char *filename);
    static void send_file(struct mg_connection *c, const char *filename);
    // static void parse_query(httpd_req_t *req, queryparts *results);
    static int redirect_or_result(mg_connection *c, mg_http_message *hm, int result);

    friend class fnHttpServiceBrowser; // allow browser to call above functions

public:    

    std::string errMsg; 

    std::string getErrMsg() { return errMsg; }
    void clearErrMsg() { errMsg.clear(); }
    void addToErrMsg(const std::string &_e) { errMsg += _e; }
    bool errMsgEmpty() { return errMsg.empty(); }

    // static esp_err_t get_handler_test(httpd_req_t *req);
    // static esp_err_t get_handler_index(httpd_req_t *req);
    // static esp_err_t get_handler_file_in_query(httpd_req_t *req);
    // static esp_err_t get_handler_file_in_path(httpd_req_t *req);
    // static esp_err_t get_handler_print(httpd_req_t *req);
    static int get_handler_print(struct mg_connection *c);
    // static esp_err_t get_handler_modem_sniffer(httpd_req_t *req);
    static int get_handler_swap(struct mg_connection *c, struct mg_http_message *hm);
    static int get_handler_mount(struct mg_connection *c, struct mg_http_message *hm);
    static int get_handler_eject(mg_connection *c, mg_http_message *hm);

    // static esp_err_t post_handler_config(httpd_req_t *req);
    static int post_handler_config(struct mg_connection *c, struct mg_http_message *hm);

    static int get_handler_browse(mg_connection *c, mg_http_message *hm);

    
    void start();
    void stop();
    void service();
    bool running(void) {
        return state.hServer != nullptr;
    }
};

extern fnHttpService fnHTTPD;
#endif // HTTPSERVICE_H
