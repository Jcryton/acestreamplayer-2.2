#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_network.h>
#include "connection.h"

#ifndef WIN32
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif

Connection::Connection( p2p_object_t *vlc_obj, const std::string host, const int port )
    : m_vlcobj(vlc_obj)
    , m_host(host)
    , m_port(port)
    , m_socket(INVALID_SOCKET)
    , m_retval("")
    , m_connected(false)
{
}

Connection::~Connection()
{
    disconnect();
}

bool Connection::connect()
{
    if( m_connected )
        return true;

    m_socket = net_ConnectTCP( m_vlcobj, m_host.c_str(), m_port );
    if( m_socket == SOCKET_ERROR )
    {
        msg_Warn( m_vlcobj, "[Connection]: Could not connect to engine" );
        msg_P2PLog(m_vlcobj, "[connection.cpp::connect]: Cannot connect to engine");
        return false;
    }

    m_connected = true;
    msg_P2PLog(m_vlcobj, "[connection.cpp::connect]: Connected successfully");
    return true;
}

void Connection::disconnect()
{
    if( !m_connected )
        return;

    msg_P2PLog(m_vlcobj, "[connection.cpp::disconnect]: Shutting down connection");
    net_Close( m_socket );
    m_socket = INVALID_SOCKET;
}

bool Connection::sendMsg( const std::string &event ) const
{
    if( !m_connected )
        return false;

    msg_P2PLog(m_vlcobj, "[connection.cpp::sendMsg]: Sending: %s", event.c_str());

    std::string _msg = event + "\r\n";
	ssize_t _res = net_Write( m_vlcobj, m_socket, NULL, _msg.c_str(), _msg.length() );

    if( _res == SOCKET_ERROR ) {
        msg_Err( m_vlcobj, "[Connection]: Cannot send command." );
        msg_P2PLog(m_vlcobj, "[connection.cpp::sendMsg]: Cannot send command.");
        return false;
    }
    return true;
}

bool Connection::sendMsgSync( const std::string &cmd, std::string &retval )
{
    m_retval = "x";
    if( !sendMsg(cmd) )
        return false;

    int _tries = 450;
    while( _tries > 0 ) {
        if( m_retval.compare("x") ) {
            msg_P2PLog(m_vlcobj, "[connection.cpp::sendMsgSync]: got retval %s", m_retval.c_str() );
            break;
        }
        msleep( 100000 );  // ~0.10 secs
        --_tries;
    }
    
    if( m_retval.length() == 0 ) {
        msg_P2PLog(m_vlcobj, "[connection.cpp::sendMsgSync]: no retval" );
        return false;
    }
    else if( m_retval == "NOTREADY" ) {
        retval = "";
        return false;
    }
    
    retval = m_retval;
    return true;
}

bool Connection::recvMsg( std::string &msg )
{
    if( !m_connected ) 
	{
        msg_P2PLog(m_vlcobj, "[connection.cpp::recvMsg]: not connected, set command to SHUTDOWN");
        msg_Warn(m_vlcobj, "[Connection]: not connected, set command to SHUTDOWN");
        disconnect();
        msg = "SHUTDOWN";
        return false;
    }

    char _buf[ BUFFER_LEN ];
    msg = "";
    while( true ) {
		ssize_t _res = net_Read( m_vlcobj, m_socket, NULL,  _buf, BUFFER_LEN, false );
        if( _res <= 0 ) {
            msg_P2PLog(m_vlcobj, "[connection.cpp::recvMsg]: Reading error, set command to SHUTDOWN");
            msg_Warn(m_vlcobj, "[Connection]: Reading error");
            disconnect();
            msg = "SHUTDOWN";
            return false;
        }

        msg.append( _buf, _res );
		size_t _found_diez = 0;
        if( !msg.compare( msg.size() - 2, 2, "\r\n" ) ) {
            msg.erase( msg.size() - 2 );
			_found_diez = msg.find("##");
			if( _found_diez != std::string::npos ) {
				if( msg.find("\r\n", _found_diez) ) {
					m_retval = msg.substr( _found_diez + 2, msg.find( "\r\n", _found_diez ) );
					msg.erase( _found_diez, m_retval.length()+4 );
				}
				else	{
					m_retval = msg.substr( _found_diez + 2 );
					msg.erase( _found_diez );
				}
				break;
			}
			else 
				break;
        }
    }

    msg_P2PLog(m_vlcobj, "[connection.cpp::recvMsg]: Received : %s", msg.c_str() );
    return true;
}

std::string Connection::getHost() const
{
    return m_host;
}

int Connection::getPort() const
{
    return m_port;
}

bool Connection::connected() const
{
    return m_connected;
}

void Connection::setPort( const int port )
{
    if( !m_connected )
        m_port = port;
}
