#include "codec.h"
#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/EventLoopThread.h>
#include <muduo/net/TcpClient.h>
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
#define IMG_WIDTH 640   // 需传输图像的宽
#define IMG_HEIGHT 480  // 需传输图像的高
//默认格式为CV_8UC3
#define BUFFER_SIZE IMG_WIDTH*IMG_HEIGHT*3
#define SERV_PORT 6666

class faceDetectClient : boost::noncopyable
{
public:
		

faceDetectClient(EventLoop* loop,const InetAddress& serverAddr) : client_(loop,serverAddr,"faceDetectClient"),codec_(boost::bind(&faceDetectClient::onStringMessage1,this,_1,_2,_3))
	{
		client_.setConnectionCallback(boost::bind(&faceDetectClient::onConnection, this, _1));
		client_.setMessageCallback(boost::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
		client_.enableRetry();
		
	}
	
	
	void connect()
	{
		client_.connect();
	}

	void disconnect()
	{
		client_.disconnect();
	}
	
	void write(std::string message)
	{
		MutexLockGuard lock(mutex_);
		if(connection_)
		{
			codec_.send(get_pointer(connection_),message);
		}
	}

	
private:

	void onConnection(const TcpConnectionPtr& conn)
	{
		LOG_INFO << conn->localAddress().toIpPort() << " -> " << conn->peerAddress().toIpPort() << " is " << (conn ->connected() ? "up" : "DOWN");
		
		MutexLockGuard lock(mutex_);
		if(conn->connected())
		{
			connection_=conn;
		}
		else
		{
			connection_.reset();
		}
	}
	void onStringMessage1(const TcpConnectionPtr&,const muduo::string& message,Timestamp)
	{
		cout<<"<<< "<<message.c_str();
	}
	
	TcpClient client_;
	LengthHeaderCodec codec_;
	MutexLock mutex_;
	TcpConnectionPtr connection_;
};
	
