#include <iostream>

#include <boost/asio.hpp>

#include "cin_input_validation.h"
#include "thread_safe_queue.h"
#include "tcp_server.h"
#include "packet_builder.h"


void Producer(Buffer<std::string>& buffer)
{
	while (true)
	{
		auto data = tools::GetValidValueFromUser<std::string>(
			" Enter the string: ",
			[](std::string const& val)
			{
				return val.length() < 64
					&& std::all_of(val.begin(), val.end(), isdigit);
			}
		);
		std::cout << std::endl;
		std::sort(data.begin(), data.end(), std::greater<std::remove_reference<decltype(*data.begin())>::type>());
		decltype(data) converted;
		converted.reserve(data.size() + data.size() / 2 + data.size() % 2);
		for (size_t i = 0; i < data.size(); ++i)
			converted += i % 2 == 0
			? std::string("KB")
			: std::string(1, data[i]);
		buffer.push(converted);
	}
}

void Consumer(Buffer<std::string>& buffer, ip::tcp::server& server)
{
	while (true)
	{
		auto data_to_process = buffer.pop();
		std::cout <<"Data of producer: "<< data_to_process<< std::endl;
		int64_t sum = 0;
		for (auto item : data_to_process)
			if (isdigit(item))
				sum += item - '0';
		server.send_data(sum);
	}
}


int main()
{
	unsigned short port_number{ 12345 };

	boost::asio::io_service io_service;
	boost::asio::io_service::work work{ io_service };
	boost::thread io_thread{
		[&io_service]()
		{
			io_service.run();
		}
	};
	
	ip::tcp::server server{ io_service, port_number };
	server.start_acception();

	Buffer<std::string> buffer;
	boost::thread one{ Producer, std::ref(buffer)};
	boost::thread two{ Consumer, std::ref(buffer), std::ref(server)};
	one.join();
	two.join();
	return EXIT_SUCCESS;
}
