
#include "tcp_server.h"
#include "client_connection.h"
#include "Parser.h"
#include "value.h"
#include "LoggerAOF.h"







void loadDBAOF(const std::string& aofFileName) {
	auto stream = std::ifstream(aofFileName);
	
	auto& db = DB::getInstance();

	auto commands = CommandFactory::getInstance().create();

	while (!stream.eof()) {
		try {
			auto value = parse(stream);
			auto response = commands.handle(value);
		}
		catch (std::exception e) {
		}
	}
}




void main() {



	auto& aofFactory = AOFLogger_Factory::getInstance();
	aofFactory.setType(AOF_Type::FSync);
	std::string aofFileName = "aof.txt";
	aofFactory.setOutputFile(aofFileName);
	loadDBAOF(aofFileName);



	boost::asio::io_service io_service;
	tcp_server<client_connection_thread> server(io_service, 6379);

	server.start_accept();

	io_service.run();

	server.stop();

	for (auto& connection : server.getConnections()) {
		connection->join();
	}


	{
		size_t offset = 0;
		Parser parser(offset);

		std::string str = "*2\r\n$6\r\nfoo";
		parser.parse(str);

		str += "bar\r\n:1\r\n";
		auto val = parser.parse(str);
	}

	{
		size_t offset = 0;
		Parser parser(offset);

		std::string str = "+Hello";
		parser.parse(str);

		str += "Wolrd\r\n";
		auto val = parser.parse(str);
	}

	{
		size_t offset = 0;
		Parser parser(offset);

		std::string str = "*2\r\n:10";
		parser.parse(str);

		str += "2\r\n*2\r\n*1\r\n:10\r\n+Hello World\r\n";
		auto val = parser.parse(str);
	}

	{
		size_t offset = 0;
		Parser parser(offset);

		std::string str = "*1\r\n:102\r\n";

		auto val = parser.parse(str);
	}

	/*{
		std::string str = ":102\r\n";
		std::string_view str_view = str;
		Parser parser;
		auto val = parser.parse(str_view);
	}

	{
		Parser parser;

		std::string str = ":10";
		parser.parse(str);

		str += "2\r\n";
		auto val = parser.parse(str);
	}*/




	{
		std::string str = "+OK\r\n";

		std::string_view str_view = str;
		Value val = parse(str_view);
	}
	{
		std::string str = ":102\r\n";

		std::string_view str_view = str;
		Value val = parse(str_view);

	}
	{
		std::string str = "$6\r\nfoobar\r\n";

		std::string_view str_view = str;
		Value val = parse(str_view);
	}
	{
		std::string str = "$-1\r\n";

		std::string_view str_view = str;
		Value val = parse(str_view);
	}
	{
		std::string str = "*2\r\n$3\r\nfoo\r\n$3\r\nbar\r\n";

		std::string_view str_view = str;
		Value val = parse(str_view);
	}




}