#include"../include/socketImageTransfer.h"

void TCPServer::openSocket(std::string server_address, int port)
{


	sClinet = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	memset(&addrServer,0,sizeof(addrServer));

	inet_pton(AF_INET, server_address.c_str(), &addrServer.sin_addr);
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(port);//连接端口

	std::cout<<"waiting server accept.."<<std::endl;
	int flag = connect(sClinet, (struct sockaddr*)&addrServer, sizeof(addrServer));//连接到服务器端
	if(flag ==-1)
		std::cout<<"connect failed..."<<std::endl;
	else
		std::cout<<"connect success"<<std::endl;

   

}

string TCPServer::receiveCommand()
{
  
    if (recv(sClinet, recvBuf, 16, 0) != -1)
    {
        std::cout<<"receive test text: "<<recvBuf<<std::endl;
    }
	return recvBuf;


}


void TCPServer::closed() 
{
	close(sClinet);
}

void TCPServer::sendImage(cv::Mat image)
{
    imencode(".jpg", image, data_encode);
	int len_encode = data_encode.size();
	std::string len = std::to_string(len_encode);
	int length = len.length();
	for (int i = 0; i < 16 - length; i++)
	{
		len = len + " ";
	}
	//发送长度
	int tempSend=0;
	tempSend = send(sClinet, len.c_str(), strlen(len.c_str()), 0);
	if(tempSend<0)
	{
		throw "Socket is abnormal";
	}
	//分段发送编码
	char send_char[SENDBUFSIZE + 1];
	int temp = len_encode;
	int sended = 0;
	std::vector<uchar>::iterator b;
	int n=0;
	while (true)
	{
		n++;
		b = data_encode.begin() + sended;
		if (temp >= SENDBUFSIZE)
		{
			std::copy(b, b + SENDBUFSIZE, send_char);
			tempSend = send(sClinet, send_char, SENDBUFSIZE, 0);
			if (tempSend<0)
			{
				throw "Socket is abnormal!";
			}
			sended += tempSend;
			temp -= SENDBUFSIZE;
		}
		else if (temp == 0)
		{
			break;
		}
		else
		{
			std::copy(b, b + temp, send_char);
			tempSend = send(sClinet, send_char, temp, 0);
			if (tempSend<0)
			{
				throw "Socket is abnormal!";
			}
			//std::cout<<n<<" : "<<tempSend<<std::endl;
			sended += tempSend;
			//std::cout << sended << std::endl;
			break;
		}
	}
}

void TCPServer::sendDepthImg()
{
	sendImage(this->depth);


}




