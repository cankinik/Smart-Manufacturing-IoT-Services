#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <vector>
#include <thread>
#include <algorithm>
#include "dialogplanedit.h"

using namespace std;

bool serverActive = false;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{    
    delete ui;
}


void MainWindow::on_MainApplicationButton_clicked()
{
    system("bash ../EndGameStarterScript.sh");
}

void serverExecuteThread()
{
    system("bash ../ServerStarterScript.sh");
}

void MainWindow::on_StartServerButton_clicked()
{
    if (serverActive)
    {
        cout << "Server is already active!" << endl;
    }
    else
    {
        serverActive = true;
        system("ifconfig | grep -o -P 'inet 192.168.{0,7}'");
        std::thread leftFeedThread;
        leftFeedThread = std::thread(serverExecuteThread);
        leftFeedThread.detach();
    }
//    if (fork() == 0)
//    {
//        system("echo this is the parent");
//    }
//    else
//    {
//        system("echo this is the child");
//        system("bash ../ServerStarterScript.sh");
//    }
}

void MainWindow::on_TakePictureButton_clicked()
{
    system("bash ../PictureTakingStarterScript.sh");
}

void MainWindow::on_CalibrateCamerasBUtton_clicked()
{
    system("bash ../CameraCalibratingStarterScript.sh");
}

void MainWindow::on_KillServerButton_clicked()
{
    serverActive = false;
    system("npx kill-port 3000");
}

void MainWindow::on_ColorCalibrateButton_clicked()
{
    system("bash ../ColorCalibratingStarterScript.sh");
}


void MainWindow::on_UpdateFacesButton_clicked()
{
    system("rm /home/cankinik/Desktop/SeniorProject/FaceRecognitionFiles/label_map.txt");
    system("rm /home/cankinik/Desktop/SeniorProject/FaceRecognitionFiles/face2.xml");
}

void MainWindow::on_UIButton_clicked()
{
    DialogPlanEdit plan_edit;
    plan_edit.setModal(false);
    plan_edit.exec();
}
