/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QPushButton *MainApplicationButton;
    QPushButton *TakePictureButton;
    QPushButton *StartServerButton;
    QPushButton *CalibrateCamerasBUtton;
    QPushButton *KillServerButton;
    QPushButton *ColorCalibrateButton;
    QPushButton *UpdateFacesButton;
    QPushButton *UIButton;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(800, 600);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        MainApplicationButton = new QPushButton(centralwidget);
        MainApplicationButton->setObjectName(QString::fromUtf8("MainApplicationButton"));
        MainApplicationButton->setGeometry(QRect(100, 120, 181, 91));
        TakePictureButton = new QPushButton(centralwidget);
        TakePictureButton->setObjectName(QString::fromUtf8("TakePictureButton"));
        TakePictureButton->setGeometry(QRect(340, 120, 161, 91));
        StartServerButton = new QPushButton(centralwidget);
        StartServerButton->setObjectName(QString::fromUtf8("StartServerButton"));
        StartServerButton->setGeometry(QRect(560, 120, 161, 91));
        CalibrateCamerasBUtton = new QPushButton(centralwidget);
        CalibrateCamerasBUtton->setObjectName(QString::fromUtf8("CalibrateCamerasBUtton"));
        CalibrateCamerasBUtton->setGeometry(QRect(340, 260, 161, 91));
        KillServerButton = new QPushButton(centralwidget);
        KillServerButton->setObjectName(QString::fromUtf8("KillServerButton"));
        KillServerButton->setGeometry(QRect(560, 260, 161, 91));
        ColorCalibrateButton = new QPushButton(centralwidget);
        ColorCalibrateButton->setObjectName(QString::fromUtf8("ColorCalibrateButton"));
        ColorCalibrateButton->setGeometry(QRect(120, 260, 161, 91));
        UpdateFacesButton = new QPushButton(centralwidget);
        UpdateFacesButton->setObjectName(QString::fromUtf8("UpdateFacesButton"));
        UpdateFacesButton->setGeometry(QRect(340, 390, 161, 91));
        UIButton = new QPushButton(centralwidget);
        UIButton->setObjectName(QString::fromUtf8("UIButton"));
        UIButton->setGeometry(QRect(560, 390, 161, 91));
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 22));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        MainApplicationButton->setText(QCoreApplication::translate("MainWindow", "Start Main Application", nullptr));
        TakePictureButton->setText(QCoreApplication::translate("MainWindow", "Take Pictures", nullptr));
        StartServerButton->setText(QCoreApplication::translate("MainWindow", "Start Server", nullptr));
        CalibrateCamerasBUtton->setText(QCoreApplication::translate("MainWindow", "Calibrate Cameras", nullptr));
        KillServerButton->setText(QCoreApplication::translate("MainWindow", "Kill Server", nullptr));
        ColorCalibrateButton->setText(QCoreApplication::translate("MainWindow", "Color Calibrate", nullptr));
        UpdateFacesButton->setText(QCoreApplication::translate("MainWindow", "Update Faces", nullptr));
        UIButton->setText(QCoreApplication::translate("MainWindow", "UI Button", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
