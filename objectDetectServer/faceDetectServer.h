#include "codec.h"
#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/EventLoopThread.h>
#include <muduo/net/TcpServer.h>
#include <boost/noncopyable.hpp>

#include <boost/bind.hpp>

#include <set>
#include<opencv2/opencv.hpp>
#include<iostream>
#include<stdio.h>
#include<string.h>
#include<unistd.h>

#include<errno.h>

using namespace muduo;
using namespace muduo::net;
using namespace std;
using namespace cv;

//待传输图像默认大小为 640*480，可修改
#define IMAGE_W 640   // 需传输图像的宽
#define IMAGE_H 480  // 需传输图像的高
//默认格式为CV_8UC3
#define BUFFER_SIZE IMAGE_W*IMAGE_H*3
#define SERV_PORT 6666


class faceDetectServer : boost::noncopyable
{
public:
	

	faceDetectServer(EventLoop* loop, const InetAddress& listenAddr) : server_(loop, listenAddr, "faceDetectServer"),codec_(boost::bind(&faceDetectServer::onStringMessage, this, _1, _2, _3))
	{
		server_.setConnectionCallback(boost::bind(&faceDetectServer::onConnection, this, _1));
		server_.setMessageCallback(boost::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
		img1 = Mat :: zeros(IMAGE_W, IMAGE_H, CV_8UC3);
	}
	
	void start()
	{
		server_.start();
	}
	
private:
	void onConnection(const TcpConnectionPtr& conn)
	{
		LOG_INFO << conn->localAddress().toIpPort() << " -> "
             << conn->peerAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");

		if (conn->connected())
		{
			connections_.insert(conn);
		}
		else
		{
			connections_.erase(conn);
		}
	}
	
	void onStringMessage(const TcpConnectionPtr&,const muduo::string& message,Timestamp)
	{
		for (ConnectionList::iterator it = connections_.begin();it != connections_.end();++it)
		{
			for(int i = 0; i < IMAGE_H; i++)
			{
				int num1 = IMAGE_W * i * 3;
				uchar* ucdata = img1.ptr<uchar>(i);
				for(int j = 0; j < IMAGE_W * 3; j++)
				{
					ucdata[j] = message[num1 + j];
				}
			}
		}
		cv::imshow("",img1);
        	cv::waitKey(30);					
	}
	cv :: Mat img1;	
	typedef std::set<TcpConnectionPtr> ConnectionList;
	TcpServer server_;
	LengthHeaderCodec codec_;
	ConnectionList connections_;
};
