#include"faceDetectClient.h"
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
	
	g_file = ::fopen("./faceDetectClient_log", "ae");//打开文件
	Logger::setOutput(dummyOutput);//设置输出到文件,dummyOutput为函数
    	Logger::setFlush(dummyFlush);
	LOG_INFO << "pid = "<<getpid();
	EventLoopThread loopThread;
	uint16_t port = static_cast<uint16_t>(SERV_PORT);
	InetAddress serverAddr("127.0.0.1",port);
	
	faceDetectClient client(loopThread.startLoop(),serverAddr);
	
	client.connect();
	
	std::string S_image(IMG_HEIGHT * IMG_WIDTH * 3,'\0');
	cv::VideoCapture capture(0);
	cv::Mat image;
	while(1)
	{
		if(!capture.isOpened())
		{
			LOG_ERROR << "Video can't open!" ;
			return 0;
		}
		capture >> image;
		if(image.empty())
		{
			LOG_ERROR << "image is wrong!" ;
			return 0;
		}
		if(image.cols != IMG_WIDTH || image.rows != IMG_HEIGHT || image.type() != CV_8UC3)
		{
			LOG_ERROR << "image.cols = " << image.cols <<", image.rows = " << image.rows ;
			return 0;
		}
		for(int i = 0; i < IMG_HEIGHT; i++)
		{
			int num1 = i * IMG_WIDTH *3;
			uchar* ucdata = image.ptr<uchar>(i);
			for(int j = 0; j < IMG_WIDTH * 3; j++)
			{
				S_image[num1 + j] = ucdata[j];
			}
			
		}
		client.write(S_image);
	}
	client.disconnect();
	CurrentThread::sleepUsec(1000*1000);
	
}
		
	
