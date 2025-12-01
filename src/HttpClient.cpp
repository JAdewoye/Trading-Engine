//----------------------------------------------------------------------------------
// Inlcudes 
//----------------------------------------------------------------------------------
#include "HttpClient.h"
#include <iostream>
#include <string>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/hmac.h>
#include <openssl/bio.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <boost/json.hpp>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/connect.hpp>

using boost::asio::ip::tcp;
namespace json = boost::json;
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = net::ssl;
//----------------------------------------------------------------------------------
// Function Definitions
//----------------------------------------------------------------------------------
HttpClient::OrderResponse
HttpClient::placeOrder(std::string const& body)
{
    OrderResponse response;
    try{
        auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
        long long timestamp_ms = now.time_since_epoch().count();
        std::string timestamp_str = std::to_string(timestamp_ms);

        std::string pre_hash_message = timestamp_str + "POST" + BITGET_TRADE_ENDPOINT + body;
        std::string signature = calculate_access_sign(SECRET_KEY, pre_hash_message);
        
        http::request<http::string_body> req{http::verb::post, BITGET_TRADE_ENDPOINT, 11};
        req.set(http::field::host, BITGET_HOST);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req.set(http::field::content_type, "application/json");

        req.set("ACCESS-KEY", API_KEY);                  
        req.set("ACCESS-SIGN", signature);               
        req.set("ACCESS-PASSPHRASE", PASSPHRASE);        
        req.set("ACCESS-TIMESTAMP", timestamp_str);       
        req.set("locale", "en-US");

#ifdef DEMO_TRADING
        req.set("paptrading", "1");
#endif 

        req.body() = body;
        req.prepare_payload();

        response = sendRequest(req);

    }catch(std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return response;
}
//----------------------------------------------------------------------------------
std::string
HttpClient::prepareOrderBody(const std::string& symbol, const std::string& side, double price, double quantity)
{
    json::value body_json =
    {
        {"symbol", symbol},
        {"productType", USER_PRODUCT_TYPE},
        {"marginMode", USER_MARGINE_MODE},
        {"marginCoin", USER_MARGIN_COIN},
        {"size", std::to_string(quantity)},
        {"force", "gtc"},
        {"price", std::to_string(price)},
        {"side", side},
        {"tradeside", "open"},
        {"orderType", USER_ORDER_TYPE},
        {"clientOid", "121211212122"}
    };

    return json::serialize(body_json);
}
//----------------------------------------------------------------------------------
std::string
HttpClient::calculate_access_sign(const std::string& secret_key, const std::string& message_string)
{
    unsigned char digest[EVP_MAX_MD_SIZE]; 
    unsigned int len = 0;

    HMAC(EVP_sha256(), 
         secret_key.c_str(), secret_key.length(), 
         reinterpret_cast<const unsigned char*>(message_string.c_str()), message_string.length(), 
         digest, &len);

    BIO *b64 = BIO_new(BIO_f_base64());
    BIO *bmem = BIO_new(BIO_s_mem());
    
    b64 = BIO_push(b64, bmem); 

    BIO_write(b64, digest, len);
    BIO_flush(b64);

    BUF_MEM *bptr;
    BIO_get_mem_ptr(b64, &bptr);
    
    std::string signature(bptr->data, bptr->length);

    BIO_free_all(b64);

    // Bitget usually requires no trailing newline, so remove it if present
    if (!signature.empty() && signature.back() == '\n') {
        signature.pop_back();
    }
    
    return signature;
}
//----------------------------------------------------------------------------------
HttpClient::OrderResponse
HttpClient::sendRequest(http::request<http::string_body>& req)
{
    OrderResponse response;
    beast::error_code ec;

    try
    {
        // SSL context
        ssl::context ctx(ssl::context::tlsv12_client);
        ctx.set_default_verify_paths(ec);
        if (ec) {
            // Handle error if paths cannot be loaded (e.g., config is missing)
            std::cerr << "SSL Context Error loading default paths: " << ec.message() << std::endl;
            throw beast::system_error{ec, "SSL Context Error"};
        }

        // TODO : In production, set proper verification mode and load CA certificates
        ctx.set_verify_mode(ssl::verify_none);

        net::io_context ioc;
        tcp::resolver resolver(ioc);
        beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);
        
        // Set SNI hostname
        if (!SSL_set_tlsext_host_name(stream.native_handle(), BITGET_HOST)) {
            beast::error_code ec{static_cast<int>(::ERR_get_error()), 
                                 net::error::get_ssl_category()};
            throw beast::system_error{ec};
        }

        // Resolve the host and connect
        auto const results = resolver.resolve(BITGET_HOST, "443");
        beast::get_lowest_layer(stream).connect(results);

        // Perform SSL handshake
        stream.handshake(ssl::stream_base::client);

        // Send the HTTP request
        http::write(stream, req);

        // Receive the HTTP response
        beast::flat_buffer buffer;
        http::response<http::string_body> res;
        http::read(stream, buffer, res);

        // Parse the response
        parseResponse(res, response);

        // Graceful shutdown
        stream.shutdown(ec);
        
        // Note: ec may be set to stream_truncated, which is normal for some servers
        if (ec && ec != net::error::eof && ec != ssl::error::stream_truncated) {
            std::cerr << "Shutdown error: " << ec.message() << "\n";
        }
    }
    catch (const boost::system::system_error& e)
    {
       std::cerr << "sendRequest exception: " << e.what() << "\n";

    }

    return response;
}
//----------------------------------------------------------------------------------
void
HttpClient::parseResponse(http::response<http::string_body>& res, HttpClient::OrderResponse& response)
{
    // Parse the response
    int http_status = res.result_int();

    try {
        json::value jsonResponse = json::parse(res.body());
        json::object& obj = jsonResponse.as_object();
        
        if (obj.contains("code")) {
            response.code = json::value_to<std::string>(obj["code"]);
        }
        if (obj.contains("msg")) {
            response.msg = json::value_to<std::string>(obj["msg"]);
        }
        if (obj.contains("orderId")) {
            response.order_id = json::value_to<std::string>(obj["orderId"]);
        }
        if (obj.contains("customOrderId")) {
            response.custom_order_id = json::value_to<std::string>(obj["customOrderId"]);
        }
        if (obj.contains("requestTime")) {
            response.request_time = json::value_to<long long>(obj["requestTime"]);
        }
        
        response.success = (response.code == "00000" || http_status == 200);
        
    } catch (std::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << "\n";
        response.success = false;
        response.msg = "Failed to parse response JSON";
    }
}