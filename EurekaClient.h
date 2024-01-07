#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>
#include <string>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <errno.h>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>


namespace po = boost::program_options;

using namespace std;

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class EurekaClient{
public:
    EurekaClient(string eurekaHost, int eurekaPort, string appName, int port);
    ~EurekaClient();

    int init(string, int, string, int);

    int registryService(string);
    int searchService(string);
    http::response<http::dynamic_body> requestEureka(http::request<http::string_body> req);

private:
    string  m_strEurekaHost;
    int  m_iEurekaPort;
    string  m_strAppName;
    int     m_iPort;

    
};
