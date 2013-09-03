/*
 * Copyright (c) 2013 Juniper Networks, Inc. All rights reserved.
 */

#ifndef __IFMAP_CHANNEL_H__
#define __IFMAP_CHANNEL_H__

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated"
#endif
#include <boost/asio/ssl.hpp>
#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include <boost/asio/streambuf.hpp>
#include <boost/function.hpp>

class IFMapStateMachine;
class IFMapManager;
class Timer;

class IFMapChannel {
public:
    static const int kSocketCloseTimeout;

    IFMapChannel(IFMapManager *manager, const std::string& url,
                 const std::string& user, const std::string& passwd,
                 const std::string& certstore);

    virtual ~IFMapChannel() { }

    void set_sm(IFMapStateMachine *state_machine) {
        state_machine_ = state_machine;
    }
    IFMapStateMachine *state_machine() { return state_machine_; }

    void ChannelUseCertAuth(const std::string& url);

    void RetrieveHostPort(const std::string& url);

    virtual void ReconnectPreparation();

    virtual void DoResolve();

    void ReadResolveResponse(const boost::system::error_code& error,
                boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
    virtual void DoConnect(bool is_ssrc);

    virtual void DoSslHandshake(bool is_ssrc);

    virtual void SendNewSessionRequest();

    virtual void NewSessionResponseWait();

    // return 0 for success and -1 for failure
    virtual int ExtractPubSessionId();

    virtual void SendSubscribe();

    virtual void SubscribeResponseWait();

    // return 0 for success and -1 for failure
    virtual int ReadSubscribeResponseStr();

    virtual void SendPollRequest();

    virtual void PollResponseWait();

    virtual int ReadPollResponse();

    void ProcResponse(const boost::system::error_code& error,
                      size_t header_length);
    uint64_t get_sequence_number() { return sequence_number_; }

    uint64_t get_recv_msg_cnt() { return recv_msg_cnt_; }
    
    uint64_t get_sent_msg_cnt() { return sent_msg_cnt_; }

    uint64_t get_reconnect_attempts() { return reconnect_attempts_; }

    std::string get_url() { return url_; }

    std::string get_publisher_id() { return pub_id_; }

    std::string get_session_id() { return session_id_; }

    void increment_recv_msg_cnt() { recv_msg_cnt_++; }

    void increment_sent_msg_cnt() { sent_msg_cnt_++; }

    void increment_reconnect_attempts() { reconnect_attempts_++; }

    void clear_recv_msg_cnt() { recv_msg_cnt_ = 0; }

    void clear_sent_msg_cnt() { sent_msg_cnt_ = 0; }

    std::string get_connection_status() {
        switch (connection_status_) {
        case NOCONN:
            return std::string("No Connection");
        case DOWN:
            return std::string("Down");
        case UP:
            return std::string("Up");
        default:
            break;
        }

        return std::string("Invalid");
    }

private:
    enum ResponseState {
        NONE = 0,
        NEWSESSION = 1,
        SUBSCRIBE = 2,
        POLLRESPONSE = 3
    };
    enum ConnectionStatus {
        NOCONN = 0,
        DOWN = 1,
        UP = 2
    };
    typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> SslStream;
    typedef boost::function<void(const boost::system::error_code& error,
                            size_t header_length)
                           > ProcCompleteMsgCb;

    SslStream *GetSocket(ResponseState response_state);
    ProcCompleteMsgCb GetCallback(ResponseState response_state);
    void CloseSockets(const boost::system::error_code& error,
                      boost::asio::deadline_timer *socket_close_timer);

    IFMapManager *manager_;
    boost::asio::ip::tcp::resolver resolver_;
    boost::asio::ssl::context ctx_;
    std::auto_ptr<SslStream> ssrc_socket_;
    std::auto_ptr<SslStream> arc_socket_;
    std::string url_;
    std::string username_;
    std::string password_;
    std::string b64_auth_str_;
    std::string pub_id_;
    std::string session_id_;
    std::string host_;
    std::string port_;
    IFMapStateMachine *state_machine_;
    boost::asio::streambuf reply_;
    std::ostringstream reply_ss_;
    ResponseState response_state_;
    uint64_t sequence_number_;
    uint64_t recv_msg_cnt_;
    uint64_t sent_msg_cnt_;
    uint64_t reconnect_attempts_;
    ConnectionStatus connection_status_;
    boost::asio::ip::tcp::endpoint endpoint_;
};

#endif /* __IFMAP_CHANNEL_H__ */