#include "mythread.h"

myThread::myThread(QTcpSocket* socket,QObject *parent)
    : QThread{parent}
{
    num = 1;
    len = 0;
    myBuffer.resize(0);
    m_socket = socket;
}

myThread::~myThread()
{
    free(file);
}

void myThread::save(const QByteArray content, const char *name)
{
    file->open(QIODevice::ReadWrite | QIODevice::Append);
    if(file->isOpen())
    {
        file->write(content);
        qInfo() << "文件已保存至桌面，文件名称 " << name;
    }
    else
    {
        qInfo() << "文件存储失败，原因是文件未能打开该";
    }
    file->close();
}

void myThread::openFile(const char *fileName)
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "\\" + fileName;
    file = new QFile(path);
    file->open(QIODevice::ReadWrite);
    if(file->isOpen())
    {
        qInfo() << "文件创建成功，文件名称" << path;
    }
    else
    {
        qInfo() << "文件创建失败" << path;
    }
    file->close();
}

void myThread::run()
{
    connect(m_socket, &QTcpSocket::readyRead, this, &myThread::OnReadyRead);
    exec();
}

void myThread::OnReadyRead()
{
    if(m_socket->bytesAvailable() <= 0)
        return;
    QByteArray buffer(m_socket->readAll());
    myBuffer.append(buffer);

   //获取文件大小
   if(num == 1)
   {
       memcpy(&len, myBuffer.data(), sizeof(long long));    //数据长度
       myBuffer.remove(0, sizeof(long long));
       num = 2;
   }

   if(num == 2)
   {
       if(myBuffer.size() < len)
           return;
       content.append(myBuffer,len);
       myBuffer.remove(0,len);
       num = 3;
    }

  if(num == 3)
  {
      if(myBuffer.size() < sizeof(int))
          return;

      memcpy(&len, myBuffer.data(), sizeof(int));    //数据长度
      myBuffer.remove(0, sizeof(int));

      if(myBuffer.size() < len)
          return;
      char* name = new char[len + 1];
      memcpy(name, myBuffer.data(), len);
      name[len] = '\0';
      openFile(name);
      save(content, name);
      emit runOver();
      num = 1;
      len = 0;
      myBuffer.clear();
      myBuffer.resize(0);
  }
}
