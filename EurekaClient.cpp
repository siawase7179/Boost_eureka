#include "EurekaClient.h"

EurekaClient::EurekaClient(string eurekaHost, int eurekaPort, string appName, int port)
    : m_strEurekaHost(eurekaHost), m_iEurekaPort(eurekaPort), m_strAppName(appName), m_iPort(port) {
    
}

EurekaClient::~EurekaClient(){

}


http::response<http::dynamic_body> EurekaClient::requestEureka(http::request<http::string_body> request){
    // IO 컨텍스트 생성
    net::io_context io_context;

    tcp::resolver resolver(io_context);

    // DNS 조회
    char acEurekaPort[12];
    memset(acEurekaPort, 0x00, sizeof(acEurekaPort));
    sprintf(acEurekaPort, "%d", m_iEurekaPort);
    auto results = resolver.resolve(m_strEurekaHost, (string)acEurekaPort);

    // 소켓 연결
    tcp::socket socket(io_context);
    net::connect(socket, results.begin(), results.end());

    // HTTP 요청 전송
    http::write(socket, request);

    // 버퍼 생성
    beast::flat_buffer buffer;

    // HTTP 응답 읽기
    http::response<http::dynamic_body> response;
    http::read(socket, buffer, response);

    return response;
}

int EurekaClient::registryService(string status){
    try {
        boost::property_tree::ptree newPt;
        boost::property_tree::ptree subPt;

        boost::property_tree::ptree dataCenterInfoPt;
        boost::property_tree::ptree portPt;
        portPt.put("$", m_iPort);
        portPt.put("@enabled", false);

        dataCenterInfoPt.put("@class", "com.netflix.appinfo.InstanceInfo$DefaultDataCenterInfo");
        dataCenterInfoPt.put("name", "MyOwn");

        char hostname[128];
        memset(hostname, 0x00, sizeof(hostname));
        gethostname(hostname, sizeof(hostname));
        std::cout<<hostname<<std::endl;

        hostent * record = gethostbyname(hostname);
        in_addr * address = (in_addr * )record->h_addr;
        std::string ip_address = inet_ntoa(* address);

        char acPort[12];
        memset(acPort, 0x00, sizeof(acPort));
        sprintf(acPort, "%d", m_iPort);

        std::string instanceId = (string)hostname + ":" + m_strAppName + ":" + (string)acPort;
        subPt.put("hostName", ip_address);
        subPt.put("app", m_strAppName);
        subPt.put("instanceId", instanceId);
        subPt.put("ipAddr", ip_address);
        subPt.put("status", status);

        subPt.add_child("port", portPt);
        subPt.add_child("dataCenterInfo", dataCenterInfoPt);
        
        newPt.add_child("instance", subPt);

        // JSON 객체를 문자열로 변환
        std::ostringstream oss;
        boost::property_tree::write_json(oss, newPt);
        std::string jsonData = oss.str();

        std::cout<<jsonData<<std::endl;

        // HTTP 요청 생성
        http::request<http::string_body> request{http::verb::post, "/eureka/apps/"+m_strAppName, 11};
        request.set(http::field::host, m_strEurekaHost);
        request.set(http::field::accept, "application/json");
        request.set(http::field::content_type, "application/json");
        request.body() = jsonData;
        request.prepare_payload();

        http::response<http::dynamic_body> response = requestEureka(request);

        if (response.result() != http::status::ok && response.result() != http::status::no_content) {
            return -1;
        } else {
            
        }
    } catch (std::exception const& e) {
        std::cerr<<"Error:"<<e.what()<<std::endl;
        return -1;
    }

    return 0;
}


int EurekaClient::searchService(string serviceName) {
    try{
        http::request<http::string_body> request{http::verb::get, "/eureka/apps/"+serviceName, 11};
        request.set(http::field::host, m_strEurekaHost);
        request.set(http::field::accept, "application/json");

        http::response<http::dynamic_body> response = requestEureka(request);
       
        if (response.result() != http::status::ok && response.result() != http::status::no_content) {
            std::cerr<<response.result()<<std::endl;
            return -1;
        } else {
            std::string response_body = boost::beast::buffers_to_string(response.body().data());
            std::cout << "Response Body: " << response_body << std::endl;

            boost::property_tree::ptree pt;
            std::istringstream jsonStream(response_body);
            boost::property_tree::read_json(jsonStream, pt);


            size_t instanceCount = std::distance(pt.get_child("application.instance").begin(), pt.get_child("application.instance").end());
            std::cout << "Instance Count: " << instanceCount << std::endl;

            // "instance" 배열 아래의 모든 요소에 대한 반복 작업
            for (const auto& instance : pt.get_child("application.instance")) {
                std::string instanceId = instance.second.get<std::string>("instanceId", "");
                std::cout << "Instance ID: " << instanceId << std::endl;

                std::string hostName = instance.second.get<std::string>("hostName");
                std::cout << "hostName: " << hostName << std::endl;

                std::string port = instance.second.get<std::string>("port.$");
                std::cout << "port: " << port << std::endl;
            }

            // "name" 필드
            std::string appName = pt.get<std::string>("application.name", "");
            std::cout << "Application Name: " << appName << std::endl;
        }
    } catch (std::exception const& e) {
        std::cerr<<"Error:"<<e.what()<<std::endl;
        return -1;
    }

    return 0;
}

void threadFunction(EurekaClient client) {
    
}


int main(int argc, char **argv){
    string host;
    int port;
    string instance;
    po::options_description options("Options");
    options.add_options()
        ("help",     "produce this help message")
        ("host,h",  po::value<string>(&host)->required(), "server host")
        ("port,p",  po::value<int>(&port)->required(), "server port")
        ("instance,i",  po::value<string>(&instance)->required(), "instance Name")
        ;

    po::variables_map vm;

    try {
        po::store(po::command_line_parser(argc, argv).options(options).run(), vm);
        po::notify(vm);
    }catch (exception& ex) {
        cout << "Error parsing options: " << ex.what() << endl;
        cout << endl;
        cout << options << endl;
        return 1;
    }

    EurekaClient client(host, port, "C-SERVICE", 9090);

    boost::thread th(threadFunction, client);
    client.registryService("UP");
    client.searchService(instance);

    return 0;
}