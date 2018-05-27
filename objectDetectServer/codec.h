#ifndef __CODEC_H__
#define __CODEC_H__
#include<muduo/base/Logging.h>
#include<muduo/net/Buffer.h>
#include<muduo/net/Endian.h>
#include<muduo/net/TcpConnection.h>

class LengthHeaderCodec:boost::noncopyable 
{
    public:
		typedef boost::function<void (const muduo::net::TcpConnectionPtr&,
			const muduo::string& message,
			muduo::Timestamp)>StringMessageCallback; //回调类型
		
		//构造函数
		explicit LengthHeaderCodec(const StringMessageCallback& cb)
			:messageCallback_(cb)
		{
		}
        
		void onMessage(const muduo::net::TcpConnectionPtr& conn,
			muduo::net::Buffer* buf,
			muduo::Timestamp receiveTime)
		{
			while(buf->readableBytes() >= kHeaderLen) // kHeaderLen == 4
			{
				const void* data = buf->peek();//读取长度
				int32_t be32 = *static_cast<const int32_t*>(data); // SIGBUS
				const int32_t len = muduo::net::sockets::networkToHost32(be32);
				if(len > 921600 || len < 0)
				{
					LOG_ERROR << "Invalid length" << len;
					conn->shutdown();  // FIXME: disable reading
					break;
				}
				else if(buf->readableBytes() >= len + kHeaderLen)
				{
					buf->retrieve(kHeaderLen);//回收数据
					muduo::string message(buf->peek(),len);
					messageCallback_(conn,message,receiveTime);
					buf->retrieve(len);
				}
				else
				{
					LOG_INFO << "不可读" ;
					break;
				}
			}
		}
        
		
		void send(muduo::net::TcpConnection* conn,
			const muduo::StringPiece& message)
		{
			muduo::net::Buffer buf;
			buf.append(message.data(),message.size());//添加数据			
			int32_t len = static_cast<int32_t>(message.size());
			//网络字节序是大端存储
			int32_t be32 = muduo::net::sockets::hostToNetwork32(len);
			//读取长度
			buf.prepend(&be32,sizeof be32);//添加长度
			conn->send(&buf);//发送数据
		}

	private:
		StringMessageCallback messageCallback_;
		const static size_t kHeaderLen = sizeof(int32_t);
        
	
};
#endif
