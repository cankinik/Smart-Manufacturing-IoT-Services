#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_StartServerButton_clicked();

    void on_MainApplicationButton_clicked();

    void on_TakePictureButton_clicked();

    void on_CalibrateCamerasBUtton_clicked();

    void on_KillServerButton_clicked();

    void on_ColorCalibrateButton_clicked();

    void on_UpdateFacesButton_clicked();

    void on_UIButton_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
