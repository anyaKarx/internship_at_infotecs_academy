#include <iostream>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "tcp_client.h"
#include "packet_builder.h"

int main()
{

	const char me[10] = "127.0.0.1";
	unsigned short port_number{ 12345 };

	boost::asio::io_service io_service;

	ip::tcp::client client{ io_service };

	auto connect_to_server = [me, port_number](decltype(client)& connect_me)
	{
		while (!connect_me.is_connected())
			connect_me.connect(me, std::to_string(port_number));
	};

	protocol::packet_builder<protocol::cei::packet> builder;
	while (true)
	{
		if (!client.is_connected())
		{
			connect_to_server(client);
			if (!client.is_connected())
				// todo customize wait timeout
				boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
			continue;
		}

		protocol::cei::packet packet;
		size_t const bytes_to_read = sizeof(packet),
			bytes_were_read = client.read((uint8_t*)&packet, bytes_to_read);
		if (bytes_to_read != bytes_were_read)
		{
			std::cout << "Accepted " << bytes_were_read << ", but expected " << bytes_to_read << std::endl
				<< ". Client will disconnect." << std::endl;
			client.disconnect();
			continue;
		}

		try
		{
			builder.check_accepted_packet(packet);
		}
		catch (std::logic_error const& e)
		{
			std::cout << "Error: " << e.what() << std::endl
				<< ". Client will disconnect." << std::endl;
			client.disconnect();
			continue;
		}


		std::cout << "		Incoming frame:" << std::endl
			<< "Packet id: "<< packet.header.packet_id << std::endl;
		if (std::to_string(packet.data_send.data).size() >= 2 && packet.data_send.data % 32 == 0)
			std::cout << " Data from program 1: " << packet.data_send.data << std::endl;
		else
			std::cout << "Error! Data from program 1 does not meet the conditions." << std::endl;
			

	}
}
