#pragma once

#include <unordered_map>

#include <boost/asio.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/bind.hpp>

#include "packet_builder.h"
#include "tcp_connection.h"

namespace ip
{
	namespace tcp
	{
		class server
		{
			boost::asio::io_service& _io_service;
			boost::asio::ip::tcp::acceptor _acceptor;
			std::unordered_map<boost::shared_ptr<connection>::element_type*, boost::shared_ptr<connection>> _connections;
			boost::shared_mutex _connections_change;
			protocol::packet_builder<protocol::cei::packet> build;
		public:
			server(decltype(_io_service) io_service, unsigned short port)
				: _io_service(io_service)
				, _acceptor(io_service, boost::asio::ip::tcp::endpoint{ boost::asio::ip::tcp::v4(), port })
			{
				start_acception();
			}

			void send_data(int64_t data_to_send)
			{
				std::vector<uint8_t> packet = build.get_packet(protocol::cei::payload{ data_to_send });
				protocol::cei::packet* packet_ptr = (protocol::cei::packet*)packet.data();
				std::cout << "Outgoung frame:" << std::endl
					<< "\tHeader " << std::endl << "\t{" << std::endl
					<< "\t\tVersion " << std::to_string(packet_ptr->header.version) << std::endl
					<< "\t\tLength " << std::to_string(packet_ptr->header.length) << std::endl
					<< "\t\tPacketId " << std::to_string(packet_ptr->header.packet_id) << std::endl
					<< "\t}" << std::endl
					<< "\tPayload " << std::to_string(packet_ptr->data_send.data) << std::endl;
				write((uint8_t const*)packet.data(), packet.size());
			}

			void write(uint8_t const* bytes, size_t size)
			{
				boost::shared_lock<decltype(_connections_change)> connections_change_lock{ _connections_change };
				for (auto& connected : _connections)
					connected.second->write(
						bytes,
						size,
						boost::bind(
							&server::_handle_writing_completion, this,
							boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3
						)
					);
			}
			size_t get_connections_count()
			{
				boost::shared_lock<decltype(_connections_change)> connections_change_lock{ _connections_change };
				return _connections.size();
			}
			void start_acception()
			{
				auto connection = connection::create(_io_service);

				_acceptor.async_accept(
					connection->get_socket(),
					boost::bind(&server::handle_connection, this, connection, boost::asio::placeholders::error)
				);
			}
		private:
			void handle_connection(
				boost::shared_ptr<connection> connection,
				boost::system::error_code const& error
			)
			{
				if (!error)
				{
					auto& socket = connection->get_socket();
					socket.set_option(boost::asio::ip::tcp::socket::keep_alive(true));
#ifdef _DEBUG
					socket.set_option(boost::asio::ip::tcp::socket::debug(true));
#endif

					auto connection_raw_ptr = connection.get();
					boost::unique_lock<decltype(_connections_change)> connections_change_lock{ _connections_change };
					_connections.emplace(connection_raw_ptr, std::move(connection));
				}

				start_acception();
			}

			void _handle_writing_completion(
				boost::shared_ptr<connection> connection,
				std::size_t bytes_transferred,
				const boost::system::error_code& error
			) {
				if (error)
				{
					std::thread(
						[connection, this]()
						{
							boost::unique_lock<decltype(_connections_change)> connections_change_lock{ _connections_change };
							auto connection_iter = _connections.find(connection.get());
							if (connection_iter != _connections.end())
							{
								_connections.erase(connection_iter);
							}
						}
					).detach();
				}
			}
			
		};
	}
}
