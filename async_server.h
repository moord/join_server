#ifndef ASYNC_SERVER_H
#define ASYNC_SERVER_H

#include <cstdlib>
#include <memory>
#include <boost/asio.hpp>
#include <queue>
#include "dbadapter.h"


using boost::asio::ip::tcp;

class session
        : public std::enable_shared_from_this<session>
{
public:
    session(tcp::socket socket)
        : socket_(std::move(socket)){}

    void start()
    {
        do_read();
    }

private:
    void do_read()
    {
        auto self(shared_from_this());
        boost::asio::async_read_until(socket_, buff, '\n',
                                      [this, self](boost::system::error_code ec, std::size_t read_bytes)
        {
            if (!ec)
            {
                std::istream in(&buff);
                std::string msg;
                std::getline(in, msg);

                dba.Parse(msg,answers);

                do_write();
            }
        });
    }

    void do_write()
    {
        auto self(shared_from_this());
        socket_.async_write_some(boost::asio::buffer(answers.front()),
                                 [this, self](boost::system::error_code ec,std::size_t /*length*/)
        {
            if (!ec)
            {
                answers.pop();
                if(!answers.empty()){
                    do_write();
                }else{
                    do_read();
                }
            }
        });
    }

    tcp::socket socket_;

    boost::asio::streambuf buff;
    std::queue<std::string> answers;

    DBAdapter dba;
};

class server
{
public:
    server(boost::asio::io_context& io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
    {
        do_accept();
    }

private:
    void do_accept()
    {
        acceptor_.async_accept(
                    [this](boost::system::error_code ec, tcp::socket socket)
        {
            if (!ec)
            {
                std::make_shared<session>(std::move(socket))->start();
            }

            do_accept();
        });
    }

    tcp::acceptor acceptor_;
};

#endif // ASYNC_SERVER_H
