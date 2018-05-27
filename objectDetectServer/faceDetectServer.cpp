#include"faceDetectServer.h"
FILE* g_file;
void dummyOutput(const char* msg, int len)
{
    if (g_file)
    {//将内容写到文件里
        fwrite(msg, 1, len, g_file);
    }
}

void dummyFlush()
{
    fflush(g_file);//清空输出
}
int main(int argc,char* argv[])
{
	g_file = ::fopen("./faceDetectSever_log", "ae");//打开文件
	Logger::setOutput(dummyOutput);//设置输出到文件,dummyOutput为函数
    	Logger::setFlush(dummyFlush);
	LOG_INFO << "pid = " << getpid();
	EventLoop loop;
	uint16_t port = static_cast<uint16_t>(SERV_PORT);
	InetAddress serverAddr(port);
	faceDetectServer server(&loop, serverAddr);
	server.start();
	loop.loop();
}
	
