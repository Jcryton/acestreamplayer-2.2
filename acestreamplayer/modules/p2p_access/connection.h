#ifndef __P2P_PLUGIN_CONNECTION__HEADER__
#define __P2P_PLUGIN_CONNECTION__HEADER__

#include <p2p_object.h>
#include <string>


const int BUFFER_LEN = 1024;
const int DEFAULT_PORT = 62062;
const std::string DEFAULT_ADDRESS = "127.0.0.1";

class Connection
{
public:
    Connection( p2p_object_t *, const std::string = DEFAULT_ADDRESS, const int = DEFAULT_PORT );
    ~Connection();

    bool connect();
    void disconnect();

    bool sendMsg( const std::string &) const;
    bool sendMsgSync(const std::string&, std::string& );
    bool recvMsg( std::string & );

    std::string getHost() const;
    int getPort() const;
    bool connected() const;
    void setPort( const int );

private:
    p2p_object_t* m_vlcobj;
    std::string  m_host;
    int m_port;
    int m_socket;
    std::string m_retval;
    bool m_connected;
};

#endif