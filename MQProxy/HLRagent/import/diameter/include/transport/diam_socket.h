/**************************************************************************
类    名: CDiamACETransport
类 功 能: socket连接类,完成连接的建立和数据收发
时    间: 2012年8月7日
**************************************************************************/
#ifndef __DIAM_SOCKET_H__
#define __DIAM_SOCKET_H__

#include <util/diam_ace.h>
#include <string>

#ifndef IPPROTO_SCTP
#define IPPROTO_SCTP 132
#endif // !IPPROTO_SCTP

class DiamSocket
{
public:
    DiamSocket(bool status = true);
    virtual ~DiamSocket();

    int close();
    int send(void *data, size_t length);
    int recv(void *data, size_t length, int timeout = 1);

    bool status();
    void set_status(bool status);
    int prtlType() {
        return prtl_type_;
    }
    int getLocalAddr(ACE_INET_Addr& addr);
    int getRemoteAddr(ACE_INET_Addr& addr);
    ACE_SOCK_Stream& Stream() {
        return stream_;
    }

    virtual int connect(const char* localAddr, int localPort, const char* remoteAddr, int remotePort, int protocol)
        {return -1;}
    virtual int complete() {return -1;}
    virtual int listen(ACE_INET_Addr localAddr, int protocol) {return -1;}
    virtual int accept(DiamSocket*& dataTrans) {return -1;}

    virtual bool passive() { return true;}

protected:
    int AceAsynchResults(int rc);
    int AceIOResults(int rc);
    int HandOverStream(DiamSocket* &dest, DiamSocket* &src);

protected:
    bool             status_;
    ACE_SOCK_Stream  stream_;
    int              prtl_type_;
};

class DiamSocketClient : public DiamSocket
{
public:
    DiamSocketClient();
    virtual ~DiamSocketClient();

    int connect(const char* localAddr, int localPort, const char* remoteAddr, int remotePort, int protocol);

    int complete();

    virtual bool passive() {
        return false;
    }

private:
    ACE_SOCK_Connector  m_Connector;
};

class DiamSocketServer : public DiamSocket
{
public:
    DiamSocketServer();
    virtual ~DiamSocketServer();

public:
    int listen(ACE_INET_Addr localAddr, int protocol);

    int accept(DiamSocket*& dataTrans);

private:
    ACE_SOCK_Acceptor  m_Acceptor;
};

#endif //__DIAM_SCTP_H__

