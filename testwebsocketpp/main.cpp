#include <stdio.h>

#include <iostream>
#include <functional>
#include <mutex>
#include <thread>

#define ASIO_STANDALONE
#include "websocketpp/config/asio_no_tls.hpp"
#include "websocketpp/server.hpp"


class CWSServer {
public:
	class CConnection {
	public:
		explicit CConnection (websocketpp::server<websocketpp::config::asio> &s) :m_server(s) {
		}
		virtual ~CConnection (void) {
		}

		bool has (websocketpp::connection_hdl& hdl) {
			std::lock_guard<std::recursive_mutex> lock (m_mutex);

			const auto itr = m_connections.find(hdl);
			if (itr == m_connections.end()) {
				return false;
			} else {
				return true;
			}
		}

		void add (websocketpp::connection_hdl &hdl, std::string id) {
			std::lock_guard<std::recursive_mutex> lock (m_mutex);

			if (!has(hdl)) {
				m_connections.insert(std::pair<websocketpp::connection_hdl, std::string>(hdl, id));
			}
		}

		void remove (websocketpp::connection_hdl& hdl) {
			std::lock_guard<std::recursive_mutex> lock (m_mutex);

			if (has(hdl)) {
				m_connections.erase(hdl);
			}
		}

		// broadcast
		bool send (const uint8_t* buff, size_t size) {
			std::lock_guard<std::recursive_mutex> lock (m_mutex);

			std::map<websocketpp::connection_hdl, std::string>::const_iterator itr = m_connections.begin();
			for (; itr != m_connections.end(); ++ itr) {
				websocketpp::connection_hdl hdl = itr->first;
				send (hdl, buff, size);
			}

			return true;
		}

		bool send (const std::string &msg) {
			std::lock_guard<std::recursive_mutex> lock (m_mutex);

			std::map<websocketpp::connection_hdl, std::string>::const_iterator itr = m_connections.begin();
			for (; itr != m_connections.end(); ++ itr) {
				websocketpp::connection_hdl hdl = itr->first;
				send (hdl, msg);
			}

			return true;
		}

		// unicast
		bool send (websocketpp::connection_hdl& hdl, const uint8_t* buff, size_t size) {
			std::lock_guard<std::recursive_mutex> lock (m_mutex);

			websocketpp::lib::error_code ec;
			const auto itr = m_connections.find(hdl);
			if (itr == m_connections.end()) {
				return false;
			} else {
				m_server.send(itr->first, buff, size, websocketpp::frame::opcode::binary, ec);
				if (ec) {
					std::cout << ec.message() << std::endl;
					return false;
				}
			}

			return true;
		}

		bool send (websocketpp::connection_hdl& hdl, const std::string &msg) {
			std::lock_guard<std::recursive_mutex> lock (m_mutex);

			websocketpp::lib::error_code ec;
			if (!has(hdl)) {
				return false;
			}

			const auto itr = m_connections.find(hdl);
			m_server.send(itr->first, msg, websocketpp::frame::opcode::TEXT, ec);
			if (ec) {
				std::cout << ec.message() << std::endl;
				return false;
			}

			return true;
		}


		void close (websocketpp::connection_hdl& hdl) {
			std::lock_guard<std::recursive_mutex> lock (m_mutex);

			websocketpp::lib::error_code ec;
			const auto itr = m_connections.find(hdl);
			if (itr != m_connections.end()) {
				m_server.close(itr->first, websocketpp::close::status::normal, "terminate...", ec);
				if (ec) {
					std::cout << ec.message() << std::endl;
				}				
				remove(hdl);
			}
		}

//TODO
		void dump (void) {
			std::lock_guard<std::recursive_mutex> lock (m_mutex);

			websocketpp::lib::error_code ec;
			for (const auto itr : m_connections) {
				auto con = m_server.get_con_from_hdl(itr.first);
//				con->
			}
		}

	private:
		std::map<websocketpp::connection_hdl, std::string, std::owner_less<websocketpp::connection_hdl>> m_connections;
		websocketpp::server<websocketpp::config::asio> &m_server;

		std::recursive_mutex m_mutex;
		
	};

public:
	explicit CWSServer (int port) : m_con(m_server), m_port(port) {
	}
	virtual ~CWSServer (void) {
	}


	void init (void) {
		m_server.init_asio();
		m_server.set_reuse_addr(true);
		m_server.clear_access_channels(websocketpp::log::alevel::all);

		std::function<bool(websocketpp::connection_hdl)> _on_validate = [this](websocketpp::connection_hdl hdl) { return on_validate(hdl); };
		std::function<void(websocketpp::connection_hdl, websocketpp::server<websocketpp::config::asio>::message_ptr)> _on_message =
			[this](websocketpp::connection_hdl hdl, websocketpp::server<websocketpp::config::asio>::message_ptr msg) { on_message(hdl, msg); };
		std::function<void(websocketpp::connection_hdl)> _on_close = [this](websocketpp::connection_hdl hdl) { on_close(hdl); };

		// register handlers
		m_server.set_validate_handler(std::move(_on_validate));
		m_server.set_message_handler(std::move(_on_message));
//		m_server.set_fail_handler(&CWSServer::on_fail);
		m_server.set_close_handler(std::move(_on_close));
	}

	void run (void) {
		try {
			m_server.listen(m_port);
			m_server.start_accept();
			m_server.run();

		} catch (websocketpp::exception const &e) {
			std::cout << e.m_msg << std::endl;
		}
	}

	bool send (const uint8_t *buff, size_t size) {
		return m_con.send (buff, size);
	}

	bool send (const std::string &msg) {
		return m_con.send (msg);
	}

	bool send (websocketpp::connection_hdl& hdl, const uint8_t* buff, size_t size) {
		return m_con.send (hdl, buff, size);
	}

	bool send (websocketpp::connection_hdl& hdl, const std::string &msg) {
		return m_con.send (hdl, msg);
	}

	void close (websocketpp::connection_hdl& hdl) {
		m_con.close (hdl);
	}

	bool on_validate (websocketpp::connection_hdl hdl) {
		websocketpp::server<websocketpp::config::asio>::connection_ptr con = m_server.get_con_from_hdl(hdl);
		websocketpp::uri_ptr uri = con->get_uri();
		std::string query = uri->get_query();
		if (query.empty()) {
			// reject if no query parameter provided
			std::cout << "no query" << std::endl;
			return false;
		}

		std::cout << "query:" << query << std::endl;
		std::string param = get_query_param(query);
		std::string value = get_query_value(query);
		if (param != "id") {
			std::cout << "invalid query param: " << param << std::endl;
			return false;
		}

		std::string id = value;
		m_con.add (hdl, id);
		return true;
	}

	void on_message (websocketpp::connection_hdl hdl, websocketpp::server<websocketpp::config::asio>::message_ptr msg) {
		websocketpp::server<websocketpp::config::asio>::connection_ptr con = m_server.get_con_from_hdl(hdl);
		websocketpp::uri_ptr uri = con->get_uri();
		std::string query = uri->get_query();

		// debug echo
		std::cout << "query:" << query << " " << msg->get_header() << " " << msg->get_payload() << std::endl;
		send(hdl, (uint8_t*)msg->get_payload().c_str(), msg->get_payload().length());
	}

	void on_close (websocketpp::connection_hdl hdl) {
		close (hdl);
	}

private:
	std::string get_query_param (const std::string &s) const {
		std::size_t pos = s.find('=');
		if (pos == std::string::npos) {
			return "";
		} else {
			return s.substr(0, pos);
		}
	}

	std::string get_query_value (const std::string &s) const {
		std::size_t pos = s.find('=');
		if (pos == std::string::npos) {
			return "";
		} else {
			return s.substr(pos + 1);
		}
	}

	websocketpp::server<websocketpp::config::asio> m_server;
	CConnection m_con;
	int m_port;
};

int main (void) {
	CWSServer s{50000};
	s.init();

	std::thread t([&s]{
		while (1) {
			std::string _in;
			std::getline(std::cin, _in);
			if (_in.length() > 0) {
				std::cout << "getline: " <<_in << std::endl;
				s.send (_in);
			}
		}
	});

	s.run();
	t.join();

	return 0;
}
